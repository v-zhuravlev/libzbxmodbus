/*
** Zabbix
** Copyright (C) 2001-2015 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include <sys/ipc.h>
#include <sys/sem.h>

#include "sysinc.h"
#include "module.h"

#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <byteswap.h>

#include "datatype.h"

#define MODBUS_READ_COIL_1 1
#define MODBUS_READ_DINPUTS_2 2
#define MODBUS_READ_H_REGISTERS_3 3
#define MODBUS_READ_I_REGISTERS_4 4

#define MODBUS_GET_BE_32BIT(tab_int16, index) (((uint32_t)tab_int16[(index)]) << 16) | tab_int16[(index) + 1]
#define MODBUS_GET_MLE_32BIT(tab_int16, index) (((uint32_t)tab_int16[(index) + 1]) << 16) | tab_int16[(index)]
#define MODBUS_GET_MBE_32BIT(tab_int16, index) (((uint32_t)bswap_16(tab_int16[(index)])) << 16) | bswap_16(tab_int16[(index) + 1])
#define MODBUS_GET_LE_32BIT(tab_int16, index) (((uint32_t)bswap_16(tab_int16[(index) + 1])) << 16) | bswap_16(tab_int16[(index)])


#define MODBUS_GET_BE_64BIT(tab_int16, index) (((uint64_t)tab_int16[index]) << 48) | (((uint64_t)tab_int16[index + 1]) << 32) | (((uint64_t)tab_int16[index + 2]) << 16) | tab_int16[index + 3]
#define MODBUS_GET_MLE_64BIT(tab_int16, index) (((uint64_t)tab_int16[index + 3]) << 48) | (((uint64_t)tab_int16[index + 2]) << 32) | (((uint64_t)tab_int16[index + 1]) << 16) | tab_int16[index]
#define MODBUS_GET_MBE_64BIT(tab_int16, index) (((uint64_t)bswap_16(tab_int16[index])) << 48) \
    | (((uint64_t)bswap_16(tab_int16[index + 1])) << 32) \
    | (((uint64_t)bswap_16(tab_int16[index + 2])) << 16) \
    | bswap_16(tab_int16[index + 3])
#define MODBUS_GET_LE_64BIT(tab_int16, index) (((uint64_t)bswap_16(tab_int16[index + 3])) << 48) \
    | (((uint64_t)bswap_16(tab_int16[index + 2])) << 32) \
    | (((uint64_t)bswap_16(tab_int16[index + 1])) << 16) \
    | bswap_16(tab_int16[index])    


#define MODBUS_MLE_CDAB 0 //Mid-Little Endian (CDAB)
#define MODBUS_MLE_CDAB_STR "MLE"
#define MODBUS_BE_ABCD 1 //Big Endian (ABCD)
#define MODBUS_BE_ABCD_STR "BE"
#define MODBUS_MBE_BADC 2 //Mid-Big Endian (BADC)
#define MODBUS_MBE_BADC_STR "MBE"
#define MODBUS_LE_DCBA 3 //Little Endian (DCBA)
#define MODBUS_LE_DCBA_STR "LE"


#define MODBUS_PDU_ADDRESS_0    0
#define MODBUS_PROTOCOL_ADDRESS_1    1

#define NSEMS 128

#define LOCK_PORT(x)    sem_lock(x)
#define UNLOCK_PORT(x)  sem_unlock(x)


//semaphore constants
#define MODBUS_SEM_KEY "."
int MODBUS_SEM_ID = -1;

#define ZBX_MUTEX_ERROR -1


#define MAX_RETRIES 10

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/* the variable keeps timeout setting for item processing */
static int  item_timeout = 0;

int zbx_modbus_read_registers(AGENT_REQUEST *request, AGENT_RESULT *result);
void create_modbus_context(char *con_string, modbus_t **ctx_out, int *lock_required_out, short *lock_key);
int param_is_empty(char *param_to_check);
int initsem();
void sem_lock ();
void sem_unlock();
void sem_uninit (int semid);

static ZBX_METRIC keys[] =
/*      KEY                     FLAG        FUNCTION            TEST PARAMETERS */
{
    {"modbus_read_registers",   CF_HAVEPARAMS,  zbx_modbus_read_registers, NULL},
    {"modbus_read",   CF_HAVEPARAMS,  zbx_modbus_read_registers, NULL},
    {NULL}
};

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_api_version                                           *
 *                                                                            *
 * Purpose: returns version number of the module interface                    *
 *                                                                            *
 * Return value: ZBX_MODULE_API_VERSION_ONE - the only version supported by   *
 *               Zabbix currently                                             *
 *                                                                            *
 ******************************************************************************/
int zbx_module_api_version()
{
    return ZBX_MODULE_API_VERSION_ONE;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_item_timeout                                          *
 *                                                                            *
 * Purpose: set timeout value for processing of items                         *
 *                                                                            *
 * Parameters: timeout - timeout in seconds, 0 - no timeout set               *
 *                                                                            *
 ******************************************************************************/
void    zbx_module_item_timeout(int timeout)
{
    item_timeout = timeout;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_item_list                                             *
 *                                                                            *
 * Purpose: returns list of item keys supported by the module                 *
 *                                                                            *
 * Return value: list of item keys                                            *
 *                                                                            *
 ******************************************************************************/
ZBX_METRIC  *zbx_module_item_list()
{
    return keys;
}

//generate hash for nsem pseudo unique number.
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_modbus_read_registers                                          *
 *                                                                            *
 * Purpose: a main entry point for processing of an item                      *
 *                                                                            *
 * Parameters:  request - structure that contains item key and parameters      *
 *              request->key - item key without parameters                    *
 *              request->nparam - number of parameters                        *
 *              request->timeout - processing should not take longer than     *
 *                                 this number of seconds                     *
 *              request->params[N-1] - pointers to item key parameters        *
 *                                                                            *
 *              result - structure that will contain result                    *
 *                                                                            *
 * Return value: SYSINFO_RET_FAIL - function failed, item will be marked      *
 *                                 as not supported by zabbix                 *
 *               SYSINFO_RET_OK - success                                     *
 *                                                                            *
 * Comment: get_rparam(request, N-1) can be used to get a pointer to the Nth  *
 *          parameter starting from 0 (first parameter). Make sure it exists  *
 *          by checking value of request->nparam.                             *
 *                                                                            *
 ******************************************************************************/
int zbx_modbus_read_registers(AGENT_REQUEST *request, AGENT_RESULT *result)
{
    char    *param1, *param2,*param3,*param4,*param5,*param6,*param7;

    if (request->nparam <4) //check if mandatory params are provided
    {
        SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
        return SYSINFO_RET_FAIL;
    }

    param1 = get_rparam(request, 0);
    if(param_is_empty(param1)) {
        SET_MSG_RESULT(result, strdup("No connection address provided."));
        return SYSINFO_RET_FAIL;
    }
    param2 = get_rparam(request, 1);
    if(param_is_empty(param2)) {
        SET_MSG_RESULT(result, strdup("No slave id provided."));
        return SYSINFO_RET_FAIL;
    }
    param3 = get_rparam(request, 2);
    if(param_is_empty(param3)) {
        SET_MSG_RESULT(result, strdup("No register to read provided."));
        return SYSINFO_RET_FAIL;
    }
    param4 = get_rparam(request, 3);
    if(param_is_empty(param4)) {
        SET_MSG_RESULT(result, strdup("No Modbus function provided! Please provide either 1,2,3,4."));
        return SYSINFO_RET_FAIL;
    }


    modbus_t *ctx;
    int lock_required;
    short lock_key = 0;
    

    create_modbus_context(param1,&ctx,&lock_required, &lock_key);
    if (ctx == NULL) {
            SET_MSG_RESULT(result, strdup("Unable to create the libmodbus context"));
            modbus_free(ctx);
            return SYSINFO_RET_FAIL;
    }
    
    //<slave_id> set slave id 
    char *endptr;
    errno = 0;
    int slave_id = strtol(param2,&endptr, 0);
    if (errno!=0 || *endptr != '\0') {
        SET_MSG_RESULT(result, strdup("Check slaveid parameter"));
        modbus_free(ctx);
        return SYSINFO_RET_FAIL;
    }
    modbus_set_slave(ctx, slave_id);

    //<reg> set register to start from
    errno = 0;
    int reg_start = strtol(param3,&endptr, 0);
    if (errno!=0 || *endptr != '\0') {
        SET_MSG_RESULT(result, strdup("Check register to read"));
        modbus_free(ctx);
        return SYSINFO_RET_FAIL;
    }

    //set function to use
    errno = 0;
    int function = strtol(param4,&endptr, 0);
    if (errno!=0 || *endptr != '\0') {
        SET_MSG_RESULT(result, strdup("Check function (1,2,3,4) used"));
        modbus_free(ctx);
        return SYSINFO_RET_FAIL;
    }


	datatype_syntax_t	datatype_syntax;

	switch (function)
	{
		case MODBUS_READ_COIL_1:
		case MODBUS_READ_DINPUTS_2:
			datatype_syntax = BIT_SYNTAX;
			break;
		case MODBUS_READ_H_REGISTERS_3:
		case MODBUS_READ_I_REGISTERS_4:
			datatype_syntax = REGISTER_SYNTAX;
			break;
		default:
			SET_MSG_RESULT(result, strdup("Check function (1,2,3,4) used"));
			modbus_free(ctx);
			return SYSINFO_RET_FAIL;
	}

	char	*error = NULL;
	int	regs_to_read;

	if (-1 == (regs_to_read = parse_datatype(datatype_syntax, param5 = get_rparam(request, 4) /* datatype */, &error)))
	{
		SET_MSG_RESULT(result, error);
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

	if (64 < regs_to_read)	/* FIXME ideally tab_reg[] and tab_reg_bits[] should be allocated dynamically */
	{
		SET_MSG_RESULT(result, strdup("Cannot read so much at once."));
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

    int end = MODBUS_BE_ABCD; //<endianness> endianness LE(0) BE(1) MBE(2) MLE(3) default BE
    if (request->nparam > 4) { //optional params provided
        param6 = get_rparam(request, 5); //32-64bit endiannes
        if(param6) {
            //endianness to use
            if (!strcmp(MODBUS_BE_ABCD_STR,param6))
                end = MODBUS_BE_ABCD;
            else if (!strcmp(MODBUS_MBE_BADC_STR, param6))
                end = MODBUS_MBE_BADC;
            else if (!strcmp(MODBUS_MLE_CDAB_STR, param6))
                end = MODBUS_MLE_CDAB;
            else if (!strcmp(MODBUS_LE_DCBA_STR, param6))
                end = MODBUS_LE_DCBA;
            else {
                errno = 0;
                end = strtol(param6,&endptr, 0);
                if ( (end != MODBUS_LE_DCBA &&
                    end != MODBUS_BE_ABCD && 
                    end != MODBUS_MBE_BADC &&
                    end != MODBUS_MLE_CDAB ) ||
                    (errno!=0 || *endptr != '\0') )  {
                    SET_MSG_RESULT(result, strdup("Check endiannes used: BE,LE,MLE,MBE."));
                    modbus_free(ctx);
                    return SYSINFO_RET_FAIL;
                }

            }

        }

        param7 = get_rparam(request, 6); //PDU
        if(param7) {//PDU <first reg> check
            //int first_reg=atoi(param7); 

            errno = 0;
            int first_reg = strtol(param7,&endptr, 0);
            if ( (first_reg != MODBUS_PROTOCOL_ADDRESS_1 && first_reg != MODBUS_PDU_ADDRESS_0) ||
                        (errno!=0 || *endptr != '\0') )  {
                SET_MSG_RESULT(result, strdup("Check addressing scheme(PDU,PROTOCOL) used"));
                modbus_free(ctx);
                return SYSINFO_RET_FAIL;
            }

            if (first_reg == MODBUS_PROTOCOL_ADDRESS_1){
                    reg_start=reg_start-1;
            }
        }

    }


    modbus_set_response_timeout(ctx, item_timeout, 0);

    //read part

    uint16_t tab_reg[64];//temp vars
    uint8_t tab_reg_bits[64];

    if (lock_required == 1 ) LOCK_PORT(lock_key);

    if (modbus_connect(ctx) == -1) {
        SET_MSG_RESULT(result, strdup(modbus_strerror(errno)));
        modbus_free(ctx);
        if (lock_required == 1 ) UNLOCK_PORT(lock_key);
        return SYSINFO_RET_FAIL;
    }

    int rc;//modbus return_code
    switch (function) {
        case MODBUS_READ_COIL_1:
            rc = modbus_read_bits(ctx, reg_start, regs_to_read, tab_reg_bits);  
        break;
        case MODBUS_READ_DINPUTS_2:
            rc = modbus_read_input_bits(ctx, reg_start, regs_to_read, tab_reg_bits);
        break;
        case MODBUS_READ_H_REGISTERS_3:
            rc = modbus_read_registers(ctx, reg_start, regs_to_read, tab_reg);
        break;
        case MODBUS_READ_I_REGISTERS_4:
            rc = modbus_read_input_registers(ctx, reg_start, regs_to_read, tab_reg);
        break;
        default :
            SET_MSG_RESULT(result, strdup("Check function (1,2,3,4) used"));
            //close connection
            modbus_close(ctx);
            if (lock_required == 1 ) UNLOCK_PORT(lock_key);
            modbus_free(ctx);
            return SYSINFO_RET_FAIL;
        break;
    }
    //close connection
    modbus_close(ctx);
    if (lock_required == 1 ) UNLOCK_PORT(lock_key);
    modbus_free(ctx);

    if (rc == -1) {
        SET_MSG_RESULT(result, strdup(modbus_strerror(errno)));
        return SYSINFO_RET_FAIL;
    }

    //post-parsing

	set_result_based_on_datatype(result, param5, reg_start, tab_reg_bits, sizeof(tab_reg_bits), tab_reg, sizeof(tab_reg), end);
/*
    uint16_t temp_arr[4];     //output based on datatype
    switch(datatype){

    case MODBUS_BIT:
        SET_UI64_RESULT(result, tab_reg_bits[0]);
    break;

    case MODBUS_UINT16:
        SET_UI64_RESULT(result, tab_reg[0]);
    break;

    case MODBUS_SIGNED_INT:
        //use float type in zabbix item
        SET_DBL_RESULT(result, (int16_t) tab_reg[0]);
    break;
    float f;
    uint32_t i;
    case MODBUS_FLOAT:
        switch( end )
        {
            case MODBUS_LE_DCBA:
                i = MODBUS_GET_LE_32BIT(tab_reg, 0);
                break;
            case MODBUS_BE_ABCD:
                i = MODBUS_GET_BE_32BIT(tab_reg, 0);
                break;
            case MODBUS_MBE_BADC:
                i = MODBUS_GET_MBE_32BIT(tab_reg, 0);
                break;
            case MODBUS_MLE_CDAB:
                i = MODBUS_GET_MLE_32BIT(tab_reg, 0);
                break;
            default:
                return SYSINFO_RET_FAIL;
                break;
        }
        memcpy(&f, &i, sizeof(float));
        SET_DBL_RESULT(result, f);
    break;

    case MODBUS_UINT32:

        switch( end )
        {
            case MODBUS_LE_DCBA:
                SET_UI64_RESULT(result, (uint32_t)MODBUS_GET_LE_32BIT(tab_reg, 0));
                break;
            case MODBUS_BE_ABCD:
                SET_UI64_RESULT(result, (uint32_t)MODBUS_GET_BE_32BIT(tab_reg, 0));
                break;
            case MODBUS_MBE_BADC:
                SET_UI64_RESULT(result, (uint32_t)MODBUS_GET_MBE_32BIT(tab_reg, 0));
                break;
            case MODBUS_MLE_CDAB:
                SET_UI64_RESULT(result, (uint32_t)MODBUS_GET_MLE_32BIT(tab_reg, 0));
                break;
            default:
                return SYSINFO_RET_FAIL;
                break;
        }
    break;

    case MODBUS_SIGNED_INT32:
        switch( end )
        {
            case MODBUS_BE_ABCD:
                temp_arr[0] = tab_reg[0];
                temp_arr[1] = tab_reg[1];
                break;
            case MODBUS_LE_DCBA:
                temp_arr[0] = bswap_16(tab_reg[1]);
                temp_arr[1] = bswap_16(tab_reg[0]);
                break;
            case MODBUS_MBE_BADC:
                temp_arr[0] = bswap_16(tab_reg[0]);
                temp_arr[1] = bswap_16(tab_reg[1]);
                break;
            case MODBUS_MLE_CDAB:
                temp_arr[0] = tab_reg[1];
                temp_arr[1] = tab_reg[0];
                break;
            default:
                return SYSINFO_RET_FAIL;
                break;
        }
        SET_DBL_RESULT(result, ((int32_t)MODBUS_GET_INT32_FROM_INT16(temp_arr,0)));
    break;
    case MODBUS_UINT64:
        switch( end )
        {
            case MODBUS_LE_DCBA:
                SET_UI64_RESULT(result, ((uint64_t)MODBUS_GET_LE_64BIT(tab_reg,0)));
                break;
            case MODBUS_BE_ABCD:
                SET_UI64_RESULT(result, ((uint64_t)MODBUS_GET_BE_64BIT(tab_reg,0)));
                break;
            case MODBUS_MBE_BADC:
                SET_UI64_RESULT(result, ((uint64_t)MODBUS_GET_MBE_64BIT(tab_reg,0)));
                break;
            case MODBUS_MLE_CDAB:
                SET_UI64_RESULT(result, ((uint64_t)MODBUS_GET_MLE_64BIT(tab_reg,0)));
                break;
            default:
                return SYSINFO_RET_FAIL;
                break;
        }
    break;
    double d;
    uint64_t i64;
    case MODBUS_FLOAT64:
        switch( end )
        {
            case MODBUS_LE_DCBA:
                i64 = MODBUS_GET_LE_64BIT(tab_reg, 0);
                break;
            case MODBUS_BE_ABCD:
                i64 = MODBUS_GET_BE_64BIT(tab_reg, 0);
                break;
            case MODBUS_MBE_BADC:
                i64 = MODBUS_GET_MBE_64BIT(tab_reg, 0);
                break;
            case MODBUS_MLE_CDAB:
                i64 = MODBUS_GET_MLE_64BIT(tab_reg, 0);
                break;
            default:
                return SYSINFO_RET_FAIL;
                break;
        }
        memcpy(&d, &i64, sizeof(double));
        SET_DBL_RESULT(result, d);
    break;
    case MODBUS_BULK_FORMULA:
        SET_TEXT_RESULT(result, bulk_response_to_json(*tab_reg, reg_start, param5));
    break;
    default :
        SET_MSG_RESULT(result, strdup("Check datatype provided."));
        return SYSINFO_RET_FAIL;
    break;
    }
*/

    return SYSINFO_RET_OK;
}


/******************************************************************************
 *                                                                            *
 * Function: zbx_module_init                                                  *
 *                                                                            *
 * Purpose: the function is called on agent startup                           *
 *          It should be used to call any initialization routines             *
 *                                                                            *
 * Return value: ZBX_MODULE_OK - success                                      *
 *               ZBX_MODULE_FAIL - module initialization failed               *
 *                                                                            *
 * Comment: the module won't be loaded in case of ZBX_MODULE_FAIL             *
 *                                                                            *
 ******************************************************************************/
int zbx_module_init()
{
    if (ZBX_MUTEX_ERROR == (MODBUS_SEM_ID = initsem())) {               
            printf("libzbxmodbus: unable to create semaphores. Please check maximum allowed nsem, it must be no less than %d. See limits in /proc/sys/kernel/sem\n", NSEMS);
            return ZBX_MODULE_FAIL;
    }

    printf("libzbxmodbus: loaded version: %s\n",VERSION);
/*This function should perform the necessary initialization for the module (if any).
If successful, it should return ZBX_MODULE_OK. Otherwise, it should return ZBX_MODULE_FAIL.*/
    return ZBX_MODULE_OK;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_module_uninit                                                *
 *                                                                            *
 * Purpose: the function is called on agent shutdown                          *
 *          It should be used to cleanup used resources if there are any      *
 *                                                                            *
 * Return value: ZBX_MODULE_OK - success                                      *
 *               ZBX_MODULE_FAIL - function failed                            *
 *                                                                            *
 ******************************************************************************/
int zbx_module_uninit()
{

    sem_uninit(MODBUS_SEM_ID);
    return ZBX_MODULE_OK;
}

int param_is_empty(char *param_to_check) {
    return (param_to_check[0] == '\0') ? 1: 0;
}

void create_modbus_context(char *con_string, modbus_t **ctx_out, int *lock_required_out, short *lock_key) {

    char first_char = con_string[0];

    if (first_char == '/') {//then its rtu(serial con)
        *lock_required_out = 1;
        // -- next code is to parse first arg and find all required to connect to rtu successfully
        char rtu_port[100];
        int rtu_speed = 9600;
        char rtu_parity = 'N';
        int rtu_bits = 8;
        int rtu_stop_bit = 1;

        sscanf(con_string,"%s %d %c %d %d",rtu_port,&rtu_speed,&rtu_parity,&rtu_bits,&rtu_stop_bit);
        *lock_key = hash(rtu_port) % NSEMS;
        *ctx_out = modbus_new_rtu(rtu_port, rtu_speed, rtu_parity, rtu_bits, rtu_stop_bit);

    }
    else {//its TCP (encapsulated or Modbus TCP)

        char host[100];
        int port = MODBUS_TCP_DEFAULT_PORT;

        if (strncmp(con_string, "enc://", strlen("enc://")) == 0) {

            *lock_required_out = 1;
            con_string += strlen("enc://");
            sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
            *lock_key = hash(host) % NSEMS;
            *ctx_out = modbus_new_rtutcp(host, port);

        } else if (strncmp(con_string, "tcp://", strlen("tcp://")) == 0) {

            *lock_required_out = 0;
            con_string += strlen("tcp://");
            sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
            *lock_key = hash(host) % NSEMS;
            *ctx_out = modbus_new_tcp(host, port);

        }
        else {//try Modbus TCP

            *lock_required_out = 0;
            sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
            *lock_key = hash(host) % NSEMS;
            *ctx_out = modbus_new_tcp(host, port);

        }
    }

    return;
}


int initsem()  /* sem_key from ftok() */
{
    int nsems = NSEMS;
    key_t sem_key;
    int semid;
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;

    if (-1 == (sem_key = ftok(MODBUS_SEM_KEY,'z'))) {
        //zbx_error("cannot create IPC key for path '%s': %s",
                //MODBUS_SEM_KEY, zbx_strerror(errno));
        return ZBX_MUTEX_ERROR;
    }


    semid = semget(sem_key, nsems, IPC_CREAT | IPC_EXCL | 0600);

    if (semid >= 0) { /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 1;

        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1) {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return ZBX_MUTEX_ERROR; /* error, check errno */
            }
        }
    } else if (errno == EEXIST) { /* someone else got it first */
        int ready = 0;
        semid = semget(sem_key, nsems, 0); /* get the id */
        if (semid < 0) return semid; /* error, check errno */

        /* wait for other process to initialize the semaphore: */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETRIES && !ready; i++) {
            semctl(semid, nsems-1, IPC_STAT, arg);
            if (arg.buf->sem_otime != 0) {
                ready = 1;
            } else {
                sleep(1);
            }
        }
        if (!ready) {
            errno = ETIME;
            return ZBX_MUTEX_ERROR;
        }
    } else {
        return semid; /* error, check errno */
    }
    return semid;
}


void sem_lock (int sem_num) {
    struct sembuf sb;

    sb.sem_num = sem_num;
    sb.sem_op = -1;  /* set to allocate resource */
    sb.sem_flg = SEM_UNDO;

    if (semop(MODBUS_SEM_ID, &sb, 1) == -1) {
        //zabbix_log(LOG_LEVEL_ERROR, "Failed to lock semaphore for semid: %d",MODBUS_SEM_ID);
    }

}


void sem_unlock (int sem_num) {
    struct sembuf sb;

    sb.sem_num = sem_num;
    sb.sem_op = 1;  /* free resource */
    sb.sem_flg = SEM_UNDO;

    if (semop(MODBUS_SEM_ID, &sb, 1) == -1) {
       //zabbix_log(LOG_LEVEL_ERROR, "Failed to unlock semaphore for semid: %d",MODBUS_SEM_ID);
    }
}



void sem_uninit (int semid) {

     if (semctl(semid, 0, IPC_RMID) == -1) {
       //zabbix_log(LOG_LEVEL_ERROR, "Failed to destroy semaphore set for semid: %d",MODBUS_SEM_ID);
         printf("libzbxmodbus: failed to destroy semaphore set for semid: %d",MODBUS_SEM_ID);
    }
}

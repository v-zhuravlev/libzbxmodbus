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

// clang-format off
#include "sysinc.h"
#include "module.h"
// clang-format on
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "datatype.h"
#include "endianness.h"
#include "semaphores.h"

#define MODBUS_READ_COIL_1 1
#define MODBUS_READ_DINPUTS_2 2
#define MODBUS_READ_H_REGISTERS_3 3
#define MODBUS_READ_I_REGISTERS_4 4

#define MODBUS_PDU_ADDRESS_0 0
#define MODBUS_PROTOCOL_ADDRESS_1 1

#define LOCK_PORT(x) semaphore_lock(x)
#define UNLOCK_PORT(x) semaphore_unlock(x)

/* the variable keeps timeout setting for item processing */
static int item_timeout = 0;

int  zbx_modbus_read_registers(AGENT_REQUEST *request, AGENT_RESULT *result);
void create_modbus_context(char *con_string, modbus_t **ctx_out, int *lock_required_out, short *lock_key);
int  param_is_empty(char *param_to_check);

static ZBX_METRIC keys[] =
	/*      KEY                     FLAG        FUNCTION            TEST PARAMETERS */
	{{"modbus_read_registers", CF_HAVEPARAMS, zbx_modbus_read_registers, NULL},
		{"modbus_read", CF_HAVEPARAMS, zbx_modbus_read_registers, NULL}, {NULL}};

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
void zbx_module_item_timeout(int timeout)
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
ZBX_METRIC *zbx_module_item_list()
{
	return keys;
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
	char *param1, *param2, *param3, *param4, *param7;

	if (request->nparam < 4)	// check if mandatory params are provided
	{
		SET_MSG_RESULT(result, strdup("Invalid number of parameters."));
		return SYSINFO_RET_FAIL;
	}

	param1 = get_rparam(request, 0);
	if (param_is_empty(param1))
	{
		SET_MSG_RESULT(result, strdup("No connection address provided."));
		return SYSINFO_RET_FAIL;
	}
	param2 = get_rparam(request, 1);
	if (param_is_empty(param2))
	{
		SET_MSG_RESULT(result, strdup("No slave id provided."));
		return SYSINFO_RET_FAIL;
	}
	param3 = get_rparam(request, 2);
	if (param_is_empty(param3))
	{
		SET_MSG_RESULT(result, strdup("No register to read provided."));
		return SYSINFO_RET_FAIL;
	}
	param4 = get_rparam(request, 3);
	if (param_is_empty(param4))
	{
		SET_MSG_RESULT(result, strdup("No Modbus function provided! Please provide either 1,2,3,4."));
		return SYSINFO_RET_FAIL;
	}

	modbus_t *ctx;
	int       lock_required;
	short     lock_key = 0;

	create_modbus_context(param1, &ctx, &lock_required, &lock_key);
	if (ctx == NULL)
	{
		SET_MSG_RESULT(result, strdup("Unable to create the libmodbus context"));
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

	//<slave_id> set slave id
	char *endptr;
	errno = 0;
	int slave_id = strtol(param2, &endptr, 0);
	if (errno != 0 || *endptr != '\0')
	{
		SET_MSG_RESULT(result, strdup("Check slaveid parameter"));
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}
	modbus_set_slave(ctx, slave_id);

	//<reg> set register to start from
	errno = 0;
	int reg_start = strtol(param3, &endptr, 0);
	if (errno != 0 || *endptr != '\0')
	{
		SET_MSG_RESULT(result, strdup("Check register to read"));
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

	// set function to use
	errno = 0;
	int function = strtol(param4, &endptr, 0);
	if (errno != 0 || *endptr != '\0')
	{
		SET_MSG_RESULT(result, strdup("Check function (1,2,3,4) used"));
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

	datatype_syntax_t datatype_syntax;

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

	char *		  error = NULL;
	int		  regs_to_read;
	datatype_parse_t *result_layout;

	if (-1 ==
		(regs_to_read = parse_datatype(
			 datatype_syntax, get_rparam(request, 4) /* datatype */, &result_layout, &error)))
	{
		SET_MSG_RESULT(result, error);
		modbus_free(ctx);
		return SYSINFO_RET_FAIL;
	}

	if (64 < regs_to_read) /* FIXME ideally tab_reg[] and tab_reg_bits[] should be allocated dynamically */
	{
		SET_MSG_RESULT(result, strdup("Cannot read so much at once."));
		modbus_free(ctx);
		free(result_layout);
		return SYSINFO_RET_FAIL;
	}

	endianness_code_t endianness;

	if (-1 == parse_endianness(get_rparam(request, 5) /* endianness */, &endianness, &error))
	{
		SET_MSG_RESULT(result, error);
		modbus_free(ctx);
		free(result_layout);
		return SYSINFO_RET_FAIL;
	}

	if (request->nparam > 4)
	{						// optional params provided
		param7 = get_rparam(request, 6);	// PDU
		if (param7)
		{	// PDU <first reg> check
			// int first_reg=atoi(param7);

			errno = 0;
			int first_reg = strtol(param7, &endptr, 0);
			if ((first_reg != MODBUS_PROTOCOL_ADDRESS_1 && first_reg != MODBUS_PDU_ADDRESS_0) ||
				(errno != 0 || *endptr != '\0'))
			{
				SET_MSG_RESULT(result, strdup("Check addressing scheme(PDU,PROTOCOL) used"));
				modbus_free(ctx);
				free(result_layout);
				return SYSINFO_RET_FAIL;
			}

			if (first_reg == MODBUS_PROTOCOL_ADDRESS_1)
			{
				reg_start = reg_start - 1;
			}
		}
	}

	modbus_set_response_timeout(ctx, item_timeout, 0);

	// read part

	uint16_t tab_reg[64];	// temp vars
	uint8_t  tab_reg_bits[64];

	if (lock_required == 1)
		LOCK_PORT(lock_key);

	if (modbus_connect(ctx) == -1)
	{
		SET_MSG_RESULT(result, strdup(modbus_strerror(errno)));
		modbus_free(ctx);
		free(result_layout);
		if (lock_required == 1)
			UNLOCK_PORT(lock_key);
		return SYSINFO_RET_FAIL;
	}

	int rc;	// modbus return_code
	switch (function)
	{
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
		default:
			SET_MSG_RESULT(result, strdup("Check function (1,2,3,4) used"));
			// close connection
			modbus_close(ctx);
			if (lock_required == 1)
				UNLOCK_PORT(lock_key);
			modbus_free(ctx);
			free(result_layout);
			return SYSINFO_RET_FAIL;
			break;
	}
	// close connection
	modbus_close(ctx);
	if (lock_required == 1)
		UNLOCK_PORT(lock_key);
	modbus_free(ctx);

	if (rc == -1)
	{
		SET_MSG_RESULT(result, strdup(modbus_strerror(errno)));
		free(result_layout);
		return SYSINFO_RET_FAIL;
	}

	// post-parsing

	set_result_based_on_datatype(result, result_layout, reg_start, tab_reg_bits, tab_reg, endianness);
	free(result_layout);

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
	if (-1 == semaphore_init())
		return ZBX_MODULE_FAIL;

	printf("libzbxmodbus: loaded version: %s\n", VERSION);
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
	semaphore_uninit();
	return ZBX_MODULE_OK;
}

int param_is_empty(char *param_to_check)
{
	return (param_to_check[0] == '\0') ? 1 : 0;
}

void create_modbus_context(char *con_string, modbus_t **ctx_out, int *lock_required_out, short *lock_key)
{
	char first_char = con_string[0];

	if (first_char == '/')
	{	// then its rtu(serial con)
		*lock_required_out = 1;
		// -- next code is to parse first arg and find all required to connect to rtu successfully
		char rtu_port[100];
		int  rtu_speed = 9600;
		char rtu_parity = 'N';
		int  rtu_bits = 8;
		int  rtu_stop_bit = 1;

		sscanf(con_string, "%s %d %c %d %d", rtu_port, &rtu_speed, &rtu_parity, &rtu_bits, &rtu_stop_bit);
		*lock_key = get_resourceid(rtu_port);
		*ctx_out = modbus_new_rtu(rtu_port, rtu_speed, rtu_parity, rtu_bits, rtu_stop_bit);
	}
	else
	{	// its TCP (encapsulated or Modbus TCP)

		char host[100];
		int  port = MODBUS_TCP_DEFAULT_PORT;

		if (strncmp(con_string, "enc://", strlen("enc://")) == 0)
		{
			*lock_required_out = 1;
			con_string += strlen("enc://");
			sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
			*lock_key = get_resourceid(host);
			*ctx_out = modbus_new_rtutcp(host, port);
		}
		else if (strncmp(con_string, "tcp://", strlen("tcp://")) == 0)
		{
			*lock_required_out = 0;
			con_string += strlen("tcp://");
			sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
			*lock_key = get_resourceid(host);
			*ctx_out = modbus_new_tcp(host, port);
		}
		else
		{	// try Modbus TCP

			*lock_required_out = 0;
			sscanf(con_string, "%99[^:]:%99d[^\n]", host, &port);
			*lock_key = get_resourceid(host);
			*ctx_out = modbus_new_tcp(host, port);
		}
	}

	return;
}

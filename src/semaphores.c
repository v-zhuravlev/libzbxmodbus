#include <sys/ipc.h>
#include <sys/sem.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "semaphores.h"

#define NSEMS 128

int MODBUS_SEM_ID = -1;

union semun
{
	int		 val;
	struct semid_ds *buf;
	unsigned short * array;
};

int semaphore_init(void)
{
	union semun   arg;
	struct sembuf sb;

	if (-1 == (MODBUS_SEM_ID = semget(IPC_PRIVATE, NSEMS, 0600)))
	{
		printf("libzbxmodbus: unable to create semaphores: %s\nPlease check maximum allowed nsem, it must be "
		       "no less than %d. See limits in /proc/sys/kernel/sem\n",
			strerror(errno), NSEMS);
		return -1;
	}

	sb.sem_op = 1;
	sb.sem_flg = 0;
	arg.val = 1;

	for (sb.sem_num = 0; sb.sem_num < NSEMS; sb.sem_num++)
	{
		/* do a semop() to "free" the semaphores. */
		/* this sets the sem_otime field, as needed below. */
		if (0 == semop(MODBUS_SEM_ID, &sb, 1))
			continue;

		printf("libzbxmodbus: unable to initialize semaphores: %s\n", strerror(errno));

		/* clean up */
		if (-1 == semctl(MODBUS_SEM_ID, 0, IPC_RMID))
		{
			printf("libzbxmodbus: unable to clean up semaphores: %s\nYou may need to remove semaphore set "
			       "with id %d manually.\n",
				strerror(errno), MODBUS_SEM_ID);
		}

		MODBUS_SEM_ID = -1;
		return -1;
	}

	return 0;
}

// generate hash for nsem pseudo unique number.
static unsigned long hash(const char *str)
{
	unsigned long hash = 5381;
	int	   c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int get_resourceid(const char *description)
{
	return hash(description) % NSEMS;
}

void semaphore_lock(int sem_num)
{
	struct sembuf sb;

	sb.sem_num = sem_num;
	sb.sem_op = -1; /* set to allocate resource */
	sb.sem_flg = SEM_UNDO;

	if (-1 == semop(MODBUS_SEM_ID, &sb, 1))
		printf("libzbxmodbus: failed to lock semaphore for semid %d: %s\n", MODBUS_SEM_ID, strerror(errno));
}

void semaphore_unlock(int sem_num)
{
	struct sembuf sb;

	sb.sem_num = sem_num;
	sb.sem_op = 1; /* free resource */
	sb.sem_flg = SEM_UNDO;

	if (-1 == semop(MODBUS_SEM_ID, &sb, 1))
		printf("libzbxmodbus: failed to unlock semaphore for semid %d: %s\n", MODBUS_SEM_ID, strerror(errno));
}

void semaphore_uninit(void)
{
	if (-1 == semctl(MODBUS_SEM_ID, 0, IPC_RMID))
		printf("libzbxmodbus: failed to destroy semaphore set for semid: %d\n", MODBUS_SEM_ID);

	MODBUS_SEM_ID = -1;
}

#ifndef LIBZBXMODBUS_SEMAPHORES_H
#define LIBZBXMODBUS_SEMAPHORES_H

int  semaphore_init(void);
int  get_resourceid(const char *description);
void semaphore_lock(int resourceid);
void semaphore_unlock(int resourceid);
void semaphore_uninit(void);

#endif /* LIBZBXMODBUS_SEMAPHORES_H */

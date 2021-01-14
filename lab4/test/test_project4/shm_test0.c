#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define SHM_KEY 1


void shm_task0(void)
{
    uint64_t key, shmid;
    int i = 0;
    uint32_t print_location = 1;

    key = SHM_KEY;

    shmid = shmget(key);
    sys_move_cursor(0, print_location);
    if (shmid == -1)
    {
        printf("share memory task fault!\n");
        return;
    }
    // com = (shm_ipc *)shmat(shmid);
    char *addr = shmat(shmid);
    if (addr == -1)
    {
        printf("share memory task fault!\n");
        return;
    }
    sys_sleep(2);

    while (i < 10)
    {
        addr[i] = 'A' + i;
        i++;
        addr[i] = 0;
        sys_sleep(1);
    }
    shmdt(addr);
    sys_exit();
}

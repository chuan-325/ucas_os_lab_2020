#include "lock.h"
#include "stdio.h"
#include "syscall.h"
#include "test.h"

int is_init = FALSE;
static char blank[] = {"                                             "};

const int mutex_key = 1345;

void lock_task1(void)
{
    int print_location = 5;
    const int lock_id = binsemget(mutex_key);
    while (1)
    {
        int i;

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        for (i = 0; i < 2000; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK1] Applying for a lock %d.           \n", i);
        }
        binsemop(lock_id, BINSEM_OP_LOCK);

        for (i = 0; i < 20000; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK1] Has acquired lock and running.(%d)           \n", i);
        }

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        sys_move_cursor(1, print_location);
        printf("> [TASK1] Has acquired lock and exited.           \n");

        binsemop(lock_id, BINSEM_OP_UNLOCK);
    }
}

void lock_task2(void)
{
    int print_location = 6;
    const lock_id = binsemget(mutex_key);
    while (1)
    {
        int i;

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        for (i = 0; i < 2000; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK2] Applying for a lock. %d           \n", i);
        }

        binsemop(lock_id, BINSEM_OP_LOCK);

        for (i = 0; i < 20000; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK2] Has acquired lock and running.(%d)           \n", i);
        }

        sys_move_cursor(1, print_location);
        printf("%s", blank);

        sys_move_cursor(1, print_location);
        printf("> [TASK2] Has acquired lock and exited.           \n");

        binsemop(lock_id, BINSEM_OP_UNLOCK);
    }
}

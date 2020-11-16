#include "lock.h"
#include "sched.h"
#include "syscall.h"

/* binsem */
binsem_t binsem[BINSEM_NUM];
int do_binsemget(int key)
{
    int i;
    for (i = 0; i < BINSEM_NUM; i++)
    {
        if (!binsem[i].taken)
            continue;

        if (binsem[i].key == key)
            return i;
    }
    for (i = 0; i < BINSEM_NUM; i++)
    {
        if (!binsem[i].taken)
        {
            binsem[i].key = key;
            binsem[i].taken = 1;
            binsem[i].sem = 1; // op_unlock
            queue_init(&(binsem[i].blocked));
            return i;
        }
    }
    return -1;
}
int do_binsemop(int binsem_id, int op)
{
    if (!binsem[binsem_id].taken)
        return -1;

    if (op == BINSEM_OP_LOCK)
    {
        while (binsem[binsem_id].sem == 0) // LOCK(locked)
            do_block(&(binsem[binsem_id].blocked));

        if (binsem[binsem_id].sem == 1)
        { //lock(unlocked)
            binsem[binsem_id].sem = 0;
        }
    }
    else if (op == BINSEM_OP_UNLOCK)
    {
        if (binsem[binsem_id].sem == 0)
        {
            binsem[binsem_id].sem = 1;
            do_unblock_all(&(binsem[binsem_id].blocked));
        }
    }
    return 0;
}

/* mutex_lock */
void do_mutex_lock_init(mutex_lock_t *lock)
{
    lock->status = UNLOCKED;
    lock->prev = NULL;
    lock->next = NULL;
    queue_init(&(lock->blocked));
}
void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    while (lock->status == LOCKED)
    // chk after lock_acquire failed
    {
        do_block(&(lock->blocked));
    }
    // UNLOCKED: success
    lock->status = LOCKED;
    queue_push(&(current_running->lock_queue), (void *)lock);
}
void do_mutex_lock_release(mutex_lock_t *lock)
{
    lock->status = UNLOCKED;
    do_unblock_all(&(lock->blocked));
    queue_remove(&(current_running->lock_queue), (void *)lock);
}

/* spin_lock */
void spin_lock_init(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}
void spin_lock_acquire(spin_lock_t *lock)
{
    while (LOCKED == lock->status)
    {
    };
    lock->status = LOCKED;
}
void spin_lock_release(spin_lock_t *lock)
{
    lock->status = UNLOCKED;
}
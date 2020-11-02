#include "lock.h"
#include "sched.h"
#include "syscall.h"

/* mutex_lock */
// mutex initialization ([*lock])
void do_mutex_lock_init(mutex_lock_t *lock)
{
    lock->status = UNLOCKED;
    lock->prev = NULL;
    lock->next = NULL;
    queue_init(&(lock->blocked));
}
// mutex acquire ([*lock])
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
// mutex release ([*lock])
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
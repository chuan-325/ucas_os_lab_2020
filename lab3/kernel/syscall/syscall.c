#include "fs.h"
#include "sem.h"
#include "irq.h"
#include "cond.h"
#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "barrier.h"
#include "syscall.h"

void system_call_helper(uint64_t fn, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    syscall[fn](arg1, arg2, arg3);
}

int sys_spawn(task_info_t *info)
{
    invoke_syscall(SYSCALL_SPAWN, (uint64_t)info, IGNORE, IGNORE);
}

void sys_exit(void)
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, (uint64_t)time, IGNORE, IGNORE);
}

int sys_kill(pid_t pid)
{
    return (int)invoke_syscall(SYSCALL_KILL, (uint64_t)pid, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
    return (int)invoke_syscall(SYSCALL_WAITPID, pid, IGNORE, IGNORE);
}
void sys_get_timer()
{
    invoke_syscall(SYSCALL_GET_TIMER, IGNORE, IGNORE, IGNORE);
}
void sys_do_scheduler()
{
    invoke_syscall(SYSCALL_SCHEDULER, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (uint64_t)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, (uint64_t)x, (uint64_t)y, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (uint64_t)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (uint64_t)lock, IGNORE, IGNORE);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (uint64_t)lock, IGNORE, IGNORE);
}

void condition_init(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_INIT, (uint64_t)condition, IGNORE, IGNORE);
}

void condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_WAIT, (uint64_t)lock, (uint64_t)condition, IGNORE);
}
void condition_signal(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_SIGNAL, (uint64_t)condition, IGNORE, IGNORE);
}

void condition_broadcast(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_BROADCAST, (uint64_t)condition, IGNORE, IGNORE);
}

void semaphore_init(semaphore_t *s, int val)
{
    invoke_syscall(SYSCALL_SEMAPHORE_INIT, (uint64_t)s, (uint64_t)val, IGNORE);
}
void semaphore_up(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_UP, (uint64_t)s, IGNORE, IGNORE);
}

void semaphore_down(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_DOWN, (uint64_t)s, IGNORE, IGNORE);
}

void barrier_init(barrier_t *barrier, int goal)
{
    invoke_syscall(SYSCALL_BARRIER_INIT, (uint64_t)barrier, (uint64_t)goal, IGNORE);
}

void barrier_wait(barrier_t *barrier)
{
    invoke_syscall(SYSCALL_BARRIER_WAIT, (uint64_t)barrier, IGNORE, IGNORE);
}

int sys_read_shell_buff(char *buff)
{
    invoke_syscall(SYSCALL_READ_SHELL_BUFF, (uint64_t)buff, IGNORE, IGNORE);
}

void sys_process_show(void)
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, (uint64_t)line1, (uint64_t)line2, IGNORE);
}

pid_t sys_getpid()
{
    return (pid_t)invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}

void sys_mkfs()
{
    invoke_syscall(SYSCALL_FS_INIT, IGNORE, IGNORE, IGNORE);
}

int sys_mkdir(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_MKDIR, (uint64_t)name, IGNORE, IGNORE);
}

int sys_readdir(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_READDIR, (uint64_t)name, IGNORE, IGNORE);
}

int sys_enterdir(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_ENTERDIR, (uint64_t)name, IGNORE, IGNORE);
}

int sys_rmdir(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_RMDIR, (uint64_t)name, IGNORE, IGNORE);
}

int sys_print_fs(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_PRINT, (uint64_t)name, IGNORE, IGNORE);
}

int sys_mknod(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_MKNOD, (uint64_t)name, IGNORE, IGNORE);
}

int sys_fopen(char *name, uint32_t access)
{
    return (int)invoke_syscall(SYSCALL_FS_OPEN, (uint64_t)name, (uint64_t)access, IGNORE);
}

int sys_fwrite(uint32_t fd, char *buff, uint32_t size)
{
    return (int)invoke_syscall(SYSCALL_FS_WRITE, (uint64_t)fd, (uint64_t)buff, (uint64_t)size);
}

int sys_cat(char *name)
{
    return (int)invoke_syscall(SYSCALL_FS_CAT, (uint64_t)name, IGNORE, IGNORE);
}

int sys_fread(uint32_t fd, char *buff, uint32_t size)
{
    return (int)invoke_syscall(SYSCALL_FS_READ, (uint64_t)fd, (uint64_t)buff, (uint64_t)size);
}

int sys_close(uint32_t fd)
{
    return (int)invoke_syscall(SYSCALL_FS_CLOSE, (uint64_t)fd, IGNORE, IGNORE);
}

uint32_t sys_net_recv(uint64_t rd, uint64_t rd_phy, uint64_t daddr)
{
    return (uint32_t)invoke_syscall(SYSCALL_NET_RECV, (uint64_t)rd, (uint64_t)rd_phy, (uint64_t)daddr);
}

void sys_net_send(uint64_t td, uint64_t td_phy)
{
    invoke_syscall(SYSCALL_NET_SEND, td, td_phy, IGNORE);
}

void sys_init_mac()
{
    invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE);
}

int binsemget(int key)
{
    invoke_syscall(SYSCALL_BINSEM_GET, (uint64_t)key, IGNORE, IGNORE);
}

int binsemop(int binsem_id, int op)
{
    invoke_syscall(SYSCALL_BINSEM_OP, (uint64_t)binsem_id, (uint64_t)op, IGNORE);
}

char sys_read_keyboard()
{
    invoke_syscall(SYSCALL_READ_KEYBOARD, IGNORE, IGNORE, IGNORE);
}
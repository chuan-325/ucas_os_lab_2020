#include "fs.h"
#include "string.h"
#include "common.h"

static uint8_t zero[SECTOR_SIZE]; // always zero
static uint8_t sup[SECTOR_SIZE];  // super block buffer
static super_block_t *super_block = NULL;

static fd_t fds[NUM_FD];
static inode_entry_t current_dir_entry;





int readdir(char *name)
{
    return 0;
}



int mkdir(char *name)
{
     return 0;
}

int rmdir(char *name)
{
     return 0;
}

 
void read_super_block()
{
     
}

int mknod(char *name)
{
    
    return 0; // failure
}

int open(char *name, uint32_t access)
{
     
    return -1; // open failure
}

int write(uint32_t fd, char *buff, uint32_t size)
{
    
    return 0;
}

int read(uint32_t fd, char *buff, uint32_t size)
{
     
    return 0;
}

int close(uint32_t fd)
{
     
    return 0;
}

 
int cat(char *name)
{
     
    return -1; // open failure
}

 
void init_fs()
{
    
}

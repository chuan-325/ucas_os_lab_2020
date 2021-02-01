#ifndef INCLUDE_FS_H_
#define INCLUDE_FS_H_

#include "type.h"

// ---------------
// | super block |
// ---------------
// |  inode map  |
// ---------------
// | sector map  |
// ---------------
// |    inode    |
// ---------------
// |    data     |
// ---------------

#define KFS_MAGIC (0x66666666)
#define OFFSET_256M (0X10000000)
#define OFFSET_512M (0x20000000)
#define OFFSET_1G (0x40000000)
#define OFFSET_2G (0x80000000)
#define OFFSET_3G (0xC0000000)

//#define FS_SIZE (0x20000000)
#define FS_SIZE (0x10000000)
#define OFFSET_FS OFFSET_256M
#define NUM_IMAP_SECTOR 1
#define SECTOR_SIZE 512
#define NUM_FD 15

#define ENTRY_SECTOR 8

#define TYPE_FILE 1
#define TYPE_DIR 2

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3

// offset is sector offset
typedef struct super_block
{
     

} super_block_t;

typedef struct inode_entry 
{
    
} inode_entry_t;

 
typedef struct dir_entry  
{
     
} dir_entry_t;

typedef struct fd
{
     
} fd_t;

// extern inode_entry_t current_dir_entry;

void init_fs();
 
void read_super_block();

 
int readdir(char *name);
int mkdir(char *name);
int rmdir(char *name);
int mknod(char *name);

int open(char *name, uint32_t access);
int write(uint32_t fd, char *buff, uint32_t size);
int read(uint32_t fd, char *buff, uint32_t size);
int close(uint32_t fd);
int cat(char *name);

#endif
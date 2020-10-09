#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FILE "./image"
#define ARGS "[--extended] [--vm] <bootblock> <executable-file> ..."

#define SECTOR_SIZE 512
#define OS_SIZE_LOC 496
#define BOOT_LOADER_SIG_OFFSET 0x1fe
#define BOOT_LOADER_SIG_1 0x55
#define BOOT_LOADER_SIG_2 0xaa
#define BOOT_MEM_LOC 0x7c00
#define OS_MEM_LOC 0x1000

/* structure to store command line options */
static struct
{
    int vm;
    int extended;
} options;

// global flpsz
static int KN_flpsz = -1; // sector number of os-kernel

/* prototypes of local functions */
static void create_image(int nfiles, char *files[]);
static void error(char *fmt, ...);
static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp);
static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr *ehdr);
static void write_segment(Elf64_Phdr *phdr, FILE *fp, FILE *img); // delete "Elf64_Ehdr *ehdr, ..., int *nbytes, int *first"
static void write_os_size(FILE *img);
static void write_user_thread_segment(Elf64_Ehdr ehdr, Elf64_Phdr phdr, FILE *fp, FILE *img, int *nbytes, int *first);

int main(int argc, char *argv[])
{
    // argc = cnt, *argv[] = input arguments
    options.extended = 0;
    options.vm = 0;
    int cnt = 0;
    argv++;

    for (int j = 1; j < argc; j++)
    {
        if ((*argv)[0] == '-' && (*argv)[1] == '-') // right fmt options
        {
            // test option
            if ((*argv)[2] == 'e')
                options.extended = 1;
            else if ((*argv)[2] == 'v')
                options.vm = 1;
            else
            {
                printf("Error: no such Arg!\n");
                exit(1);
            }
            argv++;
            continue;
        }
        else if ((*argv)[0] == '-' || (*argv)[1] == '-') // wrong fmt options
        {
            printf("Error: wrong format for args! Example: %s\n", ARGS);
            exit(1);
        }
        // files
        cnt++;
    }

    create_image(cnt, argv);
    return 0;
}

static void create_image(int nfiles, char *files[])
{
    FILE *image = fopen(IMAGE_FILE, "w+");

    if (!image)
    {
        printf("Error: no such file to open!\n");
        exit(1);
    }

    for (int i = 0; i < nfiles; i++)
    {
        // fopen
        FILE *fp = fopen(files[i], "r+");
        if (!fp)
        {
            printf("Error: no such file to open!\n");
            exit(1);
        }

        // read hdrs
        Elf64_Ehdr *b_ehdr = (Elf64_Ehdr *)malloc(sizeof(Elf64_Ehdr)); // buffer_ehdr
        read_ehdr(b_ehdr, fp);
        Elf64_Phdr *b_phdr = (Elf64_Phdr *)malloc(sizeof(Elf64_Phdr)); // buffer_phdr
        read_phdr(b_phdr, fp, 0, b_ehdr);
        //printf("off %x,vaddr %x,memsz %x,filesz %lx\n", b_phdr->p_offset, b_phdr->p_vaddr, b_phdr->p_memsz, b_phdr->p_filesz);

        // [-extended]
        if (options.extended)
            printf("0x%x: %s\n\t\toffset:0x%x\tvaddr:0x%x\n\t\tfilesz:0x%x\tmemsz:0x%x\n", (unsigned int)b_phdr->p_vaddr, files[i], (unsigned int)b_phdr->p_offset, (unsigned int)b_phdr->p_vaddr, (unsigned int)b_phdr->p_filesz, (unsigned int)b_phdr->p_memsz);

        // write
        write_segment(b_phdr, fp, image);

        // free
        free(b_ehdr);
        free(b_phdr);
        fclose(fp);
    }
    write_os_size(image); // fix: from write, for multi-core situation
    fclose(image);
}

// READ
// read *ehdr from *fp
static void read_ehdr(Elf64_Ehdr *ehdr, FILE *fp)
{
    fread(ehdr, sizeof(Elf64_Ehdr), 1, fp);
}
// read *phdr from *fp
static void read_phdr(Elf64_Phdr *phdr, FILE *fp, int ph, Elf64_Ehdr *ehdr)
{
    fseek(fp, (ehdr->e_phoff) * (sizeof(char)), SEEK_SET);
    fread(phdr, sizeof(Elf64_Phdr), 1, fp);
}

// WRITE
// write program segemeny to the file *img
static void write_segment(Elf64_Phdr *phdr, FILE *fp, FILE *img)
{
    int flpsz = (phdr->p_memsz - 1) / SECTOR_SIZE + 1;
    KN_flpsz += flpsz; // fix for multi-core situation
    flpsz *= SECTOR_SIZE;
    // printf("KN_flpsz=%d\n", KN_flpsz);
    char *buffer = (char *)malloc(flpsz * (sizeof(char)));
    memset(buffer, 0, flpsz);
    fseek(fp, (phdr->p_offset) * (sizeof(char)), SEEK_SET);
    fread(buffer, sizeof(char), phdr->p_filesz, fp);
    fwrite(buffer, sizeof(char), flpsz, img);
}

// write nbytes of OS to the file *img
static void write_os_size(FILE *img)
{
    // printf("KN_flpsz=%d\n", KN_flpsz);
    fseek(img, OS_SIZE_LOC, SEEK_SET);
    fwrite(&KN_flpsz, sizeof(int), 1, img);
}

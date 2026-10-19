#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tosfs.h"

#define IMAGE_PATH "data/futosfs.img"
#define MAX_INODES 32

static void exit_with_error(const char *context)
{
    perror(context);
    exit(EXIT_FAILURE);
}

static char *block_address(char *image, __u32 block_no)
{
    return image + (size_t)block_no * TOSFS_BLOCK_SIZE;
}

static void print_superblock(const struct tosfs_superblock *superblock)
{
    printf("Superblock\n");
    printf("  magic:        %#x\n", superblock->magic);
    printf("  block bitmap: " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(superblock->block_bitmap));
    printf("  inode bitmap: " PRINTF_BINARY_PATTERN_INT32 "\n", PRINTF_BYTE_TO_BINARY_INT32(superblock->inode_bitmap));
    printf("  block size:   %u\n", superblock->block_size);
    printf("  blocks:       %u\n", superblock->blocks);
    printf("  inodes:       %u\n", superblock->inodes);
    printf("  root inode:   %u\n", superblock->root_inode);
}

static void print_inodes(const struct tosfs_superblock *superblock, const struct tosfs_inode *inodes)
{
    printf("Inodes\n");
    for (int i = 0; i < MAX_INODES; i++)
    {
        if (!(superblock->inode_bitmap & (1u << i)))
            continue;
        const struct tosfs_inode *inode = &inodes[i];
        printf("  %u: block %u, %s, %u bytes, %u link%s\n", inode->inode, inode->block_no,
               S_ISDIR(inode->mode) ? "directory" : "file", inode->size, inode->nlink,
               inode->nlink > 1 ? "s" : "");
    }
}

static void print_directory(char *image, const struct tosfs_inode *directory)
{
    const struct tosfs_dentry *entries = (const struct tosfs_dentry *)block_address(image, directory->block_no);
    size_t entry_count = TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry);

    printf("Root directory\n");
    for (size_t i = 0; i < entry_count; i++)
        if (entries[i].inode != 0)
            printf("  %.*s (inode %u)\n", TOSFS_MAX_NAME_LENGTH, entries[i].name, entries[i].inode);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <image>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
        exit_with_error("open");

    struct stat file_info;
    if (fstat(fd, &file_info) < 0)
        exit_with_error("fstat");

    size_t image_size = (size_t)file_info.st_size;

    char *image = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (image == MAP_FAILED)
        exit_with_error("mmap");

    struct tosfs_superblock *superblock = (struct tosfs_superblock *)image;

    if (superblock->magic != TOSFS_MAGIC)
    {
        fprintf(stderr, "Invalid TOSFS image: wrong magic number\n");
        exit(EXIT_FAILURE);
    }

    struct tosfs_inode *inodes = (struct tosfs_inode *)block_address(image, TOSFS_INODE_BLOCK);
    print_superblock(superblock);
    print_inodes(superblock, inodes);
    print_directory(image, &inodes[superblock->root_inode]);

    if (munmap(image, image_size) < 0)
        exit_with_error("munmap");
    close(fd);

    return EXIT_SUCCESS;
}
#define FUSE_USE_VERSION 26

#include <fuse_lowlevel.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tosfs.h"

#define IMAGE_PATH "data/futosfs.img"
#define MAX_INODES 32
#define DENTRIES_PER_BLOCK (TOSFS_BLOCK_SIZE / sizeof(struct tosfs_dentry))
#define PERMISSION_MASK (S_IRWXU | S_IRWXG | S_IRWXO)

#define min(x, y) ((x) < (y) ? (x) : (y))

static char *image;
static struct tosfs_superblock *superblock;
static struct tosfs_inode *inodes;

static void futosfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);
static void futosfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
static void futosfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);
static void futosfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);
static void futosfs_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);
static void futosfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);

static struct fuse_lowlevel_ops futosfs_oper =
    {
        .lookup = futosfs_lookup,
        .getattr = futosfs_getattr,
        .setattr = futosfs_setattr,
        .readdir = futosfs_readdir,
        .read = futosfs_read,
        .create = futosfs_create,
};

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

static int hello_stat(struct tosfs_inode ino, struct stat *stbuf)
{
    if (!(superblock->inode_bitmap & (1u << ino.inode)))
        return -1;
    stbuf->st_ino = ino.inode;
    stbuf->st_mode = ino.mode;
    stbuf->st_nlink = ino.nlink;
    stbuf->st_uid = ino.uid;
    stbuf->st_gid = ino.gid;
    stbuf->st_size = ino.size;
    return 0;
}

static void futosfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi)
{
    (void)fi;
    struct stat stbuf;
    memset(&stbuf, 0, sizeof(stbuf));
    if (ino >= MAX_INODES || hello_stat(inodes[ino], &stbuf) == -1)
        fuse_reply_err(req, ENOENT);
    else
        fuse_reply_attr(req, &stbuf, 1.0);
}

static int inode_exists(fuse_ino_t ino)
{
    return ino < MAX_INODES && (superblock->inode_bitmap & (1u << ino));
}

static struct tosfs_dentry *directory_entries(const struct tosfs_inode *directory)
{
    return (struct tosfs_dentry *)block_address(image, directory->block_no);
}

struct dirbuf
{
    char *p;
    size_t size;
};

static void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name, fuse_ino_t ino)
{
    struct stat stbuf;
    size_t oldsize = b->size;
    b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
    b->p = (char *)realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
    fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf, (off_t)b->size);
}

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize, off_t off, size_t maxsize)
{
    if (off < (off_t)bufsize)
        return fuse_reply_buf(req, buf + off, min(bufsize - (size_t)off, maxsize));
    else
        return fuse_reply_buf(req, NULL, 0);
}

static void futosfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
    (void)fi;
    if (!inode_exists(ino))
    {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (!S_ISDIR(inodes[ino].mode))
    {
        fuse_reply_err(req, ENOTDIR);
        return;
    }

    struct dirbuf b;
    memset(&b, 0, sizeof(b));
    const struct tosfs_dentry *entries = directory_entries(&inodes[ino]);
    for (size_t i = 0; i < DENTRIES_PER_BLOCK; i++)
    {
        if (entries[i].inode == 0)
            continue;
        char name[TOSFS_MAX_NAME_LENGTH + 1];
        snprintf(name, sizeof(name), "%.*s", TOSFS_MAX_NAME_LENGTH, entries[i].name);
        dirbuf_add(req, &b, name, entries[i].inode);
    }
    reply_buf_limited(req, b.p, b.size, off, size);
    free(b.p);
}

static __u32 find_entry(const struct tosfs_inode *directory, const char *name)
{
    if (strlen(name) > TOSFS_MAX_NAME_LENGTH)
        return 0;
    const struct tosfs_dentry *entries = directory_entries(directory);
    for (size_t i = 0; i < DENTRIES_PER_BLOCK; i++)
        if (entries[i].inode != 0 && strncmp(entries[i].name, name, TOSFS_MAX_NAME_LENGTH) == 0)
            return entries[i].inode;
    return 0;
}

static void futosfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
    if (!inode_exists(parent) || !S_ISDIR(inodes[parent].mode))
    {
        fuse_reply_err(req, ENOENT);
        return;
    }

    __u32 ino = find_entry(&inodes[parent], name);
    if (!inode_exists(ino))
    {
        fuse_reply_err(req, ENOENT);
        return;
    }

    struct fuse_entry_param e;
    memset(&e, 0, sizeof(e));
    e.ino = ino;
    e.attr_timeout = 1.0;
    e.entry_timeout = 1.0;
    hello_stat(inodes[ino], &e.attr);
    fuse_reply_entry(req, &e);
}

static void futosfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi)
{
    (void)fi;
    if (!inode_exists(ino))
    {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (S_ISDIR(inodes[ino].mode))
    {
        fuse_reply_err(req, EISDIR);
        return;
    }

    reply_buf_limited(req, block_address(image, inodes[ino].block_no), inodes[ino].size, off, size);
}

static __u32 find_free_inode(void)
{
    for (__u32 ino = TOSFS_ROOT_INODE + 1; ino + TOSFS_INODE_BLOCK < superblock->blocks; ino++)
        if (!(superblock->inode_bitmap & (1u << ino)))
            return ino;
    return 0;
}

static int dir_add(const struct tosfs_inode *directory, const char *name, __u32 ino)
{
    struct tosfs_dentry *entries = directory_entries(directory);
    for (size_t i = 0; i < DENTRIES_PER_BLOCK; i++)
    {
        if (entries[i].inode != 0)
            continue;
        memset(&entries[i], 0, sizeof(entries[i]));
        entries[i].inode = ino;
        memcpy(entries[i].name, name, strlen(name));
        return 0;
    }
    return -1;
}

static void futosfs_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi)
{
    if (!inode_exists(parent) || !S_ISDIR(inodes[parent].mode))
    {
        fuse_reply_err(req, ENOENT);
        return;
    }
    if (strlen(name) > TOSFS_MAX_NAME_LENGTH)
    {
        fuse_reply_err(req, ENAMETOOLONG);
        return;
    }
    if (find_entry(&inodes[parent], name) != 0)
    {
        fuse_reply_err(req, EEXIST);
        return;
    }

    __u32 ino = find_free_inode();
    if (ino == 0 || dir_add(&inodes[parent], name, ino) == -1)
    {
        fuse_reply_err(req, ENOSPC);
        return;
    }

    const struct fuse_ctx *context = fuse_req_ctx(req);
    struct tosfs_inode *inode = &inodes[ino];
    inode->inode = ino;
    inode->block_no = ino + TOSFS_INODE_BLOCK;
    inode->uid = (__u16)context->uid;
    inode->gid = (__u16)context->gid;
    inode->mode = (__u16)(S_IFREG | (mode & PERMISSION_MASK));
    inode->perm = (__u16)(mode & PERMISSION_MASK);
    inode->size = 0;
    inode->nlink = 1;
    memset(block_address(image, inode->block_no), 0, TOSFS_BLOCK_SIZE);

    tosfs_set_bit(superblock->inode_bitmap, ino);
    tosfs_set_bit(superblock->block_bitmap, inode->block_no);
    superblock->inodes++;

    struct fuse_entry_param e;
    memset(&e, 0, sizeof(e));
    e.ino = ino;
    e.attr_timeout = 1.0;
    e.entry_timeout = 1.0;
    hello_stat(*inode, &e.attr);
    fuse_reply_create(req, &e, fi);
}

static void futosfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi)
{
    (void)attr;
    (void)to_set;
    futosfs_getattr(req, ino, fi);
}

int main(int argc, char *argv[])
{
    int fd = open(IMAGE_PATH, O_RDWR);
    if (fd < 0)
        exit_with_error("open");

    struct stat file_info;
    if (fstat(fd, &file_info) < 0)
        exit_with_error("fstat");

    size_t image_size = (size_t)file_info.st_size;

    image = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (image == MAP_FAILED)
        exit_with_error("mmap");

    superblock = (struct tosfs_superblock *)image;

    if (superblock->magic != TOSFS_MAGIC)
    {
        fprintf(stderr, "Invalid TOSFS image: wrong magic number\n");
        exit(EXIT_FAILURE);
    }
    inodes = (struct tosfs_inode *)block_address(image, TOSFS_INODE_BLOCK);

    print_superblock(superblock);
    print_inodes(superblock, inodes);
    print_directory(image, &inodes[superblock->root_inode]);

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_chan *ch;
    char *mountpoint;
    int err = -1;

    if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
        (ch = fuse_mount(mountpoint, &args)) != NULL)
    {
        struct fuse_session *se;

        se = fuse_lowlevel_new(&args, &futosfs_oper,
                               sizeof(futosfs_oper), NULL);
        if (se != NULL)
        {
            if (fuse_set_signal_handlers(se) != -1)
            {
                fuse_session_add_chan(se, ch);
                err = fuse_session_loop(se);
                fuse_remove_signal_handlers(se);
                fuse_session_remove_chan(ch);
            }
            fuse_session_destroy(se);
        }
        fuse_unmount(mountpoint, ch);
    }
    fuse_opt_free_args(&args);

    if (munmap(image, image_size) < 0)
        exit_with_error("munmap");
    close(fd);

    return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
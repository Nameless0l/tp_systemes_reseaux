#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define TEST_FILE_PATH "data/test.txt"

static void exit_with_error(const char *context)
{
    perror(context);
    exit(EXIT_FAILURE);
}

static void print_file(const char *path)
{
    fflush(stdout);
    pid_t child = fork();
    if (child == -1)
        exit_with_error("fork");
    if (child == 0) {
        execlp("cat", "cat", path, (char *)NULL);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }
    if (waitpid(child, NULL, 0) == -1)
        exit_with_error("waitpid");
}

static size_t get_file_size(int fd)
{
    struct stat file_info;
    if (fstat(fd, &file_info) == -1)
        exit_with_error("fstat");
    return (size_t)file_info.st_size;
}

static void reverse_bytes(char *bytes, size_t size)
{
    for (size_t i = 0; i < size / 2; i++) {
        char saved = bytes[i];
        bytes[i] = bytes[size - 1 - i];
        bytes[size - 1 - i] = saved;
    }
}

static void reverse_file(const char *path)
{
    int fd = open(path, O_RDWR);
    if (fd == -1)
        exit_with_error("open");

    size_t size = get_file_size(fd);
    printf("Size of %s: %zu bytes\n", path, size);

    if (size > 0) {
        char *content = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (content == MAP_FAILED)
            exit_with_error("mmap");
        reverse_bytes(content, size);
        if (munmap(content, size) == -1)
            exit_with_error("munmap");
    }

    close(fd);
}

int main(void)
{
    printf("Before:\n");
    print_file(TEST_FILE_PATH);

    reverse_file(TEST_FILE_PATH);

    printf("After:\n");
    print_file(TEST_FILE_PATH);
    printf("\n");
    return EXIT_SUCCESS;
}

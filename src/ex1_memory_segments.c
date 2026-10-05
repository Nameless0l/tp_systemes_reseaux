#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define LABEL_WIDTH 14
#define PID_TEXT_SIZE 16

int data_global = 1;
int bss_global = 0;

static void print_segment(const char *name, const void *address)
{
    printf("%-*s %p\n", LABEL_WIDTH, name, address);
}

static void show_memory_map(void)
{
    char pid_text[PID_TEXT_SIZE];
    snprintf(pid_text, sizeof pid_text, "%d", getpid());
    fflush(stdout);

    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return;
    }
    if (child == 0) {
        execlp("pmap", "pmap", "-X", pid_text, (char *)NULL);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }
    if (waitpid(child, NULL, 0) == -1)
        perror("waitpid");
}

int main(void)
{
    const char *string_literal = "Hello world";
    int stack_variable = 0;
    size_t mapping_size = (size_t)sysconf(_SC_PAGESIZE);

    int *heap_variable = malloc(sizeof *heap_variable);
    if (heap_variable == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    void *mapping = mmap(NULL, mapping_size, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap");
        free(heap_variable);
        return EXIT_FAILURE;
    }

    print_segment("Data", &data_global);
    print_segment("BSS", &bss_global);
    print_segment("Str", string_literal);
    print_segment("Heap", heap_variable);
    print_segment("Stack", &stack_variable);
    print_segment("Main Function", (void *)main);
    print_segment("LibC Function", (void *)printf);
    print_segment("Mmap", mapping);

    show_memory_map();

    if (munmap(mapping, mapping_size) == -1)
        perror("munmap");
    free(heap_variable);
    return EXIT_SUCCESS;
}

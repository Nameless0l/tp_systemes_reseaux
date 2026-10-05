#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>


int global_var_init = 42;
int global_var_not_init;

void print_segment(const char *name, const void *address)
{
    printf("%s : %p\n", name, address);
}

void show_memory_map(void)
{
    char pid_str[16];
    snprintf(pid_str, sizeof pid_str, "%d", getpid());
    // printf("%d", getpid());
    fflush(stdout);
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        return;
    }
    if (child == 0)
    {
        execlp("pmap", "pmap", "-X", pid_str, (char *)NULL);
        perror("execlp");
        _exit(EXIT_FAILURE);
    }
    if (waitpid(child, NULL, 0) == -1)
        perror("waitpid");
}

int main(void)
{
    const char *str = "Hello world";
    int stack_var = 0;
    int *ptr = (int *)malloc(sizeof(int));

    print_segment("Str", str);
    print_segment("Heap", ptr);
    print_segment("Stack", &stack_var);
    print_segment("Data", &global_var_init);
    print_segment("BSS", &global_var_not_init);
    print_segment("Main Function", &main);
    print_segment("LibC Function", &printf);
    show_memory_map();
    free(ptr);
    return 0;
}

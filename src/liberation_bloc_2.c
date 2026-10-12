#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BIRTHYEAR_FADE 2004
#define BYTES_FOR_BIRTHYEAR_FADE (4 * sizeof(int))
#define NAME_COLLEGUE "MBASSI"
#define BYTES_FOR_NAME_COLLEGUE (6 * sizeof(int))
#define CURRENT_MONTH 12L
#define BYTES_FOR_CURRENT_MONTH (2 * sizeof(long))

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

static HEADER *free_list = NULL;

void *malloc_3is(size_t size)
{
    HEADER *header = sbrk((intptr_t)(sizeof(HEADER) + size));
    if (header == (void *)-1)
        return NULL;
    header->ptr_next = NULL;
    header->bloc_size = size;
    return header + 1;
}

void free_3is(void *pointer)
{
    if (pointer == NULL)
        return;
    HEADER *block = (HEADER *)pointer - 1;
    HEADER *previous = NULL;
    HEADER *current = free_list;
    while (current != NULL && current < block) {
        previous = current;
        current = current->ptr_next;
    }
    block->ptr_next = current;
    if (previous == NULL)
        free_list = block;
    else
        previous->ptr_next = block;
}

static void print_free_list(void)
{
    printf("Free list:");
    if (free_list == NULL)
        printf(" empty");
    for (HEADER *block = free_list; block != NULL; block = block->ptr_next)
        printf(" [%p, %zu bytes]", (void *)(block + 1), block->bloc_size);
    printf("\n");
}

int main(void)
{
    printf("Our starting pointer is at: %p\n", sbrk(0));

    int *number_allocated = malloc_3is(BYTES_FOR_BIRTHYEAR_FADE);
    if (number_allocated == NULL) {
        perror("malloc_3is");
        exit(EXIT_FAILURE);
    }
    *number_allocated = BIRTHYEAR_FADE;
    printf("FADE birthyear is %d, stored at %p\n", *number_allocated, (void *)number_allocated);

    char **string_allocated = malloc_3is(BYTES_FOR_NAME_COLLEGUE);
    if (string_allocated == NULL) {
        perror("malloc_3is");
        exit(EXIT_FAILURE);
    }
    *string_allocated = NAME_COLLEGUE;
    printf("Colleague's name is %s, stored at %p\n", *string_allocated, (void *)string_allocated);

    long *long_number_allocated = malloc_3is(BYTES_FOR_CURRENT_MONTH);
    if (long_number_allocated == NULL) {
        perror("malloc_3is");
        exit(EXIT_FAILURE);
    }
    *long_number_allocated = CURRENT_MONTH;
    printf("Current month is %ld, stored at %p\n", *long_number_allocated, (void *)long_number_allocated);

    print_free_list();

    free_3is(string_allocated);
    printf("\nAfter freeing the name\n");
    print_free_list();

    free_3is(number_allocated);
    printf("\nAfter freeing the birthyear\n");
    print_free_list();

    free_3is(long_number_allocated);
    printf("\nAfter freeing the month\n");
    print_free_list();

    return EXIT_SUCCESS;
}

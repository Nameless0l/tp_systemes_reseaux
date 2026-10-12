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
#define BYTES_FOR_OVERFLOW_TEST 16

#define MAGIC_NUMBER ((long)0xBADA110CBADA110CL)
#define ALIGNMENT sizeof(long)

typedef struct HEADER_TAG {
    struct HEADER_TAG *ptr_next;
    size_t bloc_size;
    long magic_number;
} HEADER;

static HEADER *free_list = NULL;

static size_t align_size(size_t size)
{
    return (size + ALIGNMENT - 1) / ALIGNMENT * ALIGNMENT;
}

static long *trailing_magic(HEADER *header)
{
    return (long *)((char *)(header + 1) + header->bloc_size);
}

static void check_magic_numbers(HEADER *block)
{
    if (block->magic_number == MAGIC_NUMBER && *trailing_magic(block) == MAGIC_NUMBER)
        return;
    fflush(stdout);
    fprintf(stderr, "Memory overflow detected on block %p\n", (void *)(block + 1));
    abort();
}

void *malloc_3is(size_t size)
{
    size_t aligned_size = align_size(size);
    HEADER *header = sbrk((intptr_t)(sizeof(HEADER) + aligned_size + sizeof(long)));
    if (header == (void *)-1)
        return NULL;
    header->ptr_next = NULL;
    header->bloc_size = aligned_size;
    header->magic_number = MAGIC_NUMBER;
    *trailing_magic(header) = MAGIC_NUMBER;
    return header + 1;
}

void free_3is(void *pointer)
{
    if (pointer == NULL)
        return;
    HEADER *block = (HEADER *)pointer - 1;
    check_magic_numbers(block);
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

    char *buffer = malloc_3is(BYTES_FOR_OVERFLOW_TEST);
    if (buffer == NULL) {
        perror("malloc_3is");
        exit(EXIT_FAILURE);
    }
    printf("\nWriting %d bytes into a %d-byte block at %p\n",
           BYTES_FOR_OVERFLOW_TEST + 1, BYTES_FOR_OVERFLOW_TEST, (void *)buffer);
    for (int i = 0; i <= BYTES_FOR_OVERFLOW_TEST; i++)
        buffer[i] = 'A';
    free_3is(buffer);

    return EXIT_SUCCESS;
}

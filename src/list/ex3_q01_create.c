#include <stdio.h>
#include <stdlib.h>

#define LIST_SIZE 5

typedef struct Element Element;
struct Element {
    int value;
    Element *next;
};

typedef struct List List;
struct List {
    Element *first;
};

static Element *element_new(int value, Element *next)
{
    Element *element = malloc(sizeof *element);
    if (element == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    element->value = value;
    element->next = next;
    return element;
}

static List list_create_first_integers(int count)
{
    List list = { NULL };
    for (int value = count - 1; value >= 0; value--)
        list.first = element_new(value, list.first);
    return list;
}

static void list_free(List *list)
{
    Element *element = list->first;
    while (element != NULL) {
        Element *next = element->next;
        free(element);
        element = next;
    }
    list->first = NULL;
}

int main(void)
{
    List list = list_create_first_integers(LIST_SIZE);

    for (const Element *element = list.first; element != NULL; element = element->next)
        printf("%d ", element->value);
    printf("\n");

    list_free(&list);
    return EXIT_SUCCESS;
}

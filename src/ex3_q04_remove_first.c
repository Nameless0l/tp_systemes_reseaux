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

static int list_length(const List *list)
{
    int length = 0;
    for (const Element *element = list->first; element != NULL; element = element->next)
        length++;
    return length;
}

static void list_print(const List *list)
{
    for (const Element *element = list->first; element != NULL; element = element->next)
        printf("<%p> %d\n", (const void *)element, element->value);
}

static void show_list(const char *title, const List *list)
{
    printf("%s (length %d):\n", title, list_length(list));
    list_print(list);
}

static void list_remove_first(List *list)
{
    Element *removed = list->first;
    if (removed == NULL)
        return;
    list->first = removed->next;
    free(removed);
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
    show_list("Initial list", &list);

    list_remove_first(&list);
    show_list("After removing the first element", &list);

    list_free(&list);
    return EXIT_SUCCESS;
}

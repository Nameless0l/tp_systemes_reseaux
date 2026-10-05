#include <stdio.h>
#include <stdlib.h>

#define LIST_SIZE 5
#define APPENDED_VALUE 10
#define PREPENDED_VALUE -1
#define OTHER_LIST_SIZE 3

typedef struct Element Element;
struct Element {
    int value;
    Element *previous;
    Element *next;
};

typedef struct List List;
struct List {
    Element *first;
    Element *last;
};

static Element *element_new(int value)
{
    Element *element = malloc(sizeof *element);
    if (element == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    element->value = value;
    element->previous = NULL;
    element->next = NULL;
    return element;
}

static void list_append(List *list, int value)
{
    Element *element = element_new(value);
    element->previous = list->last;
    if (list->last == NULL)
        list->first = element;
    else
        list->last->next = element;
    list->last = element;
}

static void list_prepend(List *list, int value)
{
    Element *element = element_new(value);
    element->next = list->first;
    if (list->first == NULL)
        list->last = element;
    else
        list->first->previous = element;
    list->first = element;
}

static List list_create_first_integers(int count)
{
    List list = { NULL, NULL };
    for (int value = 0; value < count; value++)
        list_append(&list, value);
    return list;
}

static int list_length(const List *list)
{
    int length = 0;
    for (const Element *element = list->first; element != NULL; element = element->next)
        length++;
    return length;
}

static void print_element(const Element *element)
{
    printf("<%p> %d\n", (const void *)element, element->value);
}

static void list_print(const List *list)
{
    for (const Element *element = list->first; element != NULL; element = element->next)
        print_element(element);
}

static void list_print_backward(const List *list)
{
    for (const Element *element = list->last; element != NULL; element = element->previous)
        print_element(element);
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
    if (list->first == NULL)
        list->last = NULL;
    else
        list->first->previous = NULL;
    free(removed);
}

static void list_remove_last(List *list)
{
    Element *removed = list->last;
    if (removed == NULL)
        return;
    list->last = removed->previous;
    if (list->last == NULL)
        list->first = NULL;
    else
        list->last->next = NULL;
    free(removed);
}

static void list_concat(List *destination, List *source)
{
    if (source->first == NULL)
        return;
    if (destination->last == NULL) {
        destination->first = source->first;
    } else {
        destination->last->next = source->first;
        source->first->previous = destination->last;
    }
    destination->last = source->last;
    source->first = NULL;
    source->last = NULL;
}

static int square(int value)
{
    return value * value;
}

static List list_map(const List *list, int (*transform)(int))
{
    List result = { NULL, NULL };
    for (const Element *element = list->first; element != NULL; element = element->next)
        list_append(&result, transform(element->value));
    return result;
}

static void list_free(List *list)
{
    while (list->first != NULL)
        list_remove_first(list);
}

int main(void)
{
    List list = list_create_first_integers(LIST_SIZE);
    show_list("Initial list", &list);

    list_remove_first(&list);
    show_list("After removing the first element", &list);

    list_remove_last(&list);
    show_list("After removing the last element", &list);

    list_append(&list, APPENDED_VALUE);
    show_list("After appending a value", &list);

    list_prepend(&list, PREPENDED_VALUE);
    show_list("After prepending a value", &list);

    List other = list_create_first_integers(OTHER_LIST_SIZE);
    list_concat(&list, &other);
    show_list("After concatenating another list", &list);
    show_list("Other list after concatenation", &other);

    List squares = list_map(&list, square);
    show_list("Squares", &squares);
    printf("Squares read backward:\n");
    list_print_backward(&squares);

    list_free(&list);
    list_free(&squares);
    return EXIT_SUCCESS;
}

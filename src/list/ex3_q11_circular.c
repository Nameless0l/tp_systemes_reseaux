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
};

static Element *element_new(int value)
{
    Element *element = malloc(sizeof *element);
    if (element == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    element->value = value;
    element->previous = element;
    element->next = element;
    return element;
}

static Element *list_last(const List *list)
{
    return list->first == NULL ? NULL : list->first->previous;
}

static void link_between(Element *previous, Element *next, Element *element)
{
    element->previous = previous;
    element->next = next;
    previous->next = element;
    next->previous = element;
}

static void list_append(List *list, int value)
{
    Element *element = element_new(value);
    if (list->first == NULL)
        list->first = element;
    else
        link_between(list_last(list), list->first, element);
}

static void list_prepend(List *list, int value)
{
    Element *element = element_new(value);
    if (list->first != NULL)
        link_between(list_last(list), list->first, element);
    list->first = element;
}

static List list_create_first_integers(int count)
{
    List list = { NULL };
    for (int value = 0; value < count; value++)
        list_append(&list, value);
    return list;
}

static int list_length(const List *list)
{
    if (list->first == NULL)
        return 0;
    int length = 0;
    const Element *element = list->first;
    do {
        length++;
        element = element->next;
    } while (element != list->first);
    return length;
}

static void print_element(const Element *element)
{
    printf("<%p> %d\n", (const void *)element, element->value);
}

static void list_print(const List *list)
{
    if (list->first == NULL)
        return;
    const Element *element = list->first;
    do {
        print_element(element);
        element = element->next;
    } while (element != list->first);
}

static void list_print_backward(const List *list)
{
    const Element *last = list_last(list);
    if (last == NULL)
        return;
    const Element *element = last;
    do {
        print_element(element);
        element = element->previous;
    } while (element != last);
}

static void show_list(const char *title, const List *list)
{
    printf("%s (length %d):\n", title, list_length(list));
    list_print(list);
}

static void list_remove_element(List *list, Element *removed)
{
    if (removed->next == removed) {
        list->first = NULL;
    } else {
        removed->previous->next = removed->next;
        removed->next->previous = removed->previous;
        if (list->first == removed)
            list->first = removed->next;
    }
    free(removed);
}

static void list_remove_first(List *list)
{
    if (list->first != NULL)
        list_remove_element(list, list->first);
}

static void list_remove_last(List *list)
{
    if (list->first != NULL)
        list_remove_element(list, list_last(list));
}

static void list_concat(List *destination, List *source)
{
    if (source->first == NULL)
        return;
    if (destination->first == NULL) {
        destination->first = source->first;
    } else {
        Element *destination_last = list_last(destination);
        Element *source_last = list_last(source);
        destination_last->next = source->first;
        source->first->previous = destination_last;
        source_last->next = destination->first;
        destination->first->previous = source_last;
    }
    source->first = NULL;
}

static int square(int value)
{
    return value * value;
}

static List list_map(const List *list, int (*transform)(int))
{
    List result = { NULL };
    if (list->first == NULL)
        return result;
    const Element *element = list->first;
    do {
        list_append(&result, transform(element->value));
        element = element->next;
    } while (element != list->first);
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

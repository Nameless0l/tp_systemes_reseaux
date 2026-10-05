#include <stdio.h>
#include <stdlib.h>

typedef struct Element Element;
struct Element
{
    int number;
    Element *next;
};

typedef struct List List;
struct List
{
    Element *first;
};

void insert_element(List *list, int new_)
{
    Element *new_element = malloc(sizeof(*new_element));
    if(list==NULL || new_element==NULL ){
        exit(EXIT_FAILURE);
    }
    new_element->number = new_;
    new_element->next = list->first ;
    list->first = new_element;
}

List list_of_n_first_number(int n)
{
    List list = {NULL};
    for (int i = n - 1; i >= 0; i--)
        insert_element(&list, i);
    return list;
}

int list_length(const List *list)
{
    int length = 0;
    for (const Element *element = list->first; element != NULL; element = element->next)
        length++;
    return length;
}

void list_print(const List *list)
{
    for (const Element *element = list->first; element != NULL; element = element->next)
        printf("<%p> %d\n", (const void *)element, element->number);
}

void delete_first_element(List *list)
{
    if (list->first == NULL)
        return;
    Element *removed = list->first;
    list->first = removed->next;
    free(removed);
}

void delete_last_element(List *list)
{
    if (list->first == NULL)
        return;
    if (list->first->next == NULL) {
        free(list->first);
        list->first = NULL;
        return;
    }
    Element *before_last = list->first;
    while (before_last->next->next != NULL)
        before_last = before_last->next;
    free(before_last->next);
    before_last->next = NULL;
}


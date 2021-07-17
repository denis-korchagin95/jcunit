#ifndef JCUNIT_LIST_ITERATOR_H
#define JCUNIT_LIST_ITERATOR_H 1

#include <stdbool.h>

#include "list.h"

struct list_iterator
{
    struct list * next;
    struct list * end;
};

struct slist_iterator
{
    struct slist * next;
    struct slist * end;
};

typedef void * list_iterator_visiter_func(void * object, void * context);

void list_iterator_init(struct list_iterator * iterator, struct list * head, struct list * end);
void slist_iterator_init(struct slist_iterator * iterator, struct slist * head, struct slist * end);
bool list_iterator_finished(struct list_iterator * iterator);
bool slist_iterator_finished(struct slist_iterator * iterator);

void * list_iterator_visit(struct list_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context);
void * slist_iterator_visit(struct slist_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context);

#endif /* JCUNIT_LIST_ITERATOR_H */

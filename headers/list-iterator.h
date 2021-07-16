#ifndef JCUNIT_LIST_ITERATOR_H
#define JCUNIT_LIST_ITERATOR_H 1

#include <stdbool.h>

#include "list.h"

struct list_iterator
{
    struct list * next;
    struct list * end;
};

typedef void * list_iterator_visiter_func(void * object, void * context);

void list_iterator_init(struct list_iterator * iterator, struct list * head, struct list * end);
bool list_iterator_finished(struct list_iterator * iterator);

void * list_iterator_visit(struct list_iterator * iterator, list_iterator_visiter_func * visiter_func, void * extra);

#endif /* JCUNIT_LIST_ITERATOR_H */

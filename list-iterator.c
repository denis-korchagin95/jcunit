#include <stddef.h>
#include <assert.h>


#include "headers/list-iterator.h"


void list_iterator_init(struct list_iterator * iterator, struct list * head, struct list * end)
{
    assert(iterator != NULL);

    iterator->next = head;
    iterator->end = end;
}

void slist_iterator_init(struct slist_iterator * iterator, struct slist * head, struct slist * end)
{
    assert(iterator != NULL);

    iterator->next = head;
    iterator->end = end;
}

bool list_iterator_finished(struct list_iterator * iterator)
{
    assert(iterator != NULL);

    return iterator->next == iterator->end;
}

bool slist_iterator_finished(struct slist_iterator * iterator)
{
    assert(iterator != NULL);

    return iterator->next == iterator->end;
}

void * list_iterator_visit(struct list_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context)
{
    assert(iterator != NULL);

    if (iterator->next == iterator->end) {
        return NULL;
    }
    void * result = visiter_func((void *)iterator->next, context);
    iterator->next = iterator->next->next;
    return result;
}

void * slist_iterator_visit(struct slist_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context)
{
    assert(iterator != NULL);

    if (iterator->next == iterator->end) {
        return NULL;
    }
    void * result = visiter_func((void *)iterator->next, context);
    iterator->next = iterator->next->next;
    return result;
}

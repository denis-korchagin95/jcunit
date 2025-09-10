#include "headers/list.h"
#include "headers/allocator.h"

static void do_list_add(struct list * new, struct list * prev, struct list * next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

struct slist * make_slist(void)
{
    main_pool_alloc(struct slist, list)
    list->next = list;
    return list;
}

void list_append(struct list * head, struct list * new)
{
    do_list_add(new, head->prev, head);
}

struct list * list_shift(struct list * head)
{
    struct list * next = head->next;
    list_remove(next);
    return next;
}

void list_remove(struct list * item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
}

struct slist ** slist_get_end(struct slist * head)
{
    struct slist ** next = &head->next;
    while (*next != head) {
        next = &(*next)->next;
    }
    return next;
}

unsigned int slist_count(struct slist * head)
{
    unsigned int count = 0;
    slist_foreach(iterator, head, {
        ++count;
    });
    return count;
}

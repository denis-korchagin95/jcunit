#include "headers/list.h"
#include "headers/allocate.h"

static void do_list_add(struct list * new, struct list * prev, struct list * next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

struct list * make_list(void)
{
    struct list * list = alloc_list();
    list_init(list);
    return list;
}

void list_append(struct list * head, struct list * new)
{
    do_list_add(new, head->prev, head);
}

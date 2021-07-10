#ifndef JCUNIT_LIST_H
#define JCUNIT_LIST_H 1

struct list
{
    struct list * next;
    struct list * prev;
};

#define offset_of(type, member) ((size_t)&((type *)0)->member)

#define list_get_owner(entry_ptr, type, member) ((type *)((entry_ptr) - offset_of(type, member)))

#define list_init(ptr)          \
    do                          \
    {                           \
        (ptr)->next = (ptr);    \
        (ptr)->prev = (ptr);    \
    }                           \
    while(0)

#define list_is_empty(head) ((head)->next == (head))

struct list * make_list(void);

void list_append(struct list * head, struct list * new);

#endif /* JCUNIT_LIST_H */

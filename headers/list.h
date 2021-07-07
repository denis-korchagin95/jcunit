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

#define list_add(head, new)                 \
    do                                      \
    {                                       \
        (head)->next->prev = new;           \
        (new)->next = (head)->next;         \
        (new)->prev = (head);               \
        (head)->next = (new);               \
    }                                       \
    while(0)

#endif /* JCUNIT_LIST_H */

#ifndef JCUNIT_LIST_H
#define JCUNIT_LIST_H 1

struct list
{
    struct list * next;
    struct list * prev;
};

struct slist
{
    struct slist * next;
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

#define slist_init(ptr)         \
    do                          \
    {                           \
        (ptr)->next = (ptr);    \
    }                           \
    while(0)

#define slist_protect(ptr)      \
    do                          \
    {                           \
        (ptr)->next = (ptr);    \
    }                           \
    while(0)

#define slist_foreach(iterator_name, head, body)                                        \
    do                                                                                  \
    {                                                                                   \
        struct slist * ___begin = (head);                                               \
        struct slist * iterator_name = ___begin->next;                                  \
        for (; iterator_name != ___begin; iterator_name = iterator_name->next) body     \
    }                                                                                   \
    while(0)

#define slist_foreach_safe(iterator_name, head, body)                                                           \
    do                                                                                                          \
    {                                                                                                           \
        struct slist * ___begin = (head);                                                                       \
        struct slist * iterator_name = ___begin->next;                                                          \
        struct slist * ___next = iterator_name->next;                                                           \
        for (; iterator_name != ___begin; iterator_name = ___next, ___next = iterator_name->next)               \
            body                                                                                                \
    }                                                                                                           \
    while(0)

#define list_is_empty(head) ((head)->next == (head))

#define slist_append(end, new)      \
    do                              \
    {                               \
        (new)->next = *(end);       \
        *(end) = (new);             \
        (end) = &(new)->next;       \
    }                               \
    while(0)

struct slist * make_slist(void);

void list_append(struct list * head, struct list * new);
void list_remove(struct list * item);
struct list * list_shift(struct list * head);
struct slist ** slist_get_end(struct slist * head);
unsigned int slist_count(struct slist * head);

#endif /* JCUNIT_LIST_H */

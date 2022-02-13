/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
 *
 * This file is part of JCUnit
 *
 * For the full license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation of version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
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

#define list_foreach(iterator_name, head, body)                                     \
    do                                                                              \
    {                                                                               \
        struct list * ___begin = (head);                                            \
        struct list * iterator_name = ___begin->next;                               \
        for (; iterator_name != ___begin; iterator_name = iterator_name->next) body \
    }                                                                               \
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

struct list * make_list(void);
struct slist * make_slist(void);

void list_append(struct list * head, struct list * new);
void list_remove(struct list * item);
struct list * list_shift(struct list * head);
struct slist ** slist_get_end(struct slist * head);

#endif /* JCUNIT_LIST_H */

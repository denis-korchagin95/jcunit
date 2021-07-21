/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021 Denis Korchagin <denis.korchagin.1995@gmail.com>
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

struct slist * make_slist(void)
{
    struct slist * list = alloc_slist();
    list->next = list;
    return list;
}

void list_append(struct list * head, struct list * new)
{
    do_list_add(new, head->prev, head);
}

struct slist ** slist_get_end(struct slist * head)
{
    struct slist ** next = &head->next;
    while (*next != head) {
        next = &(*next)->next;
    }
    return next;
}

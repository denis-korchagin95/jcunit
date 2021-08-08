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
    struct slist * next = iterator->next->next;
    void * result = visiter_func((void *)iterator->next, context);
    iterator->next = next;
    return result;
}

/**
 * JCUnit - a very simple unit testing framework for C
 *
 * Copyright (C) 2021-2022 Denis Korchagin <denis.korchagin.1995@gmail.com>
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
#ifndef JCUNIT_LIST_ITERATOR_H
#define JCUNIT_LIST_ITERATOR_H 1

#include <stdbool.h>

#include "list.h"

struct list_iterator
{
    struct list * next;
    struct list * end;
};

struct slist_iterator
{
    struct slist * next;
    struct slist * end;
};

typedef void * list_iterator_visiter_func(void * object, void * context);

void list_iterator_init(struct list_iterator * iterator, struct list * head, struct list * end);
void slist_iterator_init(struct slist_iterator * iterator, struct slist * head, struct slist * end);
bool list_iterator_finished(struct list_iterator * iterator);
bool slist_iterator_finished(struct slist_iterator * iterator);

void * list_iterator_visit(struct list_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context);
void * slist_iterator_visit(struct slist_iterator * iterator, list_iterator_visiter_func * visiter_func, void * context);

#endif /* JCUNIT_LIST_ITERATOR_H */

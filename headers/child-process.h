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
#ifndef JCUNIT_CHILD_PROCESS_H
#define JCUNIT_CHILD_PROCESS_H 1

#define RUN_MODE_CAPTURE_STDOUT (0)
#define RUN_MODE_CAPTURE_STDERR (1)

#define ERROR_CODE_NONE             (0)
#define ERROR_CODE_FILE_NOT_FOUND   (1)
#define ERROR_CODE_NOT_EXECUTABLE   (2)
#define ERROR_CODE_READ_CHILD_DATA  (3)

struct process_output
{
    char * buffer;
    unsigned int size;
    unsigned int len;
    unsigned int error_code;
};

void child_process_run(const char * path, char * argv[], struct process_output * output, unsigned int mode);

#endif /* JCUNIT_CHILD_PROCESS_H */

#ifndef JCUNIT_FS_H
#define JCUNIT_FS_H 1

#include <stdbool.h>

bool is_file_exists(const char * pathname);
bool is_file_executable(const char * pathname);

#endif /* JCUNIT_FS_H */

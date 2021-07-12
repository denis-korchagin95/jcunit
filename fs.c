#include <sys/stat.h>

#include "headers/fs.h"

bool is_file_exists(const char * pathname)
{
    struct stat file_stat;
    return stat(pathname, &file_stat) == 0;
}

bool is_file_executable(const char * pathname)
{
    struct stat file_stat;
    return stat(pathname, &file_stat) == 0 && (file_stat.st_mode & S_IEXEC);
}

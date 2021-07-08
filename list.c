#include "headers/list.h"
#include "headers/allocate.h"


struct list * make_list(void)
{
    struct list * list = alloc_list();
    list_init(list);
    return list;
}

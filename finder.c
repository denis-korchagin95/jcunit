#include <stddef.h>


#include "headers/finder.h"


struct requirement * find_requirement_by_kind(struct test_case * test_case, unsigned int kind)
{
    struct requirement * requirement;
    list_foreach(iterator, &test_case->requirements, {
        requirement = list_get_owner(iterator, struct requirement, list_entry);
        if (requirement->kind == kind) {
            return requirement;
        }
    });
    return NULL;
}

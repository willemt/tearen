
#include <stdbool.h>
#include <assert.h>
#include "tea_vec.h"
#include "r_draw.h"

/**
 * Hash a string into a unsigned int.
 * This is a pretty good hash function. */
unsigned long r_hash_string(
    const void *e1
)
{
    const unsigned char *str = e1;
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

long r_cmp_string(
    const void *e1,
    const void *e2
)
{
    return strcmp(e1, e2);
}

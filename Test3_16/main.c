#include <stdio.h>
#include "dbg_malloc.h"

int main()
{
    char * p0[3];
    char * p1;
    int i, j;

    for(i = 0; i<3; i++){
        p0[i] = malloc(10);
        for(j = 0; j < 10; j++) p0[i][j] = i * 10 + j;
    }
    p1 = strdup("This_is_dbg_malloc_test.");

    printf("(dbg_malloc()‚ÌŒ‹‰Ê)\n\n");
    dbg_print_alloc_count(stdout);
    dbg_print_all_alloc_block(stdout);

    free(p1);
    for(i = 0; i < 3; i++)
        free(p0[i]);

    printf("(dbg_free()‚ÌŒ‹‰Ê)\n\n");
    dbg_print_alloc_count(stdout);
    dbg_print_all_alloc_block(stdout);

    exit(0);
}

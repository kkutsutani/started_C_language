#ifndef _DBG_MALLOC_H_INCLUDED_
#define _DBG_MALLOC_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void * dbg_malloc(size_t size, const char * file, int line);
void * dbg_calloc(size_t number, size_t size, const char * file, int line);
void * dbg_realloc(void * ptr, size_t size, const char * file, int line);
char * dbg_strdup(const char * str, const char * file, int line);
void dbg_free( void * ptr, const char * file, int line );

void dbg_print_alloc_count(FILE *fp);
void dbg_print_alloc_block(FILE * fp, const char * file, int line);
void dbg_print_all_alloc_block(FILE * fp);

#ifndef DBG_MALLOC_NOCHANGE
#define malloc(s)       dbg_malloc((s), __FILE__, __LINE__)
#define calloc(n, s)    dbg_calloc((n), (s), __FILE__, __LINE__)
#define realloc(p, s)   dbg_realloc((p), (s), __FILE__, __LINE__)
#define strdup(str)     dbg_strdup((str), __FILE__, __LINE__)
#define free(p)         dbg_free((p), __FILE__, __LINE__)
#endif

#endif /* _DBG_MALLOC_H_INCLUDED_ */

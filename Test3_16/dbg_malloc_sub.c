#include "dbg_malloc_p.h"

void * dbg_calloc(size_t number, size_t size, const char * file, int line)
{
    void * p;
    p = dbg_malloc(number * size, file, line);
    if(p) memset(p, 0, number * size);
    return(p);
}

void * dbg_realloc(void * ptr, size_t size, const char * file, int line)
{
    void * p;
    struct memory_block * mbp;
    struct memory_block ** hash;

    p = dbg_malloc(size, file, line);
    if((p != NULL) && (ptr != NULL)){
        hash = &(dbg_memory_block_hash[DBG_HASH_FUNC(ptr)]);
        for(mbp = *hash; mbp; mbp = mbp->hash_next ){
            if( mbp->data == ptr ) break;
        }
        if( (mbp != NULL) && (size > mbp->size)) size = mbp->size;
        memcpy(p, ptr, size);
        dbg_free(ptr, file, line);
    }
    return (p);
}

char * dbg_strdup(const char * str, const char * file, int line)
{
    int len;
    void * p;
    len = strlen(str) + 1;
    p = dbg_malloc(len, file, line);
    if(p==NULL) return (NULL);
    memcpy(p, str, len);
    return (p);
}

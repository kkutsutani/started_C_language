#define _CRT_SECURE_NO_WARNINGS
#include "dbg_malloc_p.h"
void dbg_print_alloc_count(FILE *fp)
{
    int number;
    struct memory_count * mcp;

    dbg_init();

    fprintf(fp, "malloc() count            = %u\n", dbg_malloc_count);
    fprintf(fp, "free() count              = %u\n", dbg_free_count);
    fprintf(fp, "malloc() all block number = %u\n", dbg_malloc_number);
    fprintf(fp, "malloc() all block size   = %u\n", dbg_malloc_size);
    fprintf(fp, "\n");

    fprintf(fp, "  number  alloc  free  size  (filenama line)\n");

    number = 0;
    for(mcp = dbg_memory_count_head; mcp; mcp = mcp->next){
        number++;
        fprintf(fp, "#%03d %6d %8u %8u %8d (%s %d)\n",
                number, mcp->number, mcp->alloc_count, mcp->
                free_count, mcp->size, mcp->file, mcp->line);
    }
    fprintf(fp, "\n");
}

static void print_alloc_block(FILE * fp, struct memory_count * mcp)
{
    struct memory_block * mbp;
    struct free_count * fcp;
    int n, number;

    fprintf(fp, "(%s %d)\n", mcp->file, mcp->line);
    fprintf(fp, "  number:%d size:%d\n",
            mcp->number, mcp->size);
    fprintf(fp, "  alloc (count:%u size:%u)\n",
            mcp->alloc_count, mcp->alloc_size);
    fprintf(fp, "    free  (count:%u size:%u)\n",
            mcp->free_count, mcp->free_size);

    fprintf(fp, "\n");
    if(mcp->free_list){
        for (fcp = mcp->free_list; fcp; fcp = fcp->next){
            fprintf(fp, " free(): (%s %d) count:%u size:%u\n",
                    fcp->file, fcp->line, fcp->count, fcp->
                    size);
        }
        fprintf(fp, "\n");
    }

#define PRINT_NUM 16
    number = 0;
    for(mbp = mcp->block_list; mbp; mbp = mbp->next){
        number++;
        fprintf(fp, " #%03d ID:%d size:%d pointer:0x%08x date:%s", number, mbp->id, mbp->size, (int)mbp->data, ctime(&mbp->t));


        for(n = 0; n < mbp->size; n++){
            if(( n % PRINT_NUM) == 0) fprintf(fp, "\n %08x:", n);
            if((n % (PRINT_NUM / 2)) == 0 ) fprintf(fp, " ");
            fprintf(fp, "%02x ", (int)((char *)(mbp->data))[n]);
        }
        fprintf(fp, "\n\n");
    }
}

void dbg_print_alloc_block(FILE * fp, const char * file, int line)
{
    struct memory_count * mcp;
    dbg_init();
    for(mcp = dbg_memory_count_head; mcp; mcp = mcp->next){
        if( (!strcmp(file, mcp->file)) && ((line == 0) || (line == mcp->line)) )
            print_alloc_block(fp, mcp);
    }
}

void dbg_print_all_alloc_block(FILE * fp)
{
    struct memory_count * mcp;
    dbg_init();
    for(mcp = dbg_memory_count_head; mcp; mcp = mcp->next)
        print_alloc_block(fp, mcp);
}

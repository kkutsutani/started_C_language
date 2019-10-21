#include "dbg_malloc_p.h"

/* malloc() 統計情報 */
unsigned int dbg_malloc_count   = 0;    /* malloc() 呼び出し回数 */
unsigned int dbg_free_count     = 0;    /* free() 呼び出し回数 */
unsigned int dbg_malloc_number  = 0;    /* 現在のメモリ取得数 */
unsigned int dbg_malloc_size    = 0;    /* 現在のメモリ取得量 */

struct memory_count * dbg_memory_count_head = NULL;         /* リストの先頭 */
struct memory_count * dbg_memory_count_tail = NULL;         /* リストの終端 */
struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];  /* ハッシュ用配列 */

#define LOG(m, f, l) \
  fprintf(stderr, \
  "%s:line%d:%s(): " m " (FILE:%s, LINE:%d)\n", \
  __FILE__, __LINE__, __FUNCTION__, f, l)

void dbg_init()
{
    static int initialized = 0;
    if( initialized ) return;
    dbg_memory_count_head = NULL;
    dbg_memory_count_tail = NULL;
    memset(dbg_memory_block_hash, 0, sizeof(dbg_memory_block_hash));
    initialized = 1;
}

void * dbg_malloc(size_t size, const char * file, int line)
{
    void * p = NULL;
    static int id = 0;
    struct memory_block * mbp = NULL;
    struct memory_count * mcp = NULL;
    struct memory_block ** hash;

    dbg_init();     /* G変数等の初期化 */

    if(!size){                                  /* サイズが0かの判断 */
        LOG("size of zero!", file, line);       /* エラー内容を出力 */
        goto err;                               /* サイズが0なので、何も登録せずに終了 */
    }

    /* ファイル名、行番号の検索 */
    for(mcp = dbg_memory_count_head; mcp; mcp = mcp->next ){    /* リストの先頭から検索 */
        /*
         * fileは__FILE__によって静的に与えられるので、strcmp()を使用せずに
         * ポインタを直接比較する
         */
        if( (mcp->line == line) && (mcp->file == file)) break;  /* リストが存在するので、行とファイル名を確認 */
    }                                                           /* 行とファイル名が合致していれば抜ける */

    /* 検索して見つからなかったときはデータベースを新規に作成する */
    if(mcp == NULL){                                    /* NULL: 見つからなかった場合 */
        mcp = malloc( sizeof(struct memory_count) );    /* mallocの場所ごとに管理するメモリを確保 */
        if(mcp == NULL){
            LOG("lack of memory for mcp!", file, line);
            goto err;
        }

        /*
         * 構造体を初期化する。fileは__FILE__によって静的に与えられ、
         * ポインタを直接比較するので、strdup()などでコピーしてはいけない。
         */
        memset(mcp, 0, sizeof(*mcp));   /* 確保したメモリ(構造体)を初期化する */
        mcp->file = file;               /* ファイル名を登録 */
        mcp->line = line;               /* 行を登録 */

        /* リストの終端に追加する */
        mcp->prev = dbg_memory_count_tail;      /* prev: 前の */
        mcp->next = NULL;
        if(dbg_memory_count_tail)               /* リストの終端が既に設定されているか */
            dbg_memory_count_tail->next = mcp;  /* リスト終端の次のポインタを設定 */
        else                                    /* リストの終端が設定されていない場合 */
            dbg_memory_count_head = mcp;        /* リストの先頭に設定 */
        dbg_memory_count_tail = mcp;
    }

    mbp = malloc( sizeof(struct memory_block) );
    p = malloc(size);                               /* mallocでユーザーが指定したサイズ */
    if( (mbp==NULL) || (p==NULL) ){
        LOG("lack of memory!", file, line);
        goto err;
    }

    mbp->id = id++;             /* メモリ領域ごとの固有ID */
    mbp->size = size;           /* メモリサイズ */
    mbp->t = time(NULL);        /* 指定された時刻 */
    mbp->mcp = mcp;             /* バックポインタ(対応するリストのポインタ) */
    mbp->data = p;              /* 確保されたメモリ領域へのポインタ */

    /* リストの先頭に追加 */
    mbp->prev = NULL;
    mbp->next = mcp->block_list;
    if( mcp->block_list )               /* 現在のリストにmbpが登録されているなら */
        mcp->block_list->prev = mbp;    /* 現在リストのmbpの次として登録 */
    mcp->block_list = mbp;              /* 現在のリストにmbpを登録 */

    /* ハッシュリストの先頭に追加 */
    hash = &( dbg_memory_block_hash[DBG_HASH_FUNC(p)] );    /* hash = &hash[ハッシュ値] */
    mbp->hash_prev = NULL;          /* 前 */
    mbp->hash_next = *hash;         /* 次 にhash配列[hash]の値をセット */
    if(*hash)                       /* ハッシュ用配列に登録されてる？ */
        (*hash)->hash_prev = mbp;   /* 前 に登録 */
    *hash = mbp;                    /* ハッシュ用配列にセット */

    /* 統計情報を更新 */
    mcp->number++;
    mcp->alloc_count++;
    mcp->size += size;
    mcp->alloc_size += size;
    dbg_malloc_count++;
    dbg_malloc_number++;
    dbg_malloc_size += size;

    return (p);

err:
    if(mbp) free(mbp);
    if(p) free(p);
    return (NULL);
}

void dbg_free( void * ptr, const char * file, int line )
{
    struct memory_block * mbp;
    struct memory_block ** hash;
    struct memory_count * mcp;
    struct free_count * fcp;

    dbg_init();

    if(!ptr){
        LOG("NULL pointer!", file, line);
        goto err;
    }

    /* ハッシュを検索 */
    hash = &(dbg_memory_block_hash[DBG_HASH_FUNC(ptr)]);
    for(mbp = *hash; mbp; mbp = mbp->hash_next){
        if(mbp->data == ptr) break;
    }

    if(!mbp){
        LOG("not found!", file, line);
        /*
         * ハッシュに発見できない場合には、通常のmalloc()で獲得された領域が
         * dbg_free()に渡されたと考えられるので、free()する。
         */
        free(ptr);
        return;
    }

    mcp = mbp->mcp;
    if(mcp == NULL){
        LOG("invalid pointer!", file, line);
        goto err;
    }

    /* free_countのリストを検索 */
    for( fcp = mcp->free_list; fcp; fcp = fcp->next ){
        if( (fcp->line == line) && (fcp->file == file) ) break;
    }

    /* 検索して見つからなかったときにはデータベースを新規に作成する */
    if( fcp == NULL ){
        fcp = malloc( sizeof(struct free_count) );
        if( fcp == NULL ){
            LOG("lack of memory for fcp!", file, line);
            goto err;
        }

        memset(fcp, 0, sizeof(*fcp) );
        fcp->file = file;
        fcp->line = line;

        /* リストの先頭に追加 */
        fcp->next = mcp->free_list;
        mcp->free_list = fcp;
    }

    /* リストから削除 */
    if(mbp->prev)
        mbp->prev->next = mbp->next;
    else
        mcp->block_list = mbp->next;
    if( mbp->next )
        mbp->next->prev = mbp->prev;

    /* ハッシュリストから削除 */
    if( mbp->hash_prev )
        mbp->hash_prev->hash_next = mbp->hash_next;
    else
        *hash = mbp->hash_next;
    if( mbp->hash_next )
        mbp->hash_next->hash_prev = mbp->hash_next;

    /* 統計情報を更新 */
    mcp->number--;
    mcp->free_count++;
    mcp->size -= mbp->size;
    mcp->free_size += mbp->size;
    fcp->count++;
    fcp->size += mbp->size;
    dbg_free_count++;
    dbg_malloc_number--;
    dbg_malloc_size -= mbp->size;

    free(mbp);
    free(ptr);

err:
    return;
}

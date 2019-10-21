#ifndef _DBG_MALLOC_P_H_INCLUDED_
#define _DBG_MALLOC_P_H_INCLUDED_

#define DBG_MALLOC_NOCHANGE

#include <time.h>
#include "dbg_malloc.h"

/*
 * ソース中でdbg_malloc()を利用している場所ごとに作成される構造体
 */
struct memory_count {
    const char *file;                   /* ファイル名 */
    int line;                           /* 行番号 */
    int number;                         /* 現在獲得されている領域の個数 */
    unsigned int alloc_count;           /* 過去に獲得された領域の総数 */
    unsigned int free_count;            /* 過去に開放された領域の総数 */
    int size;                           /* 現在獲得されている領域の総サイズ */
    unsigned int alloc_size;            /* 過去に獲得された領域の総サイズ */
    unsigned int free_size;             /* 過去に開放された領域の総サイズ */
    struct memory_block * block_list;   /* 現在獲得している領域のリスト */
    struct free_count * free_list;      /* 過去に開放した領域のリスト */
    struct memory_count * prev;         /* 前のstruct memory_count へのポインタ */
    struct memory_count * next;         /* 次のstruct memory_count へのポインタ */
};

/*
 * メモリ管理用のブロック。1回のdbg_malloc()呼び出しごとに1個作成され、
 * 対応する memory_countのblock_list メンバに接続される。
 */
struct memory_block {
    int id;                             /* ID番号。メモリ領域ごとの固有の値が入る */
    int size;                           /* 獲得サイズ */
    time_t t;                           /* 獲得された時刻 */
    struct memory_count * mcp;          /* バックポインタ */
    struct memory_block * prev;         /* struct memory_count の block_listのリンク */
    struct memory_block * next;         /* struct memory_count の block_listのリンク */
    struct memory_block * hash_prev;    /* ハッシュ用のリンク */
    struct memory_block * hash_next;    /* ハッシュ用のリンク */
    void * data;                        /* malloc()によって確保されたメモリ領域へのポインタ */
};

/*
 * free() 管理用のブロック。
 * どのmalloc()がどのfree()で開放されたかの統計情報を管理する。
 * 対応する memory_countのfree_list メンバに接続される。
 */
struct free_count {
    const char * file;          /* ファイル名 */
    int line;                   /* 行番号 */
    unsigned int count;         /* 過去に開放された領域の個数 */
    unsigned int size;          /* 過去に開放された領域の総サイズ */
    struct free_count * next;   /* 次のstruct free_count へのポインタ */
};

/* malloc() 統計情報 */
extern unsigned int dbg_malloc_count;                   /* malloc() 呼び出し回数 */
extern unsigned int dbg_free_count;                     /* free() 呼び出し回数 */
extern unsigned int dbg_malloc_number;                  /* 現在のメモリ取得数 */
extern unsigned int dbg_malloc_size;                    /* 現在のメモリ取得量 */

extern struct memory_count * dbg_memory_count_head;     /* リストの先頭 */
extern struct memory_count * dbg_memory_count_tail;     /* リストの終端 */

/*
 * struct memory_block 検索用のハッシュ。
 * アドレスはたぶん4バイトアラインメントされているので、下2桁は
 * 必ず4の倍数になってしまう。ハッシュ数、ハッシュ関数には注意すること
 */
#define DBG_HASH_NUM 997
#define DBG_HASH_FUNC(p) \
  (( \
  (((unsigned long int)(p) >> 4) & 0xf) + \
  (( unsigned long int)(p) >> 8) \
  ) % DBG_HASH_NUM)
/* ハッシュ用配列 */
extern struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];

void dbg_init();

#endif /* _DBG_MALLOC_P_H_INCLUDED_ */

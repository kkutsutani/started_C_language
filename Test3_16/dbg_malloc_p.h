#ifndef _DBG_MALLOC_P_H_INCLUDED_
#define _DBG_MALLOC_P_H_INCLUDED_

#define DBG_MALLOC_NOCHANGE

#include <time.h>
#include "dbg_malloc.h"

/*
 * �\�[�X����dbg_malloc()�𗘗p���Ă���ꏊ���Ƃɍ쐬�����\����
 */
struct memory_count {
    const char *file;                   /* �t�@�C���� */
    int line;                           /* �s�ԍ� */
    int number;                         /* ���݊l������Ă���̈�̌� */
    unsigned int alloc_count;           /* �ߋ��Ɋl�����ꂽ�̈�̑��� */
    unsigned int free_count;            /* �ߋ��ɊJ�����ꂽ�̈�̑��� */
    int size;                           /* ���݊l������Ă���̈�̑��T�C�Y */
    unsigned int alloc_size;            /* �ߋ��Ɋl�����ꂽ�̈�̑��T�C�Y */
    unsigned int free_size;             /* �ߋ��ɊJ�����ꂽ�̈�̑��T�C�Y */
    struct memory_block * block_list;   /* ���݊l�����Ă���̈�̃��X�g */
    struct free_count * free_list;      /* �ߋ��ɊJ�������̈�̃��X�g */
    struct memory_count * prev;         /* �O��struct memory_count �ւ̃|�C���^ */
    struct memory_count * next;         /* ����struct memory_count �ւ̃|�C���^ */
};

/*
 * �������Ǘ��p�̃u���b�N�B1���dbg_malloc()�Ăяo�����Ƃ�1�쐬����A
 * �Ή����� memory_count��block_list �����o�ɐڑ������B
 */
struct memory_block {
    int id;                             /* ID�ԍ��B�������̈悲�Ƃ̌ŗL�̒l������ */
    int size;                           /* �l���T�C�Y */
    time_t t;                           /* �l�����ꂽ���� */
    struct memory_count * mcp;          /* �o�b�N�|�C���^ */
    struct memory_block * prev;         /* struct memory_count �� block_list�̃����N */
    struct memory_block * next;         /* struct memory_count �� block_list�̃����N */
    struct memory_block * hash_prev;    /* �n�b�V���p�̃����N */
    struct memory_block * hash_next;    /* �n�b�V���p�̃����N */
    void * data;                        /* malloc()�ɂ���Ċm�ۂ��ꂽ�������̈�ւ̃|�C���^ */
};

/*
 * free() �Ǘ��p�̃u���b�N�B
 * �ǂ�malloc()���ǂ�free()�ŊJ�����ꂽ���̓��v�����Ǘ�����B
 * �Ή����� memory_count��free_list �����o�ɐڑ������B
 */
struct free_count {
    const char * file;          /* �t�@�C���� */
    int line;                   /* �s�ԍ� */
    unsigned int count;         /* �ߋ��ɊJ�����ꂽ�̈�̌� */
    unsigned int size;          /* �ߋ��ɊJ�����ꂽ�̈�̑��T�C�Y */
    struct free_count * next;   /* ����struct free_count �ւ̃|�C���^ */
};

/* malloc() ���v��� */
extern unsigned int dbg_malloc_count;                   /* malloc() �Ăяo���� */
extern unsigned int dbg_free_count;                     /* free() �Ăяo���� */
extern unsigned int dbg_malloc_number;                  /* ���݂̃������擾�� */
extern unsigned int dbg_malloc_size;                    /* ���݂̃������擾�� */

extern struct memory_count * dbg_memory_count_head;     /* ���X�g�̐擪 */
extern struct memory_count * dbg_memory_count_tail;     /* ���X�g�̏I�[ */

/*
 * struct memory_block �����p�̃n�b�V���B
 * �A�h���X�͂��Ԃ�4�o�C�g�A���C�������g����Ă���̂ŁA��2����
 * �K��4�̔{���ɂȂ��Ă��܂��B�n�b�V�����A�n�b�V���֐��ɂ͒��ӂ��邱��
 */
#define DBG_HASH_NUM 997
#define DBG_HASH_FUNC(p) \
  (( \
  (((unsigned long int)(p) >> 4) & 0xf) + \
  (( unsigned long int)(p) >> 8) \
  ) % DBG_HASH_NUM)
/* �n�b�V���p�z�� */
extern struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];

void dbg_init();

#endif /* _DBG_MALLOC_P_H_INCLUDED_ */

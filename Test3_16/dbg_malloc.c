#include "dbg_malloc_p.h"

/* malloc() ���v��� */
unsigned int dbg_malloc_count   = 0;    /* malloc() �Ăяo���� */
unsigned int dbg_free_count     = 0;    /* free() �Ăяo���� */
unsigned int dbg_malloc_number  = 0;    /* ���݂̃������擾�� */
unsigned int dbg_malloc_size    = 0;    /* ���݂̃������擾�� */

struct memory_count * dbg_memory_count_head = NULL;         /* ���X�g�̐擪 */
struct memory_count * dbg_memory_count_tail = NULL;         /* ���X�g�̏I�[ */
struct memory_block * dbg_memory_block_hash[DBG_HASH_NUM];  /* �n�b�V���p�z�� */

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

    dbg_init();     /* G�ϐ����̏����� */

    if(!size){                                  /* �T�C�Y��0���̔��f */
        LOG("size of zero!", file, line);       /* �G���[���e���o�� */
        goto err;                               /* �T�C�Y��0�Ȃ̂ŁA�����o�^�����ɏI�� */
    }

    /* �t�@�C�����A�s�ԍ��̌��� */
    for(mcp = dbg_memory_count_head; mcp; mcp = mcp->next ){    /* ���X�g�̐擪���猟�� */
        /*
         * file��__FILE__�ɂ���ĐÓI�ɗ^������̂ŁAstrcmp()���g�p������
         * �|�C���^�𒼐ڔ�r����
         */
        if( (mcp->line == line) && (mcp->file == file)) break;  /* ���X�g�����݂���̂ŁA�s�ƃt�@�C�������m�F */
    }                                                           /* �s�ƃt�@�C���������v���Ă���Δ����� */

    /* �������Č�����Ȃ������Ƃ��̓f�[�^�x�[�X��V�K�ɍ쐬���� */
    if(mcp == NULL){                                    /* NULL: ������Ȃ������ꍇ */
        mcp = malloc( sizeof(struct memory_count) );    /* malloc�̏ꏊ���ƂɊǗ����郁�������m�� */
        if(mcp == NULL){
            LOG("lack of memory for mcp!", file, line);
            goto err;
        }

        /*
         * �\���̂�����������Bfile��__FILE__�ɂ���ĐÓI�ɗ^�����A
         * �|�C���^�𒼐ڔ�r����̂ŁAstrdup()�ȂǂŃR�s�[���Ă͂����Ȃ��B
         */
        memset(mcp, 0, sizeof(*mcp));   /* �m�ۂ���������(�\����)������������ */
        mcp->file = file;               /* �t�@�C������o�^ */
        mcp->line = line;               /* �s��o�^ */

        /* ���X�g�̏I�[�ɒǉ����� */
        mcp->prev = dbg_memory_count_tail;      /* prev: �O�� */
        mcp->next = NULL;
        if(dbg_memory_count_tail)               /* ���X�g�̏I�[�����ɐݒ肳��Ă��邩 */
            dbg_memory_count_tail->next = mcp;  /* ���X�g�I�[�̎��̃|�C���^��ݒ� */
        else                                    /* ���X�g�̏I�[���ݒ肳��Ă��Ȃ��ꍇ */
            dbg_memory_count_head = mcp;        /* ���X�g�̐擪�ɐݒ� */
        dbg_memory_count_tail = mcp;
    }

    mbp = malloc( sizeof(struct memory_block) );
    p = malloc(size);                               /* malloc�Ń��[�U�[���w�肵���T�C�Y */
    if( (mbp==NULL) || (p==NULL) ){
        LOG("lack of memory!", file, line);
        goto err;
    }

    mbp->id = id++;             /* �������̈悲�Ƃ̌ŗLID */
    mbp->size = size;           /* �������T�C�Y */
    mbp->t = time(NULL);        /* �w�肳�ꂽ���� */
    mbp->mcp = mcp;             /* �o�b�N�|�C���^(�Ή����郊�X�g�̃|�C���^) */
    mbp->data = p;              /* �m�ۂ��ꂽ�������̈�ւ̃|�C���^ */

    /* ���X�g�̐擪�ɒǉ� */
    mbp->prev = NULL;
    mbp->next = mcp->block_list;
    if( mcp->block_list )               /* ���݂̃��X�g��mbp���o�^����Ă���Ȃ� */
        mcp->block_list->prev = mbp;    /* ���݃��X�g��mbp�̎��Ƃ��ēo�^ */
    mcp->block_list = mbp;              /* ���݂̃��X�g��mbp��o�^ */

    /* �n�b�V�����X�g�̐擪�ɒǉ� */
    hash = &( dbg_memory_block_hash[DBG_HASH_FUNC(p)] );    /* hash = &hash[�n�b�V���l] */
    mbp->hash_prev = NULL;          /* �O */
    mbp->hash_next = *hash;         /* �� ��hash�z��[hash]�̒l���Z�b�g */
    if(*hash)                       /* �n�b�V���p�z��ɓo�^����Ă�H */
        (*hash)->hash_prev = mbp;   /* �O �ɓo�^ */
    *hash = mbp;                    /* �n�b�V���p�z��ɃZ�b�g */

    /* ���v�����X�V */
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

    /* �n�b�V�������� */
    hash = &(dbg_memory_block_hash[DBG_HASH_FUNC(ptr)]);
    for(mbp = *hash; mbp; mbp = mbp->hash_next){
        if(mbp->data == ptr) break;
    }

    if(!mbp){
        LOG("not found!", file, line);
        /*
         * �n�b�V���ɔ����ł��Ȃ��ꍇ�ɂ́A�ʏ��malloc()�Ŋl�����ꂽ�̈悪
         * dbg_free()�ɓn���ꂽ�ƍl������̂ŁAfree()����B
         */
        free(ptr);
        return;
    }

    mcp = mbp->mcp;
    if(mcp == NULL){
        LOG("invalid pointer!", file, line);
        goto err;
    }

    /* free_count�̃��X�g������ */
    for( fcp = mcp->free_list; fcp; fcp = fcp->next ){
        if( (fcp->line == line) && (fcp->file == file) ) break;
    }

    /* �������Č�����Ȃ������Ƃ��ɂ̓f�[�^�x�[�X��V�K�ɍ쐬���� */
    if( fcp == NULL ){
        fcp = malloc( sizeof(struct free_count) );
        if( fcp == NULL ){
            LOG("lack of memory for fcp!", file, line);
            goto err;
        }

        memset(fcp, 0, sizeof(*fcp) );
        fcp->file = file;
        fcp->line = line;

        /* ���X�g�̐擪�ɒǉ� */
        fcp->next = mcp->free_list;
        mcp->free_list = fcp;
    }

    /* ���X�g����폜 */
    if(mbp->prev)
        mbp->prev->next = mbp->next;
    else
        mcp->block_list = mbp->next;
    if( mbp->next )
        mbp->next->prev = mbp->prev;

    /* �n�b�V�����X�g����폜 */
    if( mbp->hash_prev )
        mbp->hash_prev->hash_next = mbp->hash_next;
    else
        *hash = mbp->hash_next;
    if( mbp->hash_next )
        mbp->hash_next->hash_prev = mbp->hash_next;

    /* ���v�����X�V */
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

#include <unistd.h>
#include <limits.h>
#include "boruvka/alloc.h"
#include "boruvka/bucketheap.h"

static size_t pagesize(void)
{
    return sysconf(_SC_PAGESIZE);
}

bor_bucketheap_t *borBucketHeapNew(bor_bucketheap_key key, void *data)
{
    bor_bucketheap_t *bh;
    size_t segm_size = pagesize();

    bh = BOR_ALLOC(bor_bucketheap_t);
    bh->bucket = borSegmArrNew(sizeof(bor_list_t), segm_size);
    bh->bucket_size = 0;
    bh->node_size = 0;
    bh->lowest_key = ULONG_MAX;
    bh->key = key;
    bh->data = data;

    return bh;
}


void borBucketHeapDel(bor_bucketheap_t *bh)
{
    if (bh->bucket)
        borSegmArrDel(bh->bucket);
    BOR_FREE(bh);
}

void borBucketHeapAdd(bor_bucketheap_t *bh, bor_bucketheap_node_t *n)
{
    unsigned long key, i;
    bor_list_t *bucket;

    key = bh->key(n, bh->data);

    if (key >= bh->bucket_size){
        for (i = bh->bucket_size; i <= key; ++i){
            bucket = borSegmArrGet(bh->bucket, i);
            borListInit(bucket);
        }

        bh->bucket_size = key + 1;
    }

    bucket = borSegmArrGet(bh->bucket, key);
    borListAppend(bucket, n);
    ++bh->node_size;
    if (key < bh->lowest_key)
        bh->lowest_key = key;
}
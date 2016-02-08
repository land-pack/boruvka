#include <string.h>
#include <stdio.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>
#include "msg-schema-common.h"
#include "msg-schema.schema.c"

#define EQF(a, b, member) \
    do { \
    if (a->member != b->member){ \
        fprintf(stderr, "DIFF on member %s\n", #member); \
        return 0; \
    } \
    } while (0)

#define EQARR(a, b, member) \
    if (a->member##_size != b->member##_size) return 0; \
    if (a->member##_alloc != b->member##_alloc) return 0; \
    if (memcmp(a->member, b->member, sizeof(*a->member) * a->member##_size) != 0) \
        return 0

int msgSubEq(const test_submsg_t *a, const test_submsg_t *b)
{
    EQF(a, b, sval);
    EQF(a, b, lval_neco);
    EQF(a, b, i16val);
    EQARR(a, b, arr);
    return 1;
}

int msg2ArrEq(const test_msg2_arr_t *a, const test_msg2_arr_t *b)
{
    EQARR(a, b, ai8);
    EQARR(a, b, au8);
    EQARR(a, b, ai16);
    EQARR(a, b, au16);
    EQARR(a, b, ai32);
    EQARR(a, b, au32);
    EQARR(a, b, ai64);
    EQARR(a, b, au64);
    EQARR(a, b, ac);
    EQARR(a, b, auc);
    EQARR(a, b, as);
    EQARR(a, b, aus);
    EQARR(a, b, ai);
    EQARR(a, b, aui);
    EQARR(a, b, al);
    EQARR(a, b, aul);
    EQARR(a, b, af);
    EQARR(a, b, ad);
    return 1;
}

int msg2Eq(const test_msg2_t *a, const test_msg2_t *b)
{
    int i;

    EQF(a, b, i8);
    EQF(a, b, u8);
    EQF(a, b, i16);
    EQF(a, b, u16);
    EQF(a, b, i32);
    EQF(a, b, u32);
    EQF(a, b, i64);
    EQF(a, b, u64);
    EQF(a, b, c);
    EQF(a, b, uc);
    EQF(a, b, s);
    EQF(a, b, us);
    EQF(a, b, i);
    EQF(a, b, ui);
    EQF(a, b, l);
    EQF(a, b, ul);
    EQF(a, b, f);
    EQF(a, b, d);

    if (!msgSubEq(&a->sub, &b->sub))
        return 0;

    if (a->subs_size != b->subs_size)
        return 0;
    if (a->subs_alloc != b->subs_alloc)
        return 0;
    for (i = 0; i < a->subs_size; ++i){
        if (!msgSubEq(a->subs + i, b->subs + i))
            return 0;
    }

    if (!msg2ArrEq(&a->subarr, &b->subarr))
        return 0;

    return 1;
}

#define RNDF(m, rnd, member) \
    m->member = borRand(&rnd, -1E5, 1E5)
#define RNDARR(m, rnd, member) \
    do { \
        int i; \
        m->member##_size = borRand(&rnd, 0, 10000); \
        m->member##_alloc = m->member##_size; \
        if (m->member##_alloc > 0){ \
            m->member = (void *)BOR_ALLOC_ARR(char, sizeof(*m->member) * m->member##_alloc); \
        } \
        for (i = 0; i < m->member##_size; ++i){ \
            m->member[i] = borRand(&rnd, -1E5, 1E5); \
        } \
    } while (0)

void msgSubRand(test_submsg_t *m)
{
    bor_rand_t rnd;
    borRandInit(&rnd);

    borMsgInit(m, &schema_test_submsg_t);
    RNDF(m, rnd, sval);
    RNDF(m, rnd, lval_neco);
    RNDF(m, rnd, i16val);
    RNDARR(m, rnd, arr);
}

void msg2ArrRand(test_msg2_arr_t *m)
{
    bor_rand_t rnd;
    borRandInit(&rnd);

    borMsgInit(m, &schema_test_msg2_arr_t);
    RNDARR(m, rnd, ai8);
    RNDARR(m, rnd, au8);
    RNDARR(m, rnd, ai16);
    RNDARR(m, rnd, au16);
    RNDARR(m, rnd, ai32);
    RNDARR(m, rnd, au32);
    RNDARR(m, rnd, ai64);
    RNDARR(m, rnd, au64);
    RNDARR(m, rnd, ac);
    RNDARR(m, rnd, auc);
    RNDARR(m, rnd, as);
    RNDARR(m, rnd, aus);
    RNDARR(m, rnd, ai);
    RNDARR(m, rnd, aui);
    RNDARR(m, rnd, al);
    RNDARR(m, rnd, aul);
    RNDARR(m, rnd, af);
    RNDARR(m, rnd, ad);
}

void msg2Rand(test_msg2_t *m)
{
    int i;
    bor_rand_t rnd;
    borRandInit(&rnd);

    borMsgInit(m, &schema_test_msg2_t);
    RNDF(m, rnd, i8);
    RNDF(m, rnd, u8);
    RNDF(m, rnd, i16);
    RNDF(m, rnd, u16);
    RNDF(m, rnd, i32);
    RNDF(m, rnd, u32);
    RNDF(m, rnd, i64);
    RNDF(m, rnd, u64);
    RNDF(m, rnd, c);
    RNDF(m, rnd, uc);
    RNDF(m, rnd, s);
    RNDF(m, rnd, us);
    RNDF(m, rnd, i);
    RNDF(m, rnd, ui);
    RNDF(m, rnd, l);
    RNDF(m, rnd, ul);
    RNDF(m, rnd, f);
    RNDF(m, rnd, d);

    msgSubRand(&m->sub);

    m->subs_size = borRand(&rnd, 0, 100);
    m->subs_alloc = m->subs_size;
    if (m->subs_alloc > 0)
        m->subs = BOR_ALLOC_ARR(test_submsg_t, m->subs_alloc);
    for (i = 0; i < m->subs_size; ++i)
        msgSubRand(m->subs + i);

    msg2ArrRand(&m->subarr);
}


/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_FIBO_H__
#define __FER_FIBO_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FER_FIBO_CONS_SIZE (sizeof(unsigned int) << 3)

/**
 * Fibo - Fibonnacci Heap (priority queue)
 * ========================================
 *
 */

/** vvvv */
struct _bor_fibo_node_t {
    unsigned int degree; /*!< Degree of node */
    int mark;
    struct _bor_fibo_node_t *parent; /*!< Parent node */
    bor_list_t children;             /*!< List of children */
    bor_list_t list;                 /*!< Connection into list of siblings */
};
typedef struct _bor_fibo_node_t bor_fibo_node_t;

/**
 * Callback that should return true if {n1} is smaller than {n2}.
 */
typedef int (*fer_fibo_lt)(const bor_fibo_node_t *n1,
                           const bor_fibo_node_t *n2,
                           void *data);

struct _bor_fibo_t {
    bor_list_t root;      /*!< List of root nodes */
    bor_fibo_node_t *min; /*!< Minimal node */
    bor_fibo_node_t *cons[FER_FIBO_CONS_SIZE]; /*!< Consolidation array
                                                    with bitlength(degree)
                                                    elements */
    unsigned int max_degree; /*!< Maximal degree used in .cons */

    fer_fibo_lt lt; /*!< "Less than" callback provided by user */
    void *data;
};
typedef struct _bor_fibo_t bor_fibo_t;
/** ^^^^ */


/**
 * Functions
 * ----------
 */

/**
 * Creates new empty Fibonnacci heap.
 * Callback for comparison must be provided.
 */
bor_fibo_t *ferFiboNew(fer_fibo_lt less_than, void *data);

/**
 * Deletes fibonnacci heap.
 * Note that individual nodes are not disconnected from heap.
 */
void ferFiboDel(bor_fibo_t *fibo);

/**
 * Returns true if heap is empty.
 */
_fer_inline int ferFiboEmpty(const bor_fibo_t *f);

/**
 * Returns minimal node.
 */
_fer_inline bor_fibo_node_t *ferFiboMin(bor_fibo_t *f);

/**
 * Adds node to heap.
 */
_fer_inline void ferFiboAdd(bor_fibo_t *f, bor_fibo_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
bor_fibo_node_t *ferFiboExtractMin(bor_fibo_t *f);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased (or you are not sure) call ferFiboUpdate()
 * instead.
 */
void ferFiboDecreaseKey(bor_fibo_t *f, bor_fibo_node_t *n);

/**
 * Updates position of node in heap.
 * This generalized (and slower) version of ferFiboDecreaseKey() function.
 */
_fer_inline void ferFiboUpdate(bor_fibo_t *f, bor_fibo_node_t *n);

/**
 * Del node from heap.
 */
void ferFiboRemove(bor_fibo_t *f, bor_fibo_node_t *n);


/**** INLINES ****/
_fer_inline int ferFiboEmpty(const bor_fibo_t *f)
{
    return ferListEmpty(&f->root);
}

_fer_inline bor_fibo_node_t *ferFiboMin(bor_fibo_t *f)
{
    return f->min;
}

_fer_inline void ferFiboAdd(bor_fibo_t *f, bor_fibo_node_t *n)
{
    n->degree = 0;
    n->mark   = 0;
    n->parent = 0;
    ferListInit(&n->children);
    ferListAppend(&f->root, &n->list);

    if (!f->min || f->lt(n, f->min, f->data))
        f->min = n;
}

_fer_inline void ferFiboUpdate(bor_fibo_t *f, bor_fibo_node_t *n)
{
    ferFiboRemove(f, n);
    ferFiboAdd(f, n);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_FIBO_H__ */


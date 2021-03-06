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

#ifndef __BOR_SORT_H__
#define __BOR_SORT_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Sort Algorithms
 * ================
 */

struct _bor_radix_sort_t {
    bor_real_t key;
    int val;
};
typedef struct _bor_radix_sort_t bor_radix_sort_t;

/**
 * Sorts array {rs} using radix sort by key.
 */
void borRadixSort(bor_radix_sort_t *rs, bor_radix_sort_t *tmp, size_t len);

/**
 * Sorts an array {arr} using radix sort algorithm.
 * Each element of the array is considered as pointer to a struct and an
 * {offset} is offset of bor_real_t key inside that struct.
 * If {descending} is set to true, elements are sorted in descending order.
 */
void borRadixSortPtr(void **arr, void **tmp_arr, size_t arrlen,
                     size_t offset, int descending);

/**
 * Callback for list sorts.
 * Returns true if l1 < l2
 */
typedef int (*bor_list_sort_lt)(bor_list_t *l1, bor_list_t *l2, void *data);

/**
 * Insertion sort for lists.
 * It sorts the list in ascending order
 */
void borInsertSortList(bor_list_t *list, bor_list_sort_lt cb, void *data);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_SORT_H__ */

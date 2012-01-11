/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_SORT_H__
#define __FER_SORT_H__

#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Sort Algorithms
 * ================
 */

struct _fer_radix_sort_t {
    fer_real_t key;
    int val;
};
typedef struct _fer_radix_sort_t fer_radix_sort_t;

/**
 * Sorts array {rs} using radix sort by key.
 */
void ferRadixSort(fer_radix_sort_t *rs, fer_radix_sort_t *tmp, size_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_SORT_H__ */

/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#include <stdio.h>
#include <stdarg.h>
#include <boruvka/alloc.h>
#include <boruvka/vec.h>
#include <boruvka/dbg.h>

bor_vec_t *ferVecNew(size_t size)
{
    bor_vec_t *v;
    v = FER_ALLOC_ARR(bor_vec_t, size);
    return v;
}

void ferVecDel(bor_vec_t *v)
{
    FER_FREE(v);
}

void ferVecSetN(size_t size, bor_vec_t *v, ...)
{
    va_list ap;
    bor_real_t val;
    size_t i;

    va_start(ap, v);
    for (i = 0; i < size; i++){
        val = va_arg(ap, double);
        ferVecSet(v, i, val);
    }
    va_end(ap);
}

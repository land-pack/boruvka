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

#ifndef __BOR_HFUNC_H__
#define __BOR_HFUNC_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HFunc - Hash Functions
 * =======================
 */


/**
 * Robert Jenkins hash function.
 *
 * Taken from http://burtleburtle.net/bob/c/lookup3.c.
 */
uint32_t borHashJenkins(const uint32_t *k, size_t length, uint32_t initval);

/**
 * Hash function by Dan Bernstein
 */
uint32_t borHashDJB2(const char *str);

/*
 * This algorithm was created for sdbm (a public-domain reimplementation of
 * ndbm) database library. it was found to do well in scrambling bits,
 * causing better distribution of the keys and fewer splits.
 * It also happens to be a good general hashing function with good distribution.
 * The actual function is hash(i) = hash(i - 1) * 65599 + str[i]; what is
 * included below is the faster version used in gawk. [there is even a
 * faster, duff-device version]
 * The magic constant 65599 was picked out of thin air while experimenting
 * with different constants, and turns out to be a prime. This is one of
 * the algorithms used in berkeley db (see sleepycat) and elsewhere.
 */
uint32_t borHashSDBM(const char *str);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_HFUNC_H__ */

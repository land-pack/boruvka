/***
 * fermat
 * -------
 * Based on code by Makoto Matsumoto, Takuji Nishimura, and Shawn Cokus
 * Based on code by Richard J. Wagner - v1.1
 *      (http://www-personal.umich.edu/~wagnerr/MersenneTwister.html)
 *
 * Copyright (c)1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 * Copyright (c)2000 - 2009, Richard J. Wagner
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

#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <fermat/rand-mt.h>
#include <fermat/alloc.h>

_fer_inline void ferRandMTInit(fer_rand_mt_t *r, uint32_t seed);

_fer_inline uint32_t hiBit(uint32_t u);
_fer_inline uint32_t loBit(uint32_t u);
_fer_inline uint32_t loBits(uint32_t u);
_fer_inline uint32_t mixBits(uint32_t u, uint32_t v);
_fer_inline uint32_t magic(uint32_t u);
_fer_inline uint32_t twist(uint32_t m, uint32_t s0, uint32_t s1);
static uint32_t hash(time_t t, clock_t c);


fer_rand_mt_t *ferRandMTNew(uint32_t seed)
{
    fer_rand_mt_t *r;
    r = FER_ALLOC(fer_rand_mt_t);

    ferRandMTReseed(r, seed);

    return r;
}

fer_rand_mt_t *ferRandMTNew2(uint32_t *seed, uint32_t seedlen)
{
    fer_rand_mt_t *r;

    r = FER_ALLOC(fer_rand_mt_t);

    ferRandMTReseed2(r, seed, seedlen);

    return r;
}

fer_rand_mt_t *ferRandMTNewAuto(void)
{
    fer_rand_mt_t *r;
    r = FER_ALLOC(fer_rand_mt_t);

    ferRandMTReseedAuto(r);

    return r;
}

void ferRandMTDel(fer_rand_mt_t *r)
{
    free(r);
}

void ferRandMTReseed(fer_rand_mt_t *r, uint32_t seed)
{
    ferRandMTInit(r, seed);
    __ferRandMTReload(r);
}

void ferRandMTReseed2(fer_rand_mt_t *r, uint32_t *seed, uint32_t seedlen)
{
    register int i = 1;
    register uint32_t j = 0;
    register int k = FER_MAX(FER_RAND_MT_N, seedlen);

    // Seed the generator with an array of uint32's
    // There are 2^19937-1 possible initial states.  This function allows
    // all of those to be accessed by providing at least 19937 bits (with a
    // default seed length of N = 624 uint32's).  Any bits above the lower 32
    // in each element are discarded.

    ferRandMTInit(r, 19650218UL);

    for (; k; --k){
        r->state[i] ^= ((r->state[i-1] ^ (r->state[i-1] >> 30)) * 1664525UL);
        r->state[i] += (seed[j] & 0xffffffffUL) + j;
        r->state[i] &= 0xffffffffUL;

        ++i;
        ++j;

        if (i >= FER_RAND_MT_N){
            r->state[0] = r->state[FER_RAND_MT_N - 1];
            i = 1;
        }

        if (j >= seedlen)
            j = 0;
    }

    for(k = FER_RAND_MT_N - 1; k; --k){
        r->state[i] ^= ((r->state[i-1] ^ (r->state[i-1] >> 30)) * 1566083941UL);
        r->state[i] -= i;
        r->state[i] &= 0xffffffffUL;

        ++i;
        if (i >= FER_RAND_MT_N){
            r->state[0] = r->state[FER_RAND_MT_N - 1];
            i = 1;
        }
    }

    r->state[0] = 0x80000000UL;  // MSB is 1, assuring non-zero initial array

    __ferRandMTReload(r);
}

void ferRandMTReseedAuto(fer_rand_mt_t *r)
{
    FILE *urandom;
    uint32_t bigSeed[FER_RAND_MT_N];
    register uint32_t *s;
    register int i;
    register int success;

    // first try /dev/urandom
    urandom = fopen("/dev/urandom", "rb");
    if (urandom){
        s = bigSeed;
        i = FER_RAND_MT_N;
        success = 1;
        while (success && i--)
            success = fread(s++, sizeof(uint32_t), 1, urandom);
        fclose(urandom);
        if (success){
            ferRandMTReseed2(r, bigSeed, FER_RAND_MT_N);
        }
    }
    
    ferRandMTReseed(r, hash(time(NULL), clock()));
}


_fer_inline void ferRandMTInit(fer_rand_mt_t *g, uint32_t seed)
{
    // Initialize generator state with seed
    // See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
    // In previous versions, most significant bits (MSBs) of the seed affect
    // only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto.
    register uint32_t *s = g->state;
    register uint32_t *r = g->state;
    register int i = 1;
    *s++ = seed & 0xffffffffUL;
    for( ; i < FER_RAND_MT_N; ++i )
    {
        *s++ = ( 1812433253UL * ( *r ^ (*r >> 30) ) + i ) & 0xffffffffUL;
        r++;
    }
}

void __ferRandMTReload(fer_rand_mt_t *r)
{
    // Generate N new values in state
    // Made clearer and faster by Matthew Bellew (matthew.bellew@home.com)
    static const int MmN = (int)FER_RAND_MT_M - (int)FER_RAND_MT_N;
    register uint32_t *p = r->state;
    register int i;
    for(i = FER_RAND_MT_N - FER_RAND_MT_M; i--; ++p)
        *p = twist(p[FER_RAND_MT_M], p[0], p[1]);
    for(i = FER_RAND_MT_M; --i; ++p)
        *p = twist(p[MmN], p[0], p[1]);
    *p = twist(p[MmN], p[0], r->state[0]);
    
    r->left = FER_RAND_MT_N;
    r->next = r->state;
}


_fer_inline uint32_t hiBit(uint32_t u )
{
    return u & 0x80000000UL;
}

_fer_inline uint32_t loBit(uint32_t u)
{
    return u & 0x00000001UL;
}

_fer_inline uint32_t loBits(uint32_t u)
{
    return u & 0x7fffffffUL;
}

_fer_inline uint32_t mixBits(uint32_t u, uint32_t v)
{
    return hiBit(u) | loBits(v);
}

_fer_inline uint32_t magic(uint32_t u )
{
    return loBit(u) ? 0x9908b0dfUL : 0x0UL;
}

_fer_inline uint32_t twist(uint32_t m, uint32_t s0, uint32_t s1)
{
    return m ^ (mixBits(s0, s1) >> 1) ^ magic(s1);
}

static uint32_t hash(time_t t, clock_t c)
{
    // Get a uint32 from t and c
    // Better than uint32(x) in case x is floating point in [0,1]
    // Based on code by Lawrence Kirby (fred@genesis.demon.co.uk)
    
    static uint32_t differ = 0;  // guarantee time-based seeds will change
    uint32_t h1 = 0;
    uint32_t h2 = 0;
    unsigned char *p = (unsigned char *)&t;
    size_t i;

    for(i = 0; i < sizeof(t); ++i){
        h1 *= UCHAR_MAX + 2U;
        h1 += p[i];
    }

    p = (unsigned char *)&c;
    for(i = 0; i < sizeof(c); ++i)
    {
        h2 *= UCHAR_MAX + 2U;
        h2 += p[i];
    }

    return (h1 + differ++) ^ h2;
}

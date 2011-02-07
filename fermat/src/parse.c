/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#include "fermat/parse.h"
#include "fermat/dbg.h"

#define NOT_WS(c) \
    ( c != ' ' && c != '\t' && c != '\n')

/* Implementation taken from SVT project. */
int ferParseReal(const char *str, const char *strend, fer_real_t *val, char **next)
{
    char c;
    fer_real_t fract;
    fer_real_t mult;
    fer_real_t num;
    int negative = 0;
    int has_e;

    if (str >= strend)
        return -1;

    // skip initial whitespace
    while (!NOT_WS(*str))
        ++str;

    c = *str;
    *val = FER_ZERO;

    /* process sign */
    if (c == '-'){
        negative = 1;
        c = *++str;
    }else if (c == '+')
        c = *++str;

    /* process initial digits */
    while (c != 0 && NOT_WS(c) && str < strend){
        /* skip to next part */
        if (c == '.' || c == 'e' || c == 'E')
            break;

        /* not number -> invalid string */
        if (c < 48 || c > 57)
            return -1;

        *val = *val * FER_REAL(10.) + (c - 48);
        c = *++str;
    }

    if (!NOT_WS(c) || str >= strend){
        if (next)
            *next = (char *)str;
        return 0;
    }

    /* process decimal part */
    if (c == '.'){
        c = *++str;
        mult = FER_REAL(0.1);
        fract = FER_ZERO;
        while (c != 0 && NOT_WS(c) && str < strend){
            /* skip to next part */
            if (c == 'e' || c == 'E')
                break;

            /* no digit -> invalid string */
            if (c < 48 || c > 57)
                return -1;

            fract = fract + mult * (c - 48);
            mult *= FER_REAL(0.1);
            c = *++str;
        }
        *val += fract;
    }

    /* apply negative flag */
    if (negative)
        *val = *val * FER_REAL(-1.);

    if (!NOT_WS(c) || str >= strend){
        if (next)
            *next = (char *)str;
        return 0;
    }

    /* process exponent part */
    has_e = 0;
    if (c == 'e' || c == 'E'){
        c = *++str;
        negative = 0;
        num = FER_ZERO;

        if (c == '-'){
            negative = 1;
            c = *++str;
        }else if (c == '+')
            c = *++str;

        while (c != 0 && NOT_WS(c) && str < strend){
            if (c < 48 || c > 57)
                return -1;

            num = num * FER_REAL(10.) + (c - 48);
            c = *++str;
            has_e = 1;
        }

        if (negative)
            num *= FER_REAL(-1.);

        if (has_e){
            mult = FER_POW(FER_REAL(10.), num);
            *val *= mult;
        }
    }

    if (next)
        *next = (char *)str;

    return 0;
}


int ferParseVec3(const char *_str, const char *strend, fer_vec3_t *vec, char **n)
{
    fer_real_t v[3];
    size_t i;
    char *str, *next;

    str = (char *)_str;
    for (i = 0; i < 3 && str < strend; i++){
        if (ferParseReal(str, strend, &v[i], &next) != 0)
            break;
        str = next;
    }

    if (i != 3)
        return -1;

    ferVec3Set(vec, v[0], v[1], v[2]);
    if (n)
        *n = str;

    return 0;
}

int ferParseVec2(const char *_str, const char *strend, fer_vec2_t *vec, char **n)
{
    fer_real_t v[2];
    size_t i;
    char *str, *next;

    str = (char *)_str;
    for (i = 0; i < 2 && str < strend; i++){
        if (ferParseReal(str, strend, &v[i], &next) != 0)
            break;
        str = next;
    }

    if (i != 2)
        return -1;

    ferVec2Set(vec, v[0], v[1]);
    if (n)
        *n = str;

    return 0;
}

int ferParseLong(const char *str, const char *strend, long *val, char **next)
{
    char c;
    int negative = 0;

    if (str >= strend)
        return -1;

    // skip initial whitespace
    while (!NOT_WS(*str))
        ++str;

    c = *str;
    *val = 0L;

    /* process sign */
    if (c == '-'){
        negative = 1;
        c = *++str;
    }else if (c == '+')
        c = *++str;

    /* process initial digits */
    while (c != 0 && NOT_WS(c) && str < strend){
        /* not number -> invalid string */
        if (c < 48 || c > 57)
            return -1;

        *val = *val * 10L + (c - 48);
        c = *++str;
    }

    if (negative)
        *val = -1L * *val;

    if (next)
        *next = (char *)str;
    return 0;
}

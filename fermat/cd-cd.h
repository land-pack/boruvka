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

#ifndef __FER_CD_CD_H__
#define __FER_CD_CD_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Collision Detection
 * ====================
 */
struct _fer_cd_t {
    int build_flags;
    fer_cd_collide_fn collide[FER_CD_SHAPE_LEN][FER_CD_SHAPE_LEN];
};
typedef struct _fer_cd_t fer_cd_t;

/**
 * New instance of collision detection library.
 */
fer_cd_t *ferCDNew(void);

/**
 * Destructor.
 */
void ferCDDel(fer_cd_t *cd);

/**
 * Sets build flags.
 * These flags modifies building of OBB hierarchy.
 *
 * See macros:
 * - FER_CD_FIT_COVARIANCE
 * - FER_CD_FIT_CALIPERS
 * - FER_CD_FIT_CALIPERS_NUM_ROT()
 *
 * Default is FER_CD_COVARIANCE.
 */
void ferCDSetBuildFlags(fer_cd_t *cd, int flags);

/**
 * Sets collider between shape1 and shape2 (in this order).
 */
void ferCDSetCollideFn(fer_cd_t *cd, int shape1, int shape2,
                       fer_cd_collide_fn collider);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CD_H__ */

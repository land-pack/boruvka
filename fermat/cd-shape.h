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

#ifndef __FER_CD_SHAPE_H__
#define __FER_CD_SHAPE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_shape_t;

/**
 * Destructor.
 */
typedef void (*fer_cd_shape_del_fn)(struct _fer_cd_shape_t *shape);

/**
 * Support function. Returns furthest point on shape in given direction.
 * It is assumed dir is unit vector.
 */
typedef void (*fer_cd_shape_support_fn)(const struct _fer_cd_shape_t *shape,
                                        const fer_vec3_t *dir,
                                        fer_vec3_t *p);

/**
 * Returns center of shape.
 */
typedef void (*fer_cd_shape_center_fn)(const struct _fer_cd_shape_t *shape,
                                       const fer_mat3_t *rot,
                                       const fer_vec3_t *tr,
                                       fer_vec3_t *center);

/**
 * Updates min/max values along given axis
 */
typedef void (*fer_cd_shape_update_minmax_fn)(const struct _fer_cd_shape_t *shape,
                                              const fer_vec3_t *axis,
                                              const fer_mat3_t *rot,
                                              const fer_vec3_t *tr,
                                              fer_real_t *min, fer_real_t *max);

/**
 * Returns axis and half extents of bounding box that tightly fit to shape.
 */
typedef void (*fer_cd_shape_fit_obb_fn)(const struct _fer_cd_shape_t *shape,
                                        fer_vec3_t *center,
                                        fer_vec3_t *axis0,
                                        fer_vec3_t *axis1,
                                        fer_vec3_t *axis2,
                                        fer_vec3_t *half_extents,
                                        int flags);

/**
 * Updates given convex hull to cover the shape.
 * Returns 1 if shape is fully contained in convex hull.
 * Returns 0 if shape is not contained in convex hull but at least skelet
 * is conatined (and thus orientation can be obtained from convex hull).
 */
typedef int (*fer_cd_shape_update_chull_fn)(const struct _fer_cd_shape_t *shape,
                                            fer_chull3_t *chull,
                                            const fer_mat3_t *rot,
                                            const fer_vec3_t *tr);

/**
 * Updates covariance matrix {cov}, weighted center {wcenter}, {area} of
 * element and number of elements {num}.
 */
typedef void (*fer_cd_shape_update_cov_fn)(const struct _fer_cd_shape_t *shape,
                                           const fer_mat3_t *rot,
                                           const fer_vec3_t *tr,
                                           fer_vec3_t *wcenter,
                                           fer_mat3_t *cov,
                                           fer_real_t *area,
                                           int *num);
                                                 
/**
 * Dump shape in SVT format
 */
typedef void (*fer_cd_shape_dump_svt_fn)(const struct _fer_cd_shape_t *shape,
                                         FILE *out, const char *name,
                                         const fer_mat3_t *rot,
                                         const fer_vec3_t *tr);
/**
 * Shape type class.
 */
struct _fer_cd_shape_class_t {
    int type;
    fer_cd_shape_del_fn del;
    fer_cd_shape_support_fn support;
    fer_cd_shape_center_fn center;
    fer_cd_shape_fit_obb_fn fit_obb;
    fer_cd_shape_update_chull_fn update_chull;
    fer_cd_shape_update_minmax_fn update_minmax;
    fer_cd_shape_update_cov_fn update_cov;
    fer_cd_shape_dump_svt_fn dump_svt;
};
typedef struct _fer_cd_shape_class_t fer_cd_shape_class_t;


/**
 * Base shape struct.
 */
struct _fer_cd_shape_t {
    fer_cd_shape_class_t *cl;
};
typedef struct _fer_cd_shape_t fer_cd_shape_t;


void ferCDShapeUpdateCovTri(const fer_vec3_t *_p, const fer_vec3_t *_q,
                            const fer_vec3_t *_r,
                            const fer_mat3_t *rot, const fer_vec3_t *tr,
                            fer_vec3_t *wcenter, fer_mat3_t *cov,
                            fer_real_t *area, int *num);

/**
 * Shape with set offset
 */
struct _fer_cd_shape_off_t {
    fer_cd_shape_class_t *cl;
    fer_cd_shape_t *shape;
    fer_mat3_t *rot;
    fer_vec3_t *tr;
};
typedef struct _fer_cd_shape_off_t fer_cd_shape_off_t;

/**
 * New shape with offset.
 */
fer_cd_shape_off_t *ferCDShapeOffNew(fer_cd_shape_t *s,
                                     const fer_mat3_t *rot,
                                     const fer_vec3_t *tr);

/**
 * Deletes offset.
 */
void ferCDShapeOffDel(fer_cd_shape_off_t *s);

void ferCDShapeOffSupport(const fer_cd_shape_off_t *s, const fer_vec3_t *dir,
                          fer_vec3_t *p);

void ferCDShapeOffCenter(const fer_cd_shape_off_t *s,
                         const fer_mat3_t *rot, const fer_vec3_t *tr,
                         fer_vec3_t *center);

void ferCDShapeOffFitOBB(const fer_cd_shape_off_t *s,
                         fer_vec3_t *center,
                         fer_vec3_t *axis0,
                         fer_vec3_t *axis1,
                         fer_vec3_t *axis2,
                         fer_vec3_t *half_extents, int flags);

int ferCDShapeOffUpdateCHull(const fer_cd_shape_off_t *s, fer_chull3_t *chull,
                             const fer_mat3_t *rot, const fer_vec3_t *tr);

void ferCDShapeOffUpdateMinMax(const fer_cd_shape_off_t *s, const fer_vec3_t *axis,
                               const fer_mat3_t *rot, const fer_vec3_t *tr,
                               fer_real_t *min, fer_real_t *max);

void ferCDShapeOffUpdateCov(const fer_cd_shape_off_t *s,
                            const fer_mat3_t *rot, const fer_vec3_t *tr,
                            fer_vec3_t *wcenter, fer_mat3_t *cov,
                            fer_real_t *area, int *num);

void ferCDShapeOffDumpSVT(const fer_cd_shape_off_t *s,
                          FILE *out, const char *name,
                          const fer_mat3_t *rot, const fer_vec3_t *tr);


/** Shape types */
#define FER_CD_SHAPE_SPHERE      0
#define FER_CD_SHAPE_BOX         1
#define FER_CD_SHAPE_CYL         2
#define FER_CD_SHAPE_CAP         3
#define FER_CD_SHAPE_TRI         5
#define FER_CD_SHAPE_TRIMESH_TRI 6
#define FER_CD_SHAPE_OFF         7
#define FER_CD_SHAPE_LEN         8

#define FER_CD_SHAPE_TRIMESH 20

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SHAPE_H__ */


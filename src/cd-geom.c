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

#include <fermat/cd.h>
#include <fermat/alloc.h>

fer_cd_geom_t *ferCDGeomNew(void)
{
    fer_cd_geom_t *g;

    g = FER_ALLOC_ALIGN(fer_cd_geom_t, 16);
    ferVec3Set(&g->tr, FER_ZERO, FER_ZERO, FER_ZERO);
    ferMat3SetIdentity(&g->rot);
    ferListInit(&g->obbs);

    return g;
}

void ferCDGeomDel(fer_cd_geom_t *g)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    while (!ferListEmpty(&g->obbs)){
        item = ferListNext(&g->obbs);
        ferListDel(item);
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        ferCDOBBDel(obb);
    }

    free(g);
}

void ferCDGeomBuild(fer_cd_geom_t *g)
{
    // TODO: flags
    ferCDOBBMerge(&g->obbs, 0);
}

void ferCDGeomAddTriMesh(fer_cd_geom_t *g, const fer_vec3_t *pts,
                         const unsigned int *ids, size_t len)
{
    fer_cd_obb_t *obb;

    obb = ferCDOBBNewTriMesh(pts, ids, len, FER_CDOBB_TRIMESH_FAST);
    ferListAppend(&g->obbs, &obb->list);
}

void ferCDGeomAddSphere(fer_cd_geom_t *g, const fer_vec3_t *center,
                        fer_real_t radius)
{
    fer_cd_obb_t *obb;

    obb = ferCDOBBNewSphere(center, radius);
    ferListAppend(&g->obbs, &obb->list);
}


struct __collide_t {
    const fer_cd_geom_t *g1;
    const fer_cd_geom_t *g2;
    int ret;
};

static int __ferCDGeomCollideCB(const fer_cd_obb_t *obb1,
                                const fer_cd_obb_t *obb2,
                                void *data)
{
    struct __collide_t *c = (struct __collide_t *)data;

    if (ferCDTriMeshTriCollide((fer_cd_trimesh_tri_t *)obb1->shape,
                               &c->g1->rot, &c->g1->tr,
                               (fer_cd_trimesh_tri_t *)obb2->shape,
                               &c->g2->rot, &c->g2->tr)){
        c->ret = 1;
        return -1;
    }
    return 0;
}

int ferCDGeomCollide(const fer_cd_geom_t *g1, const fer_cd_geom_t *g2)
{
    struct __collide_t c;
    fer_list_t *item1, *item2;
    fer_cd_obb_t *obb1, *obb2;

    c.g1 = g1;
    c.g2 = g2;

    FER_LIST_FOR_EACH(&g1->obbs, item1){
        obb1 = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);

        FER_LIST_FOR_EACH(&g2->obbs, item2){
            obb2 = FER_LIST_ENTRY(item2, fer_cd_obb_t, list);

            c.ret = 0;
            ferCDOBBOverlapPairsCB(obb1, &g1->rot, &g1->tr,
                                   obb2, &g2->rot, &g2->tr,
                                   __ferCDGeomCollideCB, (void *)&c);
            if (c.ret)
                return 1;
        }
    }
    return 0;
}


void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        if (obb->shape->type == FER_CD_SHAPE_SPHERE){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_BOX){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CYL){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_CAP){
            // TODO

        }else if (obb->shape->type == FER_CD_SHAPE_TRIMESH_TRI){
            // TODO

        }else if (obb->shape && obb->shape->type == FER_CD_SHAPE_TRIMESH){
            ferCDTriMeshDumpSVT2((const fer_cd_trimesh_t *)obb->shape,
                                 &g->rot, &g->tr, out, name, 1);
        }
    }
}
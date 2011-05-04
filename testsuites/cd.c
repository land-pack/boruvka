#include "cu.h"
#include <fermat/cd.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include "bunny.h"
#include "data.h"

static void prv(const char *prefix, const fer_vec3_t *v)
{
    fprintf(stdout, prefix);
    ferVec3Print(v, stdout);
    fprintf(stdout, "\n");
}

/*
static void prCDOBBTri(fer_real_t x0, fer_real_t y0, fer_real_t z0,
                       fer_real_t x1, fer_real_t y1, fer_real_t z1,
                       fer_real_t x2, fer_real_t y2, fer_real_t z2)
{
    fer_vec3_t v[3];
    fer_cd_obb_t *obb;

    ferVec3Set(&v[0], x0, y0, z0);
    ferVec3Set(&v[1], x1, y1, z1);
    ferVec3Set(&v[2], x2, y2, z2);

    obb = ferCDOBBNewTri(&v[0], &v[1], &v[2]);

    prv("# tri.p[0]: ", &v[0]);
    prv("# tri.p[1]: ", &v[1]);
    prv("# tri.p[2]: ", &v[2]);

    prv("# obb.center: ", &obb->center);
    prv("# obb.axis[0]: ", &obb->axis[0]);
    prv("# obb.axis[1]: ", &obb->axis[1]);
    prv("# obb.axis[2]: ", &obb->axis[2]);
    prv("# obb.half_extents: ", &obb->half_extents);

    //ferCDOBBTriDumpSVT((fer_cd_obb_tri_t *)obb->pri, stdout, "Tri");
    //ferCDOBBDumpSVT(obb, stdout, "CDOBB");

    ferCDOBBDel(obb);
}
*/


TEST(cdOBBNew)
{
    fer_list_t obbs, *item;
    fer_cd_obb_t *obb;
    fer_cd_sphere_t *s[4];
    fer_cd_box_t *b[2];
    fer_cd_cyl_t *c[2];

    s[0] = ferCDSphereNew(0.5);
    s[1] = ferCDSphereNew(0.1);
    s[2] = ferCDSphereNew(0.3);
    s[3] = ferCDSphereNew(0.7);

    ferListInit(&obbs);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[2], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[3], 0);
    ferListAppend(&obbs, &obb->list);

    ferCDOBBMerge(&obbs, 0);

    item = ferListNext(&obbs);
    obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);

    //ferCDOBBDumpTreeSVT(obb, stdout, "spheres", NULL, NULL);

    ferCDOBBDel(obb);

    s[0] = ferCDSphereNew(0.5);
    s[1] = ferCDSphereNew(0.1);
    b[0] = ferCDBoxNew(0.1, 0.3, 0.2);
    b[1] = ferCDBoxNew(0.1, 0.2, 0.5);
    c[0] = ferCDCylNew(0.05, 0.3);
    c[1] = ferCDCylNew(0.1, 0.2);

    ferListInit(&obbs);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)b[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)b[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)c[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)c[1], 0);
    ferListAppend(&obbs, &obb->list);

    ferCDOBBMerge(&obbs, 0);

    item = ferListNext(&obbs);
    obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);

    //ferCDOBBDumpTreeSVT(obb, stdout, "s-b-c", NULL, NULL);

    ferCDOBBDel(obb);
}

TEST(cdOBBNew2)
{
    fer_vec3_t pts[5] = {
        FER_VEC3_STATIC(0., 0., 0.),
        FER_VEC3_STATIC(1., 0., 0.),
        FER_VEC3_STATIC(0., 0.3, 1.),
        FER_VEC3_STATIC(1., 1., 1.),
        FER_VEC3_STATIC(0., 1., 1.)
    };
    unsigned int ids[3 * 3] = { 0, 1, 2,
                                1, 2, 3,
                                2, 3, 4 };
    size_t len = 3;
    fer_list_t obbs, *item;
    fer_cd_obb_t *obb;
    fer_cd_sphere_t *s[4];
    fer_cd_box_t *b[2];
    fer_cd_cyl_t *c[2];
    fer_cd_trimesh_t *t[2];
    fer_mat3_t rot;
    fer_vec3_t tr;

    s[0] = ferCDSphereNew(0.5);
    s[1] = ferCDSphereNew(0.1);
    b[0] = ferCDBoxNew(0.1, 0.3, 0.2);
    b[1] = ferCDBoxNew(0.1, 0.2, 0.5);
    c[0] = ferCDCylNew(0.05, 0.3);
    c[1] = ferCDCylNew(0.1, 0.2);
    t[0] = ferCDTriMeshNew(pts, ids, len, NULL, fer_vec3_axis[0]);

    ferMat3SetRot3D(&rot, M_PI_4, -M_PI_4, 0);
    ferVec3Set(&tr, -0.5, 0.2, 0.1);
    t[1] = ferCDTriMeshNew(bunny_coords, bunny_ids, bunny_tri_len, &rot, &tr);

    ferListInit(&obbs);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)s[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)b[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)b[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)c[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)c[1], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)t[0], 0);
    ferListAppend(&obbs, &obb->list);

    obb = ferCDOBBNewShape((fer_cd_shape_t *)t[1], 0);
    ferListAppend(&obbs, &obb->list);

    ferCDOBBMerge(&obbs, 0);

    item = ferListNext(&obbs);
    obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);

    //ferCDOBBDumpTreeSVT(obb, stdout, "s-b-c-t", NULL, NULL);

    ferCDOBBDel(obb);
}

TEST(cdOBBNew3)
{
    fer_list_t obbs, *item;
    fer_cd_obb_t *obb;
    fer_cd_sphere_t *s[2];
    fer_cd_box_t *b[2];
    fer_cd_cyl_t *c[2];
    fer_cd_shape_off_t *off[6];
    fer_mat3_t rot;
    fer_vec3_t tr;
    size_t i;

    s[0] = ferCDSphereNew(0.2);
    s[1] = ferCDSphereNew(0.1);
    b[0] = ferCDBoxNew(0.1, 0.3, 0.2);
    b[1] = ferCDBoxNew(0.1, 0.2, 0.5);
    c[0] = ferCDCylNew(0.05, 0.3);
    c[1] = ferCDCylNew(0.1, 0.2);

    ferMat3SetRot3D(&rot, 0, 0, 0);
    ferVec3Set(&tr, 0, 0, 0);
    off[0] = ferCDShapeOffNew((fer_cd_shape_t *)s[0], &rot, &tr); 

    ferMat3SetRot3D(&rot, 0, 1, 0);
    ferVec3Set(&tr, 0, 0.5, 0);
    off[1] = ferCDShapeOffNew((fer_cd_shape_t *)s[1], &rot, &tr); 

    ferMat3SetRot3D(&rot, 0, M_PI_4, 0);
    ferVec3Set(&tr, 0.1, 0.1, 0.1);
    off[2] = ferCDShapeOffNew((fer_cd_shape_t *)b[0], &rot, &tr); 

    ferMat3SetRot3D(&rot, -M_PI_4, M_PI_4, 0);
    ferVec3Set(&tr, -0.1, 0.2, -0.1);
    off[3] = ferCDShapeOffNew((fer_cd_shape_t *)b[1], &rot, &tr); 

    ferMat3SetRot3D(&rot, 0, -M_PI_4, M_PI_4);
    ferVec3Set(&tr, -0.1, -0.1, -0.1);
    off[4] = ferCDShapeOffNew((fer_cd_shape_t *)c[0], &rot, &tr); 

    ferMat3SetRot3D(&rot, M_PI_4, 0, 0);
    ferVec3Set(&tr, -0.2, 0.2, 0.2);
    off[5] = ferCDShapeOffNew((fer_cd_shape_t *)c[1], &rot, &tr); 

    ferListInit(&obbs);
    for (i = 0; i < 6; i++){
        obb = ferCDOBBNewShape((fer_cd_shape_t *)off[i], 0);
        ferListAppend(&obbs, &obb->list);
    }

    ferCDOBBMerge(&obbs, 0);

    item = ferListNext(&obbs);
    obb  = FER_LIST_ENTRY(item, fer_cd_obb_t, list);

    //ferCDOBBDumpTreeSVT(obb, stdout, "new3", NULL, NULL);

    ferCDOBBDel(obb);
}


TEST(cdOBBCollide)
{
    fer_vec3_t c1, c2, a11, a12, a13, a21, a22, a23, e1, e2;
    fer_vec3_t tr1, tr2;
    fer_mat3_t rot1, rot2;
    fer_cd_obb_t *obb1, *obb2;
    int res;

    ferVec3Set(&c1, 0., 0., 0.);
    ferVec3Set(&a11, 1., 0., 0.);
    ferVec3Set(&a12, 0., 1., 0.);
    ferVec3Set(&a13, 0., 0., 1.);
    ferVec3Set(&e1, 0.5, 0.5, 0.5);
    ferVec3Set(&c2, 2., 0., 0.);
    ferVec3Set(&a21, 1., 0., 0.);
    ferVec3Set(&a22, 0., 1., 0.);
    ferVec3Set(&a23, 0., 0., 1.);
    ferVec3Set(&e2, 0.5, 0.5, 0.5);

    obb1 = ferCDOBBNew();
    ferVec3Copy(&obb1->center, &c1);
    ferVec3Copy(&obb1->axis[0], &a11);
    ferVec3Copy(&obb1->axis[1], &a12);
    ferVec3Copy(&obb1->axis[2], &a13);
    ferVec3Copy(&obb1->half_extents, &e1);
    obb2 = ferCDOBBNew();
    ferVec3Copy(&obb2->center, &c2);
    ferVec3Copy(&obb2->axis[0], &a21);
    ferVec3Copy(&obb2->axis[1], &a22);
    ferVec3Copy(&obb2->axis[2], &a23);
    ferVec3Copy(&obb2->half_extents, &e2);

    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferCDOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertTrue(res);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, -1.2, 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferCDOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    ferVec3Set(&obb2->center, 1.01, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferCDOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertTrue(res);

    ferVec3Set(&obb2->center, 1.1, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., M_PI_4, 0.);
    res = ferCDOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    ferVec3Set(&obb2->center, 1.1, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., M_PI_4);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0, 0.);
    res = ferCDOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    //fprintf(stderr, "res: %d\n", res);
    //ferCDOBBDumpSVT(obb1, stdout, "CDOBB1C");
    //ferCDOBBDumpSVT(obb2, stdout, "CDOBB2C");

    ferCDOBBDel(obb1);
    ferCDOBBDel(obb2);
}


TEST(cdCollideTriMesh)
{
    fer_vec3_t pts1[6] = { FER_VEC3_STATIC(0., 0., 0.),
                           FER_VEC3_STATIC(0.5, 0., 0.),
                           FER_VEC3_STATIC(0.4, 0.7, .0),
                           FER_VEC3_STATIC(0.1, -0.1, 0.5),
                           FER_VEC3_STATIC(0.5, 0., 0.5),
                           FER_VEC3_STATIC(0.5, 0.5, 0.5),
    };
    unsigned int ids1[8 * 3] = { 0, 1, 2,
                                 0, 1, 3,
                                 1, 3, 4,
                                 1, 2, 4,
                                 2, 4, 5,
                                 0, 3, 5,
                                 0, 5, 2,
                                 3, 4, 5 };
    size_t len1 = 8;
    fer_vec3_t pts2[5] = { FER_VEC3_STATIC(0., 0., 0.),
                           FER_VEC3_STATIC(0.5, 0.1, 0.),
                           FER_VEC3_STATIC(0.3, 0.5, .0),
                           FER_VEC3_STATIC(-0.1, 0.6, 0.),
                           FER_VEC3_STATIC(0.2, 0.2, 0.5)
    };
    unsigned int ids2[6 * 3] = { 0, 1, 4,
                                 1, 2, 4,
                                 2, 3, 4,
                                 3, 0, 4,
                                 0, 1, 3,
                                 1, 3, 2 };
    size_t len2 = 6;

    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_vec3_t tr1, tr2;
    fer_mat3_t rot1, rot2;
    int ret;


    cd = ferCDNew();

    g1 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g1, pts1, ids1, len1);
    g2 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g2, pts2, ids2, len2);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, .45, 0.1, 0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertTrue(ret);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0.6, 0.1, 0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertFalse(ret);


    ferVec3Set(&tr1, 0., M_PI_4, -M_PI_4);
    ferMat3SetRot3D(&rot1, 0., 0., -1.);
    ferVec3Set(&tr2, 0.6, 0.1, -0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertFalse(ret);


    ferVec3Set(&tr1, 0., M_PI_4, -M_PI_4);
    ferMat3SetRot3D(&rot1, 0., 0., -1.);
    ferVec3Set(&tr2, 0., 0.1, -0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertTrue(ret);

    /*
    //ferCDGeomDumpSVT(g1, stdout, "g1");
    //ferCDGeomDumpSVT(g2, stdout, "g2");
    ferCDGeomDumpOBBSVT(g1, stdout, "g1");
    ferCDGeomDumpOBBSVT(g2, stdout, "g2");
    DBG("ret: %d", ret);
    */


    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);
}

TEST(cdCollideTriMesh2)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_vec3_t tr1, tr2;
    fer_mat3_t rot1, rot2;
    int ret;


    cd = ferCDNew();

    g1 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g1, bunny_coords, bunny_ids, bunny_tri_len);
    g2 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g2, bunny_coords, bunny_ids, bunny_tri_len);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, .45, 0.1, 0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertTrue(ret);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 1.6, 0.1, 0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertFalse(ret);


    ferVec3Set(&tr1, 0., M_PI_4, -M_PI_4);
    ferMat3SetRot3D(&rot1, 0., 0., -1.);
    ferVec3Set(&tr2, 0.6, 1.8, 0.2);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertFalse(ret);


    ferVec3Set(&tr1, 0., M_PI_4, -M_PI_4);
    ferMat3SetRot3D(&rot1, 0., 0., -1.);
    ferVec3Set(&tr2, 0.6, 1.8, 0.1);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ret = ferCDGeomCollide(cd, g1, g2);
    assertTrue(ret);

    /*
    //ferCDGeomDumpSVT(g1, stdout, "g1");
    //ferCDGeomDumpSVT(g2, stdout, "g2");
    ferCDGeomDumpOBBSVT(g1, stdout, "g1");
    ferCDGeomDumpOBBSVT(g2, stdout, "g2");
    DBG("ret: %d", ret);
    */

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);
}

TEST(cdCollideSphere)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_vec3_t tr1, tr2;
    fer_mat3_t rot1, rot2;
    int ret;
    size_t i;
    fer_timer_t timer;


    cd = ferCDNew();

    g1 = ferCDGeomNew(cd);

    ferTimerStart(&timer);
    for (i = 0; i < protein_big_len; i++){
        ferCDGeomAddSphere2(cd, g1, protein_big_radius[i],
                                    &protein_big_center[i]);
    }
    ferTimerStop(&timer);
    DBG("addSphere1: %lu", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    ferCDGeomBuild(cd, g1);
    ferTimerStop(&timer);
    DBG("build1: %lu", ferTimerElapsedInUs(&timer));

    g2 = ferCDGeomNew(cd);
    for (i = 0; i < protein_small_len; i++){
        ferCDGeomAddSphere2(cd, g2, protein_small_radius[i],
                                    &protein_small_center[i]);
    }
    ferCDGeomBuild(cd, g2);

    //ferCDGeomDumpOBBSVT(g1, stdout, "g1");
    //ferCDGeomDumpOBBSVT(g2, stdout, "g2");

    ferVec3Set(&tr1, 0., M_PI_4, -M_PI_4);
    ferMat3SetRot3D(&rot1, 0., 0., -1.);
    ferVec3Set(&tr2, 35.6, 1.8, 0.1);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ferTimerStart(&timer);
    ret = ferCDGeomCollide(cd, g1, g2);
    ferTimerStop(&timer);
    DBG("collide1: %lu", ferTimerElapsedInUs(&timer));
    assertFalse(ret);

    ferVec3Set(&tr1, M_PI_4, 0, 0);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0.6, 11.2, 0.1);
    ferMat3SetRot3D(&rot2, M_PI_2, M_PI_4, M_PI_4);
    ferCDGeomSetRot(cd, g1, &rot1);
    ferCDGeomSetTr(cd, g1, &tr1);
    ferCDGeomSetRot(cd, g2, &rot2);
    ferCDGeomSetTr(cd, g2, &tr2);
    ferTimerStart(&timer);
    ret = ferCDGeomCollide(cd, g1, g2);
    ferTimerStop(&timer);
    DBG("collide2: %lu", ferTimerElapsedInUs(&timer));
    assertTrue(ret);

    //ferCDGeomDumpSVT(g1, stdout, "g1");
    //ferCDGeomDumpSVT(g2, stdout, "g2");
    ferCDGeomDumpSVT(g1, stdout, "g1");
    ferCDGeomDumpSVT(g2, stdout, "g2");
    DBG("ret: %d", ret);

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);
}
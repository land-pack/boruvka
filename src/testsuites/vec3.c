#include <stdio.h>
#include <cu/cu.h>
#include <mg/vec3.h>
#include <mg/dbg.h>
#include "data.h"

static void projToPlanePrint(mg_vec3_t *vs, int num);

TEST(vec3SetUp)
{
}

TEST(vec3TearDown)
{
}

TEST(vec3Alloc)
{
    mg_vec3_t *v, w;

    v = mgVec3New(0., 1., 2.);
    mgVec3Set(&w, 0., 1., 2.);
    assertTrue(mgVec3Eq(v, &w));
    mgVec3Del(v);
}

TEST(vec3Add)
{
    mg_vec3_t v;
    size_t i;

    printf("# ---- add ----\n");
    mgVec3Set(&v, MG_ZERO, MG_ZERO, MG_ZERO);
    for (i = 0; i < vecs_len; i++){
        mgVec3Add(&v, &vecs[i]);
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec3Sub)
{
    mg_vec3_t v;
    size_t i;

    printf("# ---- sub ----\n");
    mgVec3Set(&v, MG_ZERO, MG_ZERO, MG_ZERO);
    for (i = 0; i < vecs_len; i++){
        mgVec3Sub(&v, &vecs[i]);
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec3Scale)
{
    mg_vec3_t v;
    size_t i;

    printf("# ---- scale ----\n");
    mgVec3Copy(&v, &vecs[0]);
    for (i = 0; i < vecs_len; i++){
        mgVec3Scale(&v, mgVec3X(&vecs[i]));
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec3Normalize)
{
    mg_vec3_t v;
    size_t i;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs_len; i++){
        mgVec3Copy(&v, &vecs[i]);
        mgVec3Normalize(&v);
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec3Dot)
{
    mg_real_t dot;
    size_t i;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        dot = mgVec3Dot(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec3Cross)
{
    mg_vec3_t v;
    size_t i;

    printf("# ---- cross ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        mgVec3Cross(&v, &vecs[i], &vecs[i + 1]);
        printf("# %g %g %g\n", mgVec3X(&v), mgVec3Y(&v), mgVec3Z(&v));
    }
    printf("# ---- cross end ----\n\n");
}

TEST(vec3Len2)
{
    mg_real_t d;
    size_t i;

    printf("# ---- len2 ----\n");
    for (i = 0; i < vecs_len; i++){
        d = mgVec3Len2(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len2 end ----\n\n");
}

TEST(vec3Len)
{
    mg_real_t d;
    size_t i;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs_len; i++){
        d = mgVec3Len(&vecs[i]);
        printf("# %g\n", d);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec3Dist2)
{
    mg_real_t d;
    size_t i;

    printf("# ---- dist2 ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = mgVec3Dist2(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist2 end ----\n\n");
}

TEST(vec3Dist)
{
    mg_real_t d;
    size_t i;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs_len - 1; i++){
        d = mgVec3Dist(&vecs[i], &vecs[i + 1]);
        printf("# %g\n", d);
    }
    printf("# ---- dist end ----\n\n");
}

TEST(vec3SegmentDist)
{
    mg_real_t d;
    mg_vec3_t w;
    size_t i;

    printf("# ---- segment dist ----\n");
    for (i = 0; i < vecs_len - 2; i++){
        d = mgVec3PointSegmentDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &w);
        printf("# %g %g %g %g\n", d, mgVec3X(&w), mgVec3Y(&w), mgVec3Z(&w));
    }
    printf("# ---- segment dist end ----\n\n");
}

TEST(vec3TriDist)
{
    mg_real_t d;
    mg_vec3_t w;
    size_t i;

    printf("# ---- tri dist ----\n");
    for (i = 0; i < vecs_len - 3; i++){
        d = mgVec3PointTriDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &vecs[i + 3], &w);
        printf("# %g %g %g %g\n", d, mgVec3X(&w), mgVec3Y(&w), mgVec3Z(&w));
    }
    printf("# ---- tri dist end ----\n\n");
}


TEST(vec3Core)
{
    mg_vec3_t a, b, c, d;

    mgVec3Set(&a, 0., 0., 0.);
    assertTrue(mgVec3Eq(&a, mg_vec3_origin));

    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(mgVec3Eq(&a, &b));

    mgVec3Set(&a, 9., 1., 5.);
    mgVec3Copy(&b, &a);
    assertTrue(mgVec3Eq(&a, &b));

    mgVec3Set(&a, 0., 0., 0.);
    assertTrue(mgEq(0., mgVec3Len2(&a)));
    mgVec3Set(&a, 1., 1., 1.);
    assertTrue(mgEq(3., mgVec3Len2(&a)));
    mgVec3Set(&a, 1., 4., 3.);
    assertTrue(mgEq(1. + 4. * 4. + 3. * 3., mgVec3Len2(&a)));

    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(mgEq(0., mgVec3Dist2(&a, &b)));
    mgVec3Set(&a, 0., 1., .4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(mgEq(10.04, mgVec3Dist2(&a, &b)));
    /*
    DBG("%.30g", MG_EPS);
    DBG("%.30g", 10.04);
    DBG("%.30g", mgVec3Dist2(&a, &b));
    DBG_VEC3(&a, "a: ");
    DBG_VEC3(&b, "b: ");
    */

    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    mgVec3Sub2(&c, &a, &b);
    assertTrue(mgVec3Eq(&c, mg_vec3_origin));
    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    mgVec3Sub(&a, &b);
    assertTrue(mgVec3Eq(&a, mg_vec3_origin));
    mgVec3Set(&a, 1.3, 1.2, 3.4);
    mgVec3Set(&b, 0.2,  .2,  .4);
    mgVec3Sub2(&c, &a, &b);
    mgVec3Set(&d, 1.1, 1., 3.);
    assertTrue(mgVec3Eq(&c, &d));
    mgVec3Sub(&b, &a);
    mgVec3Set(&d, -1.1, -1., -3.);
    assertTrue(mgVec3Eq(&b, &d));

    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Set(&b, 1., 1.2, 3.4);
    mgVec3Add(&a, &b);
    mgVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(mgVec3Eq(&a, &d));
    mgVec3Set(&a, 1.3, 1.2, 3.4);
    mgVec3Set(&b, 0.2,  .2,  .4);
    mgVec3Add(&a, &b);
    mgVec3Set(&d, 1.5, 1.4, 3.8);
    assertTrue(mgVec3Eq(&a, &d));

    mgVec3Set(&a, 1., 1.2, 3.4);
    mgVec3Scale(&a, 2.);
    mgVec3Set(&d, 2., 2.4, 6.8);
    assertTrue(mgVec3Eq(&a, &d));
    mgVec3Set(&a, 1.3, 1.2, 3.4);
    mgVec3Scale(&a, 0.2);
    mgVec3Set(&d, 1.3 * .2, 1.2 * .2, 3.4 * .2);
    assertTrue(mgVec3Eq(&a, &d));

    mgVec3Set(&a, 2., 1.1, 5.4);
    mgVec3Normalize(&a);
    assertTrue(mgEq(1., mgVec3Len2(&a)));
    DBG("%.30g", MG_EPS);
    DBG("%.30g", 1.);
    DBG("%.30g", mgVec3Len2(&a));
    DBG_VEC3(&a, "a: ");
    mgVec3Set(&a, 1., .1, 3.4);
    mgVec3Normalize(&a);
    assertTrue(mgEq(1., mgVec3Len2(&a)));

    mgVec3Set(&a, 2., 1.1, 5.4);
    assertTrue(mgEq(mgVec3Len2(&a), mgVec3Dot(&a, &a)));
    mgVec3Set(&b, 1., 1.2, 3.4);
    assertTrue(mgEq(2. + 1.1 * 1.2 + 5.4 * 3.4, mgVec3Dot(&a, &b)));
    mgVec3Set(&a, 2., 2.4, 6.8);
    assertTrue(mgEq(2. + 2.4 * 1.2 + 6.8 * 3.4, mgVec3Dot(&a, &b)));

    mgVec3Set(&a, 1., 0., 0.);
    mgVec3Set(&b, 0., 1., 0.);
    mgVec3Cross(&c, &a, &b);
    mgVec3Set(&d, 0., 0., 1.);
    assertTrue(mgVec3Eq(&c, &d));
    mgVec3Cross(&c, &b, &a);
    mgVec3Set(&d, 0., 0., -1.);
    assertTrue(mgVec3Eq(&c, &d));
    mgVec3Set(&a, 1., 1., 1.);
    mgVec3Set(&b, 0., 1., 0.);
    mgVec3Cross(&c, &a, &b);
    mgVec3Set(&d, -1., 0., 1.);
    assertTrue(mgVec3Eq(&c, &d));
    mgVec3Cross(&c, &b, &a);
    mgVec3Set(&d, 1., 0., -1.);
    assertTrue(mgVec3Eq(&c, &d));
}

TEST(vec3PointSegmentDist)
{
    mg_vec3_t P, a, b, w, ew;
    mg_real_t dist;

    mgVec3Set(&a, 0., 0., 0.);
    mgVec3Set(&b, 1., 0., 0.);

    // extereme w == a
    mgVec3Set(&P, -1., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.1, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, .1 * .1));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -1., 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.5, 0.5, 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -0.1, -1., 2.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 5.01));
    assertTrue(mgVec3Eq(&w, &a));


    // extereme w == b
    mgVec3Set(&P, 2., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.1, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, .1 * .1));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 2., 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.5, 0.5, 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 1.1, -1., 2.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 5.01));
    assertTrue(mgVec3Eq(&w, &b));

    // inside segment
    mgVec3Set(&P, .5, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, .9, 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, .5, 1., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 1.));
    mgVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(mgVec3Eq(&w, &ew));

    mgVec3Set(&P, .5, 1., 1.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&ew, 0.5, 0., 0.);
    assertTrue(mgVec3Eq(&w, &ew));



    mgVec3Set(&a, -.5, 2., 1.);
    mgVec3Set(&b, 1., 1.5, 0.5);

    // extereme w == a
    mgVec3Set(&P, -10., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9.5 * 9.5 + 2. * 2. + 1.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, -10., 9.2, 3.4);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9.5 * 9.5 + 7.2 * 7.2 + 2.4 * 2.4));
    assertTrue(mgVec3Eq(&w, &a));

    // extereme w == b
    mgVec3Set(&P, 10., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9. * 9. + 1.5 * 1.5 + 0.5 * 0.5));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, 10., 9.2, 3.4);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 9. * 9. + 7.7 * 7.7 + 2.9 * 2.9));
    assertTrue(mgVec3Eq(&w, &b));

    // inside ab
    mgVec3Set(&a, -.1, 1., 1.);
    mgVec3Set(&b, 1., 1., 1.);
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointSegmentDist2(&P, &a, &b, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&ew, 0., 1., 1.);
    assertTrue(mgVec3Eq(&w, &ew));
}


TEST(vec3PointTriDist)
{
    mg_vec3_t P, a, b, c, w, P0;
    mg_real_t dist;

    mgVec3Set(&a, -1., 0., 0.);
    mgVec3Set(&b, 0., 1., 1.);
    mgVec3Set(&c, -1., 0., 1.);

    mgVec3Set(&P, -1., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &a));

    mgVec3Set(&P, 0., 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &b));

    mgVec3Set(&P, -1., 0., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &c));

    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, NULL);
    assertTrue(mgEq(dist, 2./3.));


    // region 4
    mgVec3Set(&P, -2., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &a)));
    assertTrue(mgVec3Eq(&w, &a));
    mgVec3Set(&P, -2., 0.2, -1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &a)));
    assertTrue(mgVec3Eq(&w, &a));

    // region 2
    mgVec3Set(&P, -1.3, 0., 1.2);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &c)));
    assertTrue(mgVec3Eq(&w, &c));
    mgVec3Set(&P, -1.2, 0.2, 1.1);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &c)));
    assertTrue(mgVec3Eq(&w, &c));

    // region 6
    mgVec3Set(&P, 0.3, 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &b)));
    assertTrue(mgVec3Eq(&w, &b));
    mgVec3Set(&P, .1, 1., 1.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, mgVec3Dist2(&P, &b)));
    assertTrue(mgVec3Eq(&w, &b));

    // region 1
    mgVec3Set(&P, 0., 1., 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &b));
    mgVec3Set(&P, -1., 0., 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    assertTrue(mgVec3Eq(&w, &c));
    mgVec3Set(&P, -0.5, 0.5, 2.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 1.));
    mgVec3Set(&P0, -0.5, 0.5, 1.);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 3
    mgVec3Set(&P, -2., -1., 0.7);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 2.));
    mgVec3Set(&P0, -1., 0., 0.7);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 5
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 2./3.));
    mgVec3Set(&P0, -2./3., 1./3., 1./3.);
    assertTrue(mgVec3Eq(&w, &P0));

    // region 0
    mgVec3Set(&P, -0.5, 0.5, 0.5);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));
    mgVec3Set(&P, -0.5, 0.5, 0.7);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));
    mgVec3Set(&P, -0.5, 0.5, 0.9);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.));
    assertTrue(mgVec3Eq(&w, &P));

    mgVec3Set(&P, 0., 0., 0.5);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.5));
    mgVec3Set(&P0, -.5, .5, .5);
    assertTrue(mgVec3Eq(&w, &P0));

    mgVec3Set(&a, -1., 0., 0.);
    mgVec3Set(&b, 0., 1., -1.);
    mgVec3Set(&c, 0., 1., 1.);
    mgVec3Set(&P, 0., 0., 0.);
    dist = mgVec3PointTriDist2(&P, &a, &b, &c, &w);
    assertTrue(mgEq(dist, 0.5));
    mgVec3Set(&P0, -.5, .5, 0.);
    assertTrue(mgVec3Eq(&w, &P0));
    //fprintf(stderr, "dist: %lf\n", dist);
}

TEST(vec3PointInTri)
{
    mg_vec3_t v[5];

    mgVec3Set(&v[0], 1., 1., 0.);
    mgVec3Set(&v[1], 0., 0., 0.);
    mgVec3Set(&v[2], 2., 0.5, 0.);
    mgVec3Set(&v[3], 0., 2., 0.);
    assertTrue(mgVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    mgVec3Set(&v[0], 1., 1., 0.);
    mgVec3Set(&v[1], 0., 0., 0.);
    mgVec3Set(&v[2], 2., -1., 0.);
    mgVec3Set(&v[3], 0., 2., 0.);
    assertFalse(mgVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    mgVec3Set(&v[0], 1., 1., 0.);
    mgVec3Set(&v[1], 0., 0., 0.);
    mgVec3Set(&v[2], 0., 0., 0.);
    mgVec3Set(&v[3], 0., 0., 0.);
    assertFalse(mgVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    mgVec3Set(&v[0], 1., 1., 0.);
    mgVec3Set(&v[1], 1., 0., 0.);
    mgVec3Set(&v[2], 0., 0., 0.);
    mgVec3Set(&v[3], -1., 0., 0.);
    assertFalse(mgVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    mgVec3Set(&v[4], 0., 0., 0.);
    mgVec3Set(&v[1], -1., 1., -1.);
    mgVec3Set(&v[2], 1., 1., -1.);
    mgVec3Set(&v[3], 0., 1., 2.);
    mgVec3ProjToPlane(&v[4], &v[1], &v[2], &v[3], &v[0]);
    assertTrue(mgVec3PointInTri(&v[0], &v[1], &v[2], &v[3]));

    mgVec3Set(&v[1], 0., 0., 0.);
    mgVec3Set(&v[2], -1., 1., -1.);
    mgVec3Set(&v[3], 1., 0.8, -1.);
    mgVec3Set(&v[4], 0., 1.3, 2.2);
    mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]);
    assertTrue(mgVec3PointInTri(&v[0], &v[2], &v[3], &v[4]));
}

static void projToPlanePrint(mg_vec3_t *vs, int num)
{
    size_t i;

    printf("------\n");
    printf("Point color: 0.8 0.8 0.8\n");
    printf("Face color: 0 1 0\n");
    printf("Name: Proj %d - Plane\n", num);
    printf("Points:\n");
    for (i=0; i < 3; i++)
        printf("%g %g %g\n", mgVec3X(&vs[i + 2]), mgVec3Y(&vs[i + 2]), mgVec3Z(&vs[i + 2]));
    printf("Faces: 0 1 2\n");

    printf("------\n");
    printf("Point color: 1 0 0 \n");
    printf("Name: Proj %d - OPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", mgVec3X(&vs[1]), mgVec3Y(&vs[1]), mgVec3Z(&vs[1]));

    printf("------\n");
    printf("Point color: 0 0 1 \n");
    printf("Name: Proj %d - PPoint\n", num);
    printf("Points:\n");
    printf("%g %g %g\n", mgVec3X(&vs[0]), mgVec3Y(&vs[0]), mgVec3Z(&vs[0]));
}

TEST(vec3ProjToPlane)
{
    mg_vec3_t v[6];

    mgVec3Set(&v[1], 0., 0., 0.);
    mgVec3Set(&v[2], 0., 0., 0.);
    mgVec3Set(&v[3], 1., 0., 0.);
    mgVec3Set(&v[4], 0., 1., 0.);
    mgVec3Set(&v[5], 0., 0., 0.);
    assertTrue(mgEq(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]), MG_ZERO));
    assertTrue(mgVec3Eq(&v[0], &v[5]));

    mgVec3Set(&v[1], 0., 0., 1.);
    mgVec3Set(&v[2], 0., 0., 0.);
    mgVec3Set(&v[3], 1., 0., 0.);
    mgVec3Set(&v[4], 0., 1., 0.);
    mgVec3Set(&v[5], 0., 0., 0.);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    assertTrue(mgVec3Eq(&v[0], &v[5]));

    mgVec3Set(&v[1], 0., 0., -1.);
    mgVec3Set(&v[2], 0., 0., 0.);
    mgVec3Set(&v[3], 1., 0., 0.);
    mgVec3Set(&v[4], 0., 1., 0.);
    mgVec3Set(&v[5], 0., 0., 0.);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    assertTrue(mgVec3Eq(&v[0], &v[5]));

    mgVec3Set(&v[1], 0., 0., 1.);
    mgVec3Set(&v[2], -1., -1., -1.);
    mgVec3Set(&v[3], 1., -1., -1.);
    mgVec3Set(&v[4], -1., 1., -1.);
    mgVec3Set(&v[5], 0., 0., -1.);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    assertTrue(mgVec3Eq(&v[0], &v[5]));

    mgVec3Set(&v[1], 0.5, 10., 0.5);
    mgVec3Set(&v[2], 1., 0., 0.);
    mgVec3Set(&v[3], -1., 0., 1.);
    mgVec3Set(&v[4], -1., 0., -1.);
    mgVec3Set(&v[5], 0.5, 0., 0.5);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    assertTrue(mgVec3Eq(&v[0], &v[5]));
    fprintf(stderr, "--\n");
    assertTrue(mgVec3Eq(&v[0], &v[5]));
    fprintf(stderr, "%.30lf %.30lf %.30lf\n", MG_EPS, MG_EPS, MG_EPS);
    fprintf(stderr, "%.30lf %.30lf %.30lf\n",
            mgVec3X(&v[0]), mgVec3Y(&v[0]), mgVec3Z(&v[0]));
    fprintf(stderr, "%.30lf %.30lf %.30lf\n",
            mgVec3X(&v[5]), mgVec3Y(&v[5]), mgVec3Z(&v[5]));

    mgVec3Set(&v[1], 0.5, 10., 0.5);
    mgVec3Set(&v[2], -1., -1., -1.);
    mgVec3Set(&v[3], 1., 1., 1.);
    mgVec3Set(&v[4], 0., 0., 0.);
    assertFalse(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);

    mgVec3Set(&v[1], 0., 0., 1.);
    mgVec3Set(&v[2], -1., -1., -1.);
    mgVec3Set(&v[3], 1., 1., 1.);
    mgVec3Set(&v[4], -1., 1., 1.);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    projToPlanePrint(v, 1);

    mgVec3Set(&v[1], 3., -2., 1.);
    mgVec3Set(&v[2], -1., -2., -1.);
    mgVec3Set(&v[3], 1., 2., 1.8);
    mgVec3Set(&v[4], -1.4, 1., 1.2);
    assertTrue(mgVec3ProjToPlane(&v[1], &v[2], &v[3], &v[4], &v[0]) > MG_ZERO);
    projToPlanePrint(v, 2);
}


TEST(vec3Centroid)
{
    mg_vec3_t v[5];

    mgVec3Set(&v[0], 0., 0., 0.);
    mgVec3Set(&v[1], 1., 0., 0.);
    mgVec3Set(&v[2], 0., 1., 0.);
    mgVec3TriCentroid(&v[0], &v[1], &v[2], &v[3]);

    mgVec3Set(&v[4], 1./3., 1./3., 0.);
    assertTrue(mgVec3Eq(&v[3], &v[4]));
}

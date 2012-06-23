#include <stdio.h>
#include "cu.h"

#include <boruvka/vec2.h>
#include <boruvka/mat3.h>
#include "data.h"

static bor_vec2_t *v[4];
static bor_vec2_t *t, *p;

TEST(vec2SetUp)
{
    int i;

    t = borVec2New(0., 0.);
    p = borVec2New(0., 0.);


    for (i=0; i < 4; i++)
        v[i] = borVec2New(0., 0.);
}

TEST(vec2TearDown)
{
    int i;

    borVec2Del(t);
    borVec2Del(p);
    for (i=0; i < 4; i++)
        borVec2Del(v[i]);
}

TEST(vec2Init)
{
    borVec2Set(v[0], 1., 2.);
    borVec2Set(v[1], 2., 3.);
    borVec2Sub2(v[2], v[1], v[0]);
    borVec2Sub2(v[3], v[1], v[0]);
    borVec2Scale(v[3], 2.);

    borVec2Set(t, 1., 2.);
    assertTrue(borVec2Eq(v[0], t));

    borVec2Set(t, 2., 3.);
    assertTrue(borVec2Eq(v[1], t));

    borVec2Set(t, 1., 1.);
    assertTrue(borVec2Eq(v[2], t));

    borVec2Set(t, 2., 2.);
    assertTrue(borVec2Eq(v[3], t));


    borVec2Copy(v[0], v[1]);
    borVec2Set(t, 2., 3.);
    assertTrue(borVec2Eq(v[0], t));
    assertTrue(borVec2Eq(v[0], v[1]));

    borVec2Sub2(v[0], v[1], v[2]);
    borVec2Set(t, 1., 2.);
    assertTrue(borVec2Eq(v[0], t));
}

TEST(vec2Clone)
{
    bor_vec2_t *vc = borVec2Clone(v[0]);
    assertTrue(borVec2Eq(vc, v[0]));
    borVec2Del(vc);
}


TEST(vec2Operators)
{
    borVec2Set(v[0], 1., 2.);
    borVec2Set(v[1], 1., 2.);
    borVec2Set(v[2], 2., 4.);
    borVec2Set(v[3], 0., 0.);

    assertTrue(borVec2Eq(v[0], v[1]));
    assertTrue(borVec2NEq(v[0], v[2]));

    borVec2Add(v[0], v[1]);
    assertTrue(borVec2Eq(v[2], v[0]));

    borVec2Sub(v[0], v[2]);
    assertTrue(borVec2Eq(v[0], v[3]));
}

TEST(vec2Length)
{
    borVec2Set(v[0], 10., 0.);
    borVec2Set(v[1], 6., 4.);
    borVec2Set(v[2], 1., 1.);

    assertTrue(borEq(BOR_SQRT(borVec2Len2(v[0])), 10.));
    assertTrue(borEq(borVec2Len2(v[1]), BOR_REAL(52.)));
    assertTrue(borEq(borVec2Dist2(v[1], v[2]), BOR_REAL(34.)));

    assertTrue(borEq(borVec2Dist2(v[1], v[2]), borVec2Dist2(v[2], v[1])));

    assertTrue(borVec2Eq(v[0], v[0]));
}

TEST(vec2AngleArea)
{
    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 1., 1.);
    borVec2Set(v[2], 0., 1.);

    assertTrue(borEq(borVec2Angle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(borEq(borVec2Angle(v[2], v[0], v[1]), M_PI_4));
    assertTrue(borEq(borVec2Angle(v[0], v[1], v[2]), M_PI_4));
    assertTrue(borEq(borVec2Angle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(borEq(borVec2Angle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(borEq(borVec2Angle(v[1], v[2], v[0]), M_PI_2));

    assertTrue(borEq(borVec2SignedAngle(v[1], v[0], v[2]), M_PI_4));
    assertTrue(borEq(borVec2SignedAngle(v[2], v[0], v[1]), -1. * M_PI_4));
    assertTrue(borEq(borVec2SignedAngle(v[0], v[1], v[2]), -1. * M_PI_4));
    assertTrue(borEq(borVec2SignedAngle(v[2], v[1], v[0]), M_PI_4));
    assertTrue(borEq(borVec2SignedAngle(v[0], v[2], v[1]), M_PI_2));
    assertTrue(borEq(borVec2SignedAngle(v[1], v[2], v[0]), -1. * M_PI_2));


    assertTrue(borEq(borVec2Area2(v[0], v[1], v[2]), 1));
    assertTrue(borEq(borVec2Area2(v[1], v[2], v[0]), 1));
    assertTrue(borEq(borVec2Area2(v[2], v[0], v[1]), 1));

    assertTrue(borEq(borVec2Area2(v[2], v[1], v[0]), -1));
    assertTrue(borEq(borVec2Area2(v[1], v[0], v[2]), -1));
    assertTrue(borEq(borVec2Area2(v[0], v[2], v[1]), -1));
}

TEST(vec2Intersect)
{
    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 6., 0.);
    borVec2Set(v[2], 3., -3.);
    borVec2Set(v[3], 3., 3.);

    assertEquals(borVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    borVec2Set(t, 3., 0.);
    assertTrue(borVec2Eq(p, t));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 6., 0.);
    borVec2Set(v[2], 10., 10.);
    borVec2Set(v[3], 3., 3.);
    assertNotEquals(borVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 3., 3.);
    borVec2Set(v[2], -1., 0.);
    borVec2Set(v[3], 2., 2.);
    assertEquals(borVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    borVec2Set(t, 2., 2.);
    assertTrue(borVec2Eq(p, t));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 3., 3.);
    borVec2Set(v[2], -1., 0.);
    borVec2Set(v[3], 4., 3.);
    assertEquals(borVec2IntersectPoint(v[0], v[1], v[2], v[3], p), 0);
    borVec2Set(t, 1.5, 1.5);
    assertTrue(borVec2Eq(t, p));
}

TEST(vec2SegmentInRect)
{
    bor_vec2_t *s[2], *l[2];

    s[0] = borVec2New(0., 0.);
    s[1] = borVec2New(0., 0.);
    l[0] = borVec2New(0., 0.);
    l[1] = borVec2New(0., 0.);

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 0.);
    borVec2Set(v[2], 2., 2.);
    borVec2Set(v[3], 0., 2.);

    borVec2Set(l[0], 1., 1.);
    borVec2Set(l[1], 3., 3.);
    assertEquals(borVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((borVec2Eq2(s[0], 1., 1.) && borVec2Eq2(s[1], 2., 2.))
            ||
        (borVec2Eq2(s[0], 2., 2.) && borVec2Eq2(s[1], 1., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    borVec2Set(l[0], -1., 1.);
    borVec2Set(l[1], 3., 1.);
    assertEquals(borVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((borVec2Eq2(s[0], 0., 1.) && borVec2Eq2(s[1], 2., 1.))
            ||
        (borVec2Eq2(s[0], 2., 1.) && borVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
        printf("[%f %f] - [%f %f]\n", borVec2X(s[0]), borVec2Y(s[1]),
                borVec2X(s[1]), borVec2Y(s[1]));
    }

    borVec2Set(l[0], -1., 1.);
    borVec2Set(l[1], 1., 1.);
    assertEquals(borVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((borVec2Eq2(s[0], 0., 1.) && borVec2Eq2(s[1], 1., 1.))
            ||
        (borVec2Eq2(s[0], 1., 1.) && borVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    borVec2Set(l[0], 0.5, 1.5);
    borVec2Set(l[1], 1.5, 0.5);
    assertEquals(borVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((borVec2Eq2(s[0], 0.5, 1.5) && borVec2Eq2(s[1], 1.5, 0.5))
            ||
        (borVec2Eq2(s[0], 1.5, 0.5) && borVec2Eq2(s[1], 0.5, 1.5))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    borVec2Set(l[0], 0., 1.);
    borVec2Set(l[1], 2., 1.);
    assertEquals(borVec2SegmentInRect(v[0], v[1], v[2], v[3], l[0], l[1], s[0], s[1]), 0);
    if ((borVec2Eq2(s[0], 0., 1.) && borVec2Eq2(s[1], 2., 1.))
            ||
        (borVec2Eq2(s[0], 2., 1.) && borVec2Eq2(s[1], 0., 1.))){
        assertTrue(1);
    }else{
        assertTrue(0);
    }

    borVec2Del(s[0]);
    borVec2Del(s[1]);
    borVec2Del(l[0]);
    borVec2Del(l[1]);
}

TEST(vec2Projection)
{
    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 0., 2.);
    assertEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(borVec2Eq2(p, 1., 1.));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 3., 3.);
    assertNotEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 3., 4.);
    assertNotEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 0., 0.);
    assertEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(borVec2Eq2(p, 0., 0.));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 2., 2.);
    assertEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(borVec2Eq2(p, 2., 2.));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 2., 2.);
    borVec2Set(v[2], 1., 1.);
    assertEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(borVec2Eq2(p, 1., 1.));

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], -2., -2.);
    borVec2Set(v[2], 0., -2.);
    assertEquals(borVec2ProjectionPointOntoSegment(v[0], v[1], v[2], p), 0);
    assertTrue(borVec2Eq2(p, -1., -1.));
}

TEST(vec2AngleSameDir)
{
    bor_real_t angle;
    bor_mat3_t *tr = borMat3New();
    bor_vec2_t w;

    borVec2Set(v[0], 1., 1.);
    borVec2Set(v[1], 2., 2.);
    angle = borVec2AngleSameDir(v[0], v[1]);
    assertTrue(borEq(angle, 0.));
    angle = borVec2AngleSameDir(v[1], v[0]);
    assertTrue(borEq(angle, 0.));

    borVec2Set(v[0], 0., 1.);
    borVec2Set(v[1], 1., 0.);
    angle = borVec2AngleSameDir(v[0], v[1]);
    assertTrue(borEq(angle, M_PI_2));
    angle = borVec2AngleSameDir(v[1], v[0]);
    assertTrue(borEq(angle, -1. * M_PI_2));


    // --- Oriented segments:
    printf("# vecAngleSameDir - Oriented segments\n");
    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 1., 1.);
    borVec2Set(v[2], 0., 0.);
    borVec2Set(v[3], 2., 2.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(borEq(angle, 0.));


    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 1., 1.);
    borVec2Set(v[2], 2., 0.);
    borVec2Set(v[3], 2., 2.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);
    assertTrue(borEq(angle, -1. * M_PI_4));

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[0]), borVec2Y(v[0]));
    printf("%f %f\n", borVec2X(v[1]), borVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");
    borMat3SetIdentity(tr);
    borMat3Rot(tr, angle);
    borVec2Copy(&w, v[2]);
    borMat3MulVec2(v[2], tr, &w);
    borVec2Copy(&w, v[3]);
    borMat3MulVec2(v[3], tr, &w);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");

    borVec2Set(v[0], 0., 0.);
    borVec2Set(v[1], 1., 1.);
    borVec2Set(v[2], 2., 0.);
    borVec2Set(v[3], 2., 2.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);
    assertTrue(borEq(angle, M_PI_4 + M_PI_2));


    borVec2Set(v[0], 2., 3.);
    borVec2Set(v[1], 3., 4.);
    borVec2Set(v[2], 1., 1.);
    borVec2Set(v[3], 3., 7.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[0]), borVec2Y(v[0]));
    printf("%f %f\n", borVec2X(v[1]), borVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");
    borMat3SetIdentity(tr);
    borMat3Rot(tr, angle);
    borVec2Copy(&w, v[2]);
    borMat3MulVec2(v[2], tr, &w);
    borVec2Copy(&w, v[3]);
    borMat3MulVec2(v[3], tr, &w);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");

    borVec2Sub2(t, v[1], v[0]);
    borVec2Scale(t, 1./BOR_SQRT(borVec2Len2(t)));
    borVec2Sub2(p, v[3], v[2]);
    borVec2Scale(p, 1./BOR_SQRT(borVec2Len2(p)));
    assertTrue(borVec2Eq(t, p));


    borVec2Set(v[0], 2., 3.);
    borVec2Set(v[1], 2., 4.);
    borVec2Set(v[2], 1., 1.);
    borVec2Set(v[3], 1., 7.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[3], v[2]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[0]), borVec2Y(v[0]));
    printf("%f %f\n", borVec2X(v[1]), borVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");
    borMat3SetIdentity(tr);
    borMat3Rot(tr, angle);
    borVec2Copy(&w, v[2]);
    borMat3MulVec2(v[2], tr, &w);
    borVec2Copy(&w, v[3]);
    borMat3MulVec2(v[3], tr, &w);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");

    borVec2Sub2(t, v[1], v[0]);
    borVec2Scale(t, 1./BOR_SQRT(borVec2Len2(t)));
    borVec2Sub2(p, v[2], v[3]);
    borVec2Scale(p, 1./BOR_SQRT(borVec2Len2(p)));
    assertTrue(borVec2Eq(t, p));


    borVec2Set(v[0], 1., 0.);
    borVec2Set(v[1], 1., -1.);
    borVec2Set(v[2], -3., 1.);
    borVec2Set(v[3], 2., 2.);
    angle = borVec2AngleSegsSameDir(v[0], v[1], v[2], v[3]);

    printf("Name: AB\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[0]), borVec2Y(v[0]));
    printf("%f %f\n", borVec2X(v[1]), borVec2Y(v[1]));
    printf("----\n");
    printf("Name: CD\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");
    borMat3SetIdentity(tr);
    borMat3Rot(tr, angle);
    borVec2Copy(&w, v[2]);
    borMat3MulVec2(v[2], tr, &w);
    borVec2Copy(&w, v[3]);
    borMat3MulVec2(v[3], tr, &w);
    printf("Name: CD rotated\n");
    printf("Poly:\n");
    printf("%f %f\n", borVec2X(v[2]), borVec2Y(v[2]));
    printf("%f %f\n", borVec2X(v[3]), borVec2Y(v[3]));
    printf("----\n");

    borVec2Sub2(t, v[0], v[1]);
    borVec2Scale(t, 1./BOR_SQRT(borVec2Len2(t)));
    borVec2Sub2(p, v[2], v[3]);
    borVec2Scale(p, 1./BOR_SQRT(borVec2Len2(p)));
    assertTrue(borVec2Eq(t, p));


    borMat3Del(tr);
}

TEST(vec2Add)
{
    size_t i;
    bor_vec2_t v;

    printf("# ---- add ----\n");
    borVec2Set(&v, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs2_len; i++){
        borVec2Add(&v, &vecs2[i]);
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- add end ----\n\n");
}

TEST(vec2Sub)
{
    size_t i;
    bor_vec2_t v;

    printf("# ---- sub ----\n");
    borVec2Set(&v, BOR_ZERO, BOR_ZERO);
    for (i = 0; i < vecs2_len; i++){
        borVec2Sub(&v, &vecs2[i]);
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- sub end ----\n\n");
}

TEST(vec2Scale)
{
    size_t i;
    bor_vec2_t v;

    printf("# ---- scale ----\n");
    borVec2Copy(&v, &vecs2[0]);
    for (i = 0; i < vecs2_len; i++){
        borVec2Scale(&v, borVec2X(&vecs2[i]));
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- scale end ----\n\n");
}

TEST(vec2Normalize)
{
    size_t i;
    bor_vec2_t v;

    printf("# ---- normalize ----\n");
    for (i = 0; i < vecs2_len; i++){
        borVec2Copy(&v, &vecs2[i]);
        borVec2Normalize(&v);
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
        borVec2ScaleToLen(&v, borVec2X(&vecs2[i]));
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- normalize end ----\n\n");
}

TEST(vec2Dot)
{
    size_t i;
    bor_real_t dot;

    printf("# ---- dot ----\n");
    for (i = 0; i < vecs2_len - 1; i++){
        dot = borVec2Dot(&vecs2[i], &vecs2[i + 1]);
        printf("# %g\n", dot);
    }
    printf("# ---- dot end ----\n\n");
}

TEST(vec2Mul)
{
    size_t i;
    bor_vec2_t v;

    printf("# ---- mul ----\n");
    for (i = 0; i < vecs2_len - 1; i++){
        borVec2MulComp2(&v, &vecs2[i], &vecs2[i + 1]);
        printf("# %g %g\n", borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- mul end ----\n\n");
}

TEST(vec2Len)
{
    size_t i;
    bor_real_t len, len2;

    printf("# ---- len ----\n");
    for (i = 0; i < vecs2_len; i++){
        len2 = borVec2Len2(&vecs2[i]);
        len = borVec2Len(&vecs2[i]);
        printf("# %g %g\n", len2, len);
    }
    printf("# ---- len end ----\n\n");
}

TEST(vec2Dist)
{
    size_t i;
    bor_real_t d, d2;

    printf("# ---- dist ----\n");
    for (i = 0; i < vecs2_len - 1; i++){
        d2 = borVec2Dist2(&vecs2[i], &vecs2[i + 1]);
        d = borVec2Dist(&vecs2[i], &vecs2[i + 1]);
        printf("# %g %g\n", d2, d);
    }
    printf("# ---- dist end ----\n\n");
}

TEST(vec2Area)
{
    size_t i;
    bor_real_t d;

    printf("# ---- area ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        d = borVec2Area2(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        printf("# %g\n", d);
    }
    printf("# ---- area end ----\n\n");
}

TEST(vec2Angle)
{
    size_t i;
    bor_real_t d, d2;

    printf("# ---- angle ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        d = borVec2Angle(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        d2 = borVec2SignedAngle(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        printf("# %g %g\n", d, d2);
    }
    printf("# ---- angle end ----\n\n");
}

TEST(vec2ProjSeg)
{
    size_t i;
    int ret;
    bor_vec2_t v;

    printf("# ---- proj seg ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        ret = borVec2ProjectionPointOntoSegment(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &v);
        printf("# %d %g %g\n", ret, borVec2X(&v), borVec2Y(&v));
    }
    printf("# ---- proj seg end ----\n\n");
}

TEST(vec2InCircle)
{
    size_t i;
    int ret;

    printf("# ---- in circle ----\n");
    for (i = 0; i < vecs2_len - 3; i++){
        ret = borVec2InCircle(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &vecs2[i + 3]);
        printf("# %d\n", ret);
    }
    printf("# ---- in circle end ----\n\n");
}

TEST(vec2LiesOn)
{
    // TODO
    /*
    size_t i;
    int ret;

    printf("# ---- lies on ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        ret = borVec2LiesOn(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        printf("# %d\n", ret);
    }
    printf("# ---- lies on end ----\n\n");
    */
}

TEST(vec2Collinear)
{
    // TODO
    /*
    size_t i;
    int ret;

    printf("# ---- collinear ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        ret = borVec2Collinear(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        printf("# %d\n", ret);
    }
    printf("# ---- collinear end ----\n\n");
    */
}

TEST(vec2InCone)
{
    size_t i;
    int ret;

    printf("# ---- in cone ----\n");
    for (i = 0; i < vecs2_len - 3; i++){
        ret = borVec2InCircle(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &vecs2[i + 3]);
        printf("# %d\n", ret);
    }
    printf("# ---- in cone end ----\n\n");
}

TEST(vec2Inter)
{
    size_t i;
    int ret, ret2, ret3;
    bor_vec2_t v;

    printf("# ---- inter ----\n");
    for (i = 0; i < vecs2_len - 3; i++){
        ret = borVec2IntersectProp(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &vecs2[i + 3]);
        ret2 = borVec2Intersect(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &vecs2[i + 3]);
        ret3 = borVec2IntersectPoint(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2], &vecs2[i + 3], &v);
        if (ret3 == 0){
            printf("# %d %d %d %g %g\n", ret, ret2, ret3, borVec2X(&v), borVec2Y(&v));
        }else{
            printf("# %d %d %d\n", ret, ret2, ret3);
        }
    }
    printf("# ---- inter end ----\n\n");
}

TEST(vec2OnLeft)
{
    size_t i;
    int ret;

    printf("# ---- on left ----\n");
    for (i = 0; i < vecs2_len - 2; i++){
        ret = borVec2OnLeft(&vecs2[i], &vecs2[i + 1], &vecs2[i + 2]);
        printf("# %d\n", ret);
    }
    printf("# ---- on left end ----\n\n");
}

TEST(vec2SegInRect)
{
    size_t i;
    int ret;
    bor_vec2_t s1, s2;

    printf("# ---- seg in rect ----\n");
    for (i = 0; i < vecs2_len - 5; i++){
        ret = borVec2SegmentInRect(&vecs2[i], &vecs2[i + 1],
                                   &vecs2[i + 2], &vecs2[i + 3],
                                   &vecs2[i + 4], &vecs2[i + 5],
                                   &s1, &s2);
        if (ret == 0){
            printf("# %d (%g %g) (%g %g)\n", ret,
                   borVec2X(&s1), borVec2Y(&s1),
                   borVec2X(&s2), borVec2Y(&s2));
        }else{
            printf("# %d\n", ret);
        }
    }
    printf("# ---- seg in rect end ----\n\n");
}


TEST(vec2BoxBoxOverlap)
{
    bor_vec2_t he[2], pos[2];
    bor_real_t rot[2];
    int ret;


    borVec2Set(&he[0], 1, 2);
    borVec2Set(&he[1], 2, 1);
    borVec2Set(&pos[0], 0, 0);
    borVec2Set(&pos[1], 0, 0);
    rot[0] = rot[1] = 0;

    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertTrue(ret);

    borVec2Set(&pos[1], 1.8, 0);
    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertTrue(ret);

    borVec2Set(&pos[1], 3.1, 0);
    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertFalse(ret);

    borVec2Set(&pos[0], -1, 0);
    borVec2Set(&pos[1], 2.1, 0);
    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertFalse(ret);

    borVec2Set(&pos[0], 0, 0);
    rot[0] = M_PI_2;
    borVec2Set(&pos[1], 0, 0);
    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertTrue(ret);

    borVec2Set(&pos[0], 0, 3.);
    rot[0] = M_PI_4;
    borVec2Set(&pos[1], 0, 0);
    ret = borVec2BoxBoxOverlap(&he[0], &pos[0], rot[0], &he[1], &pos[1], rot[1]);
    assertTrue(ret);
}

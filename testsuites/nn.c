#include <cu/cu.h>
#include <fermat/nn.h>
#include <fermat/rand-mt.h>
#include <fermat/nearest-linear.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>

#define BUILD_ELS_LEN 10000
#define BUILD_NUM_TESTS 1000
#define BUILD_NUM_NNS 5
#define BUILD_MAXSIZE 5

struct _el_t {
    fer_vec2_t w;
    fer_nn_el_t el;
    fer_list_t list;
    int added;
};
typedef struct _el_t el_t;

struct _el3_t {
    fer_vec3_t w;
    fer_nn_el_t el;
    fer_list_t list;
};
typedef struct _el3_t el3_t;


#define ADD_NUM_TESTS 1000
#define ADD_ELS_LEN 10000
#define ADD_NUM_NNS 5
#define ADD_MAXSIZE 5

static fer_real_t dist2(void *item1, fer_list_t *item2,
                        void *data)
{
    fer_vec2_t *p = (fer_vec2_t *)item1;
    el_t *el = FER_LIST_ENTRY(item2, el_t, list);

    return ferVec2Dist(p, &el->w);
}

static void test2(fer_rand_mt_t *rand,
                  fer_nn_t *n, fer_list_t *list, size_t num)
{
    fer_nn_el_t *nn[10];
    fer_list_t *nn2[10];
    el_t *el, *el2;
    fer_vec2_t p;
    size_t len, len2, i;

    ferVec2Set(&p, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
    len = ferNNNearest(n, (const fer_vec_t *)&p, num, nn);
    len2 = ferNearestLinear(list, (void *)&p, dist2, nn2, num, NULL);

    assertEquals(len, num);
    assertEquals(len2, num);

    for (i = 0; i < num; i++){
        el  = fer_container_of(nn[i], el_t, el);
        el2 = fer_container_of(nn2[i], el_t, list);

        if (el == el2){
            assertEquals(el, el2);
        }else{
            fprintf(stderr, "%.30f %.30f [%.30f] - %.30f %.30f [%.30f]\n",
                    ferVec2X(&el->w), ferVec2Y(&el->w), ferVec2Dist(&el->w, &p),
                    ferVec2X(&el2->w), ferVec2Y(&el2->w), ferVec2Dist(&el2->w, &p));
        }
    }
}

static void _nnAdd(uint8_t type, fer_nn_params_t *params)
{
    fer_rand_mt_t *rand;
    fer_nn_t *nn;
    static fer_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j;

    rand = ferRandMTNewAuto();

    nn = ferNNNew(type, params);

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferNNElInit(nn, &els[i].el, (const fer_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    for (i = 0; i < ADD_ELS_LEN; i++){
        //fprintf(stdout, "%02d:\n", i);
        ferNNAdd(nn, &els[i].el);
        //ferNNDump(vp, stdout);
    }

    for (i = 0; i < ADD_NUM_TESTS; i++){
        for (j = 1; j <= ADD_NUM_NNS; j++){
            test2(rand, nn, &els_list, j);
        }
    }

    ferNNDel(nn);
    ferRandMTDel(rand);
}

TEST(nnAdd)
{
    fer_nn_params_t params;
    fer_real_t aabb[4] = {-3, 3, -3, 3};

    ferNNParamsInit(&params);
    params.linear.dim = 2;
    params.vptree.dim = 2;
    params.gug.dim = 2;
    params.gug.aabb = aabb;
    params.gug.max_dens = 0.1;
    params.gug.expand_rate = 1.3;

    _nnAdd(FER_NN_LINEAR, &params);
    _nnAdd(FER_NN_VPTREE, &params);
    _nnAdd(FER_NN_GUG, &params);
}

static void _nnAddRm(uint8_t type, fer_nn_params_t *params)
{
    fer_rand_mt_t *rand;
    fer_nn_t *nn;
    static fer_list_t els_list;
    static int els_len = ADD_ELS_LEN;
    static el_t els[ADD_ELS_LEN];
    int i, j;

    rand = ferRandMTNewAuto();

    nn = ferNNNew(type, params);

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, -3, 3), ferRandMT(rand, -3, 3));
        ferNNElInit(nn, &els[i].el, (const fer_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    for (i = 0; i < ADD_ELS_LEN; i++){
        //fprintf(stdout, "%02d:\n", i);
        ferNNAdd(nn, &els[i].el);
        //ferNNDump(vp, stdout);
    }

    for (i = 0; i < ADD_ELS_LEN; i += 3){
        ferNNRemove(nn, &els[i].el);
        ferListDel(&els[i].list);
    }

    for (i = 0; i < ADD_NUM_TESTS; i++){
        for (j = 1; j <= ADD_NUM_NNS; j++){
            test2(rand, nn, &els_list, j);
        }
    }

    ferNNDel(nn);
    ferRandMTDel(rand);
}

TEST(nnAddRm)
{
    fer_nn_params_t params;
    fer_real_t aabb[4] = {-3, 3, -3, 3};

    ferNNParamsInit(&params);
    params.linear.dim = 2;
    params.vptree.dim = 2;
    params.gug.dim = 2;
    params.gug.aabb = aabb;
    params.gug.max_dens = 0.1;
    params.gug.expand_rate = 1.3;

    _nnAddRm(FER_NN_LINEAR, &params);
    _nnAddRm(FER_NN_VPTREE, &params);
    _nnAddRm(FER_NN_GUG, &params);
}
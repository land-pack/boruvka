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

#include <gann/gng.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/**
 * Here is implemented Growing Neural Gas algorithm  completely in static
 * functions. The purpose is to be able to get rid of some indirect calls
 * of functions (see gann_gng_t.ops struct).
 *
 * Indirect calls can be very expensive, so if you need to speed up
 * an algorithm and you really know what you are doing you can use these
 * functions.
 *
 * All functions marked as "public API" corresponds to same GNGs' functions
 * without initial underscore.
 *
 * Before including this file macros OPS() and OPS_DATA() must be defined
 * because these macros are used to expand to "operation" calls as defined
 * in gann_gng_ops_t struct.
 * Operations that are covered by OPS() and OPS_DATA() macros are:
 *    - new_node
 *    - new_node_between
 *    - del_node
 *    - input_signal
 *    - nearest
 *    - dist2
 *    - move_towards
 *
 * For example how to use this file look into gng.c.
 */


/** For public API */
static void _gannGNGRun(gann_gng_t *gng);
static void _gannGNGInit(gann_gng_t *gng);
static void _gannGNGLearn(gann_gng_t *gng);
static void _gannGNGNewNode(gann_gng_t *gng);
static void _gannGNGDecreaseErrCounters(gann_gng_t *gng);
/** Returns node's err counter */
_fer_inline fer_real_t _gannGNGNodeErrCounter(const gann_gng_t *gng,
                                              const gann_gng_node_t *n);

/** Node functions */
/** Initialize node */
_fer_inline void nodeInit(gann_gng_t *gng, gann_gng_node_t *n);
/** Adds node into network */
_fer_inline void nodeAdd(gann_gng_t *gng, gann_gng_node_t *n);

/** Set error counter of node */
_fer_inline void nodeSetErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t err);
/** Increases error counter of node */
_fer_inline void nodeIncErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t inc);
/** Scale error counter of node */
_fer_inline void nodeScaleErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                    fer_real_t s);
/** Applies accumulated error counter */
_fer_inline void nodeApplyErrCounter(gann_gng_t *gng, gann_gng_node_t *n);
/** Final destructor for nodes */
static void nodeFinalDel(gann_net_node_t *node, void *data);

/** Edge functions */
/** Creates and initializes new edge between n1 and n2 */
_fer_inline gann_gng_edge_t *edgeNew(gann_gng_t *gng, gann_gng_node_t *n1,
                                                      gann_gng_node_t *n2);
/** Deletes edge */
_fer_inline void edgeDel(gann_gng_t *gng, gann_gng_edge_t *edge);
/** Delete callback */
typedef void (*delnode_t)(gann_net_node_t *, void *);
static void delEdge(gann_net_edge_t *edge, void *data);

/** Returns node with highest error counter */
static gann_gng_node_t *nodeWithHighestErr(gann_gng_t *gng);
/** Returns q's neighbor with highest error counter and edge that connects
 *  that node with q. */
static gann_gng_node_t *nodeWithHighestErr2(gann_gng_t *gng, gann_gng_node_t *q,
                                            gann_gng_edge_t **edge);

static void _gannGNGRun(gann_gng_t *gng)
{
    size_t step;
    unsigned long cb_step;

    gannGNGInit(gng);

    step = 1;
    cb_step = 1L;
    while (!gng->ops.terminate(gng->ops.terminate_data)){
        gannGNGLearn(gng);

        if (step % gng->params.lambda == 0){
            gannGNGNewNode(gng);

            if (gng->ops.callback
                    && cb_step == gng->ops.callback_period){
                gng->ops.callback(gng->ops.callback_data);
                cb_step = 0L;
            }
            cb_step++;
        }
        gannGNGDecreaseErrCounters(gng);

        step++;
    }
}

static void _gannGNGInit(gann_gng_t *gng)
{
    const void *is;
    gann_gng_node_t *n;

    is = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));
    n  = OPS(gng, new_node)(is, OPS_DATA(gng, new_node));
    nodeAdd(gng, n);

    is = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));
    n  = OPS(gng, new_node)(is, OPS_DATA(gng, new_node));
    nodeAdd(gng, n);
}

static void _gannGNGLearn(gann_gng_t *gng)
{
    const void *input_signal;
    gann_net_node_t *nn;
    gann_gng_node_t *n1, *n2, *n;
    gann_net_edge_t *nedge;
    gann_gng_edge_t *edge;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    // 1. Get input signal
    input_signal = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));

    // 2. Find two nearest nodes to input signal
    OPS(gng, nearest)(input_signal, &n1, &n2, OPS_DATA(gng, nearest));

    // 3. Create connection between n1 and n2 if doesn't exist and set age
    //    to zero
    nedge = gannNetNodeCommonEdge(&n1->node, &n2->node);
    if (!nedge){
        edge = edgeNew(gng, n1, n2);
    }else{
        edge = fer_container_of(nedge, gann_gng_edge_t, edge);
    }
    edge->age = 0;

    // 4. Increase error counter of winner node
    dist2 = OPS(gng, dist2)(input_signal, n1, OPS_DATA(gng, dist2));
    nodeIncErrCounter(gng, n1, dist2);

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    OPS(gng, move_towards)(n1, input_signal, gng->params.eb,
                           OPS_DATA(gng, move_towards));
    // adapt also direct topological neighbors of winner node
    list = gannNetNodeEdges(&n1->node);
    ferListForEachSafe(list, item, item_tmp){
        nedge = gannNetEdgeFromNodeList(item);
        edge  = fer_container_of(nedge, gann_gng_edge_t, edge);
        nn   = gannNetEdgeOtherNode(&edge->edge, &n1->node);
        n    = fer_container_of(nn, gann_gng_node_t, node);

        // move node (5.)
        OPS(gng, move_towards)(n, input_signal, gng->params.en,
                               OPS_DATA(gng, move_towards));

        // increase age (6.)
        edge->age += 1;

        // remove edge if it has age higher than age_max (7.)
        if (edge->age > gng->params.age_max){
            edgeDel(gng, edge);
        }
    }
}

static void _gannGNGNewNode(gann_gng_t *gng)
{
    gann_gng_node_t *q, *f, *r;
    gann_gng_edge_t *eqf;
    fer_real_t err;

    // 1. Get node with highest error counter
    q = nodeWithHighestErr(gng);

    // 2. Get q's neighbor with highest error counter
    f = nodeWithHighestErr2(gng, q, &eqf);
    if (!f){
        return;
    }

    // 3. Create new node between q and f
    r = OPS(gng, new_node_between)(q, f, OPS_DATA(gng, new_node_between));
    nodeAdd(gng, r);

    // 4. Create q-r and f-r edges and remove q-f edge (which is eqf)
    edgeDel(gng, eqf);
    edgeNew(gng, q, r);
    edgeNew(gng, f, r);

    // 5. Decrease error counters of q and f
    nodeScaleErrCounter(gng, q, gng->params.alpha);
    nodeScaleErrCounter(gng, f, gng->params.alpha);

    // 6. Set error counter of new node (r)
    err  = _gannGNGNodeErrCounter(gng, q);
    err += _gannGNGNodeErrCounter(gng, q);
    err /= FER_REAL(2.);
    nodeSetErrCounter(gng, r, err);
}

static void _gannGNGDecreaseErrCounters(gann_gng_t *gng)
{
    if (gng->params.use_acc_err_counter){
        gng->err_counter_mark++;
        gng->err_counter_scale *= gng->params.beta;
    }else{
        fer_list_t *list, *item;
        gann_net_node_t *nn;
        gann_gng_node_t *n;

        list = gannNetNodes(gng->net);
        ferListForEach(list, item){
            nn = ferListEntry(item, gann_net_node_t, list);
            n  = fer_container_of(nn, gann_gng_node_t, node);
            nodeScaleErrCounter(gng, n, gng->params.beta);
        }
    }
}


/*** Node functions ***/
_fer_inline fer_real_t _gannGNGNodeErrCounter(const gann_gng_t *gng, const gann_gng_node_t *n)
{
    nodeApplyErrCounter((gann_gng_t *)gng, (gann_gng_node_t *)n);
    return n->err_counter;
}

_fer_inline void nodeInit(gann_gng_t *gng, gann_gng_node_t *n)
{
    n->err_counter = FER_ZERO;
    n->err_counter_mark = gng->err_counter_mark;
}

_fer_inline void nodeAdd(gann_gng_t *gng, gann_gng_node_t *n)
{
    nodeInit(gng, n);
    gannNetAddNode(gng->net, &n->node);
}

_fer_inline void nodeSetErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t err)
{
    n->err_counter = err;
    n->err_counter_mark = gng->err_counter_mark;
}

_fer_inline void nodeIncErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t inc)
{
    nodeApplyErrCounter(gng, n);
    n->err_counter += inc;
}

_fer_inline void nodeScaleErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                     fer_real_t s)
{
    nodeApplyErrCounter(gng, n);
    n->err_counter *= s;
}

_fer_inline void nodeApplyErrCounter(gann_gng_t *gng, gann_gng_node_t *n)
{
    size_t mark, left;
    fer_real_t err;

    if (!gng->params.use_acc_err_counter)
        return;

    mark = n->err_counter_mark;
    left = gng->err_counter_mark - mark;

    if (fer_likely(left > 0)){
        if (fer_likely(left == gng->err_counter_mark)){
            // most of nodes in mesh are not touched while ECHL phase - so
            // scale factor can be cumulated and can be used directly
            // without pow() operation
            n->err_counter *= gng->err_counter_scale;
        }else{
            err = FER_POW(gng->params.beta, (fer_real_t)left);
            n->err_counter *= err;
        }
    }

    n->err_counter_mark = gng->err_counter_mark;
}

static void nodeFinalDel(gann_net_node_t *node, void *data)
{
    gann_gng_t *gng = (gann_gng_t *)data;
    gann_gng_node_t *n;

    n = fer_container_of(node, gann_gng_node_t, node);
    OPS(gng, del_node)(n, OPS_DATA(gng, del_node));
}



/*** Edge functions ***/
_fer_inline gann_gng_edge_t *edgeNew(gann_gng_t *gng, gann_gng_node_t *n1,
                                                      gann_gng_node_t *n2)
{
    gann_gng_edge_t *e;

    e = FER_ALLOC(gann_gng_edge_t);
    e->age = 0;

    gannNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);

    return e;
}

_fer_inline void edgeDel(gann_gng_t *gng, gann_gng_edge_t *e)
{
    gannNetRemoveEdge(gng->net, &e->edge);
    free(e);
}

static void delEdge(gann_net_edge_t *edge, void *data)
{
    free(edge);
}



static gann_gng_node_t *nodeWithHighestErr(gann_gng_t *gng)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gng_node_t *n, *n_highest;
    fer_real_t err, err_highest;

    err_highest = FER_REAL_MIN;
    n_highest   = NULL;

    list = gannNetNodes(gng->net);
    ferListForEach(list, item){
        nn = ferListEntry(item, gann_net_node_t, list);
        n  = fer_container_of(nn, gann_gng_node_t, node);
        err = _gannGNGNodeErrCounter(gng, n);

        if (err > err_highest){
            err_highest = err;
            n_highest   = n;
        }

        // Accumulated error counter was applied to all nodes.
        // Now it is safe to reset all nodes' error counters.
        // Note that it is safe to don't check if accumulated error counter
        // is enabled.
        n->err_counter_mark = 0;
    }

    // Reset global accumulated error counter
    // Note that it is safe to don't check if accumulated error counter
    // is enabled.
    gng->err_counter_scale = FER_ONE;
    gng->err_counter_mark = 0;

    return n_highest;
}

static gann_gng_node_t *nodeWithHighestErr2(gann_gng_t *gng, gann_gng_node_t *q,
                                            gann_gng_edge_t **edge)
{
    fer_list_t *list, *item;
    gann_net_edge_t *ne;
    gann_gng_edge_t *e_highest;
    gann_net_node_t *nn;
    gann_gng_node_t *n, *n_highest;
    fer_real_t err, err_highest;

    err_highest = FER_REAL_MIN;
    n_highest = NULL;
    e_highest = NULL;

    list = gannNetNodeEdges(&q->node);
    ferListForEach(list, item){
        ne = gannNetEdgeFromNodeList(item);
        nn = gannNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, gann_gng_node_t, node);
        err = _gannGNGNodeErrCounter(gng, n);

        if (err > err_highest){
            err_highest = err;
            n_highest   = n;
            e_highest   = fer_container_of(ne, gann_gng_edge_t, edge);
        }
    }

    *edge = e_highest;
    return n_highest;
}


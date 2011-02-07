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

#ifndef __FER_GANN_GNG_H__
#define __FER_GANN_GNG_H__

#include <gann/net.h>

/**
 * Growing Neural Gas
 * ===================
 * Generalized implementation of Growing Neural Gas algorithm as was
 * described in:
 *      B. Fritzke, "A growing neural gas network learns topologies,"
 *      Neural Inf. Process. Syst., vol. 7, pp. 625­632, 1995.
 *
 * This implementation is not limited to 2-D, 3-D or any other dimensions.
 * In fact, nodes doesn't have assigned any weight vector - it is user's
 * responsibility to provide that.
 *
 * User must define several callbacks and fill *gann_gng_ops_t* structure,
 * gannGNG*() functions take care of the core of algorithm.
 *
 * +--------------------------------------------------------------------------------------+
 * | Algorithm works as follows:                                                          |
 * +======================================================================================+
 * | 1. Initializes network by two random nodes. [*gannGNGInit()*]                        |
 * | 2. Check termination condition                                                       |
 * | 3. Learn topology. ['gannGNGLearn()*]                                                |
 * |     1. Get random input signal                                                       |
 * |     2. Find two nearest nodes to input signal - *n1*, *n2*                           |
 * |     3. Create connection between *n1* and *n2* if doesn't exist and set age          |
 * |        to zero                                                                       |
 * |     4. Increase error counter of winner node.                                        |
 * |     5. Adapt nodes to input signal using fractions *eb* and *en*                     |
 * |     6. Increment age of all edges that incident with winner node by one              |
 * |     7. Remove all edges with age higher than *age_max*                               |
 * | 4. If the number of input signals presented so far to the network is an              |
 * |    integer multiple of the parameter *lambda*, create new node. [*gannGNGNewNode()*] |
 * |     1. Get node with highest error counter -> *q*                                    |
 * |     2. Get q's neighbor node with highest error counter -> *f*                       |
 * |     3. Create new node between *q* and *f* -> *r*                                    |
 * |     4. Create *q-r* and *f-r* edges and delete *q-f* edge.                           |
 * |     5. Decrease error counter of *q* and *f* (*alpha* parameter).                    |
 * |     6. Set error counter of *r* as average error counter of *q* and *f*.             |
 * | 5. Decrease error counters of all nodes [*gannGNGDecreaseErrCounters()*]             |
 * | 6. Go to 2.                                                                          |
 * +--------------------------------------------------------------------------------------+
 */


struct _gann_gng_node_t {
    gann_net_node_t node;

    fer_real_t err_counter;
};
typedef struct _gann_gng_node_t gann_gng_node_t;


struct _gann_gng_edge_t {
    gann_net_edge_t edge;

    int age;
};
typedef struct _gann_gng_edge_t gann_gng_edge_t;



/**
 * GNG Operations
 * ---------------
 *
 * See gann_gng_ops_t.
 */

/** vvvv */

/**
 * Create new node initialized weight vector to input_signal.
 */
typedef gann_gng_node_t *(*gann_gng_new_node)(const void *input_signal, void *);

/**
 * Create new node in between n1 and n2.
 */
typedef gann_gng_node_t *(*gann_gng_new_node_between)(const gann_gng_node_t *n1,
                                                      const gann_gng_node_t *n2,
                                                      void *);

/**
 * Deletes given node.
 */
typedef void (*gann_gng_del_node)(gann_gng_node_t *n, void *);

/**
 * Returns random input signal.
 */
typedef const void *(*gann_gng_input_signal)(void *);

/**
 * Returns (via n1 and n2) first and second nearest node to input signal.
 */
typedef void (*gann_gng_nearest)(const void *input_signal,
                                 gann_gng_node_t **n1,
                                 gann_gng_node_t **n2,
                                 void *);

/**
 * Returns squared distance between input_signal and node.
 */
typedef fer_real_t (*gann_gng_dist2)(const void *input_signal,
                                     const gann_gng_node_t *node, void *);

/**
 * Move given node towards input_signal by given fraction.
 *
 * If position on node is w and position of input_signal is v, then:
 * w = w + ((v - w) * fraction)
 */
typedef void (*gann_gng_move_towards)(gann_gng_node_t *node,
                                      const void *input_signal,
                                      fer_real_t fraction,
                                      void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*gann_gng_terminate)(void *);

/** ^^^^ */

struct _gann_gng_ops_t {
    gann_gng_new_node         new_node;
    gann_gng_new_node_between new_node_between;
    gann_gng_del_node         del_node;
    gann_gng_input_signal     input_signal;
    gann_gng_nearest          nearest;
    gann_gng_dist2            dist2;
    gann_gng_move_towards     move_towards;
    gann_gng_terminate        terminate;
};
typedef struct _gann_gng_ops_t gann_gng_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void gannGNGOpsInit(gann_gng_ops_t *ops);



/**
 * GNG Parameters
 * ---------------
 */
struct _gann_gng_params_t {
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */
};
typedef struct _gann_gng_params_t gann_gng_params_t;

/**
 * Initializes params struct to default values.
 */
void gannGNGParamsInit(gann_gng_params_t *params);



/**
 * GNG Algorithm
 * --------------
 *
 * See gann_gng_t.
 * See gann_gng_node_t.
 * See gann_gng_edge_t.
 */

struct _gann_gng_t {
    gann_net_t *net;
    gann_gng_ops_t ops;
    gann_gng_params_t params;
    void *data;
};
typedef struct _gann_gng_t gann_gng_t;


/**
 * Creates new instance of GNG algorithm.
 */
gann_gng_t *gannGNGNew(const gann_gng_ops_t *ops,
                       const gann_gng_params_t *params,
                       void *data);

/**
 * Deletes GNG.
 */
void gannGNGDel(gann_gng_t *gng);

/**
 * Runs GNG algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true:
 *
 * 1. gannGNGInit()
 * 2. Terminate?
 * 3. gannGNGLearn()
 * 4. If the number of input signals presented so far to the network is an
 *    integer multiple of the parameter lambda: gannGNGNewNode().
 * 5. gannGNGDecreaseErrCounters()
 * 6. Go to 2.
 */
void gannGNGRun(gann_gng_t *gng);

/**
 * Initializes neural network.
 *
 * Gets two input signals and create two node (unconnected) from them.
 *
 * Operations input_signal() and new_node() must be set.
 */
void gannGNGInit(gann_gng_t *gng);

/**
 * One step of learning of neutal network.
 *
 * Algorithm works as follows:
 *
 * 1. Get random input signal
 * 2. Find two nearest nodes to input signal - n1, n2
 * 3. Create connection between n1 and n2 if doesn't exist and set age to
 *    zero
 * 4. Increase error counter of winner node
 * 5. Adapt nodes to input signal using fractions eb and en
 * 6. Increment age of all edges that incident with winner node by one
 * 7. Remove all edges with age higher than age_max
 *
 * Operations input_signal(), nearest(), dist2() and move_towards() must be
 * set.
 */
void gannGNGLearn(gann_gng_t *gng);

/**
 * Creates new node.
 *
 * Algorithm works as follows:
 *
 * 1. Get node with highest error counter -> q
 * 2. Get q's neighbor node with highest error counter -> f
 * 3. Create new node between q and f -> r
 * 4. Create q-r and f-r edges and delete q-f edge.
 * 5. Decrease error counter of q and f (alpha parameter).
 * 6. Set error counter of r as average error counter of q and f.
 *
 * Operations new_node_between() must be set.
 */
void gannGNGNewNode(gann_gng_t *gng);

/**
 * Decreases error counters of all nodes by multiplication by parameter
 * beta.
 */
void gannGNGDecreaseErrCounters(gann_gng_t *gng);


/**
 * Returns error counter of node.
 *
 * Always use this function instead of direct access to struct!
 */
fer_real_t gannGNGNodeErrCounter(const gann_gng_t *gng, const gann_gng_node_t *n);

/**
 * Returns age of edge.
 *
 * Always use this function instead of direct access to struct!
 */
int gannGNGEdgeAge(const gann_gng_t *gng, const gann_gng_edge_t *edge);

/**
 * Returns net of nodes.
 */
_fer_inline gann_net_t *gannGNGNet(gann_gng_t *gng);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t gannGNGNodesLen(const gann_gng_t *gng);


/**** INLINES ****/
_fer_inline gann_net_t *gannGNGNet(gann_gng_t *gng)
{
    return gng->net;
}

_fer_inline size_t gannGNGNodesLen(const gann_gng_t *gng)
{
    return gannNetNodesLen(gng->net);
}

#endif /* __FER_GANN_GNG_H__ */


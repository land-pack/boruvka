/***
 * fermat
 * -------
 * Copyright (c)2012 Daniel Fiser <danfis@danfis.cz>
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

#include <stdio.h>
#include <string.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/gpc.h>
#include <fermat/gpc-tree.h>


/**
 * Definition of predicate
 */
struct _fer_gpc_pred_t {
    fer_gpc_pred pred;      /*!< Predicate callback */
    fer_gpc_pred_init init; /*!< Initialization callback */
    size_t memsize;         /*!< Size of allocated memory per predicate instance */
    unsigned int ndesc;     /*!< Number of descendants */
    void *data;             /*!< User data */
};
typedef struct _fer_gpc_pred_t fer_gpc_pred_t;

/**
 * Definition of class
 */
struct _fer_gpc_class_t {
    int class; /*!< Class identifier */
};
typedef struct _fer_gpc_class_t fer_gpc_class_t;


#define OPS_DATA(name) \
    if (!gpc->ops.name ## _data) \
        gpc->ops.name ## _data = gpc->ops.data;

#define OPS_CHECK(name) \
    if (!gpc->ops.name){ \
        fprintf(stderr, "Fermat :: GPC :: No ." #name " callback set.\n"); \
        exit(-1); \
    }

#define OPS_CHECK_DATA(name) \
    OPS_DATA(name) \
    OPS_CHECK(name)


void ferGPCOpsInit(fer_gpc_ops_t *ops)
{
    memset(ops, 0, sizeof(*ops));
}

void ferGPCParamsInit(fer_gpc_params_t *params)
{
    params->pop_size  = 1;
    params->max_depth = 5;
    params->data_rows = 0;

    params->tournament_size = 5;

    params->pr        = 14;
    params->pc        = 85;
    params->pm        = 1;
}



fer_gpc_t *ferGPCNew(const fer_gpc_ops_t *ops, const fer_gpc_params_t *params)
{
    fer_gpc_t *gpc;
    fer_real_t prob;

    gpc = FER_ALLOC(fer_gpc_t);

    gpc->params = *params;
    prob = gpc->params.pr + gpc->params.pc + gpc->params.pm;
    gpc->params.pr = gpc->params.pr / prob;
    gpc->params.pc = gpc->params.pc / prob;
    gpc->params.pm = gpc->params.pm / prob;

    gpc->ops = *ops;
    OPS_CHECK_DATA(fitness);
    OPS_CHECK_DATA(data_row);
    OPS_DATA(callback);

    //gpc->rand = ferRandMTNewAuto();
    gpc->rand = ferRandMTNew(9999);

    gpc->pop[0] = FER_ALLOC_ARR(fer_gpc_tree_t *, gpc->params.pop_size);
    gpc->pop[1] = FER_ALLOC_ARR(fer_gpc_tree_t *, gpc->params.pop_size);
    memset(gpc->pop[0], 0, sizeof(fer_gpc_tree_t *) * gpc->params.pop_size);
    memset(gpc->pop[1], 0, sizeof(fer_gpc_tree_t *) * gpc->params.pop_size);

    gpc->pred_size = FER_GPC_PRED_INIT_SIZE;
    gpc->pred_len  = 0;
    gpc->pred      = FER_ALLOC_ARR(fer_gpc_pred_t, gpc->pred_size);

    gpc->class_size = FER_GPC_CLASS_INIT_SIZE;
    gpc->class_len  = 0;
    gpc->class      = FER_ALLOC_ARR(fer_gpc_class_t, gpc->class_size);

    gpc->eval_results = FER_ALLOC_ARR(int, gpc->params.data_rows);

    return gpc;
}

void ferGPCDel(fer_gpc_t *gpc)
{
    size_t i;

    ferRandMTDel(gpc->rand);

    for (i = 0; i < gpc->params.pop_size; i++){
        if (gpc->pop[0][i] != NULL)
            ferGPCTreeDel(gpc->pop[0][i]);
        if (gpc->pop[1][i] != NULL)
            ferGPCTreeDel(gpc->pop[1][i]);
    }
    FER_FREE(gpc->pop[0]);
    FER_FREE(gpc->pop[1]);

    FER_FREE(gpc->pred);
    FER_FREE(gpc->class);
    FER_FREE(gpc->eval_results);
    FER_FREE(gpc);
}

int ferGPCAddPred(fer_gpc_t *gpc,
                  fer_gpc_pred pred, fer_gpc_pred_init init,
                  unsigned int num_descendants, size_t memsize,
                  void *data)
{
    if (gpc->pred_len >= gpc->pred_size){
        gpc->pred_size *= 2;
        gpc->pred = FER_REALLOC_ARR(gpc->pred, fer_gpc_pred_t, gpc->pred_size);
    }

    gpc->pred[gpc->pred_len].pred    = pred;
    gpc->pred[gpc->pred_len].init    = init;
    gpc->pred[gpc->pred_len].memsize = memsize;
    gpc->pred[gpc->pred_len].ndesc   = num_descendants;
    gpc->pred[gpc->pred_len].data    = data;
    gpc->pred_len++;

    return 0;
}

int ferGPCAddClass(fer_gpc_t *gpc, int class_id)
{
    if (gpc->class_len >= gpc->class_size){
        gpc->class_size *= 2;
        gpc->class = FER_REALLOC_ARR(gpc->class, fer_gpc_class_t, gpc->class_size);
    }

    gpc->class[gpc->class_len].class = class_id;
    gpc->class_len++;

    return 0;
}

size_t __ferGPCPredMemsize(const fer_gpc_t *gpc, unsigned int idx)
{
    return gpc->pred[idx].memsize;
}


/** Randomly generate a node with its subtree */
static fer_gpc_node_t *genRandTree(fer_gpc_t *gpc, size_t depth, size_t max_depth)
{
    unsigned int idx;
    fer_gpc_node_t *node;
    fer_gpc_node_t **desc;
    uint8_t i;

    // Randomly choose a predicate or a class.
    // If max_depth is reached, generate only a class.
    if (depth == max_depth){
        idx = ferGPCRandInt(gpc, 0, gpc->class_len);
        idx += gpc->pred_len;
    }else{
        idx = ferGPCRandInt(gpc, 0, gpc->pred_len + gpc->class_len);
    }

    if (idx >= gpc->pred_len){
        // create a new class node
        idx = idx - gpc->pred_len;
        node = ferGPCNodeNew(idx, 0, 0);

    }else{
        // create a new predicate node
        node = ferGPCNodeNew(idx, gpc->pred[idx].ndesc,
                             gpc->pred[idx].memsize);
        // initialize it
        if (gpc->pred[idx].init){
            gpc->pred[idx].init(gpc, FER_GPC_NODE_MEM(node),
                                gpc->pred[idx].data);
        }

        // and fill descendants
        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < node->ndesc; i++){
            desc[i] = genRandTree(gpc, depth + 1, max_depth);
        }
    }

    return node;
}


/** Create randomly generated initial population in gpc->pop[pop] */
static void createInitPop(fer_gpc_t *gpc, int pop)
{
    size_t i, len;
    size_t pop_other;

    pop_other = (pop + 1) % 2;

    gpc->pop_size[pop] = 0;
    gpc->pop_size[pop_other] = 0;

    len = gpc->params.pop_size;
    for (i = 0; i < len; i++){
        gpc->pop[pop][i] = ferGPCTreeNew();
        gpc->pop[pop][i]->root = genRandTree(gpc, 0, gpc->params.max_depth);
        ferGPCTreeFix(gpc->pop[pop][i]);
        gpc->pop_size[pop]++;

        gpc->pop[pop_other][i] = NULL;

        //ferGPCTreePrint(gpc->pop[0][i], stdout);
    }
}

/** Evals a tree with one data row. Returns resulting class as defined by
 *  ferGPCAddClass() function. */
static int evalTreeClass(fer_gpc_t *gpc, fer_gpc_tree_t *tree, void *data)
{
    fer_gpc_node_t *node;
    fer_gpc_node_t **desc;
    fer_gpc_pred pred;
    void *mem;
    unsigned int dispatch;

    // traverse the decision tree
    node = tree->root;
    while (node->ndesc != 0){
        pred = gpc->pred[node->idx].pred;
        mem  = FER_GPC_NODE_MEM(node);

        // find which descendant is used
        dispatch = pred(gpc, mem, data, gpc->pred[node->idx].data);
        dispatch = FER_MIN(dispatch, node->ndesc - 1);

        // descent to next node
        desc = FER_GPC_NODE_DESC(node);
        node = desc[dispatch];
    }

    return gpc->class[node->idx].class;
}

/** Evaluate a tree on all data rows and records a new fitness that is alse
 *  returned */
static fer_real_t evalTree(fer_gpc_t *gpc, fer_gpc_tree_t *tree)
{
    size_t i;
    int class;
    void *data;

    for (i = 0; i < gpc->params.data_rows; i++){
        // obtain a data row using a callback
        data = gpc->ops.data_row(gpc, i, gpc->ops.data_row_data);

        // eval the tree on the data row
        class = evalTreeClass(gpc, tree, data);
        gpc->eval_results[i] = class;
    }

    // compute a fitness value
    tree->fitness = gpc->ops.fitness(gpc, gpc->eval_results,
                                     gpc->ops.fitness_data);

    //ferGPCTreePrint(tree, stdout);

    return tree->fitness;
}

/** Evaluate whole population of decision trees */
static void evalPop(fer_gpc_t *gpc, int pop)
{
    size_t i;
    fer_real_t overall_fitness;

    overall_fitness = FER_ZERO;
    for (i = 0; i < gpc->pop_size[pop]; i++){
        overall_fitness += evalTree(gpc, gpc->pop[pop][i]);
    }

    DBG("overall_fitness: %f", overall_fitness);
}

/** Tournament selection */
static size_t ferGPCSelectionTournament(fer_gpc_t *gpc, size_t tour_size,
                                        fer_gpc_tree_t **pop, size_t pop_size)
{
    fer_real_t best_fitness;
    size_t i, sel, best = 0;

    best_fitness = -FER_REAL_MAX;
    for (i = 0; i < tour_size; i++){
        sel = ferGPCRandInt(gpc, 0, pop_size);
        if (pop[sel]->fitness > best_fitness){
            best = sel;
        }
    }

    return best;
}

static void ferGPCReproduction(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    size_t idx;

    DBG2("");
    // select an individual from population
    idx = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                    gpc->pop[from_pop], gpc->pop_size[from_pop]);

    // and copy the individual to other population
    gpc->pop[to_pop][gpc->pop_size[to_pop]] = ferGPCTreeClone(gpc, gpc->pop[from_pop][idx]);
    gpc->pop_size[to_pop]++;
}

static void ferGPCCrossover(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    DBG2("");
}


static void ferGPCMutation(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    size_t idx, node_idx;
    size_t depth;
    fer_gpc_tree_t *tree;
    fer_gpc_node_t *node, **desc;

    DBG2("");

    // select an individual from population
    idx = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                    gpc->pop[from_pop], gpc->pop_size[from_pop]);
    tree = gpc->pop[from_pop][idx];

    // choose a node that will undergo a mutation
    node_idx = ferGPCRandInt(gpc, 0, tree->num_nodes);

    // get a node from the tree
    node = ferGPCTreeNodeById(tree, node_idx, &desc, &depth);
    // delete an old subtree and generate a new one
    ferGPCNodeDel(node);
    *desc = genRandTree(gpc, 0, gpc->params.max_depth - depth);
    ferGPCTreeFix(tree);

    // copy the tree to destination array
    gpc->pop[to_pop][gpc->pop_size[to_pop]] = tree;
    ferGPCTreePrint(tree, stderr);
    gpc->pop_size[to_pop]++;

    // remove the tree from source array
    gpc->pop[from_pop][idx] = gpc->pop[from_pop][gpc->pop_size[from_pop] - 1];
    gpc->pop[from_pop][gpc->pop_size[from_pop] - 1] = NULL;
    gpc->pop_size[from_pop]--;
}

int ferGPCRun(fer_gpc_t *gpc)
{
    int step, i;
    fer_real_t action;
    int pop_cur, pop_other;

    // early exit if we don't have any classes
    if (gpc->class_len == 0)
        return -1;

    pop_cur = 0;
    pop_other = (pop_cur + 1) % 2;

    // create initial population
    createInitPop(gpc, pop_cur);

    // evaluate initial population
    evalPop(gpc, pop_cur);

    for (i = 0; i < gpc->pop_size[pop_cur]; i++){
        ferGPCTreePrint(gpc->pop[pop_cur][i], stderr);
    }
    DBG("pr, pc, pm: %f %f %f", gpc->params.pr, gpc->params.pc, gpc->params.pm);
    for (step = 0; step < 10; step++){
        DBG("Step: %d", step);

        while (gpc->pop_size[pop_other] < gpc->params.pop_size){
            // choose action
            action = ferGPCRand01(gpc);

            if (action < gpc->params.pr){
                // reproduction
                ferGPCReproduction(gpc, pop_cur, pop_other);
            }else if (action < gpc->params.pr + gpc->params.pc){
                // crossover
                ferGPCCrossover(gpc, pop_cur, pop_other);
            }else{
                // mutation
                ferGPCMutation(gpc, pop_cur, pop_other);
            }
        }

        // reset the old population
        for (i = 0; i < gpc->pop_size[pop_cur]; i++){
            ferGPCTreeDel(gpc->pop[pop_cur][i]);
            gpc->pop[pop_cur][i] = NULL;
        }
        gpc->pop_size[pop_cur] = 0;

        DBG2("=====");
        for (i = 0; i < gpc->pop_size[pop_other]; i++){
            ferGPCTreePrint(gpc->pop[pop_other][i], stderr);
        }

        pop_cur   = (pop_cur + 1) % 2;
        pop_other = (pop_cur + 1) % 2;
    }
    DBG("%f %f %f", gpc->params.pr, gpc->params.pc, gpc->params.pm);

    return 0;
}

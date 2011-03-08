/***awInputPoint(g);
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

#include <fermat/gsrm.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Print progress */
#define PR_PROGRESS(g) \
    ferTimerStop(&g->timer); \
    ferTimerPrintElapsed(&(g)->timer, stderr, " n: %d/%d, e: %d, f: %d\r", \
                         ferMesh3VerticesLen((g)->mesh), \
                         (g)->param.max_nodes, \
                         ferMesh3EdgesLen((g)->mesh), \
                         ferMesh3FacesLen((g)->mesh)); \
    fflush(stderr)

#define PR_PROGRESS_PREFIX(g, prefix) \
    ferTimerStop(&(g)->timer); \
    ferTimerPrintElapsed(&(g)->timer, stderr, prefix " n: %d/%d, e: %d, f: %d\n", \
                         ferMesh3VerticesLen((g)->mesh), \
                         (g)->param.max_nodes, \
                         ferMesh3EdgesLen((g)->mesh), \
                         ferMesh3FacesLen((g)->mesh)); \
    fflush(stderr)


struct _node_t {
    fer_vec3_t *v; /*!< Position of node (weight vector) */

    fer_real_t err_counter;  /*!< Error counter */
    size_t err_counter_mark; /*!< Mark used for accumulated error counter */

    fer_mesh3_vertex_t vert; /*!< Vertex in mesh */
    fer_cubes3_el_t cubes;   /*!< Struct for NN search */
};
typedef struct _node_t node_t;

struct _edge_t {
    int age;      /*!< Age of edge */

    fer_mesh3_edge_t edge; /*!< Edge in mesh */
};
typedef struct _edge_t edge_t;

struct _face_t {
    fer_mesh3_face_t face; /*!< Face in mesh */
};
typedef struct _face_t face_t;

struct _fer_gsrm_cache_t {
    fer_vec3_t *is;     /*!< Input signal */
    node_t *nearest[2]; /*!< Two nearest nodes */

    node_t **common_neighb;    /*!< Array of common neighbors */
    size_t common_neighb_size; /*!< Size of .common_neighb - num allocated
                                    bytes */
    size_t common_neighb_len;  /*!< Number of nodes in .common_neighb */

    size_t err_counter_mark;      /*!< Contains mark used for accumalet
                                       error counter. It holds how many
                                       times were applied parameter alpha */
    fer_real_t err_counter_scale; /*!< Accumulated error counter - alpha^mark */

    fer_real_t pp_min, pp_max; /*!< Min and max area2 of face - used in
                                    postprocessing */
};
typedef struct _fer_gsrm_cache_t fer_gsrm_cache_t;


/** Allocates and deallocates cache */
static fer_gsrm_cache_t *cacheNew(void);
static void cacheDel(fer_gsrm_cache_t *c);


/** --- Node functions --- */
/** Creates new node and sets its weight to given vector */
static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v);
/** Deletes node */
static void nodeDel(fer_gsrm_t *g, node_t *n);
/** Deletes node - proposed for ferMesh3Del2() function */
static void nodeDel2(fer_mesh3_vertex_t *v, void *data);
/** Applies cumulative error counter on node's err counter */
static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n);
/** Returns error counter of node.
 *  This function must be used instead of direct access to struct because
 *  of cumulative error counter */
static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n);
/** Scales error counter by given factor */
static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s);
/** Resets error counter - applies cumulative one and resets mark to zero */
static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n);
/** Increases error counter by given value */
static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v);
/** Reset all error counters. */
static void nodeErrCounterResetAll(fer_gsrm_t *g);
/** Scale all error counters. */
static void nodeErrCounterScaleAll(fer_gsrm_t *g);


/** --- Edge functions --- */
/** Creates new edge as connection between two given nodes */
static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2);
/** Deletes edge */
static void edgeDel(fer_gsrm_t *g, edge_t *e);
/** Deteles edge - proposed for ferMesh3Del2() function */
static void edgeDel2(fer_mesh3_edge_t *v, void *data);

/** --- Face functions --- */
static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n);
static void faceDel(fer_gsrm_t *g, face_t *e);
static void faceDel2(fer_mesh3_face_t *v, void *data);


/* Initializes mesh with three random nodes from input */
static void meshInit(fer_gsrm_t *g);

static void drawInputPoint(fer_gsrm_t *g);
/** Performes Extended Competitive Hebbian Learning */
static void echl(fer_gsrm_t *g);
static void echlConnectNodes(fer_gsrm_t *g);
static void echlMove(fer_gsrm_t *g);
static void echlUpdate(fer_gsrm_t *g);
/** Creates new node */
static void createNewNode(fer_gsrm_t *g);
/** Decreases error counter */
static void decreaseErrCounter(fer_gsrm_t *g);

/** Initializes mesh with three random nodes from input */
static void meshInit(fer_gsrm_t *g);
/** Choose random input signal and stores it in cache */
static void drawInputPoint(fer_gsrm_t *g);


/** --- ECHL functions --- */
/** Performs ECHL algorithm */
static void echl(fer_gsrm_t *g);
/** Gathers common neighbors of n1 and n2 and stores them in cache. */
static void echlCommonNeighbors(fer_gsrm_t *g, node_t *n1, node_t *n2);
/** Remove edge if it is inside thales sphere of n1, n2 and theirs common
 *  neighbors */
static void echlRemoveThales(fer_gsrm_t *g, edge_t *e, node_t *n1, node_t *n2);
/** Removes all edges that connect common neighbors between each other */
static void echlRemoveNeighborsEdges(fer_gsrm_t *g);
/** Create faces between given edge and common neighbors stored in cache's
 *  .common_neighb array */
static void echlCreateFaces(fer_gsrm_t *g, edge_t *e);
/** Connect winner nodes and if they are already connected update that
 *  connection */
static void echlConnectNodes(fer_gsrm_t *g);
/** Moves node towards input signal by given factor */
_fer_inline void echlMoveNode(fer_gsrm_t *g, node_t *n, fer_real_t k);
/** Move winner nodes towards input signal */
static void echlMove(fer_gsrm_t *g);
/** Updates all edges emitating from winning node */
static void echlUpdate(fer_gsrm_t *g);


/** -- Create New Node functions --- */
/** Performs "Create New Node" operation */
static void createNewNode(fer_gsrm_t *g);
/** Returns node with highest error counter */
static node_t *nodeWithHighestErrCounter(fer_gsrm_t *g);
/** Returns node with highests error counter that is neighbor of sq */
static node_t *nodesNeighborWithHighestErrCounter(fer_gsrm_t *g, node_t *sq);
/** Actually creates new node between sq and sf */
static node_t *createNewNode2(fer_gsrm_t *g, node_t *sq, node_t *sf);


/** Decreases error counter for all nodes */
static void decreaseErrCounter(fer_gsrm_t *g);


/** --- Postprocessing functions --- */
/** Returns (via min, max, avg arguments) minimum, maximum and average area
 *  of faces in a mesh. */
static void faceAreaStat(fer_gsrm_t *g, fer_real_t *min, fer_real_t *max,
                         fer_real_t *avg);
/** Deletes incorrect faces from mesh */
static void delIncorrectFaces(fer_gsrm_t *g);
/** Deletes incorrect edges from mesh */
static void delIncorrectEdges(fer_gsrm_t *g);
/** Merges all edges that can be merged */
static void mergeEdges(fer_gsrm_t *g);
/** Tries to finish (triangulate) surface */
static void finishSurface(fer_gsrm_t *g);
/** Deletes lonely nodes, edges and faces */
static void delLonelyNodesEdgesFaces(fer_gsrm_t *g);
/** Embed triangles everywhere it can */
static void finishSurfaceEmbedTriangles(fer_gsrm_t *g);
/** Returns true if all internal angles of face is smaller than
 *  g->param.max_angle */
static int faceCheckAngle(fer_gsrm_t *g, fer_mesh3_vertex_t *v1,
                          fer_mesh3_vertex_t *v2, fer_mesh3_vertex_t *v3);
/** Deletes one of triangles (the triangles are considered to have dihedral
 *  angle smaller than g->param.min_dangle.
 *  First is deleted face that have less incidenting faces. If both have
 *  same, faces with smaller area is deleted. */
static void delFacesDangle(fer_gsrm_t *g, face_t *f1, face_t *f2);
/** Returns true if given edge can't be used for face creation */
static int edgeNotUsable(fer_mesh3_edge_t *e);
/** Tries to finish triangle incidenting with e.
 *  It's assumend that e has already one incidenting face. */
static int finishSurfaceTriangle(fer_gsrm_t *g, edge_t *e);
/** Tries to create completely new face */
static int finishSurfaceNewFace(fer_gsrm_t *g, edge_t *e);


fer_gsrm_t *ferGSRMNew(void)
{
    fer_gsrm_t *g;

    g = FER_ALLOC(fer_gsrm_t);

    // init params:
    g->param.lambda = 200;
    g->param.eb = 0.05;
    g->param.en = 0.0006;
    g->param.alpha = 0.95;
    g->param.beta = 0.9995;
    g->param.age_max = 200;
    g->param.max_nodes = 5000;
    g->param.num_cubes = 5000;

    g->param.min_dangle = M_PI_4;
    g->param.max_angle = M_PI_2 * 1.5;
    g->param.angle_merge_edges = M_PI * 0.9;

    // initialize point cloude (input signals)
    g->is = ferPC3New();

    // init 3D mesh
    g->mesh = ferMesh3New();

    // init cubes for NN search to NULL, actual allocation will be made
    // after we know what area do we need to cover
    g->cubes = NULL;

    g->verbosity = 0;

    g->c = NULL;

    return g;
}

void ferGSRMDel(fer_gsrm_t *g)
{
    if (g->c)
        cacheDel(g->c);

    if (g->is)
        ferPC3Del(g->is);

    if (g->mesh)
        ferMesh3Del2(g->mesh, nodeDel2, (void *)g,
                              edgeDel2, (void *)g,
                              faceDel2, (void *)g);

    if (g->cubes)
        ferCubes3Del(g->cubes);

    free(g);
}

size_t ferGSRMAddInputSignals(fer_gsrm_t *g, const char *fn)
{
    return ferPC3AddFromFile(g->is, fn);
}

int ferGSRMRun(fer_gsrm_t *g)
{
    const fer_real_t *aabb;
    size_t step, step_progress;

    // check if there are some input signals
    if (ferPC3Len(g->is) <= 3){
        DBG2("No input signals!");
        return -1;
    }

    // initialize cache
    if (!g->c)
        g->c = cacheNew();

    // initialize NN search structure
    if (!g->cubes){
        aabb = ferPC3AABB(g->is);
        g->cubes = ferCubes3New(aabb, g->param.num_cubes);
    }

    // first shuffle of all input signals
    ferPC3Permutate(g->is);
    // and initialize its iterator
    ferPC3ItInit(&g->isit, g->is);

    // start timer
    ferTimerStart(&g->timer);

    // initialize mesh with three random nodes
    meshInit(g);

    step = 1;
    step_progress = 0;
    while (ferMesh3VerticesLen(g->mesh) < g->param.max_nodes){
        drawInputPoint(g);

        echl(g);

        if (fer_unlikely(step >= g->param.lambda)){
            createNewNode(g);
            step = 0;
            //ferMesh3DumpSVT(g->mesh, stdout, "1");

            if (g->verbosity >= 2
                    && fer_unlikely(step_progress % FER_GSRM_PROGRESS_REFRESH == 0)){
                PR_PROGRESS(g);
                step_progress = 0;
            }
            step_progress++;
        }

        decreaseErrCounter(g);

        step++;
    }

    if (g->verbosity >= 1){
        PR_PROGRESS(g);
        fprintf(stderr, "\n");
    }

    return 0;
}


int ferGSRMPostprocess(fer_gsrm_t *g)
{
    fer_real_t min, max, avg;

    // set up limits
    faceAreaStat(g, &min, &max, &avg);
    g->c->pp_min = min;
    g->c->pp_max = max;

    // Phase 1:
    // Remove incorrect faces from whole mesh.
    // Incorrect faces are those that have any of internal angle bigger
    // than treshold (.param.max_angle) or if dihedral angle with any other
    // face is smaller than treshold (.param.min_dangle)
    delIncorrectFaces(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -1- DIF:");
    }

    // Remove incorrect edges.
    // Incorrect edges are those that have (on one end) no incidenting
    // edge or if edge can't be used for face creation.
    delIncorrectEdges(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -1- DIE:");
    }

    // Merge edges.
    // Merge all pairs of edges that have common node that have no other
    // incidenting edges than the two.
    mergeEdges(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -1- ME: ");
    }

    // Finish surface
    finishSurface(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -1- FS: ");
    }else if (g->verbosity >= 2){
        PR_PROGRESS_PREFIX(g, " -1-");
    }


    // Phase 2:
    // Del incorrect edges again
    delIncorrectEdges(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -2- DIE:");
    }

    // finish surface
    finishSurface(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -2- FS: ");
    }else if (g->verbosity >= 2){
        PR_PROGRESS_PREFIX(g, " -2-");
    }


    // Phase 3:
    // delete lonely faces, edges and nodes
    delLonelyNodesEdgesFaces(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -3- DL: ");
    }

    // try finish surface again
    finishSurface(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -3- FS: ");
    }else if (g->verbosity >= 2){
        PR_PROGRESS_PREFIX(g, " -3-");
    }

    // Phase 4:
    // delete lonely faces, edges and nodes
    delLonelyNodesEdgesFaces(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -4- DL: ");
    }

    finishSurfaceEmbedTriangles(g);
    if (g->verbosity >= 3){
        PR_PROGRESS_PREFIX(g, " -4- FET:");
    }else if (g->verbosity >= 2){
        PR_PROGRESS_PREFIX(g, " -4-");
    }else if (g->verbosity >= 1){
        PR_PROGRESS(g);
        fprintf(stderr, "\n");
    }

    return 0;
}



static fer_gsrm_cache_t *cacheNew(void)
{
    fer_gsrm_cache_t *c;

    c = FER_ALLOC(fer_gsrm_cache_t);
    c->nearest[0] = c->nearest[1] = NULL;

    c->common_neighb_size = 3;
    c->common_neighb = FER_ALLOC_ARR(node_t *, c->common_neighb_size);
    c->common_neighb_len = 0;

    c->err_counter_mark = 0;
    c->err_counter_scale = FER_ONE;

    return c;
}

static void cacheDel(fer_gsrm_cache_t *c)
{
    free(c->common_neighb);
    free(c);
}



/** --- Node functions --- **/
static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v)
{
    node_t *n;

    n = FER_ALLOC(node_t);
    n->v = ferVec3Clone(v);

    // initialize mesh's vertex struct with weight vector
    ferMesh3VertexSetCoords(&n->vert, n->v);

    // initialize cubes struct with its own weight vector
    ferCubes3ElInit(&n->cubes, n->v);

    // add node into mesh
    ferMesh3AddVertex(g->mesh, &n->vert);
    // and add node into cubes
    ferCubes3Add(g->cubes, &n->cubes);

    // set error counter (and mark)
    n->err_counter = FER_ZERO;
    n->err_counter_mark = g->c->err_counter_mark;

    //DBG("n: %lx, vert: %lx (%g %g %g)", (long)n, (long)&n->vert,
    //    ferVec3X(&n->v), ferVec3Y(&n->v), ferVec3Z(&n->v));

    return n;
}

static void nodeDel(fer_gsrm_t *g, node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_mesh3_edge_t *edge;
    edge_t *e;
    int res;

    // remove node from mesh
    if (fer_unlikely(ferMesh3VertexEdgesLen(&n->vert) > 0)){
        // remove edges first
        list = ferMesh3VertexEdges(&n->vert);
        ferListForEachSafe(list, item, item_tmp){
            edge = ferMesh3EdgeFromVertexList(item);
            e = fer_container_of(edge, edge_t, edge);

            edgeDel(g, e);
        }
    }

    // then vertex
    res = ferMesh3RemoveVertex(g->mesh, &n->vert);
    if (fer_unlikely(res != 0)){
        DBG2("Node couldn't be removed from mesh - this shouldn't happen!");
        exit(-1);
    }

    ferVec3Del(n->v);

    // remove node from cubes
    ferCubes3Remove(g->cubes, &n->cubes);

    // Note: no need of deallocation of .vert and .cubes
    free(n);
}

static void nodeDel2(fer_mesh3_vertex_t *v, void *data)
{
    fer_gsrm_t *g = (fer_gsrm_t *)data;
    node_t *n;
    n = fer_container_of(v, node_t, vert);

    ferVec3Del(n->v);

    // remove node from cubes
    ferCubes3Remove(g->cubes, &n->cubes);

    free(n);
}

static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n)
{
    size_t mark, left;
    fer_real_t err;

    mark = n->err_counter_mark;
    left = g->c->err_counter_mark - mark;

    if (fer_likely(left > 0)){
        if (fer_likely(left == g->c->err_counter_mark)){
            // most of nodes in mesh are not touched while ECHL phase - so
            // scale factor can be cumulated and can be used directly
            // without pow() operation
            n->err_counter *= g->c->err_counter_scale;
        }else{
            err = FER_POW(g->param.beta, (fer_real_t)left);
            n->err_counter *= err;
        }
    }

    n->err_counter_mark = g->c->err_counter_mark;
}

static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    return n->err_counter;
}

static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s)
{
    nodeErrCounterApply(g, n);
    n->err_counter *= s;
    return n->err_counter;
}

static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    n->err_counter_mark = 0;
    return n->err_counter;
}

static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v)
{
    fer_real_t dist;

    dist = ferVec3Dist2(n->v, v);

    nodeErrCounterApply(g, n);
    n->err_counter += dist;
}

static void nodeErrCounterResetAll(fer_gsrm_t *g)
{
    g->c->err_counter_mark = 0;
    g->c->err_counter_scale = FER_ONE;
}

static void nodeErrCounterScaleAll(fer_gsrm_t *g)
{
    g->c->err_counter_scale *= g->param.beta;
    g->c->err_counter_mark++;
}




/** --- Edge functions --- **/
static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    edge_t *e;

    e = FER_ALLOC(edge_t);
    e->age = 0;

    ferMesh3AddEdge(g->mesh, &e->edge, &n1->vert, &n2->vert);

    //DBG("e: %lx, edge: %lx", (long)e, (long)&e->edge);

    return e;
}

static void edgeDel(fer_gsrm_t *g, edge_t *e)
{
    fer_mesh3_face_t *face;
    int res;

    // first remove incidenting faces
    while ((face = ferMesh3EdgeFace(&e->edge, 0)) != NULL){
        faceDel(g, fer_container_of(face, face_t, face));
    }

    // then remove edge itself
    res = ferMesh3RemoveEdge(g->mesh, &e->edge);
    if (fer_unlikely(res != 0)){
        DBG2("Can't remove edge - this shouldn'h happen!");
        exit(-1);
    }

    free(e);
}

static void edgeDel2(fer_mesh3_edge_t *edge, void *data)
{
    edge_t *e;
    e = fer_container_of(edge, edge_t, edge);
    free(e);
}




/** --- Face functions --- **/
static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n)
{
    face_t *f;
    fer_mesh3_edge_t *e2, *e3;
    int res;

    e2 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 0), &n->vert);
    e3 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 1), &n->vert);
    if (fer_unlikely(!e2 || !e3)){
        DBG2("Can't create face because *the* three nodes are not connected "
             " - this shouldn't happen!");
        return NULL;
    }

    f = FER_ALLOC(face_t);

    res = ferMesh3AddFace(g->mesh, &f->face, &e->edge, e2, e3);
    if (fer_unlikely(res != 0)){
        free(f);
        return NULL;
    }

    // TODO: check if face already exists

    return f;
}

static void faceDel(fer_gsrm_t *g, face_t *f)
{
    ferMesh3RemoveFace(g->mesh, &f->face);
    free(f);
}

static void faceDel2(fer_mesh3_face_t *face, void *data)
{
    face_t *f;
    f = fer_container_of(face, face_t, face);
    free(f);
}




static void meshInit(fer_gsrm_t *g)
{
    fer_vec3_t *v;
    node_t *n;
    size_t i;

    for (i = 0; i < 3; i++){
        // obtain input signal
        v = ferPC3ItGet(&g->isit);

        // create new node
        n = nodeNew(g, v);

        // move to next point
        ferPC3ItNext(&g->isit);
    }
}

static void drawInputPoint(fer_gsrm_t *g)
{
    if (ferPC3ItEnd(&g->isit)){
        // if iterator is at the end permutate point cloud again
        ferPC3Permutate(g->is);
        // and re-initialize iterator
        ferPC3ItInit(&g->isit, g->is);
    }
    g->c->is = ferPC3ItGet(&g->isit);
    ferPC3ItNext(&g->isit);
}



/** --- ECHL functions --- **/
static void echl(fer_gsrm_t *g)
{
    fer_cubes3_el_t *el[2];

    // 1. Find two nearest nodes
    ferCubes3Nearest(g->cubes, g->c->is, 2, el);
    g->c->nearest[0] = fer_container_of(el[0], node_t, cubes);
    g->c->nearest[1] = fer_container_of(el[1], node_t, cubes);

    // 2. Updates winners error counter
    nodeErrCounterInc(g, g->c->nearest[0], g->c->is);

    // 3. Connect winning nodes
    echlConnectNodes(g);

    // 4. Move winning node and its neighbors towards input signal
    echlMove(g);

    // 5. Update all edges emitating from winning node
    echlUpdate(g);
}

static void echlCommonNeighbors(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    fer_list_t *list1, *list2;
    fer_list_t *item1, *item2;
    fer_mesh3_edge_t *edge1, *edge2;
    fer_mesh3_vertex_t *o1, *o2;
    node_t *n;
    size_t len;

    // allocate enough memory for common neighbors
    if (g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n1->vert)
            && g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n2->vert)){
        len = ferMesh3VertexEdgesLen(&n1->vert);
        len = FER_MIN(len, ferMesh3VertexEdgesLen(&n2->vert));

        g->c->common_neighb = FER_REALLOC_ARR(g->c->common_neighb, node_t *, len);
        g->c->common_neighb_size = len;
    }

    list1 = ferMesh3VertexEdges(&n1->vert);
    list2 = ferMesh3VertexEdges(&n2->vert);
    len = 0;
    ferListForEach(list1, item1){
        edge1 = ferMesh3EdgeFromVertexList(item1);
        o1 = ferMesh3EdgeVertex(edge1, 0);
        if (o1 == &n1->vert)
            o1 = ferMesh3EdgeVertex(edge1, 1);

        ferListForEach(list2, item2){
            edge2 = ferMesh3EdgeFromVertexList(item2);
            o2 = ferMesh3EdgeVertex(edge2, 0);
            if (o2 == &n2->vert)
                o2 = ferMesh3EdgeVertex(edge2, 1);

            if (o1 == o2){
                n = fer_container_of(o1, node_t, vert);
                g->c->common_neighb[len] = n;
                len++;
            }
        }
    }

    g->c->common_neighb_len = len;
}

static void echlRemoveThales(fer_gsrm_t *g, edge_t *e, node_t *n1, node_t *n2)
{
    node_t *nb;
    size_t i, len;

    len = g->c->common_neighb_len;
    for (i=0; i < len; i++){
        nb = g->c->common_neighb[i];

        if (ferVec3Angle(n1->v, nb->v, n2->v) > M_PI_2){
            // remove edge
            edgeDel(g, e);
            return;
        }
    }
}

static void echlRemoveNeighborsEdges(fer_gsrm_t *g)
{
    size_t i, j, len;
    fer_mesh3_edge_t *edge;
    node_t **ns;
    edge_t *e;

    ns = g->c->common_neighb;
    len = g->c->common_neighb_len;
    if (len == 0)
        return;

    for (i = 0; i < len; i++){
        for (j = i + 1; j < len; j++){
            edge = ferMesh3VertexCommonEdge(&ns[i]->vert, &ns[j]->vert);
            if (edge != NULL){
                e = fer_container_of(edge, edge_t, edge);
                edgeDel(g, e);
            }
        }
    }
}

static void echlCreateFaces(fer_gsrm_t *g, edge_t *e)
{
    size_t i, len;
    node_t **ns;

    len = FER_MIN(g->c->common_neighb_len, 2);
    ns = g->c->common_neighb;
    for (i = 0; i < len; i++){
        faceNew(g, e, ns[i]);
    }
}

static void echlConnectNodes(fer_gsrm_t *g)
{
    fer_mesh3_edge_t *edge;
    edge_t *e;
    node_t *n1, *n2;

    n1 = g->c->nearest[0];
    n2 = g->c->nearest[1];

    // get edge connecting n1 and n2
    e = NULL;
    edge = ferMesh3VertexCommonEdge(&n1->vert, &n2->vert);
    if (edge){
        e = fer_container_of(edge, edge_t, edge);
    }

    // get common neighbors
    echlCommonNeighbors(g, n1, n2);

    if (e != NULL){
        //DBG2("Nodes are connected");

        // set age of edge to zero
        e->age = 0;

        // Remove edge if opposite node lies inside thales sphere
        echlRemoveThales(g, e, n1, n2);
    }else{
        //DBG2("Nodes are NOT connected");

        // remove all edges that connect common neighbors
        echlRemoveNeighborsEdges(g);

        // create new edge between n1 and n2
        e = edgeNew(g, n1, n2);

        // create faces with common neighbors
        echlCreateFaces(g, e);
    }
}

_fer_inline void echlMoveNode(fer_gsrm_t *g, node_t *n, fer_real_t k)
{
    fer_vec3_t v;

    // compute shifting
    ferVec3Sub2(&v, g->c->is, n->v);
    ferVec3Scale(&v, k);

    // move node
    ferVec3Add(n->v, &v);
}

static void echlMove(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *wvert, *vert;
    node_t *wn;

    wn = g->c->nearest[0];
    wvert = &wn->vert;

    // move winning node
    echlMoveNode(g, wn, g->param.eb);

    // move nodes connected with the winner
    list = ferMesh3VertexEdges(wvert);
    ferListForEach(list, item){
        edge = ferMesh3EdgeFromVertexList(item);
        vert = ferMesh3EdgeOtherVertex(edge, wvert);

        echlMoveNode(g, fer_container_of(vert, node_t, vert), g->param.en);
    }
}

static void echlUpdate(fer_gsrm_t *g)
{
    node_t *wn;
    edge_t *e;
    fer_list_t *list, *item, *tmp_item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *vert;

    wn = g->c->nearest[0];

    list = ferMesh3VertexEdges(&wn->vert);
    ferListForEachSafe(list, item, tmp_item){
        edge = ferMesh3EdgeFromVertexList(item);
        e = fer_container_of(edge, edge_t, edge);

        // increment age of edge
        e->age++;

        // if age of edge is above treshold remove edge and nodes which
        // remain unconnected
        if (e->age > g->param.age_max){
            // get other node than winning one
            vert = ferMesh3EdgeOtherVertex(edge, &wn->vert);

            // delete edge
            edgeDel(g, e);

            // check if n is connected in mesh, if not delete it
            if (ferMesh3VertexEdgesLen(vert) == 0){
                nodeDel(g, fer_container_of(vert, node_t, vert));
            }
        }
    }

    // check if winning node remains connected
    if (ferMesh3VertexEdgesLen(&wn->vert) == 0){
        nodeDel(g, wn);
    }
}




/** --- Create New Node functions --- **/
static node_t *nodeWithHighestErrCounter(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_real_t max_err, err;
    node_t *max_n, *n;
    fer_mesh3_vertex_t *vert;

    max_err = FER_REAL_MIN;
    list = ferMesh3Vertices(g->mesh);
    ferListForEach(list, item){
        vert = ferListEntry(item, fer_mesh3_vertex_t, list);
        n    = fer_container_of(vert, node_t, vert);
        err  = nodeErrCounterReset(g, n);

        if (err > max_err){
            max_err = err;
            max_n   = n;
        }
    }

    nodeErrCounterResetAll(g);

    return max_n;
}

static node_t *nodesNeighborWithHighestErrCounter(fer_gsrm_t *g, node_t *sq)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *other_vert;
    fer_real_t err, max_err;
    node_t *n, *max_n;

    max_err = FER_REAL_MIN;
    list = ferMesh3VertexEdges(&sq->vert);
    ferListForEach(list, item){
        edge = ferMesh3EdgeFromVertexList(item);
        other_vert = ferMesh3EdgeOtherVertex(edge, &sq->vert);
        n = fer_container_of(other_vert, node_t, vert);
        err = nodeErrCounter(g, n);
        if (err > max_err){
            max_err = err;
            max_n   = n;
        }
    }

    return n;
}

static node_t *createNewNode2(fer_gsrm_t *g, node_t *sq, node_t *sf)
{
    node_t *sr;
    fer_vec3_t v;

    ferVec3Add2(&v, sq->v, sf->v);
    ferVec3Scale(&v, FER_REAL(0.5));

    sr = nodeNew(g, &v);

    return sr;
}

static void createNewNode(fer_gsrm_t *g)
{
    node_t *sq, *sf, *sr;
    fer_mesh3_edge_t *edge;
    fer_real_t err;


    // get node with highest error counter and its neighbor with highest
    // error counter
    sq = nodeWithHighestErrCounter(g);
    sf = nodesNeighborWithHighestErrCounter(g, sq);
    if (!sq || !sf){
        DBG2("Can't create new node, because sq has no neighbors");
        return;
    }

    //DBG("sq: %lx, sf: %lx", (long)sq, (long)sf);

    // delete common edge of sq and sf
    edge = ferMesh3VertexCommonEdge(&sq->vert, &sf->vert);
    if (edge){
        edgeDel(g, fer_container_of(edge, edge_t, edge));
    }

    // create new node
    sr = createNewNode2(g, sq, sf);

    // set up error counters of sq, sf and sr
    err  = nodeErrCounterScale(g, sq, g->param.alpha);
    err += nodeErrCounterScale(g, sf, g->param.alpha);
    err *= FER_REAL(0.5);
    sr->err_counter = err;

    // create edges sq-sr and sf-sr
    edgeNew(g, sq, sr);
    edgeNew(g, sf, sr);
}




static void decreaseErrCounter(fer_gsrm_t *g)
{
    nodeErrCounterScaleAll(g);
}




/** --- Postprocessing functions --- */
static void faceAreaStat(fer_gsrm_t *g, fer_real_t *_min, fer_real_t *_max,
                         fer_real_t *_avg)
{
    fer_real_t area, min, max, avg;
    fer_list_t *list, *item;
    fer_mesh3_face_t *face;

    max = avg = FER_ZERO;
    min = FER_REAL_MAX;
    list = ferMesh3Faces(g->mesh);
    ferListForEach(list, item){
        face = ferListEntry(item, fer_mesh3_face_t, list);
        area = ferMesh3FaceArea2(face);

        if (area < min){
            min = area;
        }

        if (area > max){
            max = area;
        }

        avg += area;
    }

    avg /= (fer_real_t)ferMesh3FacesLen(g->mesh);

    *_min = min;
    *_max = max;
    *_avg = avg;
}

static void delIncorrectFaces(fer_gsrm_t *g)
{
    fer_mesh3_vertex_t *vs[4];
    fer_list_t *list, *item, *item_tmp;
    fer_mesh3_face_t *face, *faces[2];
    fer_mesh3_edge_t *edge;
    face_t *f, *fs[2];
    fer_real_t dangle;

    // iterate over all faces
    list = ferMesh3Faces(g->mesh);
    ferListForEachSafe(list, item, item_tmp){
        face = ferListEntry(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(face, vs);

        // check internal angle of face
        if (!faceCheckAngle(g, vs[0], vs[1], vs[2])){
            f = fer_container_of(face, face_t, face);
            faceDel(g, f);
        }
    }

    // iterate over all edges
    list = ferMesh3Edges(g->mesh);
    ferListForEachSafe(list, item, item_tmp){
        edge = ferListEntry(item, fer_mesh3_edge_t, list);

        if (ferMesh3EdgeFacesLen(edge) == 2){
            // get incidenting faces
            faces[0] = ferMesh3EdgeFace(edge, 0);
            faces[1] = ferMesh3EdgeFace(edge, 1);

            // get end points of edge
            vs[0] = ferMesh3EdgeVertex(edge, 0);
            vs[1] = ferMesh3EdgeVertex(edge, 1);

            // get remaining two points defining the two faces
            vs[2] = ferMesh3FaceOtherVertex(faces[0], vs[0], vs[1]);
            vs[3] = ferMesh3FaceOtherVertex(faces[1], vs[0], vs[1]);

            // check dihedral angle between faces and if it is smaller than
            // treshold delete one of them
            dangle = ferVec3DihedralAngle(vs[2]->v, vs[0]->v, vs[1]->v, vs[3]->v);
            if (dangle < g->param.min_dangle){
                fs[0] = fer_container_of(faces[0], face_t, face);
                fs[1] = fer_container_of(faces[1], face_t, face);
                delFacesDangle(g, fs[0], fs[1]);
            }
        }
    }
}

static void delIncorrectEdges(fer_gsrm_t *g)
{
    int madechange;
    fer_list_t *list, *item, *item_tmp;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *vs[2];
    edge_t *e;

    madechange = 1;
    while (madechange){
        madechange = 0;

        list = ferMesh3Edges(g->mesh);
        ferListForEachSafe(list, item, item_tmp){
            edge = ferListEntry(item, fer_mesh3_edge_t, list);
            vs[0] = ferMesh3EdgeVertex(edge, 0);
            vs[1] = ferMesh3EdgeVertex(edge, 1);

            if (ferMesh3VertexEdgesLen(vs[0]) == 1
                    || ferMesh3VertexEdgesLen(vs[1]) == 1
                    || edgeNotUsable(edge)){
                e = fer_container_of(edge, edge_t, edge);
                edgeDel(g, e);
                madechange = 1;
            }
        }
    }
}

static void mergeEdges(fer_gsrm_t *g)
{
    int madechange;
    fer_list_t *list, *item, *item_tmp, *list2;
    fer_mesh3_vertex_t *vert;
    fer_mesh3_vertex_t *vs[2];
    fer_mesh3_edge_t *edge[2];
    fer_real_t angle;
    edge_t *e[2];
    node_t *n[2];

    madechange = 1;
    while (madechange){
        madechange = 0;

        list = ferMesh3Vertices(g->mesh);
        ferListForEachSafe(list, item, item_tmp){
            vert = ferListEntry(item, fer_mesh3_vertex_t, list);
            if (ferMesh3VertexEdgesLen(vert) == 2){
                // get incidenting edges
                list2 = ferMesh3VertexEdges(vert);
                edge[0] = ferMesh3EdgeFromVertexList(ferListNext(list2));
                edge[1] = ferMesh3EdgeFromVertexList(ferListPrev(list2));

                // only edges that don't incident with any face can be
                // merged
                if (ferMesh3EdgeFacesLen(edge[0]) == 0
                        && ferMesh3EdgeFacesLen(edge[1]) == 0){
                    // get and points of edges
                    vs[0] = ferMesh3EdgeOtherVertex(edge[0], vert);
                    vs[1] = ferMesh3EdgeOtherVertex(edge[1], vert);

                    // compute angle between edges and check if it is big
                    // enough to perform merging
                    angle = ferVec3Angle(vs[0]->v, vert->v, vs[1]->v);
                    if (angle > g->param.angle_merge_edges){
                        // finally, we can merge edges
                        e[0] = fer_container_of(edge[0], edge_t, edge);
                        e[1] = fer_container_of(edge[1], edge_t, edge);
                        n[0] = fer_container_of(vert, node_t, vert);

                        // first, remove edges
                        edgeDel(g, e[0]);
                        edgeDel(g, e[1]);

                        // then remove node
                        nodeDel(g, n[0]);

                        // and finally create new node
                        n[0] = fer_container_of(vs[0], node_t, vert);
                        n[1] = fer_container_of(vs[1], node_t, vert);
                        edgeNew(g, n[0], n[1]);
                        madechange = 1;
                    }
                }
            }
        }
    }
}

static void finishSurface(fer_gsrm_t *g)
{
    int madechange;
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    edge_t *e;

    madechange = 1;
    while (madechange){
        madechange = 0;

        list = ferMesh3Edges(g->mesh);
        ferListForEach(list, item){
            edge = ferListEntry(item, fer_mesh3_edge_t, list);

            // if it is border edge
            if (ferMesh3EdgeFacesLen(edge) == 1){
                e = fer_container_of(edge, edge_t, edge);

                // try to finish triangle face
                if (finishSurfaceTriangle(g, e) == 0){
                    madechange = 1;

                    // try to create face incidenting with edge
                }else if (finishSurfaceNewFace(g, e) == 0){
                    madechange = 1;
                }
            }
        }
    }
}

static void delLonelyNodesEdgesFaces(fer_gsrm_t *g)
{
    fer_list_t *list, *item, *item_tmp;
    fer_mesh3_face_t *face;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *vert;
    fer_mesh3_edge_t *es[3];
    face_t *f;
    edge_t *e;
    node_t *n;

    list = ferMesh3Faces(g->mesh);
    ferListForEachSafe(list, item, item_tmp){
        face = ferListEntry(item, fer_mesh3_face_t, list);
        es[0] = ferMesh3FaceEdge(face, 0);
        es[1] = ferMesh3FaceEdge(face, 1);
        es[2] = ferMesh3FaceEdge(face, 2);

        if (ferMesh3EdgeFacesLen(es[0]) == 1
                && ferMesh3EdgeFacesLen(es[1]) == 1
                && ferMesh3EdgeFacesLen(es[2]) == 1){
            f = fer_container_of(face, face_t, face);
            faceDel(g, f);
        }

    }


    list = ferMesh3Edges(g->mesh);
    ferListForEachSafe(list, item, item_tmp){
        edge = ferListEntry(item, fer_mesh3_edge_t, list);
        if (ferMesh3EdgeFacesLen(edge) == 0){
            e = fer_container_of(edge, edge_t, edge);
            edgeDel(g, e);
        }
    }


    list = ferMesh3Vertices(g->mesh);
    ferListForEachSafe(list, item, item_tmp){
        vert = ferListEntry(item, fer_mesh3_vertex_t, list);
        if (ferMesh3VertexEdgesLen(vert) == 0){
            n = fer_container_of(vert, node_t, vert);
            nodeDel(g, n);
        }
    }
}

static void finishSurfaceEmbedTriangles(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *es[3];
    fer_mesh3_vertex_t *vs[3];
    edge_t *e;
    node_t *n[3];
    size_t i;

    list = ferMesh3Edges(g->mesh);
    ferListForEach(list, item){
        es[0] = ferListEntry(item, fer_mesh3_edge_t, list);

        vs[0] = ferMesh3EdgeVertex(es[0], 0);
        vs[1] = ferMesh3EdgeVertex(es[0], 1);
        n[0]  = fer_container_of(vs[0], node_t, vert);
        n[1]  = fer_container_of(vs[1], node_t, vert);

        echlCommonNeighbors(g, n[0], n[1]);

        for (i = 0; i < g->c->common_neighb_len; i++){
            n[2] = g->c->common_neighb[i];

            es[1] = ferMesh3VertexCommonEdge(vs[0], &n[2]->vert);
            if (!es[1] || ferMesh3EdgeFacesLen(es[1]) != 1)
                continue;

            es[2] = ferMesh3VertexCommonEdge(vs[1], &n[2]->vert);
            if (!es[2] || ferMesh3EdgeFacesLen(es[2]) != 1)
                continue;

            e = fer_container_of(es[0], edge_t, edge);
            faceNew(g, e, n[2]);
            break;
        }
    }
}

static int faceCheckAngle(fer_gsrm_t *g, fer_mesh3_vertex_t *v1,
                          fer_mesh3_vertex_t *v2, fer_mesh3_vertex_t *v3)
{
    if (ferVec3Angle(v1->v, v2->v, v3->v) > g->param.max_angle
            || ferVec3Angle(v2->v, v3->v, v1->v) > g->param.max_angle
            || ferVec3Angle(v3->v, v1->v, v2->v) > g->param.max_angle)
        return 0;
    return 1;
}

static void delFacesDangle(fer_gsrm_t *g, face_t *f1, face_t *f2)
{
    int f1_neighbors, f2_neighbors;
    size_t i;

    f1_neighbors = f2_neighbors = 0;
    for (i = 0; i < 3; i++){
        if (ferMesh3EdgeFacesLen(ferMesh3FaceEdge(&f1->face, i)) == 2)
            f1_neighbors++;
        if (ferMesh3EdgeFacesLen(ferMesh3FaceEdge(&f2->face, i)) == 2)
            f2_neighbors++;
    }

    if (f1_neighbors < f2_neighbors){
        faceDel(g, f1);
    }else if (f1_neighbors > f2_neighbors){
        faceDel(g, f2);
    }else{
        if (ferMesh3FaceArea2(&f1->face) < ferMesh3FaceArea2(&f2->face)){
            faceDel(g, f1);
        }else{
            faceDel(g, f2);
        }
    }
}

static int edgeNotUsable(fer_mesh3_edge_t *e)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *vs[2];
    fer_mesh3_edge_t *edge;
    size_t i, usable_edges;

    vs[0] = ferMesh3EdgeVertex(e, 0);
    vs[1] = ferMesh3EdgeVertex(e, 1);
    for (i = 0; i < 2; i++){
        usable_edges = 0;

        list = ferMesh3VertexEdges(vs[0]);
        ferListForEach(list, item){
            edge = ferMesh3EdgeFromVertexList(item);
            if (ferMesh3EdgeFacesLen(edge) < 2)
                usable_edges++;
        }

        if (usable_edges == 1)
            return 1;
    }

    return 0;
}

static int finishSurfaceTriangle(fer_gsrm_t *g, edge_t *e)
{
    size_t i;
    int ret;
    fer_mesh3_face_t *face;
    fer_mesh3_edge_t *es[2];
    fer_mesh3_vertex_t *vs[3];
    node_t *n[3];
    node_t *s;
    fer_real_t dangle;

    ret = -1;

    // get nodes of already existing face
    face = ferMesh3EdgeFace(&e->edge, 0);
    vs[0] = ferMesh3EdgeVertex(&e->edge, 0);
    vs[1] = ferMesh3EdgeVertex(&e->edge, 1);
    vs[2] = ferMesh3FaceOtherVertex(face, vs[0], vs[1]);

    n[0] = fer_container_of(vs[0], node_t, vert);
    n[1] = fer_container_of(vs[1], node_t, vert);
    n[2] = fer_container_of(vs[2], node_t, vert);
    echlCommonNeighbors(g, n[0], n[1]);

    // all common neighbors of n[0,1] form triangles
    for (i = 0; i < g->c->common_neighb_len; i++){
        if (ferMesh3EdgeFacesLen(&e->edge) == 2)
            break;

        s = g->c->common_neighb[i];
        if (s != n[2]){
            // check angle
            if (!faceCheckAngle(g, vs[0], vs[1], &s->vert))
                continue;

            // check dihedral angle
            dangle = ferVec3DihedralAngle(vs[2]->v, vs[0]->v, vs[1]->v, s->v);
            if (dangle < g->param.min_dangle)
                continue;

            // check if face can be created inside triplet of edges
            es[0] = ferMesh3VertexCommonEdge(vs[0], &s->vert);
            es[1] = ferMesh3VertexCommonEdge(vs[1], &s->vert);
            if ((es[0] && ferMesh3EdgeFacesLen(es[0]) == 2)
                    || (es[1] && ferMesh3EdgeFacesLen(es[1]) == 2))
                continue;

            // create edges if necessary
            if (!es[0]){
                edgeNew(g, n[0], s);
            }

            if (!es[1]){
                edgeNew(g, n[1], s);
            }

            faceNew(g, e, s);
            ret = 0;
        }
    }

    return ret;
}

static edge_t *finishSurfaceGetEdge(edge_t *e, node_t *n)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    edge_t *s, *s2;

    s2 = NULL;
    list = ferMesh3VertexEdges(&n->vert);
    ferListForEach(list, item){
        edge = ferMesh3EdgeFromVertexList(item);

        if (ferMesh3EdgeFacesLen(edge) == 0)
            return NULL;

        s = fer_container_of(edge, edge_t, edge);
        if (s != e && ferMesh3EdgeFacesLen(edge) == 1){
            // there is more than two border edges incidenting with n
            if (s2 != NULL)
                return NULL;
            s2 = s;
        }
    }

    return s2;
}

static int finishSurfaceNewFace(fer_gsrm_t *g, edge_t *e)
{
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *vs[2];
    node_t *ns[2], *ns2[2];
    edge_t *es[2];
    edge_t *e2, *e_new;

    // get start and end points
    vs[0] = ferMesh3EdgeVertex(&e->edge, 0);
    vs[1] = ferMesh3EdgeVertex(&e->edge, 1);
    ns[0] = fer_container_of(vs[0], node_t, vert);
    ns[1] = fer_container_of(vs[1], node_t, vert);

    es[0] = finishSurfaceGetEdge(e, ns[0]);
    es[1] = finishSurfaceGetEdge(e, ns[1]);

    if (es[0] != NULL && es[1] != NULL){
        // try to create both faces

        // obtain opossite nodes than wich already have from edge e
        vs[0] = ferMesh3EdgeOtherVertex(&es[0]->edge, &ns[0]->vert);
        vs[1] = ferMesh3EdgeOtherVertex(&es[1]->edge, &ns[1]->vert);
        ns2[0] = fer_container_of(vs[0], node_t, vert);
        ns2[1] = fer_container_of(vs[1], node_t, vert);

        e_new = NULL;
        edge = ferMesh3VertexCommonEdge(&ns[0]->vert, &ns2[1]->vert);
        if (edge){
            e2 = fer_container_of(edge, edge_t, edge);
        }else{
            e_new = e2 = edgeNew(g, ns[0], ns2[1]);
        }
        if (finishSurfaceTriangle(g, e) != 0){
            if (e_new)
                edgeDel(g, e_new);
            return -1;
        }

        e_new = NULL;
        if (!ferMesh3VertexCommonEdge(&ns2[0]->vert, &ns2[1]->vert)){
            e_new = edgeNew(g, ns2[0], ns2[1]);
        }

        if (finishSurfaceTriangle(g, e2) != 0){
            if (e_new)
                edgeDel(g, e_new);
            return -1;
        }

        return 0;
    }else if (es[0] != NULL){
        vs[0] = ferMesh3EdgeOtherVertex(&es[0]->edge, &ns[0]->vert);
        ns2[0] = fer_container_of(vs[0], node_t, vert);

        e_new = NULL;
        edge = ferMesh3VertexCommonEdge(&ns[1]->vert, &ns2[0]->vert);
        if (!edge){
            e_new = edgeNew(g, ns[1], ns2[0]);
        }
        if (finishSurfaceTriangle(g, e) != 0){
            if (e_new)
                edgeDel(g, e_new);
            return -1;
        }
        return 0;

    }else if (es[1] != NULL){
        vs[1] = ferMesh3EdgeOtherVertex(&es[1]->edge, &ns[1]->vert);
        ns2[1] = fer_container_of(vs[1], node_t, vert);

        e_new = NULL;
        edge = ferMesh3VertexCommonEdge(&ns[0]->vert, &ns2[1]->vert);
        if (!edge){
            e_new = edgeNew(g, ns[0], ns2[1]);
        }
        if (finishSurfaceTriangle(g, e) != 0){
            if (e_new)
                edgeDel(g, e_new);
            return -1;
        }
        return 0;
    }

    return -1;
}
/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <boruvka/net.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

bor_net_node_t *ferNetNodeNew(void)
{
    bor_net_node_t *n;
    n = FER_ALLOC(bor_net_node_t);
    return n;
}

void ferNetNodeDel(bor_net_node_t *v)
{
    FER_FREE(v);
}

bor_net_edge_t *ferNetNodeCommonEdge(const bor_net_node_t *v1,
                                       const bor_net_node_t *v2)
{
    bor_list_t *item;
    bor_list_m_t *mitem;
    const bor_net_node_t *vtmp;
    bor_net_edge_t *e;

    // set v1 as node with less edges
    if (ferNetNodeEdgesLen(v2) < ferNetNodeEdgesLen(v1)){
        FER_SWAP(v1, v2, vtmp);
    }

    FER_LIST_FOR_EACH(&v1->edges, item){
        mitem = ferListMFromList(item);
        e = FER_LIST_M_ENTRY(item, bor_net_edge_t, nlist, mitem->mark);
        if (v2 == ferNetEdgeNode(e, 0)
                || v2 == ferNetEdgeNode(e, 1)){
            return e;
        }
    }

    return NULL;
}


bor_net_edge_t *ferNetEdgeNew(void)
{
    bor_net_edge_t *e;
    e = FER_ALLOC(bor_net_edge_t);
    return e;
}

void ferNetEdgeDel(bor_net_edge_t *e)
{
    FER_FREE(e);
}

/** Returns true if two given edges have exactly one common node */
_fer_inline int ferNetEdgeTriCheckCommon(const bor_net_edge_t *e1,
                                           const bor_net_edge_t *e2)
{
    if (e1->n[0] == e2->n[0]){
        if (e1->n[1] == e2->n[1])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->n[1] == e2->n[1]){
        if (e1->n[0] == e2->n[0])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->n[1] == e2->n[0]){
        if (e1->n[0] == e2->n[1])
            return 0; // e1 and e2 have two common vertices
    }else{
        return 0; // e1 and e2 have no common node
    }
    return 1;
}

int ferNetEdgeTriCheck(const bor_net_edge_t *e1,
                        const bor_net_edge_t *e2,
                        const bor_net_edge_t *e3)
{
    // 1) Any two edges must have exactly one common node.
    // 2) Start and end vertices must differ (within one edge)
    // I think that if these two preconditions hold then it is certain that
    // edges form triangle.

    return ferNetEdgeTriCheckCommon(e1, e2)
                && ferNetEdgeTriCheckCommon(e1, e3)
                && ferNetEdgeTriCheckCommon(e2, e3);
}






bor_net_t *ferNetNew(void)
{
    bor_net_t *m;
    m = FER_ALLOC(bor_net_t);

    ferListInit(&m->nodes);
    m->nodes_len = 0;
    ferListInit(&m->edges);
    m->edges_len = 0;

    return m;
}

void ferNetDel(bor_net_t *m)
{
    ferNetDel2(m, NULL, NULL, NULL, NULL);
}

void ferNetDel2(bor_net_t *m,
                  void (*delnode)(bor_net_node_t *, void *), void *vdata,
                  void (*deledge)(bor_net_edge_t *, void *), void *edata)
{
    bor_net_node_t *v;
    bor_net_edge_t *e;
    bor_list_t *item;

    // disedgeect all edges
    while (!ferListEmpty(&m->edges)){
        item = ferListNext(&m->edges);
        e = FER_LIST_ENTRY(item, bor_net_edge_t, list);
        ferNetRemoveEdge(m, e);

        if (deledge){
            deledge(e, edata);
        }
    }

    // disedgeect all vertices
    while (!ferListEmpty(&m->nodes)){
        item = ferListNext(&m->nodes);
        v = FER_LIST_ENTRY(item, bor_net_node_t, list);
        ferNetRemoveNode(m, v);

        if (delnode){
            delnode(v, vdata);
        }
    }

    FER_FREE(m);
}

void ferNetAddNode(bor_net_t *m, bor_net_node_t *v)
{
    ferListAppend(&m->nodes, &v->list);
    m->nodes_len++;

    ferListInit(&v->edges);
    v->edges_len = 0;
}

int ferNetRemoveNode(bor_net_t *m, bor_net_node_t *v)
{
    if (!ferListEmpty(&v->edges))
        return -1;

    ferListDel(&v->list);
    m->nodes_len--;
    return 0;
}

void ferNetAddEdge(bor_net_t *m, bor_net_edge_t *e,
                     bor_net_node_t *start, bor_net_node_t *end)
{
    // assign start and end point
    e->n[0] = start;
    e->n[1] = end;

    // append edge to list of edges in vertices
    e->nlist[0].mark = 0;
    ferListAppend(&start->edges, ferListMAsList(&e->nlist[0]));
    start->edges_len++;
    e->nlist[1].mark = 1;
    ferListAppend(&end->edges, ferListMAsList(&e->nlist[1]));
    end->edges_len++;

    // add edge to list of all edges
    ferListAppend(&m->edges, &e->list);
    m->edges_len++;
}

void ferNetRemoveEdge(bor_net_t *m, bor_net_edge_t *e)
{
    // remove edge from lists in vertices
    ferListDel(ferListMAsList(&e->nlist[0]));
    e->n[0]->edges_len--;
    ferListDel(ferListMAsList(&e->nlist[1]));
    e->n[1]->edges_len--;

    // remove edge from list of all edges
    ferListDel(&e->list);
    m->edges_len--;
}


void ferNetTraverseNode(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_node_t *n, void *data),
                        void *data)
{
    bor_list_t *item;
    bor_net_node_t *n;

    FER_LIST_FOR_EACH(&net->nodes, item){
        n = FER_LIST_ENTRY(item, bor_net_node_t, list);
        cb(net, n, data);
    }
}

void ferNetTraverseEdge(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_edge_t *e, void *data),
                        void *data)
{
    bor_list_t *item;
    bor_net_edge_t *e;

    FER_LIST_FOR_EACH(&net->edges, item){
        e = FER_LIST_ENTRY(item, bor_net_edge_t, list);
        cb(net, e, data);
    }
}


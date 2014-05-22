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

#include <boruvka/hmap.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#define BOR_HMAP_INITIAL_SIZE 100


struct _bor_hmap_el_t {
    bor_list_t list;
    void *key;
};

static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata);

bor_hmap_t *borHMapNew(size_t size,
                       bor_hmap_hash_fn hash_func,
                       bor_hmap_eq_fn eq_func,
                       void *userdata)
{
    bor_hmap_t *hmap;
    size_t i;

    hmap = BOR_ALLOC(bor_hmap_t);
    hmap->table = BOR_ALLOC_ARR(bor_list_t, size);
    hmap->size = size;
    hmap->resizable = 0;
    hmap->num_elements = 0;

    hmap->hash = hash_func;
    hmap->eq   = eq_func;
    hmap->data = userdata;

    if (!hmap->eq)
        hmap->eq = _eq;

    for (i = 0; i < hmap->size; i++){
        borListInit(hmap->table + i);
    }

    return hmap;
}

bor_hmap_t *borHMapNewResizable(bor_hmap_hash_fn hash_func,
                                bor_hmap_eq_fn eq_func,
                                void *userdata)
{
    bor_hmap_t *hmap;
    size_t size;

    size = borHMapNextPrime(BOR_HMAP_INITIAL_SIZE);
    hmap = borHMapNew(size, hash_func, eq_func, userdata);
    hmap->resizable = 1;

    return hmap;
}

void borHMapDel(bor_hmap_t *h)
{
    size_t i;
    bor_list_t *list, *item;

    for (i = 0; i < h->size; i++){
        list = &h->table[i];
        while (!borListEmpty(list)){
            item = borListNext(list);
            borListDel(item);
        }
    }

    BOR_FREE(h->table);
    BOR_FREE(h);
}

bor_list_t *borHMapIDGet(const bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    bor_list_t *item;

    BOR_LIST_FOR_EACH(&m->table[id], item){
        if (m->eq(key1, item, m->data))
            return item;
    }

    return NULL;
}

void borHMapGather(bor_hmap_t *m, bor_list_t *list)
{
    size_t i;
    bor_list_t *item;

    for (i = 0; i < m->size; i++){
        while (!borListEmpty(&m->table[i])){
            item = borListNext(&m->table[i]);
            borListDel(item);
            borListAppend(list, item);
        }
    }
    m->num_elements = 0;
}

void borHMapResize(bor_hmap_t *m, size_t size)
{
    bor_list_t *old_table, *item;
    size_t old_size;
    size_t i;

    // remember old table and old size
    old_table = m->table;
    old_size  = m->size;

    // create a new empty table
    m->table = BOR_ALLOC_ARR(bor_list_t, size);
    m->size  = size;
    m->num_elements = 0;

    for (i = 0; i < m->size; i++){
        borListInit(m->table + i);
    }

    for (i = 0; i < old_size; i++){
        while (!borListEmpty(&old_table[i])){
            // remove item from the old table
            item = borListNext(&old_table[i]);
            borListDel(item);

            // insert it into a new table
            borHMapPutNoResize(m, item);
        }
    }

    BOR_FREE(old_table);
}

static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata)
{
    return key1 == key2;
}

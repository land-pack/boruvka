#include <boruvka/pairheap.h>

#define HHEAP bor_pairheap_t
#define HNODE bor_pairheap_node_t
#define HFUNC(name) ferPairHeap ## name

#include "bench-heap.c"

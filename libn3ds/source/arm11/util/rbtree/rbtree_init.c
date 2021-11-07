#include "arm11/util/rbtree.h"

void
rbtree_init(rbtree_t                 *tree,
            rbtree_node_comparator_t comparator)
{
  tree->root       = NULL;
  tree->comparator = comparator;
  tree->size       = 0;
}

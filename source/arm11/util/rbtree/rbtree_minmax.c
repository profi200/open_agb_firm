#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

static inline rbtree_node_t*
do_minmax(const rbtree_t *tree,
          int            max)
{
  rbtree_node_t *node = tree->root;

  if(node == NULL)
    return NULL;

  while(node->child[max] != NULL)
    node = node->child[max];

  return node;
}

rbtree_node_t*
rbtree_min(const rbtree_t *tree)
{
  rbtree_node_t *node;

  node = do_minmax(tree, LEFT);

  return node;
}

rbtree_node_t*
rbtree_max(const rbtree_t *tree)
{
  rbtree_node_t *node;

  node = do_minmax(tree, RIGHT);

  return node;
}

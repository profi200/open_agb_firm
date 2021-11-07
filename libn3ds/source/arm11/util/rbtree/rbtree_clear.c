#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

void
rbtree_clear(rbtree_t                 *tree,
             rbtree_node_destructor_t destructor)
{
  rbtree_node_t *node = tree->root;

  while(tree->root != NULL)
  {
    while(node->child[LEFT] != NULL)
      node = node->child[LEFT];

    if(node->child[RIGHT] != NULL)
      node = node->child[RIGHT];
    else
    {
      rbtree_node_t *parent = get_parent(node);

      if(parent == NULL)
        tree->root = NULL;
      else
        parent->child[node != parent->child[LEFT]] = NULL;

      if(destructor != NULL)
        (*destructor)(node);

      node = parent;
    }
  }

  tree->size = 0;
}

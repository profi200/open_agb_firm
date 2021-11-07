#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

rbtree_node_t*
rbtree_find(const rbtree_t      *tree,
            const rbtree_node_t *node)
{
  rbtree_node_t *tmp  = tree->root;
  rbtree_node_t *save = NULL;

  while(tmp != NULL)
  {
    int rc = (*tree->comparator)(node, tmp);
    if(rc < 0)
    {
      tmp = tmp->child[LEFT];
    }
    else if(rc > 0)
    {
      tmp = tmp->child[RIGHT];
    }
    else
    {
      save = tmp;
      tmp = tmp->child[LEFT];
    }
  }

  return save;
}

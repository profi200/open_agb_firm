#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

static inline rbtree_node_t*
do_iterate(const rbtree_node_t *node,
           int                 next)
{
  rbtree_node_t *it = (rbtree_node_t*)node;

  if(it->child[next] != NULL)
  {
    it = it->child[next];
    while(it->child[!next] != NULL)
      it = it->child[!next];
  }
  else
  {
    rbtree_node_t *parent = get_parent(node);
    while(parent != NULL && it == parent->child[next])
    {
      it = parent;
      parent = get_parent(it);
    }

    it = parent;
  }

  return it;
}

rbtree_node_t*
rbtree_node_next(const rbtree_node_t *node)
{
  return do_iterate(node, RIGHT);
}

rbtree_node_t*
rbtree_node_prev(const rbtree_node_t *node)
{
  return do_iterate(node, LEFT);
}

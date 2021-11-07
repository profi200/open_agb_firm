#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

void
rbtree_rotate(rbtree_t      *tree,
              rbtree_node_t *node,
              int           left)
{
  rbtree_node_t *tmp    = node->child[left];
  rbtree_node_t *parent = get_parent(node);

  node->child[left] = tmp->child[!left];
  if(tmp->child[!left] != NULL)
    set_parent(tmp->child[!left], node);

  tmp->child[!left] = node;
  set_parent(tmp, parent);
  if(parent != NULL)
  {
    if(node == parent->child[!left])
      parent->child[!left] = tmp;
    else
      parent->child[left] = tmp;
  }
  else
    tree->root = tmp;
  set_parent(node, tmp);
}

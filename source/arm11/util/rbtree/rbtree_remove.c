#include "arm11/util/rbtree.h"
#include "rbtree_internal.h"

static void
recolor(rbtree_t      *tree,
        rbtree_node_t *parent,
        rbtree_node_t *node)
{
  rbtree_node_t *sibling;

  while(is_black(node) && node != tree->root)
  {
    int left = (node == parent->child[LEFT]);

    sibling = parent->child[left];

    if(is_red(sibling))
    {
      set_black(sibling);
      set_red(parent);
      rbtree_rotate(tree, parent, left);
      sibling = parent->child[left];
    }

    if(is_black(sibling->child[LEFT]) && is_black(sibling->child[RIGHT]))
    {
      set_red(sibling);
      node = parent;
      parent = get_parent(node);
    }
    else
    {
      if(is_black(sibling->child[left]))
      {
        set_black(sibling->child[!left]);
        set_red(sibling);
        rbtree_rotate(tree, sibling, !left);
        sibling = parent->child[left];
      }

      if(is_black(parent))
        set_black(sibling);
      else
        set_red(sibling);
      set_black(parent);
      set_black(sibling->child[left]);

      rbtree_rotate(tree, parent, left);

      node = tree->root;
    }
  }

  if(node != NULL)
    set_black(node);
}

rbtree_node_t*
rbtree_remove(rbtree_t                 *tree,
              rbtree_node_t            *node,
              rbtree_node_destructor_t destructor)
{
  rbtree_color_t color;
  rbtree_node_t  *child, *parent, *original = node;
  rbtree_node_t  *next;

  next = rbtree_node_next(node);

  if(node->child[LEFT] != NULL && node->child[RIGHT] != NULL)
  {
    rbtree_node_t *old = node;

    node = node->child[RIGHT];
    while(node->child[LEFT] != NULL)
      node = node->child[LEFT];

    parent = get_parent(old);
    if(parent != NULL)
    {
      if(parent->child[LEFT] == old)
        parent->child[LEFT] = node;
      else
        parent->child[RIGHT] = node;
    }
    else
      tree->root = node;

    child  = node->child[RIGHT];
    parent = get_parent(node);
    color  = get_color(node);

    if(parent == old)
      parent = node;
    else
    {
      if(child != NULL)
        set_parent(child, parent);
      parent->child[LEFT] = child;

      node->child[RIGHT] = old->child[RIGHT];
      set_parent(old->child[RIGHT], node);
    }

    node->parent_color = old->parent_color;
    node->child[LEFT] = old->child[LEFT];
    set_parent(old->child[LEFT], node);
  }
  else
  {
    if(node->child[LEFT] == NULL)
      child = node->child[RIGHT];
    else
      child = node->child[LEFT];

    parent = get_parent(node);
    color  = get_color(node);

    if(child != NULL)
      set_parent(child, parent);
    if(parent != NULL)
    {
      if(parent->child[LEFT] == node)
        parent->child[LEFT] = child;
      else
        parent->child[RIGHT] = child;
    }
    else
      tree->root = child;
  }

  if(color == BLACK)
    recolor(tree, parent, child);

  if(destructor != NULL)
    (*destructor)(original);

  tree->size -= 1;

  return next;
}

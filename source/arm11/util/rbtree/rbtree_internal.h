#pragma once

#define LEFT  0
#define RIGHT 1

typedef enum rbtree_color
{
  RED   = 0,
  BLACK = 1,
} rbtree_color_t;

#define COLOR_MASK  (RED|BLACK)

static inline void
set_black(rbtree_node_t *node)
{
  node->parent_color &= ~COLOR_MASK;
  node->parent_color |= BLACK;
}

static inline void
set_red(rbtree_node_t *node)
{
  node->parent_color &= ~COLOR_MASK;
  node->parent_color |= RED;
}

static inline rbtree_color_t
get_color(const rbtree_node_t *node)
{
  if(node == NULL)
    return BLACK;
  return (rbtree_color_t)(node->parent_color & COLOR_MASK);
}

static inline int
is_black(const rbtree_node_t *node)
{
  return get_color(node) == BLACK;
}

static inline int
is_red(const rbtree_node_t *node)
{
  return get_color(node) == RED;
}

static inline rbtree_node_t*
get_parent(const rbtree_node_t *node)
{
  return (rbtree_node_t*)(node->parent_color & ~COLOR_MASK);
}

static inline void
set_parent(rbtree_node_t       *node,
           const rbtree_node_t *parent)
{
  node->parent_color = (get_color(node)) | ((uintptr_t)parent);
}

void
rbtree_rotate(rbtree_t      *tree,
              rbtree_node_t *node,
              int           left);

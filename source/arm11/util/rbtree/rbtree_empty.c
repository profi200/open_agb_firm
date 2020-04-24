#include "arm11/util/rbtree.h"

int
rbtree_empty(const rbtree_t *tree)
{
  return tree->root == NULL;
}

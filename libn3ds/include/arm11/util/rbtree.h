/**
 * @file rbtree.h
 * @brief Red-black trees.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

/// Retrieves an rbtree item.
#define rbtree_item(ptr, type, member) \
  ((type*)(((char*)ptr) - offsetof(type, member)))

typedef struct rbtree      rbtree_t;      ///< rbtree type.
typedef struct rbtree_node rbtree_node_t; ///< rbtree node type.

typedef void (*rbtree_node_destructor_t)(rbtree_node_t *Node);      ///< rbtree node destructor.
typedef int  (*rbtree_node_comparator_t)(const rbtree_node_t *lhs,
                                         const rbtree_node_t *rhs); ///< rbtree node comparator.

/// An rbtree node.
struct rbtree_node
{
  uintptr_t      parent_color; ///< Parent color.
  rbtree_node_t  *child[2];    ///< Node children.
};

/// An rbtree.
struct rbtree
{
  rbtree_node_t            *root;      ///< Root node.
  rbtree_node_comparator_t comparator; ///< Node comparator.
  size_t                   size;       ///< Size.
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes an rbtree.
 * @param tree Pointer to the tree.
 * @param comparator Comparator to use.
 */
void
rbtree_init(rbtree_t                 *tree,
            rbtree_node_comparator_t comparator);

/**
 * @brief Gets whether an rbtree is empty
 * @param tree Pointer to the tree.
 * @return A non-zero value if the tree is not empty.
 */
int
rbtree_empty(const rbtree_t *tree);

/**
 * @brief Gets the size of an rbtree.
 * @param tree Pointer to the tree.
 */
size_t
rbtree_size(const rbtree_t *tree);

/**
 * @brief Inserts a node into an rbtree.
 * @param tree Pointer to the tree.
 * @param node Pointer to the node.
 * @return The inserted node.
 */
__attribute__((warn_unused_result))
rbtree_node_t*
rbtree_insert(rbtree_t      *tree,
              rbtree_node_t *node);

/**
 * @brief Inserts multiple nodes into an rbtree.
 * @param tree Pointer to the tree.
 * @param node Pointer to the nodes.
 */
void
rbtree_insert_multi(rbtree_t      *tree,
                    rbtree_node_t *node);

/**
 * @brief Finds a node within an rbtree.
 * @param tree Pointer to the tree.
 * @param node Pointer to the node.
 * @return The located node.
 */
rbtree_node_t*
rbtree_find(const rbtree_t      *tree,
            const rbtree_node_t *node);

/**
 * @brief Gets the minimum node of an rbtree.
 * @param tree Pointer to the tree.
 * @return The minimum node.
 */
rbtree_node_t*
rbtree_min(const rbtree_t *tree);

/**
 * @brief Gets the maximum node of an rbtree.
 * @param tree Pointer to the tree.
 * @return The maximum node.
 */
rbtree_node_t*
rbtree_max(const rbtree_t *tree);

/**
 * @brief Gets the next node from an rbtree node.
 * @param node Pointer to the node.
 * @return The next node.
 */
rbtree_node_t*
rbtree_node_next(const rbtree_node_t *node);

/**
 * @brief Gets the previous node from an rbtree node.
 * @param node Pointer to the node.
 * @return The previous node.
 */
rbtree_node_t*
rbtree_node_prev(const rbtree_node_t *node);

/**
 * @brief Removes a node from an rbtree.
 * @param tree Pointer to the tree.
 * @param node Pointer to the node.
 * @param destructor Destructor to use when removing the node.
 * @return The removed node.
 */
rbtree_node_t*
rbtree_remove(rbtree_t                 *tree,
              rbtree_node_t            *node,
              rbtree_node_destructor_t destructor);

/**
 * @brief Clears an rbtree.
 * @param tree Pointer to the tree.
 * @param destructor Destructor to use when clearing the tree's nodes.
 */
void
rbtree_clear(rbtree_t                 *tree,
             rbtree_node_destructor_t destructor);

#ifdef __cplusplus
}
#endif

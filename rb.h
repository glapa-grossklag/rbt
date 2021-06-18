#ifndef RB_H
#define RB_H

#include <stdbool.h>
#include <stddef.h>

#define rb_for_each(TREE, NODE) for ((NODE) = rb_first((TREE).root); (NODE) != NULL; (NODE) = rb_next(NODE))

enum rb_color {
    RB_RED,
    RB_BLACK,
};

struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    enum rb_color color;
};

struct rb_tree {
    struct rb_node *root;
};

/*
 * Return a node red-black tree.
 */
struct rb_tree rb_tree_init(void);

/*
 * Return a new red-black tree node.
 */
struct rb_node rb_node_init(void);

/*
 * A comparison function used for insertion, search, and removal.
 *
 * Must return:
 * - less than 0 if left < right,
 * - greater than 0 if left > right,
 * - 0 if left == right.
 */
typedef int (*rb_cmp)(struct rb_node *left, struct rb_node *right);

/*
 * Insert a node into a red-black tree. If insertion is successful, return the
 * node, else return NULL since an equal node is already in the tree.
 */
struct rb_node *rb_insert(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp);

/*
 * If an equal node is in the tree, then return it, else return NULL.
 */
struct rb_node *rb_search(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp);

/*
 * Remove a node from a red-black tree. Return the removed node if successful,
 * else return NULL since the node was not in the tree.
 */
struct rb_node *rb_remove(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp);

/*
 * Return the in-order successor of the given node.
 */
struct rb_node *rb_next(struct rb_node *node);

/*
 * Return the in-order predecessor of the given node.
 */
struct rb_node *rb_prev(struct rb_node *node);

/*
 * Return the left-most node in the tree.
 */
struct rb_node *rb_first(struct rb_node *tree);

/*
 * Return the right-most node in the tree.
 */
struct rb_node *rb_last(struct rb_node *tree);

/*
 * The functions below are only needed for testing.
 */
#ifndef NDEBUG

/*
 * Return true if the red-black tree obeys the five necessary properties.
 *
 * 1. Each node is either red or black.
 * 2. All NIL leaves are black.
 * 3. If a node is red, then both its children are black.
 * 4. Every path from a given node to any of its descendant NULL leaves goes
 *    through the same number of black nodes.
 * 5. The root is black.
 */
bool rb_is_valid(struct rb_tree *tree);

#endif

#endif

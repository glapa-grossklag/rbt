#include "rb.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define RB_RED 0
#define RB_BLACK 1

#define COLOR_OF(NODE) ((uintptr_t) (NODE)->parent & 1)
#define IS_RED(NODE) (COLOR_OF(NODE) == RB_RED)
#define IS_BLACK(NODE) (COLOR_OF(NODE) == RB_BLACK)
#define PARENT_OF(NODE) ((struct rb_node *) ((uintptr_t) ((NODE)->parent) & ~3))
#define SET_PARENT(NODE, PARENT)                                                                                       \
    do {                                                                                                               \
        (NODE)->parent = ((NODE)->parent & 3) | (uintptr_t) (PARENT);                                                  \
    } while (0);
#define SET_COLOR(NODE, COLOR)                                                                                         \
    do {                                                                                                               \
        (NODE)->parent = ((NODE)->parent & ~1) | (uintptr_t) (COLOR);                                                  \
    } while (0);

#define NIL (&nil)

static struct rb_node nil = {(uintptr_t) NIL, NIL, NIL};

struct rb_tree
rb_tree_init(rb_cmp cmp) {
    struct rb_tree tree;
    tree.root = NIL;
    tree.cmp = cmp;
    SET_COLOR(tree.root, RB_BLACK);
    return tree;
}

struct rb_node
rb_node_init(void) {
    struct rb_node node;
    SET_PARENT(&node, NIL);
    node.left = NIL;
    node.right = NIL;
    SET_COLOR(&node, RB_RED);
    return node;
}

// -----------------------------------------------------------------------------
// Insertion
// -----------------------------------------------------------------------------

static void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node);

struct rb_node *
rb_insert(struct rb_tree *tree, struct rb_node *node) {
    // If the tree is empty, then the node becomes the root.
    if (tree->root == NIL) {
        SET_PARENT(node, NIL);
        node->left = NIL;
        node->right = NIL;
        SET_COLOR(node, RB_BLACK);
        tree->root = node;
        return tree->root;
    }

    // Perform a normal BST insertion.
    struct rb_node *child = tree->root;
    struct rb_node *parent = child;
    int result = 0;
    while (child != NIL) {
        parent = child;
        result = tree->cmp(node, child);

        if (result == 0) {
            // Cannot insert duplicate keys.
            return NULL;
        } else if (result < 0) {
            child = child->left;
        } else {
            child = child->right;
        }
    }

    // The value of the child is now NIL. Insert the node as a child of the parent.
    if (result < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    SET_PARENT(node, parent);
    SET_COLOR(node, RB_RED);
    node->left = NIL;
    node->right = NIL;

    // Ensure all RBT properties hold.
    rb_insert_fixup(tree, node);

    return node;
}

static void rb_rotate_left(struct rb_tree *tree, struct rb_node *node);
static void rb_rotate_right(struct rb_tree *tree, struct rb_node *node);

static void
rb_insert_fixup(struct rb_tree *tree, struct rb_node *node) {
    while (node != tree->root && IS_RED(PARENT_OF(node))) {
        if (PARENT_OF(node) == PARENT_OF(PARENT_OF(node))->left) {
            struct rb_node *uncle = PARENT_OF(PARENT_OF(node))->right;

            if (IS_RED(uncle)) {
                // Case 1
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(uncle, RB_BLACK);
                SET_COLOR(PARENT_OF(PARENT_OF(node)), RB_RED);
                node = PARENT_OF(PARENT_OF(node));
            } else {
                if (node == PARENT_OF(node)->right) {
                    // Case 2
                    node = PARENT_OF(node);
                    rb_rotate_left(tree, node);
                }
                // Case 3
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(PARENT_OF(PARENT_OF(node)), RB_RED);
                rb_rotate_right(tree, PARENT_OF(PARENT_OF(node)));
            }
        } else {
            struct rb_node *uncle = PARENT_OF(PARENT_OF(node))->left;

            if (IS_RED(uncle)) {
                // Case 1
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(uncle, RB_BLACK);
                SET_COLOR(PARENT_OF(PARENT_OF(node)), RB_RED);
                node = PARENT_OF(PARENT_OF(node));
            } else {
                if (node == PARENT_OF(node)->left) {
                    // Case 2
                    node = PARENT_OF(node);
                    rb_rotate_right(tree, node);
                }
                // Case 3
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(PARENT_OF(PARENT_OF(node)), RB_RED);
                rb_rotate_left(tree, PARENT_OF(PARENT_OF(node)));
            }
        }
    }

    SET_COLOR(tree->root, RB_BLACK);
}

// -----------------------------------------------------------------------------
// Search
// -----------------------------------------------------------------------------

struct rb_node *
rb_search(struct rb_tree *tree, struct rb_node *node) {
    struct rb_node *curr = tree->root;
    int result = 0;
    while (curr != NIL) {
        result = tree->cmp(node, curr);

        if (result == 0) {
            return curr;
        } else if (result < 0) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    // No node was found with the given key.
    return NULL;
}

// -----------------------------------------------------------------------------
// Removal
// -----------------------------------------------------------------------------

static void rb_transplant(struct rb_tree *tree, struct rb_node *u, struct rb_node *v);
static void rb_remove_fixup(struct rb_tree *tree, struct rb_node *x);

struct rb_node *
rb_remove(struct rb_tree *tree, struct rb_node *node) {
    if (!rb_search(tree, node)) {
        return NULL;
    }

    struct rb_node *child = NULL;
    int color = COLOR_OF(node); // Should this be a different type?

    if (node->left == NIL) {
        // Only a right child.
        child = node->right;
        rb_transplant(tree, node, node->right);
    } else if (node->right == NIL) {
        // Only a left child.
        child = node->left;
        rb_transplant(tree, node, node->left);
    } else {
        // Two children.
        struct rb_node *next = rb_first(node->right);
        color = COLOR_OF(next);
        child = next->right;

        if (PARENT_OF(next) == node) {
            SET_PARENT(child, next)
        } else {
            rb_transplant(tree, next, next->right);
            next->right = node->right;
            SET_PARENT(next->right, next);
        }

        rb_transplant(tree, node, next);
        next->left = node->left;
        SET_PARENT(next->left, next);
        SET_COLOR(next, COLOR_OF(node));
    }

    if (color == RB_BLACK) {
        rb_remove_fixup(tree, child);
    }

    return node;
}

static void
rb_remove_fixup(struct rb_tree *tree, struct rb_node *node) {
    struct rb_node *sibling = NULL;

    while (IS_BLACK(node) && node != tree->root) {
        if (node == PARENT_OF(node)->left) {
            sibling = PARENT_OF(node)->right;

            if (IS_RED(sibling)) {
                SET_COLOR(sibling, RB_BLACK);
                SET_COLOR(PARENT_OF(node), RB_RED);
                rb_rotate_left(tree, PARENT_OF(node));
                sibling = PARENT_OF(node)->right;
            }

            if (IS_BLACK(sibling->left) && IS_BLACK(sibling->right)) {
                SET_COLOR(sibling, RB_RED);
                node = PARENT_OF(node);
            } else {
                if (IS_BLACK(sibling->right)) {
                    SET_COLOR(sibling->left, RB_BLACK);
                    SET_COLOR(sibling, RB_RED);
                    rb_rotate_right(tree, sibling);
                    sibling = PARENT_OF(node)->right;
                }

                SET_COLOR(sibling, COLOR_OF(PARENT_OF(node)));
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(sibling->right, RB_BLACK);
                rb_rotate_left(tree, PARENT_OF(node));
                node = tree->root;
            }
        } else {
            sibling = PARENT_OF(node)->left;

            if (IS_RED(sibling)) {
                SET_COLOR(sibling, RB_BLACK);
                SET_COLOR(PARENT_OF(node), RB_RED);
                rb_rotate_right(tree, PARENT_OF(node));
                sibling = PARENT_OF(node)->left;
            }

            if (IS_BLACK(sibling->right) && IS_BLACK(sibling->left)) {
                SET_COLOR(sibling, RB_RED);
                node = PARENT_OF(node);
            } else {
                if (IS_BLACK(sibling->left)) {
                    SET_COLOR(sibling->right, RB_BLACK);
                    SET_COLOR(sibling, RB_RED);
                    rb_rotate_left(tree, sibling);
                    sibling = PARENT_OF(node)->left;
                }

                SET_COLOR(sibling, COLOR_OF(PARENT_OF(node)));
                SET_COLOR(PARENT_OF(node), RB_BLACK);
                SET_COLOR(sibling->left, RB_BLACK);
                rb_rotate_right(tree, PARENT_OF(node));
                node = tree->root;
            }
        }
    }

    SET_COLOR(node, RB_BLACK);
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

/*
 * Rotate the node to the left.
 */
static void
rb_rotate_left(struct rb_tree *tree, struct rb_node *node) {
    struct rb_node *child = node->right;
    node->right = child->left;

    if (child->left != NIL) {
        SET_PARENT(child->left, node);
    }

    SET_PARENT(child, PARENT_OF(node));

    if (PARENT_OF(node) != NIL) {
        if (node == PARENT_OF(node)->left) {
            PARENT_OF(node)->left = child;
        } else {
            PARENT_OF(node)->right = child;
        }
    } else {
        tree->root = child;
    }

    child->left = node;
    SET_PARENT(node, child);
}

/*
 * Rotate the node to the right.
 */
static void
rb_rotate_right(struct rb_tree *tree, struct rb_node *node) {
    struct rb_node *child = node->left;
    node->left = child->right;

    if (child->right != NIL) {
        SET_PARENT(child->right, node);
    }

    SET_PARENT(child, PARENT_OF(node));

    if (PARENT_OF(child) != NIL) {
        if (node == PARENT_OF(node)->right) {
            PARENT_OF(node)->right = child;
        } else {
            PARENT_OF(node)->left = child;
        }
    } else {
        tree->root = child;
    }

    child->right = node;
    SET_PARENT(node, child);
}

/*
 * Replace the subtree rooted at node n with the subtree rooted at node m.
 */
static void
rb_transplant(struct rb_tree *tree, struct rb_node *u, struct rb_node *v) {
    if (PARENT_OF(u) == NIL) {
        tree->root = v;
    } else if (u == PARENT_OF(u)->left) {
        PARENT_OF(u)->left = v;
    } else {
        PARENT_OF(u)->right = v;
    }

    SET_PARENT(v, PARENT_OF(u));
}

struct rb_node *
rb_next(struct rb_node *node) {
    if (node->right != NIL) {
        return rb_first(node->right);
    }

    if (PARENT_OF(node) == NIL) {
        return NULL;
    }

    struct rb_node *parent = PARENT_OF(node);
    while (parent != NIL && node == parent->right) {
        node = parent;
        parent = PARENT_OF(parent);
    }

    return parent == NIL ? NULL : parent;
}

struct rb_node *
rb_prev(struct rb_node *node) {
    if (node->left != NIL) {
        return rb_last(node->right);
    }

    if (PARENT_OF(node) == NIL) {
        return NULL;
    }

    struct rb_node *parent = PARENT_OF(node);
    while (parent != NIL && node == parent->left) {
        node = parent;
        parent = PARENT_OF(parent);
    }

    return parent == NIL ? NULL : parent;
}

struct rb_node *
rb_first(struct rb_node *tree) {
    struct rb_node *first = tree;
    while (first->left != NIL) {
        first = first->left;
    }
    return first == NIL ? NULL : first;
}

struct rb_node *
rb_last(struct rb_node *tree) {
    struct rb_node *last = tree;
    while (last->right != NIL) {
        last = last->right;
    }
    return last == NIL ? NULL : last;
}

bool
rb_is_empty(struct rb_tree *tree) {
    return tree->root == NIL;
}

/*
 * The functions below are only needed for testing.
 */
#ifndef NDEBUG

#include <stdio.h>

/*
 * Return the black height of a node.
 *
 * The black height of a node is defined as the number of black nodes on the
 * path from the node to any NIL leaf.
 */
static unsigned
rb_black_height(struct rb_node *node) {
    unsigned black_height = 0;
    struct rb_node *curr = node;
    while (curr != NIL) {
        if (IS_BLACK(curr)) {
            black_height += 1;
        }

        curr = curr->left;
    }

    // The current node is now NIL, meaning it is black, therefore we add one
    // to the height.
    return black_height + 1;
}

static bool rb_is_valid_helper(struct rb_node *node, unsigned expected_black_height, unsigned current_black_height);

bool
rb_is_valid(struct rb_tree *tree) {
    // Property 5: The root is black.
    if (!IS_BLACK(tree->root)) {
        fprintf(stderr, "Tree root is not black\n");
        return false;
    }

    if (PARENT_OF(tree->root) != NIL) {
        return false;
    }

    // Ensure all nodes are strictly increasing.
    struct rb_node *prev = NULL;
    struct rb_node *curr = NULL;
    rb_for_each(*tree, curr) {
        if (prev && tree->cmp(prev, curr) >= 0) {
            return false;
        }

        prev = curr;
    }

    // Save the expected black height of the root to ensure property 4.
    unsigned expected_black_height = rb_black_height(tree->root);

    // Evaluate properties 1, 2, 3, & 4.
    return rb_is_valid_helper(tree->root, expected_black_height, 0);
}

static bool
rb_is_valid_helper(struct rb_node *node, unsigned expected_black_height, unsigned current_black_height) {
    if (node == NIL) {
        // Property 2: All NIL leaves are black.
        if (!IS_BLACK(node)) {
            return false;
        }

        // Property 4: Every path from a given node to any of its descendant NIL
        //             leaves goes through the same number of black nodes.
        if (expected_black_height != current_black_height + 1) {
            fprintf(stderr, "Expected black height %u is not equal to actual black height %u\n", expected_black_height,
                    current_black_height + 1);
            return false;
        }

        // Base case.
        return true;
    }

    // Property 1: Each node is either red or black.
    if (!IS_RED(node) && !IS_BLACK(node)) {
        fprintf(stderr, "Node is neither red nor black\n");
        return false;
    }

    // Property 3: If a node is red, then both its children are black.
    if (IS_RED(node)) {
        if (!IS_BLACK(node->left) || !IS_BLACK(node->right)) {
            fprintf(stderr, "A red node has non-black children\n");
            return false;
        }
    }

    if (IS_BLACK(node)) {
        current_black_height += 1;
    }

    // Verify various structural properties true of any binary tree.
    if (PARENT_OF(node) != NIL && PARENT_OF(node)->left != node && PARENT_OF(node)->right != node) {
        return false;
    }

    if (node->left != NIL && node->right != NIL && (PARENT_OF(node->left) != node || PARENT_OF(node->right) != node)) {
        return false;
    }

    return rb_is_valid_helper(node->left, expected_black_height, current_black_height) &&
           rb_is_valid_helper(node->right, expected_black_height, current_black_height);
}

#endif

#include "rb.h"
#include <stdbool.h>
#include <stdlib.h>

#define IS_BLACK(NODE) ((NODE)->color == RB_BLACK)
#define IS_RED(NODE) ((NODE)->color == RB_RED)
#define NIL (&nil)

struct rb_node nil = {NIL, NIL, NIL, RB_BLACK};

struct rb_tree
rb_tree_init(void) {
    struct rb_tree tree;
    tree.root = NIL;
    return tree;
}

struct rb_node
rb_node_init(void) {
    struct rb_node node;
    node.parent = NIL;
    node.left = NIL;
    node.right = NIL;
    node.color = RB_RED;
    return node;
}

// -----------------------------------------------------------------------------
// Insertion
// -----------------------------------------------------------------------------

void rb_insert_fixup(struct rb_tree *tree, struct rb_node *node);

struct rb_node *
rb_insert(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp) {
    // If the tree is empty, then the node becomes the root.
    if (tree->root == NIL) {
        node->parent = NIL;
        node->left = NIL;
        node->right = NIL;
        node->color = RB_BLACK;
        tree->root = node;
        return tree->root;
    }

    // Perform a normal BST insertion.
    struct rb_node *child = tree->root;
    struct rb_node *parent = child;
    int result = 0;
    while (child != NIL) {
        parent = child;
        result = cmp(node, child);

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

    node->parent = parent;
    node->color = RB_RED;
    node->left = NIL;
    node->right = NIL;

    // Ensure all RBT properties hold.
    rb_insert_fixup(tree, node);

    return node;
}

static void rb_rotate_left(struct rb_tree *tree, struct rb_node *node);
static void rb_rotate_right(struct rb_tree *tree, struct rb_node *node);

void
rb_insert_fixup(struct rb_tree *tree, struct rb_node *node) {
    while (node != tree->root && IS_RED(node->parent)) {
        if (node->parent == node->parent->parent->left) {
            struct rb_node *uncle = node->parent->parent->right;

            if (IS_RED(uncle)) {
                // Case 1
                node->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                node->parent->parent->color = RB_RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    // Case 2
                    node = node->parent;
                    rb_rotate_left(tree, node);
                }
                // Case 3
                node->parent->color = RB_BLACK;
                node->parent->parent->color = RB_RED;
                rb_rotate_right(tree, node->parent->parent);
            }
        } else {
            struct rb_node *uncle = node->parent->parent->left;

            if (IS_RED(uncle)) {
                // Case 1
                node->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                node->parent->parent->color = RB_RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    // Case 2
                    node = node->parent;
                    rb_rotate_right(tree, node);
                }
                // Case 3
                node->parent->color = RB_BLACK;
                node->parent->parent->color = RB_RED;
                rb_rotate_left(tree, node->parent->parent);
            }
        }
    }

    tree->root->color = RB_BLACK;
}

// -----------------------------------------------------------------------------
// Search
// -----------------------------------------------------------------------------

struct rb_node *
rb_search(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp) {
    struct rb_node *curr = tree->root;
    int result = 0;
    while (curr != NIL) {
        result = cmp(node, curr);

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
rb_remove(struct rb_tree *tree, struct rb_node *node, rb_cmp cmp) {
    if (!rb_search(tree, node, cmp)) {
        return NULL;
    }

    struct rb_node *child = NULL;
    enum rb_color color = node->color;

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
        color = next->color;
        child = next->right;

        if (next->parent == node) {
            child->parent = next;
        } else {
            rb_transplant(tree, next, next->right);
            next->right = node->right;
            next->right->parent = next;
        }

        rb_transplant(tree, node, next);
        next->left = node->left;
        next->left->parent = next;
        next->color = node->color;
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
        if (node == node->parent->left) {
            sibling = node->parent->right;

            if (IS_RED(sibling)) {
                sibling->color = RB_BLACK;
                node->parent->color = RB_RED;
                rb_rotate_left(tree, node->parent);
                sibling = node->parent->right;
            }

            if (IS_BLACK(sibling->left) && IS_BLACK(sibling->right)) {
                sibling->color = RB_RED;
                node = node->parent;
            } else {
                if (IS_BLACK(sibling->right)) {
                    sibling->left->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_right(tree, sibling);
                    sibling = node->parent->right;
                }

                sibling->color = node->parent->color;
                node->parent->color = RB_BLACK;
                sibling->right->color = RB_BLACK;
                rb_rotate_left(tree, node->parent);
                node = tree->root;
            }
        } else {
            sibling = node->parent->left;

            if (IS_RED(sibling)) {
                sibling->color = RB_BLACK;
                node->parent->color = RB_RED;
                rb_rotate_right(tree, node->parent);
                sibling = node->parent->left;
            }

            if (IS_BLACK(sibling->right) && IS_BLACK(sibling->left)) {
                sibling->color = RB_RED;
                node = node->parent;
            } else {
                if (IS_BLACK(sibling->left)) {
                    sibling->right->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_left(tree, sibling);
                    sibling = node->parent->left;
                }

                sibling->color = node->parent->color;
                node->parent->color = RB_BLACK;
                sibling->left->color = RB_BLACK;
                rb_rotate_right(tree, node->parent);
                node = tree->root;
            }
        }
    }

    node->color = RB_BLACK;
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
        child->left->parent = node;
    }

    child->parent = node->parent;

    if (node->parent != NIL) {
        if (node == node->parent->left) {
            node->parent->left = child;
        } else {
            node->parent->right = child;
        }
    } else {
        tree->root = child;
    }

    child->left = node;
    node->parent = child;
}

/*
 * Rotate the node to the right.
 */
static void
rb_rotate_right(struct rb_tree *tree, struct rb_node *node) {
    struct rb_node *child = node->left;
    node->left = child->right;

    if (child->right != NIL) {
        child->right->parent = node;
    }

    child->parent = node->parent;

    if (child->parent != NIL) {
        if (node == node->parent->right) {
            node->parent->right = child;
        } else {
            node->parent->left = child;
        }
    } else {
        tree->root = child;
    }

    child->right = node;
    node->parent = child;
}

/*
 * Replace the subtree rooted at node n with the subtree rooted at node m.
 */
static void
rb_transplant(struct rb_tree *tree, struct rb_node *u, struct rb_node *v) {
    if (u->parent == NIL) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }

    v->parent = u->parent;
}

struct rb_node *
rb_next(struct rb_node *node) {
    if (node->right != NIL) {
        return rb_first(node->right);
    }

    if (node->parent == NIL) {
        return NULL;
    }

    struct rb_node *parent = node->parent;
    while (parent != NIL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }

    return parent == NIL ? NULL : parent;
}

struct rb_node *
rb_prev(struct rb_node *node) {
    if (node->left != NIL) {
        return rb_last(node->right);
    }

    if (node->parent == NIL) {
        return NULL;
    }

    struct rb_node *parent = NULL;
    while (parent != NIL && node == parent->left) {
        node = parent;
        parent = parent->parent;
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
        if (curr->color == RB_BLACK)
            black_height += 1;
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

    if (node->color == RB_BLACK) {
        current_black_height += 1;
    }

    return rb_is_valid_helper(node->left, expected_black_height, current_black_height) &&
           rb_is_valid_helper(node->right, expected_black_height, current_black_height);
}

#endif

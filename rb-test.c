#include "rb.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define container_of(ptr, type, member)                                                                                \
    ({                                                                                                                 \
        const typeof(((type *)0)->member) *__mptr = (ptr);                                                             \
        (type *)((char *)__mptr - offsetof(type, member));                                                             \
    })

#define TESTS 100000

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

struct box {
    int key;
    struct rb_node rb_node;
};

int
cmp(struct rb_node *l, struct rb_node *r) {
    struct box *lb = container_of(l, struct box, rb_node);
    struct box *rb = container_of(r, struct box, rb_node);

    if (lb->key < rb->key)
        return -1;
    if (lb->key > rb->key)
        return +1;
    return 0;
}

// ----------------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------------

/*
 * TODO:
 * - All three ops in the same test and rely on a separate "ground truth" array
 *   so that all three ops are performed on the same tree.
 * - Verify the tree is a valid binary search tree. Both structurally (e.g.,
 *   node->child->parent == node) and order (simple loop).
 */

/*
 * Test insertion of in-order elements, in [0, TESTS).
 *
 * This test does not rely on any other test and can be assumed to be a measure
 * of correctness for insertion.
 */
bool
test_insert_inorder(void) {
    struct rb_tree tree = rb_tree_init();

    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        // Insertion will only fail if a duplicate key is inserted, which won't
        // happen during in-order insertion.
        struct rb_node *inserted = rb_insert(&tree, &boxes[i].rb_node, cmp);
        if (!inserted) {
            return false;
        }
    }

    // The tree must be valid.
    if (!rb_is_valid(&tree)) {
        return false;
    }

    // The tree is valid!
    return true;
}

/*
 * Test insertion of TESTS random elements.
 *
 * This test does not rely on any other test and can be assumed to be a measure
 * of correctness for insertion.
 */
bool
test_insert_random(void) {
    struct rb_tree tree = rb_tree_init();

    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node, cmp));
    }

    // The tree must be valid.
    if (!rb_is_valid(&tree)) {
        return false;
    }

    // The tree is valid!
    return true;
}

/*
 * Both search tests follow the following pattern:
 *
 * Create a box with which to search. This new box will have the same
 * key as the box for which we're searching, but a different memory
 * address. Thus, search should return the original node, not the new
 * node.
 *
 * Create a different box with a key **not** present in any node. Search should
 * return a NULL.
 */

/*
 * Test search for in-order elements, in [0, TESTS).
 *
 * This test assumes the insertion operation is correct and should not be used
 * as the sole measure of correctness.
 */
bool
test_search_inorder(void) {
    struct rb_tree tree = rb_tree_init();

    // Build up the tree.
    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        rb_insert(&tree, &boxes[i].rb_node, cmp);
    }

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        if (found != &boxes[i].rb_node) {
            return false;
        }
    }

    // Search for elements that should **not** be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        if (found) {
            return false;
        }
    }

    // The tree is valid!
    return true;
}

/*
 * Test search for TESTS random elements.
 *
 * This test assumes the insertion operation is correct and should not be used
 * as the sole measure of correctness.
 */
bool
test_search_random(void) {
    struct rb_tree tree = rb_tree_init();

    // Build up the tree.
    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node, cmp));
    }

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        if (found != &boxes[i].rb_node) {
            return false;
        }
    }

    // Search for elements that should **not** be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        if (found) {
            return false;
        }
    }

    // The tree is valid!
    return true;
}

/*
 * Test insertion of in-order elements, in [0, TESTS).
 *
 * This test assumes the insertion and search operations are correct and should
 * not be used as the sole measure of correctness.
 */
bool
test_remove_inorder(void) {
    struct rb_tree tree = rb_tree_init();

    // Build up the tree.
    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        rb_insert(&tree, &boxes[i].rb_node, cmp);
    }

    // Remove items from the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory address
        // to search.
        struct box box;
        box.key = boxes[i].key;

        // Despite being a different box and node, the removal should remove
        // the original node.
        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        struct rb_node *removed = rb_remove(&tree, found, cmp);
        if (found != removed || removed != &boxes[i].rb_node) {
            return false;
        }

        // Should be gone now!
        found = rb_search(&tree, &box.rb_node, cmp);
        if (found) {
            return false;
        }
    }

    // The tree must be valid.
    if (!rb_is_valid(&tree)) {
        return false;
    }

    // The tree is valid!
    return true;
}

/*
 * Test removal of TESTS random elements.
 *
 * This test assumes the insertion and search operations are correct and should
 * not be used as the sole measure of correctness.
 */
bool
test_remove_random(void) {
    struct rb_tree tree = rb_tree_init();

    // Build up the tree.
    struct box boxes[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node, cmp));
    }

    // Remove items from the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory address
        // to search.
        struct box box;
        box.key = boxes[i].key;

        // Despite being a different box and node, the removal should remove
        // the original node.
        struct rb_node *found = rb_search(&tree, &box.rb_node, cmp);
        struct rb_node *removed = rb_remove(&tree, found, cmp);
        if (found != removed || removed != &boxes[i].rb_node) {
            return false;
        }

        // Should be gone now!
        found = rb_search(&tree, &box.rb_node, cmp);
        if (found) {
            return false;
        }
    }

    // The tree must be valid.
    if (!rb_is_valid(&tree)) {
        return false;
    }

    // The tree is valid!
    return true;
}

int
main(void) {
    unsigned long t = time(NULL);
    srand(t);

    fprintf(stderr, "Testing insertion... ");
    assert(test_insert_inorder());
    assert(test_insert_random());
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing search... ");
    assert(test_search_inorder());
    assert(test_search_random());
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing removal... ");
    assert(test_remove_inorder());
    assert(test_remove_random());
    fprintf(stderr, "passed\n");

    return 0;
}

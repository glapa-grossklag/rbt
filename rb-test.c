#include "rb.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TESTS 1000000

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

struct box {
    int key;
    struct rb_node rb_node;
};

int
cmp(struct rb_node *l, struct rb_node *r) {
    struct box *lb = rb_entry(l, struct box, rb_node);
    struct box *rb = rb_entry(r, struct box, rb_node);

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
 * NOTE:
 * - The arrays used in these tests are dynamically allocated. The memory is
 *   freed upon a successful test, but not upon an unsuccessful test. If tests
 *   aren't passing there are bigger issues than memory leaks.
 */

/*
 * Test insertion of in-order elements, in [0, TESTS).
 *
 * This test does not rely on any other test and can be assumed to be a measure
 * of correctness for insertion.
 */
void
test_insert_inorder(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        // Insertion will only fail if a duplicate key is inserted, which won't
        // happen during in-order insertion.
        struct rb_node *inserted = rb_insert(&tree, &boxes[i].rb_node);
        assert(inserted);
    }

    assert(rb_is_valid(&tree));
    free(boxes);
}

/*
 * Test insertion of TESTS random elements.
 *
 * This test does not rely on any other test and can be assumed to be a measure
 * of correctness for insertion.
 */
void
test_insert_random(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node));
    }

    assert(rb_is_valid(&tree));
    free(boxes);
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
void
test_search_inorder(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        rb_insert(&tree, &boxes[i].rb_node);
    }

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    // Search for elements that should **not** be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    free(boxes);
}

/*
 * Test search for TESTS random elements.
 *
 * This test assumes the insertion operation is correct and should not be used
 * as the sole measure of correctness.
 */
void
test_search_random(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node));
    }

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    // Search for elements that should **not** be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    free(boxes);
}

/*
 * Test insertion of in-order elements, in [0, TESTS).
 *
 * This test assumes the insertion and search operations are correct and should
 * not be used as the sole measure of correctness.
 */
void
test_remove_inorder(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        rb_insert(&tree, &boxes[i].rb_node);
    }

    // Remove items from the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory address
        // to search.
        struct box box;
        box.key = boxes[i].key;

        // Despite being a different box and node, the removal should remove
        // the original node.
        struct rb_node *found = rb_search(&tree, &box.rb_node);
        struct rb_node *removed = rb_remove(&tree, found);
        assert(found == removed && removed == &boxes[i].rb_node);

        // Should be gone now!
        found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    assert(rb_is_valid(&tree));

    free(boxes);
}

/*
 * Test removal of TESTS random elements.
 *
 * This test assumes the insertion and search operations are correct and should
 * not be used as the sole measure of correctness.
 */
void
test_remove_random(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].rb_node = rb_node_init();

        // Generate a key until it isn't a duplicate.
        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node));
    }

    // Remove items from the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory address
        // to search.
        struct box box;
        box.key = boxes[i].key;

        // Despite being a different box and node, the removal should remove
        // the original node.
        struct rb_node *found = rb_search(&tree, &box.rb_node);
        struct rb_node *removed = rb_remove(&tree, found);
        assert(found == removed && removed == &boxes[i].rb_node);

        // Should be gone now!
        found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    assert(rb_is_valid(&tree));

    free(boxes);
}

/*
 * Test insertion, search, and removal on the same tree. Elements are inserted in-order.
 */
void
test_all_inorder(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = i;
        boxes[i].rb_node = rb_node_init();

        struct rb_node *inserted = rb_insert(&tree, &boxes[i].rb_node);
        assert(inserted);
    }

    assert(rb_is_valid(&tree));

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    // Search for elements that should **not** be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    // Remove half of the items from the tree.
    for (ptrdiff_t i = 0; i < TESTS / 2; i += 1) {
        struct box box;
        box.key = boxes[i].key;

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        struct rb_node *removed = rb_remove(&tree, found);
        assert(found == removed && removed == &boxes[i].rb_node);

        found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    assert(rb_is_valid(&tree));

    // Search for the half that should still be in the tree, despite removal.
    for (ptrdiff_t i = TESTS / 2 + 1; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    free(boxes);
}

/*
 * Test insertion, search, and removal on the same tree. Elements are inserted randomly.
 */
void
test_all_random(void) {
    struct rb_tree tree = rb_tree_init(cmp);

    struct box *boxes = malloc(TESTS * sizeof(struct box));
    assert(boxes);

    // Build up the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        boxes[i].key = rand();
        boxes[i].rb_node = rb_node_init();

        do {
            boxes[i].key = rand();
        } while (!rb_insert(&tree, &boxes[i].rb_node));
    }

    assert(rb_is_valid(&tree));

    // Search for elements that should be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    // Search for elements that should *not* be in the tree.
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        struct box box;
        box.key = -boxes[i].key - 1; // Note the negative key. The '-1' is to deal with 0.
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    // Remove half of the items from the tree.
    for (ptrdiff_t i = 0; i < TESTS / 2; i += 1) {
        struct box box;
        box.key = boxes[i].key;

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        struct rb_node *removed = rb_remove(&tree, found);
        assert(found == removed && removed == &boxes[i].rb_node);

        found = rb_search(&tree, &box.rb_node);
        assert(!found);
    }

    assert(rb_is_valid(&tree));

    // Search for the half that should still be in the tree, despite removal.
    for (ptrdiff_t i = TESTS / 2 + 1; i < TESTS; i += 1) {
        struct box box;
        box.key = boxes[i].key;
        box.rb_node = rb_node_init();

        struct rb_node *found = rb_search(&tree, &box.rb_node);
        assert(found == &boxes[i].rb_node);
    }

    free(boxes);
}

// ----------------------------------------------------------------------------
// Driver
// ----------------------------------------------------------------------------

int
main(void) {
    unsigned long t = time(NULL);
    srand(t);

    fprintf(stderr, "%u elements\n", TESTS);

    fprintf(stderr, "Testing insertion... ");
    test_insert_inorder();
    test_insert_random();
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing search... ");
    test_search_inorder();
    test_search_random();
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing removal... ");
    test_remove_inorder();
    test_remove_random();
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing all together... ");
    test_all_inorder();
    test_all_random();
    fprintf(stderr, "passed\n");

    return EXIT_SUCCESS;
}

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

#define TESTS 1000

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

struct box {
    int n;
    struct rb_node node;
};

int
cmp(struct rb_node *l, struct rb_node *r) {
    struct box *lb = container_of(l, struct box, node);
    struct box *rb = container_of(r, struct box, node);

    if (lb->n < rb->n)
        return -1;
    if (lb->n > rb->n)
        return +1;
    return 0;
}

// ----------------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------------

/*
 * TODO:
 * - Both in-order testing and random testing. (Preferable separate functions.)
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

    return false;
}

/*
 * Test insertion of TESTS random elements.
 *
 * This test does not rely on any other test and can be assumed to be a measure
 * of correctness for insertion.
 */
bool
test_insert_random(void) {

    return false;
}

/*
 * Test search for in-order elements, in [0, TESTS).
 *
 * This test assumes the insertion operation is correct and should not be used
 * as the sole measure of correctness.
 */
bool
test_search_inorder(void) {

    return false;
}

/*
 * Test search for TESTS random elements.
 *
 * This test assumes the insertion operation is correct and should not be used
 * as the sole measure of correctness.
 */
bool
test_search_random(void) {

    return false;
}

/*
 * Test insertion of in-order elements, in [0, TESTS).
 *
 * This test assumes the insertion and search operations are correct and should not be used as
 * the sole measure of correctness.
 */
bool
test_remove_inorder(void) {

    return false;
}

/*
 * Test removal of TESTS random elements.
 *
 * This test assumes the insertion and search operations are correct and should not be used as
 * the sole measure of correctness.
 */
bool
test_remove_random(void) {

    return false;
}

void
test_insert() {
    struct rb_tree tree = rb_tree_init();

    // Create an array of TESTS boxes and nodes, insert them all into the tree.
    struct box array[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        array[i].n = i;
        array[i].node = rb_node_init();

        assert(rb_insert(&tree, &array[i].node, cmp));

        // The tree must be valid at every step.
        assert(rb_is_valid(&tree));
    }

    // Ensure all nodes are strictly increasing.
    struct rb_node *prev = NULL;
    struct rb_node *curr = NULL;
    rb_for_each(tree, curr) {
        if (prev) {
            struct box *currbox = container_of(curr, struct box, node);
            struct box *prevbox = container_of(prev, struct box, node);
            assert(currbox->n > prevbox->n);
        }

        prev = curr;
    }
}

void
test_search() {
    // Necessary prerequisites for this test.
    test_insert();

    struct rb_tree tree = rb_tree_init();

    // Create an array of TESTS boxes and nodes, insert them all into the tree.
    struct box array[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        array[i].n = i;
        array[i].node = rb_node_init();

        assert(rb_insert(&tree, &array[i].node, cmp));

        // The tree must be valid at every step.
        assert(rb_is_valid(&tree));
    }

    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory
        // address to search.
        struct box box;
        box.n = array[i].n;

        // Despite being a different box and node, the search should return the
        // original node.
        struct rb_node *found = rb_search(&tree, &box.node, cmp);
        assert(found == &array[i].node);

        assert(rb_is_valid(&tree));
    }
}

void
test_remove() {
    // Necessary prerequisites for this test.
    test_insert();
    test_search();

    struct rb_tree tree = rb_tree_init();

    // Create an array of TESTS boxes and nodes, insert them all into the tree.
    struct box array[TESTS];
    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        array[i].n = i;
        array[i].node = rb_node_init();
        rb_insert(&tree, &array[i].node, cmp);

        // The tree must be valid at every step.
        assert(rb_is_valid(&tree));
    }

    for (ptrdiff_t i = 0; i < TESTS; i += 1) {
        // Create a new box with the same data, but a different memory
        // address to search.
        struct box box;
        box.n = array[i].n;

        // Despite being a different box and node, the removal should remove
        // the original node.
        struct rb_node *found = rb_search(&tree, &box.node, cmp);
        assert(found == &array[i].node);
        rb_remove(&tree, found, cmp);

        // Should be gone now!
        found = rb_search(&tree, &box.node, cmp);
        assert(found == NULL);
        assert(rb_is_valid(&tree));
    }

    assert(rb_is_valid(&tree));
}

int
main(void) {
    unsigned long t = time(NULL);
    srand(t);

    fprintf(stderr, "Testing insertion... ");
    test_insert();
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing search... ");
    test_search();
    fprintf(stderr, "passed\n");

    fprintf(stderr, "Testing removal... ");
    test_remove();
    fprintf(stderr, "passed\n");

    return 0;
}

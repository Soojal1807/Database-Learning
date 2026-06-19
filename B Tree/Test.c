#include <stdio.h>
#include <assert.h>
#include "BTree.h"

/*Basic Insertion Test*/
void test_basic_insert_search() {
    BTree *tree = btree_create(3); // Degree 3
    
    // Insert 1 to 100
    for (int i = 1; i <= 100; i++) {
        btree_insert(tree, i);
    }

    // Verify all exist
    for (int i = 1; i <= 100; i++) {
        assert(btree_search(tree, i) == 1);
    }

    // To verify "Non-Existant" keys fail
    assert(btree_search(tree, 101) == 0);
    assert(btree_search(tree, -5) == 0);

    btree_free(tree);
    printf("Basic Insert & Search: PASS\n");
}


/*Basic Deletion Test*/
void test_deletion() {
    BTree *tree = btree_create(3);
    
    for (int i = 1; i <= 50; i++) {
        btree_insert(tree, i);
    }

    // Delete evens
    for (int i = 2; i <= 50; i += 2) {
        btree_delete(tree, i);
    }

    // Verify evens are gone, odds remain
    for (int i = 1; i <= 50; i++) {
        if (i % 2 == 0) {
            assert(btree_search(tree, i) == 0);
        } else {
            assert(btree_search(tree, i) == 1);
        }
    }

    btree_free(tree);
    printf("Deletion: PASS\n");
}


/*Main Function*/
int main() {
    printf("Starting B-Tree Tests...\n");
    
    test_basic_insert_search();
    test_deletion();
    
    printf("All tests passed.\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BTree.h" 

/*Structure Definition*/
struct BtreeNode {
    int *keys;
    struct BtreeNode **children;
    int n;
    int leaf;
    int degree; 
};

struct BTree {
    BtreeNode *root;
    int degree;
};

// Node managment - Memory Creation & Memory Allocation for Node

static BtreeNode *node_create(int leaf, int degree) {
    BtreeNode *n = calloc(1, sizeof(BtreeNode));
    if (!n) return NULL; 
    
    n->leaf = leaf;
    n->n = 0;
    n->degree = degree;
    
    n->keys = calloc(2 * degree - 1, sizeof(int));
    n->children = calloc(2 * degree, sizeof(BtreeNode*));
    
    if (!n->keys || !n->children) {
        free(n->keys);
        free(n->children);
        free(n);
        return NULL;
    }
    return n;
}

BTree *btree_create(int degree) {
    if (degree < 2) return NULL; 
    
    BTree *tree = malloc(sizeof(BTree));
    if (!tree) return NULL;
    
    tree->degree = degree;
    tree->root = node_create(1, degree);
    return tree;
}

// Searching

static int search_node(const BtreeNode *node, int key) {
    if (!node) return 0;
    
    int i = 0;
    while (i < node->n && key > node->keys[i])
        i++;
        
    if (i < node->n && key == node->keys[i])
        return 1;
        
    if (node->leaf)
        return 0;
        
    return search_node(node->children[i], key);
}

int btree_search(const BTree *tree, int key) {
    if (!tree || !tree->root) return 0;
    return search_node(tree->root, key);
}


// Insertion

/*Splitting is done with insertion to balance the node*/
static void split_child(BtreeNode *parent, int i, BtreeNode *child) {
    int degree = child->degree;
    BtreeNode *sibling = node_create(child->leaf, degree);
    sibling->n = degree - 1;

    for (int j = 0; j < degree - 1; j++)
        sibling->keys[j] = child->keys[j + degree];

    if (!child->leaf) {
        for (int j = 0; j < degree; j++)
            sibling->children[j] = child->children[j + degree];
    }

    child->n = degree - 1;

    for (int j = parent->n; j >= i + 1; j--)
        parent->children[j + 1] = parent->children[j];

    parent->children[i + 1] = sibling; 

    for (int j = parent->n - 1; j >= i; j--)
        parent->keys[j + 1] = parent->keys[j];

    parent->keys[i] = child->keys[degree - 1];
    parent->n++;
}

static void insert_notfull(BtreeNode *node, int key) {
    int i = node->n - 1;
    int degree = node->degree;

    if (node->leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->n++;
    } else {
        while (i >= 0 && key < node->keys[i])
            i--;
        i++;
        
        if (node->children[i]->n == 2 * degree - 1) {
            split_child(node, i, node->children[i]); 
            if (key > node->keys[i])
                i++;
        }
        insert_notfull(node->children[i], key);
    }
}

void btree_insert(BTree *tree, int key) { 
    if (!tree || !tree->root) return;
    
    BtreeNode *root = tree->root;
    int degree = tree->degree;

    if (root->n == 2 * degree - 1) {
        BtreeNode *new_root = node_create(0, degree);
        new_root->children[0] = root;
        tree->root = new_root;
        split_child(new_root, 0, root); 
        insert_notfull(new_root, key);
    } else {
        insert_notfull(root, key);
    }
}

// Deletion

static int get_predecessor(BtreeNode *node) {
    while (!node->leaf)
        node = node->children[node->n];
    return node->keys[node->n - 1];
}

static int get_successor(BtreeNode *node) {
    while (!node->leaf)
        node = node->children[0];
    return node->keys[0];
}


/*Merging is done to create a balance during deletion*/

static void merge_children(BtreeNode *node, int i) {
    BtreeNode *left  = node->children[i];
    BtreeNode *right = node->children[i + 1];
    int degree = node->degree;

    left->keys[degree - 1] = node->keys[i];

    for (int j = 0; j < right->n; j++)
        left->keys[degree + j] = right->keys[j];

    if (!left->leaf) {
        for (int j = 0; j <= right->n; j++)
            left->children[degree + j] = right->children[j];
    }

    left->n = 2 * degree - 1;

    for (int j = i; j < node->n - 1; j++)
        node->keys[j] = node->keys[j + 1];

    for (int j = i + 1; j < node->n; j++)
        node->children[j] = node->children[j + 1];

    node->n--;
    
    free(right->keys);
    free(right->children);
    free(right);
}



static void borrow_from_prev(BtreeNode *node, int i) {
    BtreeNode *child = node->children[i];
    BtreeNode *sibling = node->children[i - 1];

    for (int j = child->n - 1; j >= 0; j--)
        child->keys[j + 1] = child->keys[j];

    if (!child->leaf) {
        for (int j = child->n; j >= 0; j--)
            child->children[j + 1] = child->children[j];
    }

    child->keys[0] = node->keys[i - 1];

    if (!child->leaf)
        child->children[0] = sibling->children[sibling->n];

    node->keys[i - 1] = sibling->keys[sibling->n - 1];

    child->n++;
    sibling->n--;
}



static void borrow_from_next(BtreeNode *node, int i) {
    BtreeNode *child = node->children[i];
    BtreeNode *sibling = node->children[i + 1];

    child->keys[child->n] = node->keys[i];

    if (!child->leaf)
        child->children[child->n + 1] = sibling->children[0];

    node->keys[i] = sibling->keys[0];

    for (int j = 1; j < sibling->n; j++)
        sibling->keys[j - 1] = sibling->keys[j];

    if (!sibling->leaf) {
        for (int j = 1; j <= sibling->n; j++)
            sibling->children[j - 1] = sibling->children[j];
    }

    child->n++;
    sibling->n--;
}



static void fill_child(BtreeNode *node, int i) {
    int degree = node->degree;
    
    if (i != 0 && node->children[i - 1]->n >= degree)
        borrow_from_prev(node, i);
    else if (i != node->n && node->children[i + 1]->n >= degree)
        borrow_from_next(node, i);
    else {
        if (i != node->n)
            merge_children(node, i);
        else
            merge_children(node, i - 1);
    }
}



static void delete_from_node(BtreeNode *node, int key) {
    int i = 0;
    int degree = node->degree;
    
    while (i < node->n && key > node->keys[i])
        i++;

    if (i < node->n && key == node->keys[i]) {
        if (node->leaf) {
            for (int j = i; j < node->n - 1; j++)
                node->keys[j] = node->keys[j + 1];
            node->n--;
        } else {
            if (node->children[i]->n >= degree) {
                int pred = get_predecessor(node->children[i]);
                node->keys[i] = pred;
                delete_from_node(node->children[i], pred);
            } else if (node->children[i + 1]->n >= degree) {
                int succ = get_successor(node->children[i + 1]);
                node->keys[i] = succ;
                delete_from_node(node->children[i + 1], succ);
            } else {
                merge_children(node, i); 
                delete_from_node(node->children[i], key);
            }
        }
    } else {
        if (node->leaf)
            return;

        int last = (i == node->n);

        if (node->children[i]->n < degree)
            fill_child(node, i);

        if (last && i > node->n)
            delete_from_node(node->children[i - 1], key);
        else
            delete_from_node(node->children[i], key);
    }
}




void btree_delete(BTree *tree, int key) { 
    if (!tree || !tree->root || tree->root->n == 0)
        return;

    delete_from_node(tree->root, key);

    if (tree->root->n == 0 && !tree->root->leaf) {
        BtreeNode *old_root = tree->root; 
        tree->root = tree->root->children[0];
        
        free(old_root->keys);
        free(old_root->children);
        free(old_root);
    }
}


// Utility

static void print_node(const BtreeNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("[");
    for (int i = 0; i < node->n; i++) {
        printf("%d", node->keys[i]);
        if (i < node->n - 1)
            printf(", ");
    }
    printf("]\n");

    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++)
            print_node(node->children[i], depth + 1);
    }
}



void btree_print(const BTree *tree, int depth) {
    if (!tree || !tree->root) return;
    print_node(tree->root, depth);
}

//Recursively sets node free
static void free_nodes(BtreeNode *node) {
    if (!node) return;
    
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            free_nodes(node->children[i]);
        }
    }
    
    free(node->keys);
    free(node->children);
    free(node);
}

void btree_free(BTree *tree) {
    if (!tree) return;
    free_nodes(tree->root);
    free(tree);
}
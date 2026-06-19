#ifndef BTREE_H
#define BTREE_H

/* Forward declarations*/
typedef struct BtreeNode BtreeNode;
typedef struct BTree BTree;

/* Prototypes */
BTree   *btree_create(int degree);
void     btree_insert(BTree *tree, int key);
int      btree_search(const BTree *tree, int key);
void     btree_delete(BTree *tree, int key);
void     btree_print(const BTree *tree, int depth);
void     btree_free(BTree *tree);

#endif
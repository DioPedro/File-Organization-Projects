#ifndef BTREE_H
#define BTREE_H

typedef struct _HEADER header;
typedef struct _PAGE page;
typedef struct _PROMOTION promo_page;
typedef struct _BTREE btree;

btree *init_tree(char *filename);
btree *load_btree(char *filename);
long long int search_key(btree *tree, int to_search);
void insert_in_btree(btree *tree, int key, long long int offset);
void destroy_btree(btree *tree);
void update_tree_header(btree *tree);
void set_tree_in_use(btree *tree);

#endif
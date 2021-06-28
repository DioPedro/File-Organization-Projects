#include "../includes/btree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define TREE_ORDER 5
#define PAGE_SIZE 77

struct _PROMOTION {
    int value;
    int left_rrn;
    long long int offset;
};

struct _HEADER {
    char status;         // '0' = inconsistente
    int root;            // RRN da raiz 
    int next_RRN;         // RRN do prox no a ser inserido
    char thrash[68];     
};

struct _PAGE {
    char is_leaf;
    int num_of_keys;
    int RRN;
    int p[5];       // Pj (1 ≤ j ≤ q) é um ponteiro para uma subárvore
    int c[4];       // Cada Ci (1 ≤ i ≤ q – 1) é uma chave de busca. 
    
    // Cada PRi (1 ≤ i ≤ q – 1) é um campo de referência para o registro no arquivo de 
    //dados que contém o registro de dados correspondente a Ci. 
    long long int pr[4];
};

int bin_search(int keys[], int min, int max, int searched) {
    int mid = (min + max) / 2;
    while (min < max) {
        if (keys[mid] == searched)
            return mid;
        
        if (keys[mid] > searched) {
            max = mid - 1;
        } else {
            min = mid + 1;
        }

        mid = (min + max) / 2;
    }

    return mid;
}

char *generate_thrash(int len) {
    char *thrash = malloc(len * sizeof(char));
    for (int i = 0; i < len; i++)
        thrash[i] = '@';
    
    return thrash;
}

void create_root(FILE *index_fp, header *index_header, int right_child, promo_page page_data) {
    fseek(index_fp, index_header->next_RRN * PAGE_SIZE, SEEK_SET);
    
    page root_data;
    root_data.c[0] = page_data.value;
    root_data.pr[0] = page_data.offset;
    root_data.p[0] = page_data.left_rrn;
    root_data.p[1] = right_child;
    root_data.num_of_keys = 1;
    root_data.RRN = index_header->next_RRN++;
    root_data.num_of_keys = 1;
    write_page(index_fp, root_data);
}

void write_header(FILE *index_fp, int root_rrn, char status, int to_insert_rrn) {
    fwrite(&status, sizeof(char), 1, index_fp);
    fwrite(&root_rrn, sizeof(int), 1, index_fp);
    fwrite(&to_insert_rrn, sizeof(int), 1, index_fp);
    
    char *thrash;
    generate_thrash(68);
    fwrite(thrash, sizeof(char), 68, index_fp);
    free(thrash);
}

void write_page(FILE *index_fp, page to_insert) {
    fwrite(&to_insert.is_leaf, sizeof(char), 1, index_fp);
    fwrite(&to_insert.num_of_keys, sizeof(int), 1, index_fp);
    fwrite(&to_insert.RRN, sizeof(long long int), 1, index_fp);
    
    for (int i = 0; i < to_insert.num_of_keys; i++) {
        fwrite(&to_insert.p[i], sizeof(int), 1, index_fp);
        fwrite(&to_insert.c[i], sizeof(int), 1, index_fp);
        fwrite(&to_insert.pr[i], sizeof(long long int), 1, index_fp);
    }
    int last_pr = to_insert.num_of_keys;
    fwrite(&to_insert.pr[last_pr], sizeof(long long int), 1, index_fp);
}

FILE *init_tree() {
    FILE *index_fp = fopen("index.txt", "wb");

    write_header(index_fp, -1, '0', 1);

    return index_fp;
}

void read_header(FILE *index_fp, header *index_header) {
    fread(index_header->status, sizeof(char), 1, index_fp);
    fread(index_header->root, sizeof(int), 1, index_fp);
    fread(index_header->next_RRN, sizeof(int), 1, index_fp);
    fread(index_header->thrash, sizeof(char), 68, index_fp);
}

void read_page(FILE *index_fp, page *data_reg) { 
    fread(data_reg->is_leaf, sizeof(char), 1, index_fp);
    fread(data_reg->num_of_keys, sizeof(int), 1, index_fp);
    fread(data_reg->RRN, sizeof(int), 1, index_fp);
    for (int i = 0; i < data_reg->num_of_keys; i++) {
        fread(data_reg->p[i], sizeof(int), 1, index_fp);
        fread(data_reg->c[i], sizeof(int), 1, index_fp);
        fread(data_reg->pr[i], sizeof(long long int), 1, index_fp);
    }

    if (data_reg->num_of_keys == TREE_ORDER)
        fread(data_reg->p[TREE_ORDER - 1], sizeof(int), 1, index_fp);
}

long long int recursive_search(FILE *index_fp, int rrn, int to_search) {
    page content;
    fseek(index_fp, PAGE_SIZE * rrn, SEEK_SET);
    read_page(index_fp, &content);
    
    int pos = bin_search(content.c, 0, content.num_of_keys, to_search);
    if (content.c[pos] == to_search)
        return content.pr[pos];
    else if (content.is_leaf == '1')
        return -1;
    else if (content.c[pos] < to_search)
        return recursive_search(index_fp, content.p[pos + 1], to_search);
    else 
        return recursive_search(index_fp, content.p[pos], to_search);
}

long long int search_key(FILE *index_fp, int root_rrn, int to_search) {
    header index_header;
    read_header(index_fp, &index_header);
    if (index_header.root ==  -1)   // Nao tem nenhum nó ainda
        return -1;
    
    return recursive_search(index_fp, index_header.root, to_search);
}

void split(FILE *index_fp, header *btree_header, page *to_split, promo_page *promoted) {
    page new_page;
    new_page.RRN = btree_header->next_RRN; // novo nó
    new_page.is_leaf = to_split->is_leaf;

    bool was_added = FALSE;
    int p[6], pr[5], keys[5];

    // insere num vetor auxiliar os elementos ordenados
    for (int i = 0; i < 5; i++) {
        if (i == TREE_ORDER - 1 && !was_added) {
            keys[i] = promoted->value;
            p[i] = promoted->left_rrn;
            p[i + 1] = to_split->p[i];
            pr[i] = promoted->offset;
            break;   
        }

        if (was_added) {
            keys[i] = to_split->c[i - 1];
            pr[i] = to_split->pr[i - 1];
            p[i + 1] = to_split->p[i];
        }

        if (!was_added && promoted->value < to_split->c[i]) {
            keys[i] = promoted->value;
            pr[i] = promoted->offset;
            p[i] = promoted->left_rrn;
            p[i + 1] = to_split->p[i];
            was_added = TRUE;
        } else if (!was_added) {
            keys[i] = to_split->c[i];
            p[i] = to_split->p[i];
            pr[i] = to_split->pr[i];
        }
    }

    // split
    to_split->num_of_keys = TREE_ORDER/2;
    for (int i = 0; i < TREE_ORDER/2; i++) {
        to_split->c[i] = keys[i];
        to_split->p[i] = p[i];
        to_split->pr[i] = pr[i];
    }
    to_split->p[TREE_ORDER/2] = p[TREE_ORDER/2];

    new_page.num_of_keys = TREE_ORDER/2;
    for (int i = TREE_ORDER/2 + 1; i < TREE_ORDER; i++) {
        new_page.c[i] = keys[i];
        new_page.pr[i] = pr[i];
        new_page.p[i] = p[i];
    }
    new_page.p[TREE_ORDER] = p[TREE_ORDER];

    // Escreve nova pagina
    write_page(index_fp, new_page);
    
    // Nova chave a ser promovida
    promoted->value = keys[TREE_ORDER/2];           
    promoted->offset = pr[TREE_ORDER/2];
    promoted->left_rrn = to_split->RRN;
}

void insert_inner(FILE *index_fp, header *index_header, page *curr_page, promo_page *promoted_page) {
    fseek(index_fp, 1, SEEK_CUR); // Pula is_leaf

    bool was_added = FALSE;
    int p[6], pr[5], keys[5];

    // insere num vetor auxiliar os elementos ordenados
    for (int i = 0; i <= curr_page->num_of_keys; i++) {
        if (i == curr_page->num_of_keys && !was_added) {
            keys[i] = promoted_page->value;
            p[i] = promoted_page->left_rrn;
            p[i + 1] = curr_page->p[i];
            pr[i] = promoted_page->offset;
            break;   
        }

        if (was_added) {
            keys[i] = curr_page->c[i - 1];
            pr[i] = curr_page->pr[i - 1];
            p[i + 1] = curr_page->p[i];
        }

        if (!was_added && promoted_page->value < curr_page->c[i]) {
            keys[i] = promoted_page->value;
            pr[i] = promoted_page->offset;
            p[i] = promoted_page->left_rrn;
            p[i + 1] = curr_page->p[i];
            was_added = TRUE;
        } else if (!was_added) {
            keys[i] = curr_page->c[i];
            p[i] = curr_page->p[i];
            pr[i] = curr_page->pr[i];
        }
    }

    curr_page->num_of_keys++;
}

promo_page recursive_insert(FILE *index_fp, header *index_header, int curr_rrn, int key, long long int offset) {
    page content;
    fseek(index_fp, PAGE_SIZE * curr_rrn, SEEK_SET);
    read_page(index_fp, &content);
    
    promo_page promoted_page;
    promoted_page.value = key;
    
    if (content.is_leaf == '1') {
        promoted_page.value = key;
        promoted_page.offset = offset;
        promoted_page.left_rrn = -1;

        // Tem espaço
        if (content.num_of_keys < TREE_ORDER - 1) {
            insert_inner(index_fp, index_header, &content, &promoted_page);
        
        // Nao tem espaco -> split
        } else {
            split(index_fp, index_header, &content, &promoted_page);
        }

        write_page(index_fp, content);

        return promoted_page;
    }

    int pos = bin_search(content.c, 0, content.num_of_keys, key);
    if (content.c[pos] > key) {
        promoted_page = recursive_insert(index_fp, index_header, content.p[pos], key, offset);
    } else if (content.c[pos] < key) {
        promoted_page = recursive_insert(index_fp, index_header, content.p[pos + 1], key, offset);
    }

    // Rolou split
    if (promoted_page.value != key) {
        if (content.c[pos] > key)
            content.p[pos] = promoted_page.left_rrn;
        else if (content.c[pos] < key) 
            content.p[pos + 1] = promoted_page.left_rrn;

        if (content.num_of_keys < TREE_ORDER - 1) { // Tem espaço, só inserir no meio
            insert_inner(index_fp, index_header, &content, &promoted_page);
        } else {    // Sem espaço, split em nó intermediario
            split(index_fp, index_header, &content, &promoted_page);
        }
    }
    write_page(index_fp, content);
    
    return promoted_page;
}

void insert(FILE *index_fp, header *index_header, int key, long long int offset) {
    promo_page new_root;
    if (index_header->root == -1) {
        new_root.value = key;
        new_root.left_rrn = -1;
        new_root.offset = offset;
        create_root(index_fp, index_header, -1, new_root);

        return;
    }

    new_root = recursive_insert(index_fp, index_header, index_header->root, key, offset);
    if (new_root.value == key) {
        create_root(index_fp, index_header, index_header->root, new_root);
    }
}

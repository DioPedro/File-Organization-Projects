#include "../includes/btree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define TREE_ORDER 5
#define PAGE_SIZE 77

typedef enum {HEADER, DATA} REG_TYPE;

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

typedef union _REG {
    header file_header;
    page file_page; 
} REG;

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

void write_header(FILE *index_fp, int root_rrn, char status, int to_insert_rrn) {
    fwrite(&status, sizeof(char), 1, index_fp);
    fwrite(&root_rrn, sizeof(int), 1, index_fp);
    fwrite(&to_insert_rrn, sizeof(int), 1, index_fp);
    
    char *thrash;
    generate_thrash(68);
    fwrite(thrash, sizeof(char), 68, index_fp);
    free(thrash);
}

void write_new_page(FILE *index_fp, page to_insert) {
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

static void read_register(FILE *index_fp, REG *new_page, REG_TYPE reg_type) {
    if (reg_type == HEADER) {
        fread(new_page->file_header.status, sizeof(char), 1, index_fp);
        fread(new_page->file_header.root, sizeof(int), 1, index_fp);
        fread(new_page->file_header.next_RRN, sizeof(int), 1, index_fp);
        fread(new_page->file_header.thrash, sizeof(char), 68, index_fp);
    } else {
        fread(new_page->file_page.is_leaf, sizeof(char), 1, index_fp);
        fread(new_page->file_page.num_of_keys, sizeof(int), 1, index_fp);
        fread(new_page->file_page.RRN, sizeof(int), 1, index_fp);
        for (int i = 0; i < new_page->file_page.num_of_keys; i++) {
            fread(new_page->file_page.p[i], sizeof(int), 1, index_fp);
            fread(new_page->file_page.c[i], sizeof(int), 1, index_fp);
            fread(new_page->file_page.pr[i], sizeof(long long int), 1, index_fp);
        }

        if (new_page->file_page.num_of_keys == TREE_ORDER)
            fread(new_page->file_page.p[TREE_ORDER - 1], sizeof(int), 1, index_fp);
    }
}

long long int recursive_search(FILE *index_fp, int rrn, int to_search) {
    page content;
    fseek(index_fp, PAGE_SIZE * rrn, SEEK_SET);
    read_register(index_fp, &content, DATA);
    
    int pos = bin_search(content.c, 0, content.num_of_keys, to_search);
    if (content.c[pos] == to_search)
        return content.pr[pos];
    else if (content.is_leaf == '1')
        return -1;
    else if (content.c[pos] < to_search)
        return recursive_search(index_fp, content.p[pos + 1]);
    else 
        return recursive_search(index_fp, content.p[pos]);
}

long long int search_key(FILE *index_fp, int root_rrn, int to_search) {
    header index_header;
    read_register(index_fp, &index_header, HEADER);
    if (index_header.root ==  -1)   // Nao tem nenhum nó ainda
        return -1;
    
    return recursive_search(index_fp, index_header.root, to_search);
}

page split(FILE *index_fp, header *btree_header, page *to_split, int promote_key, long long int offset) {
    page new_page;
    new_page.RRN = btree_header->next_RRN++; // novo nó - RRN + 1
    new_page.is_leaf = to_split->is_leaf;

    bool was_added = FALSE;
    int p[4], pr[6], keys[5], mid_elem = to_split->c[1];

    // insere num vetor auxiliar os elementos ordenados
    for (int i = 0; i < 5; i++) {
        if (i == 4 && !was_added) {
            keys[4] = promote_key;
            p[4] = offset;
            pr[4] = to_split[4];
            pr[5] = new_page.RRN;
            break;   
        }

        if (was_added) {
            keys[i] = to_split.c[i - 1];
            p[i] = to_split->p[i - 1];
            pr[i + 1] = to_split->pr[i];
        }

        if (!was_added && promote_key < to_split->c[i]) {
            keys[i] = promote_key;
            p[i] = offset;
            pr[i] = to_split->pr[i];
            pr[i + 1] = new_page.RRN;
            was_added = TRUE;
        } else if (!was_added) {
            keys[i] = to_split.c[i];
            p[i] = to_split->p[i];
            pr[i] = to_split->pr[i]
        }
    }

    // split
    to_split->num_of_keys = TREE_ORDER/2;
    for (int i = 0; i < TREE_ORDER/2; i++) {
        to_split->c[i] = keys[i];
        to_split->p[i] = p[i];
        to_split->pr[i] = pr[i];
    }
    to_split->pr[(TREE_ORDER/2) + 1] = pr[(TREE_ORDER/2) + 1];

    new_page.num_of_keys = TREE_ORDER/2;
    for (int i = TREE_ORDER/2 + 1; i < TREE_ORDER/; i++) {
        new_page.c[i] = keys[i];
        new_page.p[i] = p[i];
        new_page.pr[i + 1] = pr[i + 1];
    }
    new_page.pr[TREE_ORDER] = pr[TREE_ORDER];

    // Escreve nova pagina
    write_new_page(index_fp, new_page);

    // retorna elemento a ser promovido
    return keys[TREE_ORDER/2];
}

void insert_in_leaf(FILE *index_fp, page curr_page, int new_key) {
    bool was_added = FALSE;
    int new_key_pos;
    for (int i = 0; i < curr_page.num_of_keys; i++) {
        if (was_added) {
            curr_page.c[i] = to_split.c[i - 1];
        }

        if (!was_added && new_key < to_split->c[i]) {
            new_key_pos = i;
            was_added = TRUE;
        }
    }
    curr_page.c[new_key_pos] = new_key;
    curr_page.num_of_keys++;
    
    write_new_page(index_fp, curr_page);
}

void insert(FILE *index_fp, int curr_rrn, int key, int *new_page_rrn, int *new_page_key) {
    page content;
    fseek(index_fp, PAGE_SIZE * curr_rrn, SEEK_SET);
    read_register(index_fp, &content, DATA);
    
    if (content.is_leaf == '1') {
        // Tem espaço
        if (content.num_of_keys < TREE_ORDER - 1) {
            int pos = bin_search(content.c, 0, content.num_of_keys, key);
            if (key < content.c[pos])
                // insere na esquerda
            else 
                // insere na direita
        } else {
        // Nao tem espaco -> split

        }
    }

    int pos = bin_search(content.c, 0, content.num_of_keys, key);
    if (content.c[pos] < to_search)
        insert();
    else 
        insert();
    
    if (content.num_of_keys == TREE_ORDER - 1) {
        // no intermediario sem espaco
    } else {
        // no intermediario com espaco
    }
}

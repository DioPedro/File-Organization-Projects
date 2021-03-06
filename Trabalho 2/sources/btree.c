/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/  

#include "../includes/btree.h"
#include "../includes/utils.h"
#include <stdio.h>
#include <stdlib.h>

#define TREE_ORDER 5
#define PAGE_SIZE 77

struct _PROMOTION {
    int value;
    int left_rrn;
    long long int offset;
};

struct _BTREE {
    header *btree_header;
    FILE *btree;
};

struct _HEADER {
    char status;          // '0' = inconsistente
    int root;             // RRN da raiz 
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

// Função que faz uma busca binária
int bin_search(int keys[], int min, int max, int searched) {
    int mid = (min + max) / 2;
    while (min < max) {
        if (keys[mid] == searched)
            return mid;
        
        if (keys[mid] > searched) {
            max = mid - 1;          // Procura na metade da esquerda
        } else {
            min = mid + 1;          // Procura na metade da direita
        }

        mid = (min + max) / 2;
    }

    return mid;
}

// Função que carrega uma árvore B
btree *load_btree(char *filename) {
    // Tentando alocar espaço na memória para a árvore
    btree *new_tree = malloc(sizeof(btree));
    if (new_tree == NULL)
        return NULL;
    
    // Salvando o ponteiro do arquivo binário
    new_tree->btree = fopen(filename, "r+b");
    if (new_tree->btree == NULL) {
        free(new_tree);
        return NULL;
    }
    
    // Tentando alocar espaço na memória para o cabeçalho da árvore B
    new_tree->btree_header = malloc(sizeof(header));
    if (new_tree->btree_header == NULL) {
        fclose(new_tree->btree);
        free(new_tree);
        return NULL;
    }

    // Caso tenhamos conseguido alocar o espaço checamos a consistência do arquivo
    fread(&new_tree->btree_header->status, sizeof(char), 1, new_tree->btree);
    if (new_tree->btree_header->status == '0') {
        destroy_btree(new_tree);
        return NULL;
    }

    // O arquivo estando consistente, lemos as informações do cabeçalho
    fread(&new_tree->btree_header->root, sizeof(int), 1, new_tree->btree);
    fread(&new_tree->btree_header->next_RRN, sizeof(int), 1, new_tree->btree);
    fseek(new_tree->btree, 68, SEEK_CUR);

    return new_tree;
}

// Função que atualiza o cabeçalho do arquivo após uso
void update_tree_header(btree *tree){
    // Reescrevendo o novo RRN da raiz e o RRN da proxima inserção
    fseek(tree->btree, 1, SEEK_SET);
    fwrite(&tree->btree_header->root, sizeof(int), 1, tree->btree);
    fwrite(&tree->btree_header->next_RRN, sizeof(int), 1, tree->btree);

    // Coloca o arquivo como consistente após todas as mudanças
    tree->btree_header->status = '1';
    fseek(tree->btree, 0, SEEK_SET);
    fwrite(&tree->btree_header->status, sizeof(char), 1, tree->btree);
}

// Função que atualiza o status do arquivo para o uso
void set_tree_in_use(btree *tree){
    fseek(tree->btree, 0, SEEK_SET);

    // Coloca o arquivo como inconsistente
    tree->btree_header->status = '0';
    fwrite(&tree->btree_header->status, sizeof(char), 1, tree->btree);
}

// Função que gera uma linha com lixo usando o tamanho passado por parâmetro
char *generate_thrash(int len) {
    char *thrash = malloc(len * sizeof(char));
    for (int i = 0; i < len; i++)
        thrash[i] = '@';

    return thrash;
}

// Função que escreve uma página de disco no arquivo de índices
void write_page(FILE *index_fp, page to_insert) {
    // Indo para o RRN certo do arquivo e escrevendo as informações da página
    fseek(index_fp, (to_insert.RRN + 1)  * PAGE_SIZE, SEEK_SET);
    fwrite(&to_insert.is_leaf, sizeof(char), 1, index_fp);
    fwrite(&to_insert.num_of_keys, sizeof(int), 1, index_fp);
    fwrite(&to_insert.RRN, sizeof(int), 1, index_fp);
    
    // Escrevendo os índices: primeiro ponteiro da esquerda, seguido da chave e offset da chave 
    for (int i = 0; i < to_insert.num_of_keys; i++) {
        fwrite(&to_insert.p[i], sizeof(int), 1, index_fp);
        fwrite(&to_insert.c[i], sizeof(int), 1, index_fp);
        fwrite(&to_insert.pr[i], sizeof(long long int), 1, index_fp);
    }
    // Escreve o ultimo ponteiro da direita
    int last_p = to_insert.num_of_keys; 
    fwrite(&to_insert.p[last_p], sizeof(int), 1, index_fp);

    // Preenchendo o que nao existe com lixo (-1)
    int pc_thrash = -1;
    long long int pr_thrash = -1;
    for (int i = to_insert.num_of_keys; i < TREE_ORDER - 1; i++) {
        fwrite(&pc_thrash, sizeof(int), 1, index_fp);
        fwrite(&pr_thrash, sizeof(long long int), 1, index_fp);
        fwrite(&pc_thrash, sizeof(int), 1, index_fp);
    }
}

// Função que cria uma página raiz
void create_root(FILE *index_fp, header *index_header, int right_child, promo_page page_data, int is_leaf) {  
    page root_data;
    root_data.is_leaf = is_leaf == 1 ? '1': '0';  // Se for passado que é passado para criar raiz, o faz
    root_data.num_of_keys = 1;                    // Toda vez começa com apenas um elemento
    root_data.RRN = index_header->next_RRN++;     // Atribui o RRN e ja atualiza o RRN do proximo no cabeçalho
    root_data.c[0] = page_data.value;             // Atribui valor da chave,
    root_data.pr[0] = page_data.offset;           //  do offset da chave e
    root_data.p[0] = page_data.left_rrn;          //  filhos da esquerda e 
    root_data.p[1] = right_child;                 //  direita
    write_page(index_fp, root_data);              // Escreve os dados 

    index_header->root = root_data.RRN;           // Atualiza RRN da raiz no cabeçalho
}

// Função que escreve o cabeçalho no arquivo de índices
void write_header(FILE *index_fp, int root_rrn, char status, int to_insert_rrn) {
    fwrite(&status, sizeof(char), 1, index_fp);
    fwrite(&root_rrn, sizeof(int), 1, index_fp);
    fwrite(&to_insert_rrn, sizeof(int), 1, index_fp);
    
    char *thrash = generate_thrash(68);
    fwrite(thrash, sizeof(char), 68, index_fp);
    free(thrash);
}

// Função que inicia uma árvore B
btree *init_tree(char *filename) {
    btree *new_btree = malloc(sizeof(btree));
    if (new_btree == NULL) 
        return NULL;

    // Tentando abrir o arquivo no modo de escrita
    new_btree->btree = fopen(filename, "wb+");
    if (new_btree->btree == NULL) {
        free(new_btree);
        return NULL;
    }
    
    // Tentando criar o cabeçalho do arquivo
    new_btree->btree_header = malloc(sizeof(header));
    if (new_btree->btree == NULL) {
        fclose(new_btree->btree);
        free(new_btree);
        return NULL;
    }

    // Iniciando o cabeçalho e escrevendo no arquivo
    new_btree->btree_header->root = -1;    // Não contém nenhum elemento
    new_btree->btree_header->next_RRN = 0; 
    new_btree->btree_header->status = '0'; // Inicia como inconsistente, pois a árvore é preenchida logo em seguida
    write_header(new_btree->btree, new_btree->btree_header->root, new_btree->btree_header->status, new_btree->btree_header->next_RRN);

    return new_btree;
}

// Função que lê uma página de registro
void read_page(FILE *index_fp, page *data_reg) { 
    fread(&(data_reg->is_leaf), sizeof(char), 1, index_fp);     // Leitura do campo "folha"
    fread(&(data_reg->num_of_keys), sizeof(int), 1, index_fp);  // Leitura do campo "nroChavesIndexadas"
    fread(&(data_reg->RRN), sizeof(int), 1, index_fp);          // Leitura do campo "RRNdoNo"
    for (int i = 0; i < data_reg->num_of_keys; i++) {           // Leitura dos campos "P_i", "C_i" e "Pr_i"
        fread(&(data_reg->p[i]), sizeof(int), 1, index_fp);
        fread(&(data_reg->c[i]), sizeof(int), 1, index_fp);
        fread(&(data_reg->pr[i]), sizeof(long long int), 1, index_fp);
    }
    fread(&(data_reg->p[data_reg->num_of_keys]), sizeof(int), 1, index_fp);

    // Leitura do ultimo ponteiro, caso haja
    if (data_reg->num_of_keys == TREE_ORDER)
        fread(&(data_reg->p[TREE_ORDER - 1]), sizeof(int), 1, index_fp);
}

// Função auxiliar de busca
long long int recursive_search(FILE *index_fp, int rrn, int to_search) {
    page content; // Lê o conteúdo
    fseek(index_fp, PAGE_SIZE * (rrn + 1), SEEK_SET); // Vai para o devido RRN
    read_page(index_fp, &content); // Lê os dados
    
    // A busca binaria retorna o elemento mais proximo da chave ou ela mesma
    int pos = bin_search(content.c, 0, content.num_of_keys - 1, to_search);
    if (content.c[pos] == to_search)
        return content.pr[pos];
    else if (content.is_leaf == '1')    // Se não tiver mais para onde ir e a busca binária não achou o elemento, retorna -1
        return -1;
    else if (content.c[pos] < to_search)
        return recursive_search(index_fp, content.p[pos + 1], to_search);
    else 
        return recursive_search(index_fp, content.p[pos], to_search);
}

// Função que procura por uma chave no arquivo de índices
long long int search_key(btree *tree, int to_search) {
    if (tree->btree_header->root ==  -1) {
        // Não tem nenhum nó ainda
        destroy_btree(tree);
        return -1;
    }
    
    return recursive_search(tree->btree, tree->btree_header->root, to_search);
}

// Função que insere ordenadamente
void ordered_insertion(FILE *index_fp, page *curr_page, promo_page *promoted_page, int *c, int *p, long long int *pr, int num_of_elements) {
    fseek(index_fp, 1, SEEK_CUR); // Pula is_leaf

    // Insere num vetor auxiliar os elementos ordenados
    bool was_added = FALSE;
    for (int i = 0; i <= num_of_elements; i++) {

        // Caso em que o lugar certo para inserir é na ultima posicao disponivel
        if (i == num_of_elements && !was_added) {
            c[i] = promoted_page->value;
            p[i] = promoted_page->left_rrn;
            p[i + 1] = curr_page->p[i];
            pr[i] = promoted_page->offset;
            break;
        }

        // Se foi adicionado ja, os elementos são "empurrados" para direita
        if (was_added) {
            c[i] = curr_page->c[i - 1];
            pr[i] = curr_page->pr[i - 1];
            p[i + 1] = curr_page->p[i];
        }

        // Verifica se é pra adicionar ou não
        if (!was_added && promoted_page->value < curr_page->c[i]) {
            c[i] = promoted_page->value;
            pr[i] = promoted_page->offset;
            p[i] = promoted_page->left_rrn;
            p[i + 1] = curr_page->p[i];
            was_added = TRUE;
        } else if (!was_added) {
            c[i] = curr_page->c[i];
            p[i] = curr_page->p[i];
            pr[i] = curr_page->pr[i];
        }
    }
}

// Função que faz o split
void split(FILE *index_fp, header *btree_header, page *to_split, promo_page *promoted) {
    page new_page;
    new_page.RRN = btree_header->next_RRN; // Novo nó
    
    // Como o nó criado é o da direita, ele será da mesma "categoria" que o nó da esquerda
    new_page.is_leaf = to_split->is_leaf;  

    // Criando vetores auxiliares para ajudar no split
    int c[5], p[6];
    long long int pr[5];
    ordered_insertion(index_fp, to_split, promoted, c, p, pr, TREE_ORDER - 1);

    // O nó da esquerda fica com a metade da esquerda dos dados
    to_split->num_of_keys = TREE_ORDER/2;
    for (int i = 0; i < TREE_ORDER/2; i++) {
        to_split->p[i] = p[i];
        to_split->c[i] = c[i];
        to_split->pr[i] = pr[i];
    }
    to_split->p[TREE_ORDER/2] = p[TREE_ORDER/2];

    // Analogamente, o nó da direita fica com a metade direita
    new_page.num_of_keys = TREE_ORDER/2;
    for (int i = TREE_ORDER/2 + 1, j = 0; i < TREE_ORDER; i++, j++) {
        new_page.p[j] = p[i];
        new_page.c[j] = c[i];
        new_page.pr[j] = pr[i];
    }
    new_page.p[TREE_ORDER/2] = p[TREE_ORDER];

    // Escrevendo a nova pagina
    write_page(index_fp, new_page);
    
    // A chave promovida é a do meio
    promoted->value = c[TREE_ORDER/2];           
    promoted->offset = pr[TREE_ORDER/2];
    promoted->left_rrn = to_split->RRN;
}

// Inserindo um índice dentro de uma página já existente
void insert_inner(FILE *index_fp, page *curr_page, promo_page *promoted_page) {   
    int c[5], p[6];
    long long int pr[5];
    ordered_insertion(index_fp, curr_page, promoted_page, c, p, pr, curr_page->num_of_keys);

    // Atualiza os valores na página 
    for (int i = 0; i <= curr_page->num_of_keys; i++) {
        curr_page->c[i] = c[i];
        curr_page->pr[i] = pr[i];
        curr_page->p[i] = p[i];
    }
    curr_page->p[curr_page->num_of_keys + 1] = p[curr_page->num_of_keys + 1];

    // E atualiza o número de elementos nela
    curr_page->num_of_keys++;
}

// Função auxiliar de inserção
promo_page recursive_insert(FILE *index_fp, header *index_header, int curr_rrn, int key, long long int offset) {
    page content;

    // Vai para o RRN da página e lê seu conteúdo
    fseek(index_fp, PAGE_SIZE * (curr_rrn  + 1), SEEK_SET);
    read_page(index_fp, &content);
    
    promo_page promoted_page;
    promoted_page.value = key;
    
    // Se a página for uma folha
    if (content.is_leaf == '1') {
        promoted_page.offset = offset;
        promoted_page.left_rrn = -1;

        // E tem espaço, insere nela 
        if (content.num_of_keys < TREE_ORDER - 1) {
            insert_inner(index_fp, &content, &promoted_page);
        
        // Se não tem espaço, precisa realizar split
        } else {
            split(index_fp, index_header, &content, &promoted_page);
            index_header->next_RRN++;
        }

        // Reescreve a página atual
        fseek(index_fp, (curr_rrn + 1) * PAGE_SIZE, SEEK_SET);
        write_page(index_fp, content);

        // E começa a voltar na recursão
        return promoted_page;
    }

    // Fazendo a busca para achar o local certo da inserção
    int pos = bin_search(content.c, 0, content.num_of_keys - 1, key);
    if (content.c[pos] > key) 
        promoted_page = recursive_insert(index_fp, index_header, content.p[pos], key, offset);
    else if (content.c[pos] < key) 
        promoted_page = recursive_insert(index_fp, index_header, content.p[pos + 1], key, offset);

    // Nos nós intermediários, verifica se houve split
    if (promoted_page.left_rrn != -1) {

        // Se houve, atualiza os ponteiros (P_i)
        if (content.c[pos] > key)
            content.p[pos] = index_header->next_RRN - 1;
        else if (content.c[pos] < key) 
            content.p[pos + 1] = index_header->next_RRN - 1;
        
        // E se tiver espaço, insere no meio
        if (content.num_of_keys < TREE_ORDER - 1) { 
            insert_inner(index_fp, &content, &promoted_page);
            
            // Como inseriu no meio, ao voltar para o nó anterior, não tem mais nada para inserir
            promoted_page.left_rrn = -1;
        } else {    
            // Se não tem espaço, necessário fazer split
            split(index_fp, index_header, &content, &promoted_page);
            index_header->next_RRN++;
        }
        
        // Reescrevendo a página
        fseek(index_fp, (curr_rrn + 1) * PAGE_SIZE, SEEK_SET);
        write_page(index_fp, content);
    } 
    return promoted_page;
}

// Função que faz a inserção no arquivo de índices
void insert_in_btree(btree *tree, int key, long long int offset) {
    promo_page new_root;

    // Se a árvore está vazia, cria a raiz e insere nela
    if (tree->btree_header->root == -1) { 
        tree->btree_header->root = tree->btree_header->next_RRN;
        new_root.value = key;
        new_root.left_rrn = -1;
        new_root.offset = offset;
        create_root(tree->btree, tree->btree_header, -1, new_root, 1);

        return;
    }
    
    // Se já tem algum elemento, faz a inserção  
    new_root = recursive_insert(tree->btree, tree->btree_header, tree->btree_header->root, key, offset);

    // Se retornou um nó promovido, chama a função "driver" (nossa create_root) e cria a nova raiz
    if (new_root.left_rrn != -1) {
        create_root(tree->btree, tree->btree_header, tree->btree_header->next_RRN - 1, new_root, 0);
        tree->btree_header->root = tree->btree_header->next_RRN - 1;
    }
}

// Função que fecha e libera a memória de uma árvore B
void destroy_btree(btree *tree) {
    fclose(tree->btree);
    free(tree->btree_header);
    free(tree);
}
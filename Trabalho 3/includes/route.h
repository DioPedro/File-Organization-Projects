/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#ifndef ROUTES_H
#define ROUTES_H

#include "utils.h"
#include "btree.h"

typedef enum _ROUTE_FIELD ROUTE_FIELD;

typedef struct _ROUTE_HEADER{
    char status;                // Consistência do arquivo
    long long int next_reg;     // Byte offset do próximo registro a ser inserido 
    int num_of_regs;            // Número de registros
    int num_of_removeds;        // Número de registros removidos
    char code_description[15];  // Descrição do codLinha
    char card_description[13];  // Descrição do aceitaCartão 
    char name_description[13];  // Descrição do nomeLinha
    char color_description[24]; // Descrição corLinha
} ROUTE_HEADER;

typedef struct _ROUTE{
    char is_removed;        // Indica se está removido ou não ("0" == true)
    int register_length;    // tamRegistro
    int route_code;         // codLinha
    char accepts_card;      // aceitaCartao
    int name_length;        // tamanhoNome
    char *route_name;       // nomeLinha
    int color_length;       // tamanhoCor
    char *color;            // corLinha
} ROUTE;

void create_route_binary(FILE *csv_fp, FILE *bin_fp);
void read_route_bin(FILE *bin_fp);
void read_route_register(FILE *bin_fp, ROUTE *valid_register, bool should_read_begining);
void search_route_by_field(FILE *bin_fp, char *field, char *value);
void print_route_register(ROUTE *data);
void insert_new_route(FILE *bin_fp, bool *inserted);
bool create_route_index_file(FILE *bin_fp, char *index_filename);
void search_route(FILE *bin_fp, btree *tree);
void insert_route_into_index_and_bin(FILE *bin_fp, btree *tree, bool *inserted);
int get_num_of_routes(FILE *bin_fp);
void free_route_strings(ROUTE *data);

#endif
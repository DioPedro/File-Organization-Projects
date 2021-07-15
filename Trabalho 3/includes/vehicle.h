/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#ifndef VEHICLES_H
#define VEHICLES_H

#include "utils.h"
#include "btree.h"

typedef enum _VEHICLE_FIELD VEHICLE_FIELD;
typedef struct _VEHICLE_HEADER {
    char status;                    // Consistência do arquivo
    long long int next_reg;         // Byte offset do próximo registro a ser inserido
    int num_of_regs;                // Número de registros
    int num_of_removeds;            // Número de registros removidos
    char prefix_description[18];    // Descrição do prefixo
    char date_description[35];      // Descrição da data
    char seats_description[42];     // Descrição da quantidade de lugares     
    char route_description[26];     // Descrição da rota
    char model_description[17];     // Descrição do modelo
    char category_description[20];  // Descrição da categoria
} VEHICLE_HEADER;

typedef struct _VEHICLE {
    char is_removed;        // Indica se está removido ou não ("0" == true)
    int register_length;    // tamRegistro
    char prefix[5];         // Prefixo
    char date[10];          // Data
    int num_of_seats;       // Número de lugares           
    int route_code;         // Código da rota
    int model_length;       // Tamanho do campo modelo
    char *model;            // Modelo
    int category_length;    // Tamanho do campo categoria  
    char *category;         // Categoria
} VEHICLE;

void create_vehicle_binary(FILE *csv_fp, FILE *bin_fp);
void read_vehicle_bin(FILE *bin_fp);
void read_vehicle_register(FILE *bin_fp, VEHICLE *valid_register, bool should_read_begining);
void search_vehicle_by_field(FILE *bin_fp, char *field, char *value);
void print_vehicle_register(VEHICLE *data, bool has_endl);
void insert_new_vehicle(FILE *bin_fp, bool *inserted);
bool create_vehicle_index_file(FILE *bin_fp, char *index_filename);
void search_vehicle(FILE *bin_fp, btree *tree);
void insert_vehicle_into_index_and_bin(FILE *bin_fp, btree *tree, bool *inserted);
int get_num_of_vehicles(FILE *bin_fp);
void free_vehicle_strings(VEHICLE *data);

#endif
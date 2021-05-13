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

typedef struct _VEHICLE_HEADER{
    char status;
    long long int next_reg;
    int num_of_regs;
    int num_of_removeds;
    char prefix_description[18];
    char date_description[35];
    char seats_description[42]; 
    char route_description[26]; 
    char model_description[17];
    char category_description[20];
} VEHICLE_HEADER;

// *_length ignores '\0'
typedef struct _VEHICLE{
    char is_removed;        // "0" == true
    int register_length;
    char prefix[5];
    char date[10];
    int num_of_seats;
    int route_code;
    int model_length;
    char *model;
    int category_length;
    char *category;
} VEHICLE;

#endif
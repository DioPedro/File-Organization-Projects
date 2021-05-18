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

typedef enum _VEHICLE_FIELD VEHICLE_FIELD;
typedef struct _VEHICLE_HEADER VEHICLE_HEADER;
typedef struct _VEHICLE VEHICLE;

void create_vehicle_binary(FILE *csv_fp, FILE *bin_fp);
void read_vehicle_bin(FILE *bin_fp);
void search_vehicle_by_field(FILE *bin_fp, char *field, char *value);

#endif
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

typedef enum _ROUTE_FIELD ROUTE_FIELD;
typedef struct _ROUTE_HEADER ROUTE_HEADER;
typedef struct _ROUTE ROUTE;

void create_route_binary(FILE *csv_fp, FILE *bin_fp);
void read_route_bin(FILE *bin_fp);
void search_route_by_field(FILE *bin_fp, char *field, char *value);
void insert_new_route(FILE *bin_fp, bool *inserted);

#endif
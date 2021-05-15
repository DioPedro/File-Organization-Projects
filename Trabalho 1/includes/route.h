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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

typedef struct _ROUTE_HEADER ROUTE_HEADER;
typedef struct _ROUTE ROUTE;

void create_route_binary(FILE *csv_fp, FILE *bin_fp);

#endif
/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include "bool.h"

typedef struct _WORDS WORDS;

char *read_word(FILE *input);
char *read_line(FILE *input);
WORDS *create_word_list();
WORDS *split_list(char *string, char delimiter);
int get_word_list_length(WORDS *word_list);
char **get_word_list(WORDS *word_list);
void print_word_list(WORDS *word_list);
void free_word_list(WORDS *word_list);

#endif
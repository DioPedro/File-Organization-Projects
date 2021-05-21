/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bool.h"

typedef enum { 
    Janeiro = 1, 
    Fevereiro, 
    Marco, 
    Abril,
    Maio,  
    Junho, 
    Julho, 
    Agosto, 
    Setembro, 
    Outubro, 
    Novembro, 
    Dezembro
} MONTH;

typedef struct _WORDS WORDS;

// String related utils
char *read_word(FILE *input);
char *read_line(FILE *input);
WORDS *create_word_list();
WORDS *split_list(char *string, char delimiter);
int get_word_list_length(WORDS *word_list);
char **get_word_list(WORDS *word_list);
void print_word_list(WORDS *word_list);
void print_string_without_terminator(char *string, int len, bool has_endl);
void free_word_list(WORDS *word_list);
char *read_inside_quotes();
void append_word(WORDS *base, char *new_word);
bool compare_strings_whithout_terminator(char *stringA, char *stringB, int size);

// General utils
void strings_creation(char *reg_field, char *word, int size);
void write_data_strings(FILE *bin_fp, char *data_field, int size);
void free_data(WORDS *word_list, char *reg_line);
int register_exists(FILE *fp);
void go_to_end_of_register(FILE *bin_fp, long long start_of_register, int reg_len);

#endif
#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"

void strings_creation(char *header_field, char *word, int size);
void write_data_strings(FILE *bin_fp, char *data_field, int size);
void free_data(WORDS *word_list, char *reg_line);

#endif
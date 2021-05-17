#include "../includes/utils.h"
/* 
    Stores header string fields with its respective value (@word), ignoring '\0' and filling with '@'
    If needed, this function can return @word length
    @size is the field size in bytes
*/
void strings_creation(char *header_field, char *word, int size){
    int cur_length = 0;
    for (; word[cur_length] != '\0'; cur_length++) 
        header_field[cur_length] = word[cur_length];

    if (cur_length == 0)
        header_field[cur_length] = '\0';

    size -= 1;
    for (; size > cur_length; size--)
        header_field[size] = '@';
}

// When writing on bin file, '\0'was being a problem, although we
// only wrote @size byte 
void write_data_strings(FILE *bin_fp, char *data_field, int size){
    for (int i = 0; i < size; i++)
        fwrite(&data_field[i], sizeof(char), 1, bin_fp);
}

// Free primary memory after a CSV read
void free_data(WORDS *word_list, char *reg_line){
    free_word_list(word_list);
    free(reg_line);
}
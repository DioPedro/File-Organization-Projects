/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/str.h"

struct _WORDS{
    char **words;
    int amount;
};

// Function that reads only one word
char* read_word(FILE* input){
    char *str;

    str = (char*) malloc(32 * sizeof(char)); 

    int pos = 0, tamanho = 32;
    
    do {
        if (pos == tamanho) {
            str = realloc(str, 2 * tamanho);
            tamanho *= 2;
        }

        str[pos] = (char) fgetc(input);
        pos++;
        if (str[pos-1] == '\r' ) pos--;
        else if (str[pos-1] == '\n') break;

    } while (str[pos-1] != ' ' && !feof(input));

    str[pos-1] = '\0';
    str = realloc(str, pos);
    
    return str;
}

// Function that reads a string of undefined size and allocates the exact size of heap memory 
char *read_line(FILE *input){
    char *str;

    str = malloc(32 * sizeof(char));

    int pos = 0, size = 32;
    
    do {
        if (pos == size) {
            str = realloc(str, 2 * size);
            size *= 2;
        }

        str[pos] = (char) fgetc(input);
        if (str[pos] == '\r') pos--;
        pos++;

    } while (str[pos-1] != '\n' && !feof(input));

    str[pos-1] = '\0'; 
    str = realloc(str, pos);
    
    return str;
}

WORDS *create_word_list(){
    WORDS *list = malloc(sizeof(WORDS));
    list->words = NULL;
    list->amount = 0;

    return list;
}

// Function that particionates a string and saves it to the word list
bool get_part_from_str(char *source_str, int *source_pos, char **dest_str, char delimiter) {
    *dest_str = NULL;
    bool is_end_of_part = FALSE;

    char c;
    int dest_pos = 0;
    do {
        c = source_str[*source_pos];

        if (c == '\0') {
            *dest_str = realloc(*dest_str, (dest_pos + 1) * sizeof(char));
            (*dest_str)[dest_pos] = '\0';
            return TRUE;
        }

        else if (c == delimiter) {
            is_end_of_part = TRUE;
            c = '\0';
        }

        *dest_str = realloc(*dest_str, (dest_pos + 1) * sizeof(char));
        (*dest_str)[dest_pos] = c;

        (*source_pos)++;
        dest_pos++;
    } while (!is_end_of_part);

    return FALSE;
}

// Function that parcionates a given string into its delimiters and save in a char** wordlist
WORDS *split_list(char *string, char delimiter){
    WORDS *split_list = create_word_list();

    bool is_over = FALSE;

    int str_pos = 0;

    do {
        split_list->words = realloc(split_list->words, (split_list->amount + 1) * sizeof(char *));
        char **cur_part = &split_list->words[split_list->amount];

        is_over = get_part_from_str(string, &str_pos, cur_part, delimiter); 
        split_list->amount++;
    } while (!is_over);

    return split_list;
}

int get_word_list_length(WORDS *word_list){
    if (word_list != NULL)
        return word_list->amount;
    
    return 0;
}

char **get_word_list(WORDS *word_list){
    if (word_list != NULL)
        return word_list->words;
    
    return NULL;
}

void print_word_list(WORDS *word_list){
    if (word_list == NULL)
        return;
    
    for (int i = 0; i < word_list->amount; i++){
        printf("Word[%d]: %s\n", i, word_list->words[i]);
    }
    printf("\n");
}

void free_word_list(WORDS *word_list){
    if (word_list == NULL)
        return;
    
    for (int i = 0; i < word_list->amount; i++){
        free(word_list->words[i]);
    }
    free(word_list->words);
    free(word_list);
}
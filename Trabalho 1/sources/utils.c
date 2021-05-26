/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/utils.h"

/* ------- String Utils ------- */
struct _WORDS{
    char **words;
    int amount;
};

// Function that reads only one word
char *read_word(FILE* input){
    char *str;

    str = malloc(32 * sizeof(char)); 

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
        } else if (c == delimiter) {
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

void print_string_without_terminator(char *string, int len, bool has_endl){
    for (int i = 0; i < len; i++)
        printf("%c", string[i]);

    if (has_endl)
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

char *read_inside_quotes(){
    char cur_char = getc(stdin);
    bool is_first_quote = (cur_char == '"');

    // If the first char isn't a quote, this function returns "NULO" and waits for the end of input
    if (!is_first_quote){
        char *nil = malloc(5 * sizeof(char));
        strcpy(nil, "NULO");
        while (cur_char != ' ' && cur_char != '\n')
            cur_char = getc(stdin);

        return nil;
    }

    int cur_len = 0, size = 16;
    char *content = malloc(size * sizeof(char));
    while (is_first_quote){
        if (cur_len == size){
            size *= 2;
            content = realloc(content, size);
        }

        cur_char = getc(stdin);

        // When reads the another '"', stop reading
        is_first_quote = (cur_char != '"');
        if (!is_first_quote)
            break;
        
        content[cur_len++] = cur_char;
    }

    // Escape ' '
    cur_char = getc(stdin);
    while (cur_char != '\n' && cur_char != ' ')
        cur_char = getc(stdin);

    if (cur_len == 0)
        return NULL;

    content = realloc(content, cur_len + 1);
    content[cur_len] = '\0';

    return content;
}

bool compare_strings_whithout_terminator(char *stringA, char *stringB, int stringA_size) {
    if (strcmp("NULO", stringB) == 0 && stringA[0] == '\0')
        return TRUE;
    
    if (stringA_size != strlen(stringB))
        return FALSE;
        
    for (int i = 0; i < stringA_size; i++) {
        if (stringA[i] != stringB[i])
            return FALSE;
    }

    return TRUE;
}

void append_word(WORDS *base, char *new_word){
    if (base == NULL) {
        printf("WORDS nulo, encerrando função\n");
        return;
    }
    
    base->words = realloc(base->words, (base->amount + 1) * sizeof(char *));
    base->words[base->amount++] = new_word;
}

/* ------ General Utils ------ */

// Stores header string fields with its respective value (@word), ignoring '\0' and filling with '@'
// If needed, this function can return @word length
// @size is the field size in bytes
void strings_creation(char *reg_field, char *word, int size){
    int cur_length = 0;
    for (; word[cur_length] != '\0'; cur_length++) 
        reg_field[cur_length] = word[cur_length];

    if (cur_length == 0)
        reg_field[cur_length] = '\0';

    size -= 1;
    for (; size > cur_length; size--)
        reg_field[size] = '@';
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

int register_exists(FILE *fp){
    char status = 'y';
    fread(&status, sizeof(char), 1, fp);

    return (status == '1');
}

void go_to_end_of_register(FILE *bin_fp, long long start_of_register, int reg_len){
    // @reg_len + 5 = total register size
    // @cur_offset - @start_of_register = how much we went through the register
    // left to end of register = total register size - how much we went through the register
    long long int cur_offset = ftell(bin_fp);
    long long int next_reg_offset = reg_len + 5 - (cur_offset - start_of_register);
    fseek(bin_fp, next_reg_offset, SEEK_CUR);
}

void set_file_in_use(FILE *bin_fp){
    char status = '0';
    fseek(bin_fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, bin_fp);
}
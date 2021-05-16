/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/route.h"

struct _ROUTE_HEADER{
    char status;
    long long int next_reg;
    int num_of_regs;
    int num_of_removeds;
    char code_description[15];
    char card_description[13];
    char name_description[13];
    char color_description[24];
};

// *_length ignores '\0'
struct _ROUTE{
    char is_removed;        // "0" == true
    int register_length;
    int route_code;
    char accepts_card;
    int name_length;
    char *route_name;
    int color_length;
    char *color;
};

/* 
    Stores header string fields with its respective value (@word), ignoring '\0' and filling with '@'
    If needed, this function can return @word length
    @size is the field size in bytes
*/
static void header_strings_creation(char *header_field, char *word, int size){
    int cur_length = 0;
    for (; word[cur_length] != '\0'; cur_length++) 
        header_field[cur_length] = word[cur_length];

    size -= 1;
    for (; size > cur_length; size--)
        header_field[size] = '@';
}

// When writing on bin file, '\0'was being a problem, although we
// only wrote @size byte 
static void write_data_strings(FILE *bin_fp, char *data_field, int size){
    for (int i = 0; i < size; i++)
        fwrite(&data_field[i], sizeof(char), 1, bin_fp);
}

static bool check_integrity(char *csv_field, ROUTE_HEADER *header){
    int length = strlen(csv_field);
    if (length == 0) 
        return FALSE;
    
    // If starts with an '*', the register is removed 
    if (csv_field[0] == '*'){
        header->num_of_removeds++;
        return FALSE;
    }

    if (strcmp(csv_field, "NULO") == 0) 
        return FALSE;

    return TRUE;
}

static void fill_register(ROUTE *data, char **word, ROUTE_HEADER *header){
    bool is_ok = check_integrity(word[0], header);
    if (!is_ok){
        data->route_code = atoi(word[0] + 1);
        data->is_removed = '0';
    } else {
        data->route_code = atoi(word[0]);
        data->is_removed = '1';
    }

    data->accepts_card = word[1][0]; // Copying the first char from string "x"  

    is_ok = check_integrity(word[2], header);
    if (!is_ok)
        data->name_length = 0;
    else { 
        data->name_length = strlen(word[2]);
        data->route_name = word[2];
    }

    is_ok = check_integrity(word[3], header);
    if (!is_ok)
        data->color_length = 0;
    else {
        data->color_length = strlen(word[3]);
        data->color = word[3];
    }
}

static void create_header(FILE *bin_fp, ROUTE_HEADER *header, WORDS *header_list){
    header->status = '0';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
    
    header->next_reg = 82;
    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);

    header->num_of_regs = 0;
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);

    header->num_of_removeds = 0;
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);

    char **words = get_word_list(header_list);
    if (words != NULL){
        header_strings_creation(header->code_description, words[0], 15);
        fwrite(&(header->code_description), sizeof(char), 15, bin_fp);
    
        header_strings_creation(header->card_description, words[1], 13);
        fwrite(&(header->card_description), sizeof(char), 13, bin_fp);
    
        header_strings_creation(header->name_description, words[2], 13);
        fwrite(&(header->name_description), sizeof(char), 13, bin_fp);
    
        header_strings_creation(header->color_description, words[3], 24);
        fwrite(&(header->color_description), sizeof(char), 24, bin_fp);
    }
}

static void write_data(FILE *bin_fp, ROUTE *data, ROUTE_HEADER *header){
    /*  
    4 bytes(route_code)  + 1 byte(accepts_card) 
    4 bytes(name_length) + 4 bytes(color_length)
    color_length         + name_length        
    */
    data->register_length = 13 + data->color_length + data->name_length;
    
    fwrite(&(data->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(data->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(data->route_code), sizeof(int), 1, bin_fp);
    fwrite(&(data->accepts_card), sizeof(char), 1, bin_fp);

    fwrite(&(data->name_length), sizeof(int), 1, bin_fp);
    if (data->name_length != 0)
        write_data_strings(bin_fp, data->route_name, data->name_length);

    fwrite(&(data->color_length), sizeof(int), 1, bin_fp);
    if (data->color_length != 0)
        write_data_strings(bin_fp, data->color, data->color_length);

    if (data->is_removed == '1')
        header->num_of_regs++;
}

// Free primary memory after a CSV read
static void free_data(WORDS *word_list, char *reg_line){
    free_word_list(word_list);
    free(reg_line);
}

static void update_header(FILE *bin_fp, ROUTE_HEADER *header){
    header->next_reg = ftell(bin_fp);
    
    // Going to the begining of the file to update it's data
    fseek(bin_fp, 0, SEEK_SET);

    header->status = '1';

    fwrite(&(header->status), sizeof(char), 1, bin_fp);
    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);
}

void create_route_binary(FILE *csv_fp, FILE *bin_fp){
    // Part of the csv with the header
    char *header_line = read_line(csv_fp);
    WORDS *header_list = split_list(header_line, ',');

    ROUTE_HEADER header;
    create_header(bin_fp, &header, header_list);

    free_word_list(header_list);
    free(header_line);

    // Part of the csv with the registers
    while (!feof(csv_fp)){
        char *reg_line = read_line(csv_fp);
        
        // Spliting and adding the reg_line of the csv to the binary file
        WORDS *word_list = split_list(reg_line, ',');

        // All lines must have 4 fields, otherwise it's EOF
        if (get_word_list_length(word_list) != 4){
            free_data(word_list, reg_line);
            break;
        }     

        ROUTE data;
        char **word = get_word_list(word_list);
        fill_register(&data, word, &header);
        write_data(bin_fp, &data, &header);

        free_data(word_list, reg_line);
    }

    // Updating the header of the bin file after the creation of the size
    update_header(bin_fp, &header);
}

/*
    bool create_bin(FILE *csv, FILE *bin)
        Verifica se o arquivo já está em uso
            se sim retornar erro
        Senao
            continua a função

        Reading the header
        header_line = read_line(csv_fp);
        WORDS *header_words = split_list(header_line, ',');
        
        ROUTE_HEADER route_header;

        route_header.status = '0';
        route.num_of_regs = 0;
        route.num_of_removeds = 0;
        route.code_description = header_words[0];
        route.card_description = header_words[1];
        route.name_description = header_words[2];
        route.color_description = header_words[3];

        Reading the line of the register
        while(!feof(csv_fp)){
            char *line = read_live(csv_fp)
        }
        

    void funcionalidade_1(FILE *csv_fp, FILE *bin_fp)
        bool created = create_bin(csv_fp, bin_fp)

        if (bool)
            print("SUCESSO")
        else
            print("DEU MERDA")
*/
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
        strings_creation(header->code_description, words[0], 15);
        fwrite(&(header->code_description), sizeof(char), 15, bin_fp);
    
        strings_creation(header->card_description, words[1], 13);
        fwrite(&(header->card_description), sizeof(char), 13, bin_fp);
    
        strings_creation(header->name_description, words[2], 13);
        fwrite(&(header->name_description), sizeof(char), 13, bin_fp);
    
        strings_creation(header->color_description, words[3], 24);
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

static void update_header(FILE *bin_fp, ROUTE_HEADER *header){
    header->next_reg = ftell(bin_fp);
    
    // Going to the begining of the file to update it's data
    fseek(bin_fp, 1, SEEK_SET);

    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);

    fseek(bin_fp, 0, SEEK_SET);
    header->status = '1';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
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

void print_card_option(char option){
    printf("Aceita cartao: ");
    
    if (option == 'S')
        printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR");
    
    if (option == 'N')
        printf("PAGAMENTO EM CARTAO E DINHEIRO");

    if (option == 'F')
        printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA");
    
    else
        printf("campo com valor nulo");

    printf("\n");
}

void print_route_bin(ROUTE *data){
    printf("Codigo da linha: %d\n", data->route_code);

    printf("Nome da linha: ");
    if (data->name_length != 0)
        print_string_without_terminator(data->route_name, data->name_length, TRUE);
    else
        printf("campo com valor nulo\n");

    printf("Cor que descreve a linha: ");
    if (data->color_length != 0)
        print_string_without_terminator(data->color, data->color_length, TRUE);
    else
        printf("campo com valor nulo\n");

    print_card_option(data->accepts_card);
    
    printf("\n");
}

void read_route_bin(FILE *bin_fp){
    // Going to the start o @number_of_registers in header to read it
    int num_of_register;
    fseek(bin_fp, 9, SEEK_SET);
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    // Jumps to the first register
    fseek(bin_fp, 69, SEEK_CUR);
    for (int i = 0; i < num_of_register; i++) {
        ROUTE cur_register;
        fread(&cur_register.is_removed, sizeof(char), 1, bin_fp);
        fread(&cur_register.register_length, sizeof(int), 1, bin_fp);

        bool should_read = (cur_register.is_removed == '1');
        if (!should_read) {
            // If register is removed, then it shouldn't be counted as read
            i--;
            fseek(bin_fp, cur_register.register_length, SEEK_CUR);
        } else {
            fread(&cur_register.route_code, sizeof(int), 1, bin_fp);

            fread(&cur_register.accepts_card, sizeof(char), 1, bin_fp);
            
            fread(&cur_register.name_length, sizeof(int), 1, bin_fp);
            if (cur_register.name_length != 0){
                cur_register.route_name = malloc(cur_register.name_length * sizeof(char));
                fread(cur_register.route_name, sizeof(char), cur_register.name_length, bin_fp);
            } 

            fread(&cur_register.color_length, sizeof(int), 1, bin_fp);
            if (cur_register.color_length != 0){
                cur_register.color = malloc(cur_register.color_length * sizeof(char));
                fread(cur_register.color, sizeof(char), cur_register.color_length, bin_fp);
            }

            print_route_bin(&cur_register);

            if (cur_register.name_length > 0)
                free(cur_register.route_name);
            if (cur_register.color_length > 0)
                free(cur_register.color);
        }
    }
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
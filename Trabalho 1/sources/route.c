/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/route.h"

enum _ROUTE_FIELD{
    Route_code,
    Accepts_card,
    Route_name,
    Color,
    Doesnt_exist
};

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

static void read_header(FILE *bin_fp, ROUTE_HEADER *header){
    if (bin_fp == NULL)
        return;

    fread(&header->status, sizeof(char), 1, bin_fp);
    fread(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fread(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fread(&header->num_of_removeds, sizeof(int), 1, bin_fp);
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

    is_ok = check_integrity(word[1], header);
    if (!is_ok)
        data->accepts_card = '\0';
    else
        data->accepts_card = word[1][0];

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
    
    if (option == 'S') {
        printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR\n");
        return;
    }
    
    if (option == 'N') {
        printf("PAGAMENTO EM CARTAO E DINHEIRO\n");
        return;
    }

    if (option == 'F') {
        printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA\n");
        return;
    }
    
    else {
        printf("campo com valor nulo\n");
        return;
    }
}

void print_route_register(ROUTE *data){
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

static void free_dynamic_fields(ROUTE *data){
    if (data->name_length != 0)
        free(data->route_name);
    if (data->color_length != 0)
        free(data->color);
}

void read_route_bin(FILE *bin_fp){
    // Going to the start o @number_of_registers in header to read it
    int num_of_register;
    fseek(bin_fp, 9, SEEK_SET);
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    if (num_of_register == 0){
        printf("Registro inexistente.\n");
        return;
    }

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

            print_route_register(&cur_register);

            if (cur_register.name_length > 0)
                free(cur_register.route_name);
            if (cur_register.color_length > 0)
                free(cur_register.color);
        }
    }
}

static ROUTE_FIELD get_field(char *field) {
    if (strcmp(field, "codLinha") == 0)
        return Route_code;
    
    if (strcmp(field, "aceitaCartao") == 0)
        return Accepts_card;
    
    if (strcmp(field, "nomeLinha") == 0)
        return Route_name;
    
    if (strcmp(field, "corLinha") == 0)
        return Color;
    
    return Doesnt_exist;
}

static void read_until_field(FILE *bin_fp, ROUTE_FIELD which_field) {
    for (int i = 0; i < which_field; i++){
        if (i == Route_code)
            fseek(bin_fp, 4, SEEK_CUR);

        if (i == Accepts_card)
            fseek(bin_fp, 1, SEEK_CUR);

        int size;
        if (i == Route_name){
            fread(&size, sizeof(int), 1, bin_fp);
            fseek(bin_fp, size, SEEK_CUR);
        }
    }
}

void read_route_register(FILE *bin_fp, ROUTE *valid_register){
    fread(&valid_register->is_removed, sizeof(char), 1, bin_fp);
    fread(&valid_register->register_length, sizeof(int), 1, bin_fp);
    fread(&valid_register->route_code, sizeof(int), 1, bin_fp);
    fread(&valid_register->accepts_card, sizeof(char), 1, bin_fp);

    fread(&valid_register->name_length, sizeof(int), 1, bin_fp);
    if (valid_register->name_length != 0){
        valid_register->route_name = malloc(valid_register->name_length * sizeof(char));
        fread(valid_register->route_name, sizeof(char), valid_register->name_length, bin_fp);
    }

    fread(&valid_register->color_length, sizeof(int), 1, bin_fp);
    if (valid_register->color_length != 0){
        valid_register->color = malloc(valid_register->color_length * sizeof(char));
        fread(valid_register->color, sizeof(char), valid_register->color_length, bin_fp);
    }
}

void search_route_by_field(FILE *bin_fp, char *field, char *value){
    ROUTE_FIELD which_field = get_field(field);
    if (which_field == Doesnt_exist){
        printf("Registro inexistente.\n");
        return;
    }

    ROUTE_HEADER header;
    fread(&header.status, sizeof(char), 1, bin_fp);
    
    fseek(bin_fp, 8, SEEK_CUR);
    fread(&header.num_of_regs, sizeof(int), 1, bin_fp);

    fseek(bin_fp, 69, SEEK_CUR);
    for (int i = 0; i < header.num_of_regs; i++){
        // Storing start of register to return later 
        long long int start_of_register = ftell(bin_fp); 

        if (!register_exists(bin_fp)){
            int reg_len;
            fread(&reg_len, sizeof(int), 1, bin_fp);
            fseek(bin_fp, reg_len, SEEK_CUR);
            
            // If register doesn't exist, it shouldn't be counted
            i--;
            continue;
        }
        
        int reg_len;
        fread(&reg_len, sizeof(int), 1, bin_fp);

        read_until_field(bin_fp, which_field);
        bool are_equal = FALSE;
        if (which_field == Route_name || which_field == Color){
            int field_size;
            fread(&field_size, sizeof(int), 1, bin_fp);
            
            char *content = malloc(field_size * sizeof(char));
            fread(content, sizeof(char), field_size, bin_fp);

            are_equal = compare_strings_whithout_terminator(value, content, field_size);
            if (!are_equal){
                go_to_end_of_register(bin_fp, start_of_register, reg_len);
            }

            free(content);
        } else {
            switch (which_field){
                int route_code;
                case Route_code:
                    fread(&route_code, sizeof(int), 1, bin_fp);
                    are_equal = (route_code == atoi(value));
                    break;

                char accepts_card;
                case Accepts_card:
                    fread(&accepts_card, sizeof(char), 1, bin_fp);
                    are_equal = compare_strings_whithout_terminator(&accepts_card, value, 1);
                    break;

                default:
                    break;
            }

            if (!are_equal){
                go_to_end_of_register(bin_fp, start_of_register, reg_len);
            }
        }

        if (are_equal){
            fseek(bin_fp, start_of_register, SEEK_SET);
            ROUTE valid_register;
            read_route_register(bin_fp, &valid_register);
            print_route_register(&valid_register);

            free_dynamic_fields(&valid_register);
        }
    }
}

static WORDS *read_entries(){
    WORDS *entries = create_word_list();
    if (entries == NULL)
        return NULL;

    char *route_code = read_word(stdin);
    if (route_code == NULL)
        return NULL;
    else
        append_word(entries, route_code);

    char *accepts_card = read_inside_quotes();
    if (accepts_card == NULL)
        return NULL;
    else
        append_word(entries, accepts_card);

    char *route_name = read_inside_quotes();
    if (route_name == NULL)
        return NULL;
    else
        append_word(entries, route_name);   

    char *color = read_inside_quotes();
    if (color == NULL)
        return NULL;
    else
        append_word(entries, color);

    return entries;
}

void insert_new_route(FILE *bin_fp, bool *inserted){
    if (bin_fp == NULL){
        printf("Falha no processamento do arquivo.\n");
        *inserted = FALSE;
        return;
    }

    fseek(bin_fp, 0, SEEK_SET);

    ROUTE_HEADER header;
    read_header(bin_fp, &header);

    // Checking if the file is valid to be used
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        *inserted = FALSE;
        return;
    } else {
        set_file_in_use(bin_fp);
    }
    
    fseek(bin_fp, header.next_reg, SEEK_SET);

    char *iterations = read_word(stdin);
    int num_registers = atoi(iterations);
    free(iterations);

    for (int i = 0; i < num_registers; i++){
        WORDS *entries  = read_entries();
        int num_of_entries = get_word_list_length(entries);
        if (num_of_entries != 4){
            printf("Faltam dados\n");
            *inserted = FALSE;
            return;
        }

        ROUTE new_route;
        fill_register(&new_route, get_word_list(entries), &header);
        write_data(bin_fp, &new_route, &header);
        
        free_word_list(entries);
    }
    
    // After adding all the new data, we update the header of the file
    update_header(bin_fp, &header);
    *inserted = TRUE;
}
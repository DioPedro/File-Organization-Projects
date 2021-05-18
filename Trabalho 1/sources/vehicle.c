/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/vehicle.h"

enum _VEHICLE_FIELD{
    Prefix,
    Date,
    Num_of_seats,
    Model,
    Category, 
    Doesnt_exist
};

struct _VEHICLE_HEADER{
    char status;
    long long int next_reg;
    int num_of_regs;
    int num_of_removeds;
    char prefix_description[18];
    char date_description[35];
    char seats_description[42]; 
    char route_description[26]; 
    char model_description[17];
    char category_description[20];
};

// *_length ignores '\0'
struct _VEHICLE{
    char is_removed;        // "0" == true
    int register_length;
    char prefix[5];
    char date[10];
    int num_of_seats;
    int route_code;
    int model_length;
    char *model;
    int category_length;
    char *category;
};

static bool check_integrity(char *csv_field, VEHICLE_HEADER *header){
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

static void fill_register(VEHICLE *data, char **word, VEHICLE_HEADER *header){
    bool is_ok = check_integrity(word[0], header);
    if (!is_ok){
        strcpy(data->prefix, word[0] + 1);
        data->is_removed = '0';
    } else {
        strings_creation(data->prefix, word[0], 5);
        data->is_removed = '1';
    }

    is_ok = check_integrity(word[1], header);
    if (!is_ok)
        strings_creation(data->date, "", 10);
    else
        strings_creation(data->date, word[1], 10);

    is_ok = check_integrity(word[2], header);
    if (!is_ok)
        data->num_of_seats = -1;
    else 
        data->num_of_seats = atoi(word[2]);

    is_ok = check_integrity(word[3], header);
    if (!is_ok)
        data->route_code = -1;
    else 
        data->route_code = atoi(word[3]);

    is_ok = check_integrity(word[4], header);
    if (!is_ok)
        data->model_length = 0;
    else {
        data->model_length = strlen(word[4]);
        data->model = word[4];
    }

    is_ok = check_integrity(word[5], header);
    if (!is_ok)
        data->category_length = 0;
    else {
        data->category_length = strlen(word[5]);
        data->category = word[5];
    }
}

static void create_header(FILE *bin_fp, VEHICLE_HEADER *header, WORDS *header_list){
    header->status = '0';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
    
    header->next_reg = 175;
    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);

    header->num_of_regs = 0;
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);

    header->num_of_removeds = 0;
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);

    char **words = get_word_list(header_list);
    if (words != NULL){
        strings_creation(header->prefix_description, words[0], 18);
        fwrite(&(header->prefix_description), sizeof(char), 18, bin_fp);
    
        strings_creation(header->date_description, words[1], 35);
        fwrite(&(header->date_description), sizeof(char), 35, bin_fp);
    
        strings_creation(header->seats_description, words[2], 42);
        fwrite(&(header->seats_description), sizeof(char), 42, bin_fp);
    
        strings_creation(header->route_description, words[3], 26);
        fwrite(&(header->route_description), sizeof(char), 26, bin_fp);

        strings_creation(header->model_description, words[4], 17);
        fwrite(&(header->model_description), sizeof(char), 17, bin_fp);

        strings_creation(header->category_description, words[5], 20);
        fwrite(&(header->category_description), sizeof(char), 20, bin_fp);
    }
}

static void write_data(FILE *bin_fp, VEHICLE *data, VEHICLE_HEADER *header){
    /*
    5 bytes (prefix)          + 10 bytes (date)
    4 bytes (num_of_seats)    + 4 bytes (route_code)
    4 bytes (model_length)    + model_length
    4 bytes (category_length) + category_length
    */
    data->register_length = 31 + data->model_length + data->category_length;
    
    fwrite(&(data->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(data->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(data->prefix), sizeof(char), 5, bin_fp);
    fwrite(&(data->date), sizeof(char), 10, bin_fp);
    fwrite(&(data->num_of_seats), sizeof(int), 1, bin_fp);
    fwrite(&(data->route_code), sizeof(int), 1, bin_fp);

    fwrite(&(data->model_length), sizeof(int), 1, bin_fp);
    if (data->model_length != 0)
        write_data_strings(bin_fp, data->model, data->model_length);

    fwrite(&(data->category_length), sizeof(int), 1, bin_fp);
    if (data->category_length != 0)
        write_data_strings(bin_fp, data->category, data->category_length);

    if (data->is_removed == '1')
        header->num_of_regs++;
}

static void update_header(FILE *bin_fp, VEHICLE_HEADER *header){
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

void create_vehicle_binary(FILE *csv_fp, FILE *bin_fp){
    // Part of the csv with the header
    char *header_line = read_line(csv_fp);
    WORDS *header_list = split_list(header_line, ',');

    VEHICLE_HEADER header;
    create_header(bin_fp, &header, header_list);

    free_word_list(header_list);
    free(header_line);

    // Part of the csv with the registers
    while (!feof(csv_fp)){
        char *reg_line = read_line(csv_fp);
        
        // Spliting and adding the reg_line of the csv to the binary file
        WORDS *word_list = split_list(reg_line, ',');

        // All lines must have 4 fields, otherwise it's EOF
        if (get_word_list_length(word_list) != 6){
            free_data(word_list, reg_line);
            break;
        }

        VEHICLE data;
        char **word = get_word_list(word_list);
        fill_register(&data, word, &header);
        write_data(bin_fp, &data, &header);

        free_data(word_list, reg_line);
    }

    update_header(bin_fp, &header);
}

void free_dynamic_fields(VEHICLE *data){
    if (data->model_length != 0)
        free(data->model);
    if (data->category_length != 0)
        free(data->category);
}

void print_vehicle_register(VEHICLE *data){
    printf("Prefixo do veiculo: %.5s\n", data->prefix);
    
    printf("Modelo do veiculo: ");
    if (data->model_length != 0)
        print_string_without_terminator(data->model, data->model_length, TRUE);
    else    
        printf("campo com valor nulo\n");

    printf("Categoria do veiculo: ");
    if (data->category_length != 0)
        print_string_without_terminator(data->category, data->category_length, TRUE);
    else
        printf("campo com valor nulo\n");

    print_date(data->date);

    printf("Quantidade de lugares sentados disponiveis: ");
    if (data->num_of_seats != -1)
        printf("%d\n", data->num_of_seats);
    else
        printf("campo com valor nulo\n");

    printf("\n");
}

void read_vehicle_bin(FILE *bin_fp){
    // Going to the start o @number_of_registers in header to read it
    int num_of_register;
    fseek(bin_fp, 9, SEEK_SET);
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    // Jumps to the first register
    fseek(bin_fp, 162, SEEK_CUR);
    for (int i = 0; i < num_of_register; i++) {
        VEHICLE cur_register;
        fread(&cur_register.is_removed, sizeof(char), 1, bin_fp);
        fread(&cur_register.register_length, sizeof(int), 1, bin_fp);

        bool should_read = (cur_register.is_removed == '1');
        if (!should_read) {
            // If register is removed, then it shouldn't be counted as read
            i--;
            fseek(bin_fp, cur_register.register_length, SEEK_CUR);
        } else {
            fread(cur_register.prefix, sizeof(char), 5, bin_fp);

            fread(cur_register.date, sizeof(char), 10, bin_fp);
            fread(&cur_register.num_of_seats, sizeof(int), 1, bin_fp);
            fread(&cur_register.route_code, sizeof(int), 1, bin_fp);
            
            fread(&cur_register.model_length, sizeof(int), 1, bin_fp);
            if (cur_register.model_length != 0){
                cur_register.model = malloc(cur_register.model_length * sizeof(char));
                fread(cur_register.model, sizeof(char), cur_register.model_length, bin_fp);
            }

            fread(&cur_register.category_length, sizeof(int), 1, bin_fp);
            if (cur_register.category_length != 0) {
                cur_register.category = malloc(cur_register.category_length * sizeof(char));
                fread(cur_register.category, sizeof(char), cur_register.category_length, bin_fp);
            }
        
            print_vehicle_register(&cur_register);

            free_dynamic_fields(&cur_register);
        }
    }
}

VEHICLE_FIELD get_field(char *field) {
    if (strcmp(field, "prefixo") == 0)
        return Prefix;
    
    if (strcmp(field, "data") == 0)
        return Date;
    
    if (strcmp(field, "quantidadeLugares") == 0)
        return Num_of_seats;
    
    if (strcmp(field, "modelo") == 0)
        return Model;
    
    if (strcmp(field, "categoria") == 0)
        return Category;
    
    return Doesnt_exist;
}

void read_until_field(FILE *bin_fp, VEHICLE_FIELD which_field) {

    for (int i = 0; i < which_field; i++){
        if (i == Prefix)
            fseek(bin_fp, 5, SEEK_CUR);

        if (i == Date)
            fseek(bin_fp, 10, SEEK_CUR);
        
        if (i == Num_of_seats)
            fseek(bin_fp, 8, SEEK_CUR);

        int size;
        if (i == Model) {
            fread(&size, sizeof(int), 1, bin_fp);
            fseek(bin_fp, size, SEEK_CUR);
        }
    }
}

bool compare_strings_whithout_terminator(char *stringA, char *stringB, int size) {
    for (int i = 0; i < size; i++) {
        if (stringA[i] != stringB[i])
            return FALSE;
    }

    return TRUE;
}

void read_vehicle_register(FILE *bin_fp, VEHICLE *valid_register){
    fread(&valid_register->is_removed, sizeof(char), 1, bin_fp);
    fread(&valid_register->register_length, sizeof(int), 1, bin_fp);
    fread(valid_register->prefix, sizeof(char), 5, bin_fp);
    fread(valid_register->date, sizeof(char), 10, bin_fp);
    fread(&valid_register->num_of_seats, sizeof(int), 1, bin_fp);
    fread(&valid_register->route_code, sizeof(int), 1, bin_fp);
    
    fread(&valid_register->model_length, sizeof(int), 1, bin_fp);
    if (valid_register->model_length > 0){
        valid_register->model = malloc(valid_register->model_length * sizeof(char));
        fread(valid_register->model, sizeof(char), valid_register->model_length, bin_fp);
    }

    fread(&valid_register->category_length, sizeof(int), 1, bin_fp);
    if (valid_register->category_length > 0){
        valid_register->category = malloc(valid_register->category_length * sizeof(char));
        fread(valid_register->category, sizeof(char), valid_register->category_length, bin_fp);
    }
}

void go_to_end_of_register(FILE *bin_fp, long long start_of_register, int reg_len){
    // @reg_len + 5 = total register size
    // @cur_offset - @start_of_register = how much we went through the register
    // left to end of register = total register size - how much we went through the register
    long long int cur_offset = ftell(bin_fp);
    long long int next_reg_offset = reg_len + 5 - (cur_offset - start_of_register);
    fseek(bin_fp, next_reg_offset, SEEK_CUR);
}

void search_by_field(FILE *bin_fp, char *field, char *value){
    VEHICLE_FIELD which_field = get_field(field);
    if (which_field == Doesnt_exist){
        printf("Registro inexistente.\n");
        return;
    }

    VEHICLE_HEADER header;
    fread(&header.status, sizeof(char), 1, bin_fp);
    
    fseek(bin_fp, 8, SEEK_CUR);
    fread(&header.num_of_regs, sizeof(int), 1, bin_fp);

    fseek(bin_fp, 162, SEEK_CUR);
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
        if (which_field == Model || which_field == Category){
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
                char prefix[5];
                case Prefix:
                    fread(prefix, sizeof(char), 5, bin_fp);
                    are_equal = compare_strings_whithout_terminator(prefix, value, 5);
                    break;
                
                char date[10];
                case Date:
                    fread(prefix, sizeof(char), 10, bin_fp);
                    are_equal = compare_strings_whithout_terminator(date, value, 10);
                    break;

                int num_of_seats;
                case Num_of_seats:
                    fread(&num_of_seats, sizeof(int), 1, bin_fp);
                    are_equal = (num_of_seats == atoi(value));
                    break;
                
                default:
                    break;
            }

            if (!are_equal){
                go_to_end_of_register(bin_fp, start_of_register, reg_len);
            } else {
                
                fseek(bin_fp, start_of_register, SEEK_SET);
                VEHICLE valid_register;
                read_vehicle_register(bin_fp, &valid_register);
                print_vehicle_register(&valid_register);

                free_dynamic_fields(&valid_register);
            }
        }
    }
}

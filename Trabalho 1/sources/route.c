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
void header_strings_creation(char *header_field, char *word, int size){
    int cur_length = 0;
    for (; word[cur_length] != '\0'; cur_length++) 
        header_field[cur_length] = word[cur_length];

    size -= 1;
    for (; size > cur_length; size--)
        header_field[size] = '@';
}

void create_header(ROUTE_HEADER header, WORDS *header_list){
    header.status = '0';
    header.next_reg = 175;
    header.num_of_regs = 0;
    header.num_of_removeds = 0;

    char **words = get_word_list(header_list);
    if (words != NULL){
        header_strings_creation(header.code_description, words[0], 15);
        header_strings_creation(header.card_description, words[1], 13);
        header_strings_creation(header.name_description, words[2], 13);
        header_strings_creation(header.color_description, words[3], 24);
    }
}

void create_register(ROUTE reg, WORDS *word_list){
    
}

void create_route_binary(FILE *csv_fp, FILE *bin_fp){
    // Part of the csv with the header
    char *header_line = read_line(csv_fp);
    WORDS *header_list = split_list(header_line, ',');
    printf("Header: %s\n", header_line);
    print_word_list(header_list);

    ROUTE_HEADER header;
    create_header(header, header_list);

    free_word_list(header_list);
    free(header_line);

    printf("%ld\n", ftell(csv_fp));

    // Part of the csv with the registers
    while (!feof(csv_fp)){
        char *reg_line = read_line(csv_fp);

        if (reg_line[0] == '\0')
            free(reg_line);
        else {
            // printf("%s\n", reg_line);

            // Spliting and adding the reg_line of the csv to the binary file
            WORDS *word_list = split_list(reg_line, ',');
            // print_word_list(word_list);
            free_word_list(word_list);
            free(reg_line);
        }
    };
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
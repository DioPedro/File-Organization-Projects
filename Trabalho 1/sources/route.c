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
    char status;                // Consistência do arquivo
    long long int next_reg;     // Byte offset do próximo registro a ser inserido 
    int num_of_regs;            // Número de registros
    int num_of_removeds;        // Número de registros removidos
    char code_description[15];  // Descrição do codLinha
    char card_description[13];  // Descrição do aceitaCartão 
    char name_description[13];  // Descrição do nomeLinha
    char color_description[24]; // Descrição corLinha
};

// Tamanho dos campos variáveis não considera o '\0'
struct _ROUTE{
    char is_removed;        // Indica se está removido ou não ("0" == true)
    int register_length;    // tamRegistro
    int route_code;         // codLinha
    char accepts_card;      // aceitaCartao
    int name_length;        // tamanhoNome
    char *route_name;       // nomeLinha
    int color_length;       // tamanhoCor
    char *color;            // corLinha
};

// Leitura do cabeçalho de um arquivo binário
static void read_header(FILE *bin_fp, ROUTE_HEADER *header){
    // Leitura dos principais campos do cabeçalho:
    //  status do arquivo, byte offset do próximo registro a ser inserido, 
    //  número de registros e número de removidos, respectivamente
    fread(&header->status, sizeof(char), 1, bin_fp);        
    fread(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fread(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fread(&header->num_of_removeds, sizeof(int), 1, bin_fp);
}

// Verifica a integridade de um campo do csv
static bool check_integrity(char *csv_field, ROUTE_HEADER *header){
    // Se o campo estiver vazio, a inserção no binário deve ser corrigida
    int length = strlen(csv_field);
    if (length == 0) 
        return FALSE;
    
    // Se o dado começa com um '*', devemos considerar como campo removido
    if (csv_field[0] == '*'){
        header->num_of_removeds++;
        return FALSE;
    }

    // Se for um NULO, a inserção no binário deve ser diferente
    if (strcmp(csv_field, "NULO") == 0) 
        return FALSE;

    return TRUE;
}

// Preenchimento do registro de uma linha, sempre verificando a integridade do campo do csv
static void fill_register(ROUTE *data, char **word, ROUTE_HEADER *header){
    // Caso o primeiro campo (codLinha) esteja certo, inserimos o inteiro no registro
    // Se não, desconsideramos o '*', inserimos o resto do inteiro, e marcamos como removido
    bool is_ok = check_integrity(word[0], header);
    if (!is_ok){
        data->route_code = atoi(word[0] + 1);
        data->is_removed = '0';
    } else {
        data->route_code = atoi(word[0]);
        data->is_removed = '1';
    }

    // Caso o campo aceitaCartao esteja ok, inserimos apenas o char no registro
    // Se não, inserimos um '\0'
    is_ok = check_integrity(word[1], header);
    if (!is_ok)
        data->accepts_card = '\0';
    else
        data->accepts_card = word[1][0];

    // Caso nomeLinha esteja ok, inserimos o tamanho do nome e o nome em si no registro
    // Se não, inserimos apenas que o tamanho é 0
    is_ok = check_integrity(word[2], header);
    if (!is_ok)
        data->name_length = 0;
    else { 
        data->name_length = strlen(word[2]);
        data->route_name = word[2];
    }

    // Análogo ao caso do nomeLinha, mas agora para a corLinha
    is_ok = check_integrity(word[3], header);
    if (!is_ok)
        data->color_length = 0;
    else {
        data->color_length = strlen(word[3]);
        data->color = word[3];
    }
}

// Escreve o cabeçalho do arquivo csv no arquivo binário
static void create_header(FILE *bin_fp, ROUTE_HEADER *header, WORDS *header_list){
    // Como começamos a escrita, o arquivo começa com status de "inconsistente"
    header->status = '0';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
    
    // Cabeçalho tem tamanho fixo de 82 e ainda não há registro dado escrito
    header->next_reg = 82;
    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);

    // Nenhum registro escrito, então apenas inicializamos com 0
    header->num_of_regs = 0;
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);

    // Se não há nenhum registro, não tem como haver removido
    header->num_of_removeds = 0;
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);

    // Agora pegamos os dados de descrição do arquivo csv e inserimos, caso 
    // a leitura deles tenha sido certa
    char **words = get_word_list(header_list);
    if (words != NULL){
        // Escrita dos 15 caracteres da descrição do codLinha
        strings_creation(header->code_description, words[0], 15);
        fwrite(&(header->code_description), sizeof(char), 15, bin_fp);

        // Escrita dos 13 caracteres da descrição do aceitaCartao
        strings_creation(header->card_description, words[1], 13);
        fwrite(&(header->card_description), sizeof(char), 13, bin_fp);
    
        // Escrita dos 13 caracteres da descrição do nomeLinha
        strings_creation(header->name_description, words[2], 13);
        fwrite(&(header->name_description), sizeof(char), 13, bin_fp);

        // Escrita dos 24 caracteres da descrição do corLinha
        strings_creation(header->color_description, words[3], 24);
        fwrite(&(header->color_description), sizeof(char), 24, bin_fp);
    }
}

// Escrita dos dados do registro de dados
static void write_data(FILE *bin_fp, ROUTE *data, ROUTE_HEADER *header){
    // Tamanho do registro =  
    // 4 bytes(codlinha)    + 1 byte(aceitaCartao) 
    // 4 bytes(tamanhoCor)  + 4 bytes(tamanhoLinha)
    // corLinha             + nomeLinha
    data->register_length = 13 + data->color_length + data->name_length;
    
    // Escrita dos campos de tamanho fixo:
    // removido, tamanhoRegistro, codLinha, aceitaCartao, respectivamente
    fwrite(&(data->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(data->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(data->route_code), sizeof(int), 1, bin_fp);
    fwrite(&(data->accepts_card), sizeof(char), 1, bin_fp);

    // Escrita do tamanho do nomeLinha: se for 0, nomeLinha não deve ser escrito 
    fwrite(&(data->name_length), sizeof(int), 1, bin_fp);
    if (data->name_length != 0)
        write_data_strings(bin_fp, data->route_name, data->name_length);

    // Escrita do tamanho do corLinha: se for 0, corLinha não deve ser escrito 
    fwrite(&(data->color_length), sizeof(int), 1, bin_fp);
    if (data->color_length != 0)
        write_data_strings(bin_fp, data->color, data->color_length);

    // Se o registro estiver removido, atualizamos o cabeçalho com mais um removido
    if (data->is_removed == '1')
        header->num_of_regs++;
}

// Atualização do cabeçalho do arquivo binário
static void update_header(FILE *bin_fp, ROUTE_HEADER *header){
    // Marca a posição do fim do arquivo
    header->next_reg = ftell(bin_fp);
    
    // Volta para o início, pulando o status do arquivo
    fseek(bin_fp, 1, SEEK_SET);

    // Reescreve o byteOffset do proximo registro a ser inserido,
    // o número de registros do arquivo e o número de removidos 
    fwrite(&(header->next_reg), sizeof(long long int), 1, bin_fp);
    fwrite(&(header->num_of_regs), sizeof(int), 1, bin_fp);
    fwrite(&(header->num_of_removeds), sizeof(int), 1, bin_fp);


    // Volta para o início e marca o arquivo como consistente, após
    // todas as modificações
    fseek(bin_fp, 0, SEEK_SET);
    header->status = '1';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
}

// Criação do binário de um arquivo de linha
void create_route_binary(FILE *csv_fp, FILE *bin_fp){
    // Lê a primeira linha do csv (cabeçalho), separa nas virgulas 
    char *header_line = read_line(csv_fp);
    WORDS *header_list = split_list(header_line, ',');

    // E cria o cabeçalho
    ROUTE_HEADER header;
    create_header(bin_fp, &header, header_list);

    free_word_list(header_list);
    free(header_line);

    // Inicia a leitura dos dados do csv
    while (!feof(csv_fp)){
        char *reg_line = read_line(csv_fp);
        
        // Separa os dados nas virgulas
        WORDS *word_list = split_list(reg_line, ',');

        // Caso não existam os 4 campos necessários, a leitura e escrita é parada
        if (get_word_list_length(word_list) != 4){
            free_data(word_list, reg_line);
            break;
        }     

        // Preenche os registro de dados e insere no binário
        ROUTE data;
        char **word = get_word_list(word_list);
        fill_register(&data, word, &header);
        write_data(bin_fp, &data, &header);

        free_data(word_list, reg_line);
    }

    // Após o fim das inserções, atualiza o cabeçalho
    update_header(bin_fp, &header);
}

// Função auxiliar para impressão do campo aceitaCartao
void print_card_option(char option){
    printf("Aceita cartao: ");
    
    if (option == 'S'){
        printf("PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR\n");
        return;
    }
    
    if (option == 'N'){
        printf("PAGAMENTO EM CARTAO E DINHEIRO\n");
        return;
    }

    if (option == 'F'){
        printf("PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA\n");
        return;
    } else {
        printf("campo com valor nulo\n");
        return;
    }
}

// Impressão de um registro do arquivo binário
void print_route_register(ROUTE *data){
    // codLinha nunca é nulo, então podemos fazer a impressão direta
    printf("Codigo da linha: %d\n", data->route_code);

    // Verifica se nomeLinha existe e imprime
    printf("Nome da linha: ");
    if (data->name_length != 0)
        print_string_without_terminator(data->route_name, data->name_length, TRUE);
    else
        printf("campo com valor nulo\n");

    // Verifica se corLinha existe e imprime
    printf("Cor que descreve a linha: ");
    if (data->color_length != 0)
        print_string_without_terminator(data->color, data->color_length, TRUE);
    else
        printf("campo com valor nulo\n");

    // Impressão do aceitaCartao
    print_card_option(data->accepts_card);
    
    printf("\n");
}

// Função auxiliar para liberar memória dinamicamente alocada no registro de linha
static void free_dynamic_fields(ROUTE *data){
    if (data->name_length != 0)
        free(data->route_name);
    if (data->color_length != 0)
        free(data->color);
}

// Leitura de um registro de linha no arquivo binário
void read_route_register(FILE *bin_fp, ROUTE *valid_register, bool should_read_begining){
    // Verificando se é necessário ler os primeiros bytes do registro (removido e tamanhoRegistro)
    if (should_read_begining){
        fread(&valid_register->is_removed, sizeof(char), 1, bin_fp);
        fread(&valid_register->register_length, sizeof(int), 1, bin_fp);
    }

    // Leitura no codLinha (nunca nulo) e aceitaCartao
    fread(&valid_register->route_code, sizeof(int), 1, bin_fp);
    fread(&valid_register->accepts_card, sizeof(char), 1, bin_fp);

    // Verifica se existe o campo nomeLinha e lê caso haja
    fread(&valid_register->name_length, sizeof(int), 1, bin_fp);
    if (valid_register->name_length != 0){
        valid_register->route_name = malloc(valid_register->name_length * sizeof(char));
        fread(valid_register->route_name, sizeof(char), valid_register->name_length, bin_fp);
    }

    // Verifica se existe o campo corLinha e lê caso haja
    fread(&valid_register->color_length, sizeof(int), 1, bin_fp);
    if (valid_register->color_length != 0){
        valid_register->color = malloc(valid_register->color_length * sizeof(char));
        fread(valid_register->color, sizeof(char), valid_register->color_length, bin_fp);
    }
}

// Leitura do arquivo binário inteiro
void read_route_bin(FILE *bin_fp){
    // Verifica a consistência do arquivo
    char status;
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Leitura do número de registros para ter controle das leituras
    int num_of_register;
    fseek(bin_fp, 8, SEEK_CUR); // Pulando o byteOffset do proximo registro a ser inserido
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    if (num_of_register == 0){
        printf("Registro inexistente.\n");
        return;
    }

    // Vai para o primeiro registro e inicia a leitura
    fseek(bin_fp, 69, SEEK_CUR);
    for (int i = 0; i < num_of_register; i++){
        // Verifica se o registro existe e seu tamanho
        ROUTE cur_register;
        fread(&cur_register.is_removed, sizeof(char), 1, bin_fp);
        fread(&cur_register.register_length, sizeof(int), 1, bin_fp);
        
        // Se o registro existe, lemos ele, se não pulamos
        bool should_read = (cur_register.is_removed == '1');
        if (!should_read) {
            // Se o registro está marcado como removido, a leitura não deve considerar 
            // ele, pois ela considera apenas registros que existem
            i--;
            fseek(bin_fp, cur_register.register_length, SEEK_CUR);
        } else {
            // Lê e imprime o registro, liberando a memória alocada durante a leitura
            read_route_register(bin_fp, &cur_register, FALSE);
            print_route_register(&cur_register);
            free_dynamic_fields(&cur_register);
        }
    }
}

// Verifica qual campo o usuário inseriu para ser buscado
static ROUTE_FIELD get_field(char *field){
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

// Pula campos que não devem ser lidos, para evitar leituras desnecessárias
static void read_until_field(FILE *bin_fp, ROUTE_FIELD which_field){
    // Esse loop para no campo anterior ao campo que vai ser lido, ou seja,
    // se queremos ler o nomeLinha, o loop pula codlinha e aceitaCartao
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

// Função que procura e imprime campo por valor 
void search_route_by_field(FILE *bin_fp, char *field, char *value){
    // Lê e verifica a consistência do cabeçalho do binário
    ROUTE_HEADER header;
    fread(&header.status, sizeof(char), 1, bin_fp);
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Verifica o campo que o usuário quer procurar
    ROUTE_FIELD which_field = get_field(field);
    if (which_field == Doesnt_exist){
        printf("Registro inexistente.\n");
        return;
    }

    // Pula o byteOffset do proximo registro a ser inserido e lê o 
    // número de registros, para controle de leitura
    fseek(bin_fp, 8, SEEK_CUR);
    fread(&header.num_of_regs, sizeof(int), 1, bin_fp);

    // Cria um booleano para caso haja registros e não ache nenhum 
    bool was_printed = FALSE;

    // Pula para o início do primero registro e inicia a leitura
    fseek(bin_fp, 69, SEEK_CUR);
    for (int i = 0; i < header.num_of_regs; i++){
        // Guarda o início do registro
        long long int start_of_register = ftell(bin_fp); 

        // Verifica se o registro existe
        // Se não, pula para o próximo registro e avança no loop
        if (!register_exists(bin_fp)){
            int reg_len;
            fread(&reg_len, sizeof(int), 1, bin_fp);
            fseek(bin_fp, reg_len, SEEK_CUR);
            
            // Se o registro não existe, não é considerado como lido
            i--;
            continue;
        }

        // Lê o tamanho do registro
    	int reg_len;
        fread(&reg_len, sizeof(int), 1, bin_fp);

        // Lê até o campo a ser verificado e inicia a comparação
        read_until_field(bin_fp, which_field);
        bool are_equal = FALSE;
        if (which_field == Route_name || which_field == Color){
            // Para campos de tamanho variável, verificamos se eles existem
            int field_size;
            fread(&field_size, sizeof(int), 1, bin_fp);

            // Lê o campo (se for tamanho 0, acaba não lendo nada) 
            char *content = malloc(field_size * sizeof(char));
            fread(content, sizeof(char), field_size, bin_fp);

            // Verifica se o valor procurado é "NULO" (o que, no caso, seria o campo de tamanho 0)
            // se não for nulo, compara o valor que o usuário procurou com o valor do campo
            if (strcmp(value, "NULO") == 0 && field_size == 0)
                are_equal = TRUE; 
            else if (field_size > 0)
                are_equal = compare_strings_whithout_terminator(content, value, field_size);

            free(content);
        } else {
            // Se for campo de tamanho fixo, devemos olhar os casos
            switch (which_field){
                // codLinha nunca é nulo, então podemos comparar os valores diretamente 
                int route_code;
                case Route_code:
                    fread(&route_code, sizeof(int), 1, bin_fp);
                    are_equal = (route_code == atoi(value));
                    break;

                // aceitaCartao pode ser nulo, então devemos comparar mais cuidadosamente
                char accepts_card;
                case Accepts_card:
                    fread(&accepts_card, sizeof(char), 1, bin_fp);
                    are_equal = compare_strings_whithout_terminator(&accepts_card, value, 1);
                    break;

                default:
                    break;
            }
        }

        // Se são iguais, volta pro início do registro e imprime ele
        // Se não, pula pro fim do registro e continua a leitura
        if (are_equal){
            fseek(bin_fp, start_of_register, SEEK_SET);
            ROUTE valid_register;
            read_route_register(bin_fp, &valid_register, TRUE);
            print_route_register(&valid_register);
            was_printed = TRUE;

            free_dynamic_fields(&valid_register);
        } else {
            go_to_end_of_register(bin_fp, start_of_register, reg_len);
        }
    }

    if (!was_printed){
        printf("Registro inexistente.\n");
    }
}

// Leitura das entradas para a função de inserção
static WORDS *read_entries(){
    // Cria a estrutura que usamos para gerar o registro 
    WORDS *entries = create_word_list();
    if (entries == NULL)
        return NULL;

    // Lê o codLinha e insere na estrutura
    char *route_code = read_word(stdin);
    if (route_code == NULL)
        return NULL;
    else
        append_word(entries, route_code);

    // Lê aceitaCartao e insere na estrutura
    char *accepts_card = read_inside_quotes();
    if (accepts_card == NULL)
        return NULL;
    else
        append_word(entries, accepts_card);

    // Lê o nomeLinha e insere na estrutura
    char *route_name = read_inside_quotes();
    if (route_name == NULL)
        return NULL;
    else
        append_word(entries, route_name);   

    // Lê o corLinha e insere na estrutura
    char *color = read_inside_quotes();
    if (color == NULL)
        return NULL;
    else
        append_word(entries, color);

    return entries;
}

// Inserção de novos registros
void insert_new_route(FILE *bin_fp, bool *inserted){
    // Vai para o início do arquivo binário para verificar se está consistente
    // e, caso esteja, muda para inconsistente para iniciarmos as inserções
    fseek(bin_fp, 0, SEEK_SET);

    ROUTE_HEADER header;
    read_header(bin_fp, &header);
    // Checando a consistência do arquivo
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        *inserted = FALSE;
        return;
    } else {
        set_file_in_use(bin_fp);
    }
    
    // Vai para o byte Offset do próximo registro a ser inserido 
    fseek(bin_fp, header.next_reg, SEEK_SET);

    // Lê o número de registros que serão inseridos
    int num_registers = -1;
    scanf("%d", &num_registers);
    char cur_char = getc(stdin);
    while (cur_char != '\n' && cur_char != ' ')
        cur_char = getc(stdin);
    
    // Inicia as inserções
    for (int i = 0; i < num_registers; i++){
        // Lê as entradas, verificando se foram inseridas corretamente
        WORDS *entries  = read_entries();
        int num_of_entries = get_word_list_length(entries);
        if (num_of_entries != 4){
            printf("Faltam dados\n");
            *inserted = FALSE;
            return;
        }

        // Cria o registro e insere no binário
        ROUTE new_route;
        fill_register(&new_route, get_word_list(entries), &header);
        write_data(bin_fp, &new_route, &header);
        free_word_list(entries);
    }
    
    // Atualiza o cabeçalho
    update_header(bin_fp, &header);
    *inserted = TRUE;
}
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
    char status;                    // Consistência do arquivo
    long long int next_reg;         // Byte offset do próximo registro a ser inserido
    int num_of_regs;                // Número de registros
    int num_of_removeds;            // Número de registros removidos
    char prefix_description[18];    // Descrição do prefixo
    char date_description[35];      // Descrição da data
    char seats_description[42];     // Descrição da quantidade de lugares     
    char route_description[26];     // Descrição da rota
    char model_description[17];     // Descrição do modelo
    char category_description[20];  // Descrição da categoria
};

// Tamanho dos campos variáveis não considera o '\0'
struct _VEHICLE{
    char is_removed;        // Indica se está removido ou não ("0" == true)
    int register_length;    // tamRegistro
    char prefix[5];         // Prefixo
    char date[10];          // Data
    int num_of_seats;       // Número de lugares           
    int route_code;         // Código da rota
    int model_length;       // Tamanho do campo modelo
    char *model;            // Modelo
    int category_length;    // Tamanho do campo categoria  
    char *category;         // Categoria
};

// Leitura do cabeçalho de um arquivo binário
static void read_header(FILE *bin_fp, VEHICLE_HEADER *header){
    // Leitura dos principais campos do cabeçalho:
    //  status do arquivo, byte offset do próximo registro a ser inserido, 
    //  número de registros e número de removidos, respectivamente
    fread(&header->status, sizeof(char), 1, bin_fp);
    fread(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fread(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fread(&header->num_of_removeds, sizeof(int), 1, bin_fp);
}

// Verifica a integridade de um campo do csv
static bool check_integrity(char *csv_field, VEHICLE_HEADER *header){
    // Se o campo estiver vazio a inserção no binário deve ser corrigida
    int length = strlen(csv_field);
    if (length == 0) 
        return FALSE;
    
    // Se o dado começa com um '*', devemos considerar como campo removido
    if (csv_field[0] == '*'){
        header->num_of_removeds++;
        return FALSE;
    }

    // Se for um NULO a inserção no binário deve ser diferente
    if (strcmp(csv_field, "NULO") == 0) 
        return FALSE;

    return TRUE;
}

// Preenchimento do registro de um veículo, sempre verificando a integridade do campo do csv
static void fill_register(VEHICLE *data, char **word, VEHICLE_HEADER *header){
    // Caso o primeiro campo (prefixo) esteja certo, inserimos o inteiro no registro
    // Se não, desconsideramos o '*', inserimos o resto do prefixo, e marcamos como removido
    bool is_ok = check_integrity(word[0], header);
    if (!is_ok){
        strcpy(data->prefix, word[0] + 1);
        data->is_removed = '0';
    } else {
        strings_creation(data->prefix, word[0], 5);
        data->is_removed = '1';
    }

    // Caso o campo data esteja ok, inserimos a data no registro
    // Se não, inserimos \0 com @ até completar o tamanho do campo
    is_ok = check_integrity(word[1], header);
    if (!is_ok)
        strings_creation(data->date, "", 10);
    else
        strings_creation(data->date, word[1], 10);

    // Caso o campo número de lugares esteja ok, inserimos o inteiro no registro
    // Se não, inserimos -1
    is_ok = check_integrity(word[2], header);
    if (!is_ok)
        data->num_of_seats = -1;
    else 
        data->num_of_seats = atoi(word[2]);

    // Caso o campo codigoLinha esteja ok, inserimos o inteiro no registro
    // Se não, inserimos -1
    is_ok = check_integrity(word[3], header);
    if (!is_ok)
        data->route_code = -1;
    else 
        data->route_code = atoi(word[3]);

    // Caso nome do Modelo esteja ok, inserimos o tamanho do nome e o nome em si no registro
    // Se não, inserimos apenas que o tamanho é 0
    is_ok = check_integrity(word[4], header);
    if (!is_ok)
        data->model_length = 0;
    else {
        data->model_length = strlen(word[4]);
        data->model = word[4];
    }

    // Análogo ao caso do modelo, mas agora para a categoria
    is_ok = check_integrity(word[5], header);
    if (!is_ok)
        data->category_length = 0;
    else {
        data->category_length = strlen(word[5]);
        data->category = word[5];
    }
}

// Função que escreve o cabeçalho do arquivo csv no arquivo binário
static void create_header(FILE *bin_fp, VEHICLE_HEADER *header, WORDS *header_list){
    // Como começamos a escrita, o arquivo começa com status de "inconsistente"
    header->status = '0';
    fwrite(&(header->status), sizeof(char), 1, bin_fp);
    
    // Cabeçalho tem tamanho fixo de 175 e ainda não há registro dado escrito
    header->next_reg = 175;
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
        // Escrita dos 18 caracteres da descrição do prefixo
        strings_creation(header->prefix_description, words[0], 18);
        fwrite(&(header->prefix_description), sizeof(char), 18, bin_fp);
    
        // Escrita dos 35 caracteres da descrição da data
        strings_creation(header->date_description, words[1], 35);
        fwrite(&(header->date_description), sizeof(char), 35, bin_fp);
    
        // Escrita dos 42 caracteres da descrição dos assentos
        strings_creation(header->seats_description, words[2], 42);
        fwrite(&(header->seats_description), sizeof(char), 42, bin_fp);
    
        // Escrita dos 26 caracteres da descrição da rota
        strings_creation(header->route_description, words[3], 26);
        fwrite(&(header->route_description), sizeof(char), 26, bin_fp);

        // Escrita dos 17 caracteres da descrição do modelo
        strings_creation(header->model_description, words[4], 17);
        fwrite(&(header->model_description), sizeof(char), 17, bin_fp);

        // Escrita dos 20 caracteres da descrição da categoria
        strings_creation(header->category_description, words[5], 20);
        fwrite(&(header->category_description), sizeof(char), 20, bin_fp);
    }
}

// Escrita dos dados do registro de dados
static void write_data(FILE *bin_fp, VEHICLE *data, VEHICLE_HEADER *header){
    // Tamanho do registro =
    // 5 bytes (prefixo)           + 10 bytes (data)
    // 4 bytes (quantidadeLugares) + 4 bytes (codigoLinha)
    // 4 bytes (tamanhoCategoria)  + Categoria
    // 4 bytes (tamanhoModelo)     + Modelo
    data->register_length = 31 + data->model_length + data->category_length;
    
    // Escrita dos campos de tamanho fixo:
    // removido, tamanhoRegistro, prefixo, data, quantidadeLugares, codigoLinha
    fwrite(&(data->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(data->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(data->prefix), sizeof(char), 5, bin_fp);
    fwrite(&(data->date), sizeof(char), 10, bin_fp);
    fwrite(&(data->num_of_seats), sizeof(int), 1, bin_fp);
    fwrite(&(data->route_code), sizeof(int), 1, bin_fp);

    // Escrita do tamanho do modelo: se for 0, modelo não deve ser escrito 
    fwrite(&(data->model_length), sizeof(int), 1, bin_fp);
    if (data->model_length != 0)
        write_data_strings(bin_fp, data->model, data->model_length);

    // Escrita do tamanho da categoria: se for 0, categoria não deve ser escrito
    fwrite(&(data->category_length), sizeof(int), 1, bin_fp);
    if (data->category_length != 0)
        write_data_strings(bin_fp, data->category, data->category_length);

    // Se o registro estiver removido, atualizamos o cabeçalho com mais um removido
    if (data->is_removed == '1')
        header->num_of_regs++;
}

// Função que atualiza o cabeçalho do arquivo binário
static void update_header(FILE *bin_fp, VEHICLE_HEADER *header){
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

// Criação do binário de um arquivo de veículo
void create_vehicle_binary(FILE *csv_fp, FILE *bin_fp){
    // Lê a primeira linha do csv (cabeçalho), separa nas virgulas 
    char *header_line = read_line(csv_fp);
    WORDS *header_list = split_list(header_line, ',');

    // E cria o cabeçalho
    VEHICLE_HEADER header;
    create_header(bin_fp, &header, header_list);

    free_word_list(header_list);
    free(header_line);

    // Inicia a leitura dos dados no csv
    while (!feof(csv_fp)){
        char *reg_line = read_line(csv_fp);
        
        // Separa os dados nas virgulas
        WORDS *word_list = split_list(reg_line, ',');

        // Caso não existam os 6 campos necessários, a leitura e escrita é parada
        if (get_word_list_length(word_list) != 6){
            free_data(word_list, reg_line);
            break;
        }

        // Preenche os registros de dados e insere no binário
        VEHICLE data;
        char **word = get_word_list(word_list);
        fill_register(&data, word, &header);
        write_data(bin_fp, &data, &header);

        free_data(word_list, reg_line);
    }

    // Após o fim das inserções, atualiza o cabeçalho
    update_header(bin_fp, &header);
}

// Função auxiliar para liberar memória dinamicamente alocada no registro de veículo
static void free_dynamic_fields(VEHICLE *data){
    if (data->model_length != 0)
        free(data->model);
    if (data->category_length != 0)
        free(data->category);
}

// Função que verifica se data existe e imprime a data na formatação certa
void print_date(char *date){
    printf("Data de entrada do veiculo na frota: ");
    if (date[0] == '\0'){
        printf("campo com valor nulo\n");
        return;
    }

    char *months[12] = {"janeiro", "fevereiro", "março", "abril",
                        "maio", "junho", "julho", "agosto",
                        "setembro", "outubro", "novembro", "dezembro"};

    char formated_date[50] = "";

    // YEAR-MO-DA
    //         ^ date[8:9] = "DA" (day)
    //      ^ date[5:6] = "MO" (month)
    // ^ date[0:3] = "YEAR" (year)
    char month[3] = "";
    strncpy(month, &date[5], 2);
    
    strncpy(formated_date, &date[8], 2);
    strcat(formated_date, " de ");
    strcat(formated_date, months[atoi(month) - 1]);
    strcat(formated_date, " de ");
    strncat(formated_date, &date[0], 4);

    printf("%s\n", formated_date);
}

// Impressão de um registro do arquivo binário
void print_vehicle_register(VEHICLE *data){
    // Prefixo nunca é nulo, então podemos fazer a impressão direta
    printf("Prefixo do veiculo: %.5s\n", data->prefix);
    
    // Verifica se modelo existe e imprime
    char model[100] = "";
    strcpy(model, "Modelo do veiculo: ");
    if (data->model_length != 0)
        strncat(model, data->model, data->model_length);
    else    
        strcat(model, "campo com valor nulo");
    printf("%s\n", model);
        
    // Verifica se categoria existe e imprime
    char category[100] = "";
    strcpy(category, "Categoria do veiculo: ");
    if (data->category_length != 0)
        strncat(category, data->category, data->category_length);
    else
        strcat(category, "campo com valor nulo");
    printf("%s\n", category);

    // Impressão da data
    print_date(data->date);

    // Verifica se existe quantidadeLugares e imprime
    printf("Quantidade de lugares sentados disponiveis: ");
    if (data->num_of_seats != -1)
        printf("%d\n", data->num_of_seats);
    else
        printf("campo com valor nulo\n");

    printf("\n");
}

// Leitura de um registro de linha no arquivo binário
void read_vehicle_register(FILE *bin_fp, VEHICLE *valid_register, bool should_read_begining){
    // Verificando se é necessário ler os primeiros bytes do registro (removido e tamanhoRegistro)
    if (should_read_begining){
        fread(&valid_register->is_removed, sizeof(char), 1, bin_fp);
        fread(&valid_register->register_length, sizeof(int), 1, bin_fp);
    }

    // Leitura do prefixo (nunca nulo), data, quantidadeLugares e codLinha
    fread(valid_register->prefix, sizeof(char), 5, bin_fp);
    fread(valid_register->date, sizeof(char), 10, bin_fp);
    fread(&valid_register->num_of_seats, sizeof(int), 1, bin_fp);
    fread(&valid_register->route_code, sizeof(int), 1, bin_fp);
    
    // Verifica se existe o campo modelo e lê caso haja
    fread(&valid_register->model_length, sizeof(int), 1, bin_fp);
    if (valid_register->model_length > 0){
        valid_register->model = malloc(valid_register->model_length * sizeof(char));
        fread(valid_register->model, sizeof(char), valid_register->model_length, bin_fp);
    }

    // Verifica se existe o campo categoria e lê caso haja
    fread(&valid_register->category_length, sizeof(int), 1, bin_fp);
    if (valid_register->category_length > 0){
        valid_register->category = malloc(valid_register->category_length * sizeof(char));
        fread(valid_register->category, sizeof(char), valid_register->category_length, bin_fp);
    }
}

// Leitura do arquivo binário inteiro
void read_vehicle_bin(FILE *bin_fp){
    // Verifica a consistência do arquivo
    char status;
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    
    // Leitura do número de registros para ter controle das leituras
    int num_of_register;
    fseek(bin_fp, 8, SEEK_CUR); // Pulando para o byteOffset do proximo registro a ser inserido
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    if (num_of_register == 0){
        printf("Registro inexistente.\n");       
        return;
    }

    // Vai para o primeiro registro e inicia a leitura
    fseek(bin_fp, 162, SEEK_CUR);
    for (int i = 0; i < num_of_register; i++){
        // Verifica se o registro existe e seu tamanho
        VEHICLE cur_register;
        fread(&cur_register.is_removed, sizeof(char), 1, bin_fp);
        fread(&cur_register.register_length, sizeof(int), 1, bin_fp);

        // Se o registro existe, lemos ele, se não pulamos
        bool should_read = (cur_register.is_removed == '1');
        if (!should_read){
            // Se o registro está marcado como removido, a leitura não deve considerar 
            // ele, pois ela considera apenas registros que existem
            i--;
            fseek(bin_fp, cur_register.register_length, SEEK_CUR);
        } else {
            // Lê e imprime o registro, liberando a memória alocada durante a leitura
            read_vehicle_register(bin_fp, &cur_register, FALSE);
            print_vehicle_register(&cur_register);
            free_dynamic_fields(&cur_register);
        }
    }
}

// Verifica qual campo o usuário inseriu para ser buscado
static VEHICLE_FIELD get_field(char *field){
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

// Pula campos que não devem ser lidos, para evitar leituras desnecessárias
static void read_until_field(FILE *bin_fp, VEHICLE_FIELD which_field){
    // Esse loop para no campo anterior ao campo que vai ser lido, ou seja,
    // se queremos ler a quantidadeLugares, o loop pula o prefixo e a data 
    for (int i = 0; i < which_field; i++){
        if (i == Prefix)
            fseek(bin_fp, 5, SEEK_CUR);

        if (i == Date)
            fseek(bin_fp, 10, SEEK_CUR);
        
        if (i == Num_of_seats)
            fseek(bin_fp, 8, SEEK_CUR);

        int size;
        if (i == Model){
            fread(&size, sizeof(int), 1, bin_fp);
            fseek(bin_fp, size, SEEK_CUR);
        }
    }
}

// Função que procura e imprime campo por valor 
void search_vehicle_by_field(FILE *bin_fp, char *field, char *value){
    // Lê e verifica a consistência do cabeçalho do binário
    VEHICLE_HEADER header;
    fread(&header.status, sizeof(char), 1, bin_fp);
    if (header.status == '0') 
        return;

    // Verifica o campo que o usuário quer procurar
    VEHICLE_FIELD which_field = get_field(field);
    if (which_field == Doesnt_exist){
        printf("Registro inexistente.\n");
        return;
    }
    
    // Pula o byteOffset do proximo registro a ser inserido e lê o 
    // número de registros, para controle de leitura
    fseek(bin_fp, 8, SEEK_CUR);
    fread(&header.num_of_regs, sizeof(int), 1, bin_fp);

    // Cria um booleano para caso haja registros e não ache nenhum com o valor procurado
    bool was_printed = FALSE;

    // Pula para o início do primero registro e inicia a leitura
    fseek(bin_fp, 162, SEEK_CUR);
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
        if (which_field == Model || which_field == Category){
            // Para campos de tamanho variável, verificamos se eles existem
            int field_size;
            fread(&field_size, sizeof(int), 1, bin_fp);
            
            // Lê o campo (se for tamanho 0, acaba não lendo nada) 
            char *content = malloc(field_size * sizeof(char));
            fread(content, sizeof(char), field_size, bin_fp);

            // Verifica se o valor procurado é "NULO" (o que, no caso, seria o campo de tamanho 0)
            // se não for nulo, compara o valor que o usuário procurou com o valor do campo
            if(strcmp(value, "NULO") == 0 && field_size == 0)
                are_equal = TRUE;
            else if (field_size > 0)
                are_equal = compare_strings_whithout_terminator(content, value, field_size);

            free(content);
        } else {
            // Se for campo de tamanho fixo, devemos olhar os casos
            switch (which_field){
                // prefixo nunca é nulo, então podemos comparar os valores diretamente 
                char prefix[5];
                case Prefix:
                    fread(prefix, sizeof(char), 5, bin_fp);
                    are_equal = compare_strings_whithout_terminator(prefix, value, 5);
                    break;
                
                // data pode ser nulo, então devemos comparar mais cuidadosamente
                char date[10];
                case Date:
                    fread(date, sizeof(char), 10, bin_fp);
                    are_equal = compare_strings_whithout_terminator(date, value, 10);
                    break;

                // quantidadeLugares pode ser nulo, então devemos comparar mais cuidadosamente
                int num_of_seats;
                case Num_of_seats:
                    fread(&num_of_seats, sizeof(int), 1, bin_fp);
                    if (strcmp(value, "NULO") == 0)
                        are_equal = (num_of_seats == -1);
                    else
                        are_equal = (num_of_seats == atoi(value));
                    break;
                
                default:
                    break;
            }                
        }

        // Se são iguais, volta pro início do registro e imprime ele
        // Se não, pula pro fim do registro e continua a leitura
        if (are_equal){
            fseek(bin_fp, start_of_register, SEEK_SET);
            VEHICLE valid_register;
            read_vehicle_register(bin_fp, &valid_register, TRUE);
            print_vehicle_register(&valid_register);
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

    // Lê o prefixo e insere na estrutura
    char *prefix = read_inside_quotes();
    if (prefix == NULL)
        return NULL;
    else 
        append_word(entries, prefix);

    // Lê a data e insere na estrutura
    char *date = read_inside_quotes();
    if (date == NULL)
        return NULL;
    else
        append_word(entries, date);

    // Lê a quantidadeLugares e insere na estrutura
    char *num_of_seats = read_word(stdin);
    if (num_of_seats == NULL)
        return NULL;
    else
        append_word(entries, num_of_seats);   

    // Lê o codigoLinha e insere na estrutura
    char *route_code = read_word(stdin);
    if (route_code == NULL)
        return NULL;
    else
        append_word(entries, route_code);

    // Lê o modelo e insere na estrutura
    char *model = read_inside_quotes();
    if (model == NULL)
        return NULL;
    else
        append_word(entries, model);

    // Lê a categoria e insere na estrutura
    char *category = read_inside_quotes();
    if (category == NULL)
        return NULL;
    else
        append_word(entries, category);

    return entries;
}

// Inserção de novos registros
void insert_new_vehicle(FILE *bin_fp, bool *inserted){
    // Vai para o início do arquivo binário para verificar se está consistente
    // e, caso esteja, muda para inconsistente para iniciarmos as inserções
    fseek(bin_fp, 0, SEEK_SET);

    VEHICLE_HEADER header;
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

    int num_registers = -1;
    scanf("%d", &num_registers);
    char cur_char = getc(stdin);
    while (cur_char != '\n' && cur_char != ' ' && cur_char != EOF)
        cur_char = getc(stdin);

    // Inicia as inserções
    for (int i = 0; i < num_registers; i++){
        // Lê as entradas, verificando se foram inseridas corretamente
        WORDS *entries  = read_entries();
        int num_of_entries = get_word_list_length(entries);
        if (num_of_entries != 6){
            printf("Faltam dados\n");
            *inserted = FALSE;
            return;
        }

        // Cria o registro e insere no binário
        VEHICLE new_vehicle;
        fill_register(&new_vehicle, get_word_list(entries), &header);
        write_data(bin_fp, &new_vehicle, &header);
        
        free_word_list(entries);
    }
    
    // Atualiza o cabeçalho
    update_header(bin_fp, &header);
    *inserted = TRUE;
}

// Função que cria uma chave a partir de um prefixo e insere no arquivo de índices
static void read_and_insert(FILE *bin_fp, btree *tree, long long int offset){
    char prefix[6];
    fread(prefix, sizeof(char), 5, bin_fp);
    prefix[5] = '\0';

    int new_key = convertePrefixo(prefix);
    insert_in_btree(tree, new_key, offset);

    // Indo para o fim do registro (pula os campos de tamanho variavel no loop)
    fseek(bin_fp, 18, SEEK_CUR);
    for (int i = 0; i < 2; i++) {
        int size;
        fread(&size, sizeof(int), 1, bin_fp);
        fseek(bin_fp, size, SEEK_CUR);
    }
}

// Função que cria um arquivo de índice para o veículo
bool create_vehicle_index_file(FILE *bin_fp, char *index_filename){
    btree *new_tree = init_tree(index_filename);
    if (new_tree == NULL) {
        printf("Falha no processamento do arquivo\n");
        return FALSE;
    }

    // Verifica a consistência do arquivo
    char status;
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0'){
        printf("Falha no processamento do arquivo.\n");
        destroy_btree(new_tree);
        return FALSE;
    }
    
    // Leitura do número de registros para ter controle das leituras
    int num_of_register;
    fseek(bin_fp, 8, SEEK_CUR); // Pulando para o byteOffset do proximo registro a ser inserido
    fread(&num_of_register, sizeof(int), 1, bin_fp);

    if (num_of_register == 0){
        printf("Registro inexistente.\n");       
        return FALSE;
    }

    // Vai para o primeiro registro e inicia a leitura
    fseek(bin_fp, 162, SEEK_CUR);
    for (int i = 0; i < num_of_register; i++){

        // Verifica se o registro existe e seu tamanho
        VEHICLE cur_register;
        fread(&cur_register.is_removed, sizeof(char), 1, bin_fp);
        fread(&cur_register.register_length, sizeof(int), 1, bin_fp);

        // Se o registro existe, lemos ele, se não pulamos
        bool should_read = (cur_register.is_removed == '1');
        if (!should_read){
            // Se o registro está marcado como removido, a leitura não deve considerar 
            // ele, pois ela considera apenas registros que existem
            i--;
            fseek(bin_fp, cur_register.register_length, SEEK_CUR);
        } else {
            long long int offset = ftell(bin_fp) - 5;
            read_and_insert(bin_fp, new_tree, offset);
        }
    }  

    // Atualizando o cabeçalho do arquivo de índices
    update_tree_header(new_tree);
    destroy_btree(new_tree);
    return TRUE;
}

// Função que procura um registro do tipo veículo usando a árvore B
void search_vehicle(FILE *bin_fp, btree *tree){
    // Verifica a consistência do arquivo
    char status;
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Lendo e convertendo o prefixo
    char *prefix = read_inside_quotes();
    int key = convertePrefixo(prefix);
    free(prefix);
    
    // Achando o byte offset do registro
    long long int offset = search_key(tree, key);
    if (offset == -1) {
        printf("Registro inexistente.\n");
        destroy_btree(tree);
        return;
    }

    // Indo até o registro, logo após o registro é lido e printado
    fseek(bin_fp, offset, SEEK_SET);
    VEHICLE valid_register;
    read_vehicle_register(bin_fp, &valid_register, TRUE);
    print_vehicle_register(&valid_register);

    // Liberando a memória utilizada
    free_dynamic_fields(&valid_register);
    destroy_btree(tree);
}

// Função que insere um veículo nos arquivos binário e de índice
void insert_vehicle_into_index_and_bin(FILE *bin_fp, btree *tree, bool *inserted){
    // Vai para o início do arquivo binário para verificar se está consistente
    // e, caso esteja, muda para inconsistente para iniciarmos as inserções
    fseek(bin_fp, 0, SEEK_SET);

    VEHICLE_HEADER header;
    read_header(bin_fp, &header);

    // Checando a consistência do arquivo
    if (header.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        destroy_btree(tree);
        *inserted = FALSE;
        return;
    } else {
        set_file_in_use(bin_fp);
        set_tree_in_use(tree);
    }

    // Vai para o byte Offset do próximo registro a ser inserido 
    fseek(bin_fp, header.next_reg, SEEK_SET);

    // Descobrindo quantos registros serão inseridos
    int num_registers = -1;
    scanf("%d", &num_registers);
    char cur_char = getc(stdin);
    while (cur_char != '\n' && cur_char != ' ' && cur_char != EOF)
        cur_char = getc(stdin);

    // Inicia as inserções
    for (int i = 0; i < num_registers; i++){
        // Lê as entradas, verificando se foram inseridas corretamente
        WORDS *entries  = read_entries();
        int num_of_entries = get_word_list_length(entries);
        if (num_of_entries != 6){
            printf("Faltam dados\n");
            *inserted = FALSE;
            return;
        }
        
        // Inserindo no arquivo de índices
        char **values = get_word_list(entries);
        int new_key = convertePrefixo(values[0]);
        insert_in_btree(tree, new_key, header.next_reg);

        // Cria o registro e insere no binário
        VEHICLE new_vehicle;
        fill_register(&new_vehicle, values, &header);
        write_data(bin_fp, &new_vehicle, &header);
        header.next_reg = ftell(bin_fp);
        
        free_word_list(entries);
    }
    
    // Atualiza o cabeçalho
    update_header(bin_fp, &header);
    update_tree_header(tree);
    destroy_btree(tree);
    *inserted = TRUE;
}
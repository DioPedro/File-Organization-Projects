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

// Função que copia uma única palavra
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

// Função que copia uma string de tamanho indefinido e alloca a memória exata para a mesma
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

// Função que cria um vetor de strings
WORDS *create_word_list(){
    WORDS *list = malloc(sizeof(WORDS));
    list->words = NULL;
    list->amount = 0;

    return list;
}

// Função que particiona uma string e a salva em uma word list
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

// Função que particiona uma string a partir de um delimitador e a salva em uma char **wordlist
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

// Função que retorna o tamanho de um vetor de strings
int get_word_list_length(WORDS *word_list){
    if (word_list != NULL)
        return word_list->amount;
    
    return 0;
}

// Função que retorna um vetor de strings
char **get_word_list(WORDS *word_list){
    if (word_list != NULL)
        return word_list->words;
    
    return NULL;
}

// Função que imprime um vetor de strings
void print_word_list(WORDS *word_list){
    if (word_list == NULL)
        return;
    
    for (int i = 0; i < word_list->amount; i++){
        printf("Word[%d]: %s\n", i, word_list->words[i]);
    }
    printf("\n");
}

// Função que imprime uma string que não tem o caracter '\0'
void print_string_without_terminator(char *string, int len, bool has_endl){
    for (int i = 0; i < len; i++)
        printf("%c", string[i]);

    if (has_endl)
        printf("\n");
}

// Função que libera a memória de um vetor de strings
void free_word_list(WORDS *word_list){
    if (word_list == NULL)
        return;
    
    for (int i = 0; i < word_list->amount; i++){
        free(word_list->words[i]);
    }
    free(word_list->words);
    free(word_list);
}

// Função que le uma palavra dentro de aspas (")
char *read_inside_quotes(){
    char cur_char = getc(stdin);
    bool is_first_quote = (cur_char == '"');

    // Se o primeiro caracter não for aspas, essa função retorna "NULO" e espera terminar o input
    if (!is_first_quote){
        char *nil = malloc(5 * sizeof(char));
        strcpy(nil, "NULO");
        while (cur_char != ' ' && cur_char != '\n')
            cur_char = getc(stdin);

        return nil;
    }
    
    // Se leu uma aspas, continua leitura até ler a segunda
    int cur_len = 0, size = 16;
    char *content = malloc(size * sizeof(char));
    while (is_first_quote){
        if (cur_len == size){
            size *= 2;
            content = realloc(content, size);
        }

        cur_char = getc(stdin);

        // Para de ler quando achar o segundo '"'
        is_first_quote = (cur_char != '"');
        if (!is_first_quote)
            break;
        
        content[cur_len++] = cur_char;
    }

    // Pulando caracters desnecessários
    cur_char = getc(stdin);
    while (cur_char != '\n' && cur_char != ' ')
        cur_char = getc(stdin);

    if (cur_len == 0)
        return NULL;

    content = realloc(content, cur_len + 1);
    content[cur_len] = '\0';

    return content;
}

// Função que compara duas strings sem o caracter delimitador
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

// Função que coloca uma nova string em um vetor de strings
void append_word(WORDS *base, char *new_word){
    if (base == NULL) {
        printf("WORDS nulo, encerrando função\n");
        return;
    }
    
    base->words = realloc(base->words, (base->amount + 1) * sizeof(char *));
    base->words[base->amount++] = new_word;
}

/* ------ General Utils ------ */

// Coloca nos campos de cabeçalho as respectivas strings ignorando \0 e preenchendo com @ quando
// necessário
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

// Escrevendo char a char de uma string no arquivo binário para ignorar o \0 
void write_data_strings(FILE *bin_fp, char *data_field, int size){
    for (int i = 0; i < size; i++)
        fwrite(&data_field[i], sizeof(char), 1, bin_fp);
}

// Liberando a memória primária
void free_data(WORDS *word_list, char *reg_line){
    free_word_list(word_list);
    free(reg_line);
}

// Função que retorna a existência de um registro
bool register_exists(FILE *fp){
    char status = 'y';
    fread(&status, sizeof(char), 1, fp);

    return (status == '1');
}

// Função que vai para o fim do registro atual
// @reg_len + 5 = tamanho total do registro
// @cur_offset - @start_of_register = quanto do registro a gente já "avançou"
// o que falta pro fim do registro = tamanho total - quanto já avançou
void go_to_end_of_register(FILE *bin_fp, long long start_of_register, int reg_len){
    long long int cur_offset = ftell(bin_fp);
    long long int next_reg_offset = reg_len + 5 - (cur_offset - start_of_register);
    fseek(bin_fp, next_reg_offset, SEEK_CUR);
}

// Função que marca um arquivo como em uso
void set_file_in_use(FILE *bin_fp){
    char status = '0';
    fseek(bin_fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, bin_fp);
}

/*
    OBS2:   retorna LIXO se a string contém algo diferente de números e letras 
            maiúsculas (com excessão do caso da OBS1)

    COMO FUNCIONA:

        Para converter os prefixos para int e garantir que prefixos diferentes 
        deem números diferentes interpretamos eles como números em base 36

        Um número N com 5 digitos dI em base 36 é escrito como:

            N = d0 * 36^0 + d1 * 36^1 + d2 * 36^2 + d3 * 36^3 + d4 * 36^4

        Nota-se que estamos escrevendo do digito menos significativo para o 
        mais significativo

        Como o nosso prefixo têm 5 bytes e eles podem assumir 36 valores
        cada, podemos interpretar cada char dele como um digito em base 36, 
        prefixos diferentes formaram números diferentes em base 36 e portanto 
        números diferentes quando convertemos para um int normal
*/
int convertePrefixo(char* str) {

    /* O registro que tem essa string como chave foi removido */
    if(str[0] == '*')
        return -1;

    /* Começamos com o primeiro digito na ordem de 36^0 = 1 */
    int power = 1;

    /* Faz a conversão char por char para chegar ao resultado */
    int result = 0;
    for(int i = 0; i < 5; i++) {

        /* 
            Interpreta o char atual como se fosse um digito
            em base 36. Os digitos da base 36 são:
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D,
            E, F, G, H, I, J, K, L, M, N, O, P, Q, R,
            S, T, U, V, W, X, Y, Z
        */
        int cur_digit;
        /* Checa pelos digitos normais e os converte para números */
        if(str[i] >= '0' && str[i] <= '9')
            cur_digit = str[i] - '0';
        /* Checa pelas letras e as converte para números */
        else if(str[i] >= 'A' && str[i] <= 'Z')
            cur_digit = 10 + str[i] - 'A';

        /*
            Multiplica o digito atual pelo ordem da posição atual
            e adiciona no resultado
            Primeira posição:   36^0 = 1
            Segunda posição:    36^1 = 36
            Terceira posição:   36^2 = 1.296
            Quarta posição:     36^3 = 46.656
            Quinta posição:     36^4 = 1.679.616
        */
        result += cur_digit * power;

        /* Aumenta a ordem atual */
        power *= 36;

    }

    return result;

}
/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/  

#include "../includes/menu.h"

typedef enum _CASE{
    Route,
    Vehicle
} CASE;

void binarioNaTela(char *nomeArquivoBinario) { 
    /* Você não precisa entender o código dessa função. */
	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "Falha no processamentodo arquivo.\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}

// @which_case é um parâmetro para decidir a formatação do arquivo (veiculo ou linha)
 
// Função que cria um arquivo binário a partir de um csv
void create_binary(CASE which_case){
    char *csv_filename = read_word(stdin);

    // Verifica se o arquivo existe 
    FILE *csv_fp = fopen(csv_filename, "r");
    if (csv_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(csv_filename);
        return;
    }
    
    char *bin_filename = read_word(stdin);
    
    // Abre para escrita o arquivo binário e verifica se foi aberto com sucesso
    FILE *bin_fp = fopen(bin_filename, "wb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        return;
    }

    // Escolhe entre os dois casos possíveis e cria o binário
    if (which_case == Vehicle)
        create_vehicle_binary(csv_fp, bin_fp);
    else
        create_route_binary(csv_fp, bin_fp);

    fclose(bin_fp);
    fclose(csv_fp);

    binarioNaTela(bin_filename);

    free(csv_filename);
    free(bin_filename);
}

// Função que imprime um arquivo binário
void print_data(CASE which_case){
    char *bin_filename = read_word(stdin);

    // Abre para leitura e verifica a existência do arquivo
    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Escolhe entre os dois binários possíveis e imprime
    if (which_case == Vehicle)
        read_vehicle_bin(bin_fp);
    else 
        read_route_bin(bin_fp);

    free(bin_filename);
    fclose(bin_fp);
} 

// Função que procura por campos com um determinado valor no binário
void search_by_field(CASE which_case){
    char *bin_filename = read_word(stdin);
    char *field_name = read_word(stdin);
    
    // Se os campos forem quantidadeLugares ou codLinha, os valores inseridos não estarão entre aspas
    // Se não, a leitura deve considerar a existência ou não delas
    char *value;
    if (strcmp(field_name, "quantidadeLugares") == 0 || strcmp(field_name, "codLinha") == 0) 
        value = read_word(stdin);
    else
        value = read_inside_quotes();

    // Abre para leitura e verifica a existência do arquivo binário
    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Escolhe entre os dois casos possíveis e procura pelo campo especificado
    if (which_case == Vehicle)
        search_vehicle_by_field(bin_fp, field_name, value);
    else
        search_route_by_field(bin_fp, field_name, value);

    free(bin_filename);
    free(field_name);
    free(value);
    fclose(bin_fp);
}

// Função que insere registros no arquivo binário
void insert(CASE which_case){
    char *bin_filename = read_word(stdin);

    // Abre para leitura e atualização e verifica a existência do arquivo binário
    FILE *bin_fp = fopen(bin_filename, "r+b");
    if (bin_fp == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Insere os novos dados, verificando se isso foi possível
    bool inserted;
    if (which_case == Vehicle)
        insert_new_vehicle(bin_fp, &inserted);
    else 
        insert_new_route(bin_fp, &inserted);

    fclose(bin_fp);

    // Se foi possível, imprime o binário na tela
    if (inserted)
        binarioNaTela(bin_filename);

    free(bin_filename);
}

void create_index(CASE which_case){
    char *bin_filename = read_word(stdin);
    char *index_filename = read_word(stdin);
    
    // Cria arquivo de indices com o nome especificado
    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        free(index_filename);
        return;
    }

    bool was_created = FALSE;
    if (which_case == Vehicle)
        was_created = create_vehicle_index_file(bin_fp, index_filename);
    else 
        was_created = create_route_index_file(bin_fp, index_filename);
    
    if (was_created)
        binarioNaTela(index_filename);

    free(bin_filename);
    free(index_filename);
    fclose(bin_fp);
}

void search_in_index_file(CASE which_case){
    char *bin_filename = read_word(stdin);
    
    // Cria arquivo de indices com o nome especificado
    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        return;
    }

    if (which_case == Vehicle)
        search_vehicle(bin_fp);
    else 
        search_route(bin_fp);
    
    free(bin_filename);
    fclose(bin_fp);
}

// Função que insere registros no arquivo binário
void extended_insert(CASE which_case){
    char *bin_filename = read_word(stdin);

    // Abre para leitura e atualização e verifica a existência do arquivo binário
    FILE *bin_fp = fopen(bin_filename, "r+b");
    if (bin_fp == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        return;
    }

    char *index_filename = read_word(stdin);
    btree *tree = load_btree(index_filename);
    if  (tree == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        free(index_filename);
        fclose(bin_fp);
        return;
    }

    // Insere os novos dados, verificando se isso foi possível
    bool inserted;
    if (which_case == Vehicle)
        insert_vehicle_into_index_and_bin(bin_fp, tree, &inserted);
    else 
        insert_route_into_index_and_bin(bin_fp, tree, &inserted);

    fclose(bin_fp);

    // Se foi possível, imprime o binário na tela
    if (inserted)
        binarioNaTela(index_filename);

    free(index_filename);
    free(bin_filename);
}

void start_program(){
    char *operation = read_word(stdin); // código da operação 
    int which_case = atoi(operation);   // caso veículo ou linha
    free(operation);

    // Operações com valor ímpar são para veículo, com valor par, para linha
    if (which_case == 1 || which_case == 2)         // Casos 1 e 2 são para gerar binário
        create_binary(which_case % 2);
    else if (which_case == 3 || which_case == 4)    // 3 e 4, para imprimir os dados
        print_data(which_case % 2);
    else if (which_case == 5 || which_case == 6)    // 5 e 6, para procura 
        search_by_field(which_case % 2);
    else if (which_case == 7 || which_case == 8)    // 7 e 8, para inserção 
        insert(which_case % 2);
    else if (which_case == 9 || which_case == 10)   // 9 e 10, criam arquivo de indice
        create_index(which_case % 2);
    else if (which_case == 11 || which_case == 12)  // 11 e 12, procuram chave pelo arquivo de indice
        search_in_index_file(which_case % 2);
    else if (which_case == 13 || which_case == 14)  // 13 e 14, insere novos valores no binario e no indice
        extended_insert(which_case % 2);
    else
        printf("Comando inválido\n");
}
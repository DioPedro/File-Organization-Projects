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

void binarioNaTela(char *nomeArquivoBinario) { /* Você não precisa entender o código dessa função. */

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

void create_binary(CASE which_case){
    char *csv_filename = read_word(stdin);

    FILE *csv_fp = fopen(csv_filename, "r");
    if (csv_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(csv_filename);
        return;
    }

    char *bin_filename = read_word(stdin);

    FILE *bin_fp = fopen(bin_filename, "wb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(bin_filename);
        return;
    }

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

void print_data(CASE which_case){
    char *bin_filename = read_word(stdin);

    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    if (which_case == Vehicle)
        read_vehicle_bin(bin_fp);
    else 
        read_route_bin(bin_fp);

    free(bin_filename);
    fclose(bin_fp);
} 

void search_by_field(CASE which_case){
    char *bin_filename = read_word(stdin);
    char *field_name = read_word(stdin);
    char *value;
    if (strcmp(field_name, "quantidadeLugares") == 0 || strcmp(field_name, "codLinha") == 0) 
        value = read_word(stdin);
    else
        value = read_inside_quotes();

    FILE *bin_fp = fopen(bin_filename, "rb");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    if (which_case == Vehicle)
        search_vehicle_by_field(bin_fp, field_name, value);
    else
        search_route_by_field(bin_fp, field_name, value);

    free(bin_filename);
    free(field_name);
    free(value);
    fclose(bin_fp);
}

void insert(CASE which_case){
    char *bin_filename = read_word(stdin);

    FILE *bin_fp = fopen(bin_filename, "r+b");
    if (bin_fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    bool inserted;
    if (which_case == Vehicle)
        insert_new_vehicle(bin_fp, &inserted);
    else 
        insert_new_route(bin_fp, &inserted);

    fclose(bin_fp);

    if (inserted)
        binarioNaTela(bin_filename);

    free(bin_filename);
}

void start_program(){
    char *operation = read_word(stdin);
    int which_case = atoi(operation);
    free(operation);

    if (which_case == 1 || which_case == 2)
        create_binary(which_case % 2);
    else if (which_case == 3 || which_case == 4)
        print_data(which_case % 2);
    else if (which_case == 5 || which_case == 6)
        search_by_field(which_case % 2);
    else if (which_case == 7 || which_case == 8)
        insert(which_case % 2);
    else
        printf("Comando inválido\n");
}
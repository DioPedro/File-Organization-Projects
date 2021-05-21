/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/utils.h"
#include "../includes/route.h"
#include "../includes/vehicle.h"

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

int main(){
    char *op = read_word(stdin);
    int operation = atoi(op);
    free(op);
    // char *csv_name = read_word(stdin);
    char *bin_name = read_word(stdin);
    printf("%s\n", bin_name);

    printf("%d %s\n", operation, bin_name);

    // FILE *csv_fp = fopen(csv_name, "r");    
    // if (csv_fp == NULL){
    //     printf("Deu ruim no csv\n");
    //     return 0;
    // }

    FILE *bin_fp = fopen(bin_name, "r+b");
    if (bin_fp == NULL){
        printf("Deu ruim no binário\n");
        return 0;
    }
    else
        printf("Abriu\n");
    
    // create_route_binary(csv_fp, bin_fp);
    // create_vehicle_binary(csv_fp, bin_fp);

    // char field[100];
    // scanf("%s", field);
    // printf("%s\n", field);

    // int len;
    // char *value = read_inside_quotes(&len);
    // printf("%s\n", value);

    // search_route_by_field(bin_fp, field, value);
    // search_vehicle_by_field(bin_fp, field, value);
    // read_route_bin(bin_fp);
    // read_vehicle_bin(bin_fp);

    // free(value);

    /* Falta arrumar a funcao de ler valor */

    // insert_new_vehicle(bin_fp);
    insert_new_route(bin_fp);


    // fclose(csv_fp);
    fclose(bin_fp);

    // binarioNaTela(bin_name);
    // binarioNaTela("./CasosT/Binary Files/linha15_saida_esperada.bin");

    // free(csv_name);
    free(bin_name);

    return 0;
}
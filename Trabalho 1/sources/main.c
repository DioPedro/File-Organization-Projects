/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/

#include "../includes/str.h"
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
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
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
    char *csv_name = read_word(stdin);
    char *bin_name = read_word(stdin);

    printf("%d %s %s\n", operation, csv_name, bin_name);

    FILE *csv_fp = fopen(csv_name, "r");    
    if (csv_fp == NULL){
        printf("Deu ruim no csv\n");
        return 0;
    }

    FILE *bin_fp = fopen(bin_name, "wb");
    if (bin_fp == NULL){
        printf("Deu ruim no binário\n");
        return 0;
    }
    
    // create_route_binary(csv_fp, bin_fp);
    create_vehicle_binary(csv_fp, bin_fp);

    fclose(csv_fp);
    fclose(bin_fp);

    binarioNaTela(bin_name);
    binarioNaTela("./CasosT/Binary Files/veiculo1_saida_esperada.bin");

    free(csv_name);
    free(bin_name);

    return 0;
}
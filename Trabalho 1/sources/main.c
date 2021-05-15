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
    
    create_route_binary(csv_fp, bin_fp);

    free(csv_name);
    free(bin_name);
    fclose(csv_fp);
    fclose(bin_fp);

    return 0;
}
/* 
**************************************************
*             TRABALHO 1 de Arquivos             *
* Alunos:                                        *
*  - Diógenes Silva Pedro, nUSP: 11883476        *
*  - Victor Henrique de Sa Silva, nUSP: 11795759 *
**************************************************
*/  

#include "../includes/intersect.h"

// Função que checa a integridade do arquivo binário
bool check_bin_integrity(FILE *bin_fp){
    // Verificando se o arquivo passado está consistente
    if (bin_fp == NULL)
        return FALSE;

    // Volta para o inicio do arquivo e coloca como inconsistente
    fseek(bin_fp, 0, SEEK_SET);
    
    char status = '0';
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0')
        return FALSE;
    
    return TRUE;
}

// Função de checagem de intersecção entre os arquivos de maneira "brute force"
int brute_intersection(FILE *vehicle_bin, FILE *route_bin){
    // Checa a integridade dos arquivos de dados 
    bool vehicle_is_ok = check_bin_integrity(vehicle_bin);
    bool route_is_ok = check_bin_integrity(route_bin);
    if (!vehicle_is_ok || !route_is_ok)
        return FILE_FAILURE;

    // Controle para conferir se houve algum match
    bool found_matches = FALSE;

    // Iterando por todos os registros, de forma aninhada, procurando por códigos 
    // linha iguais
    int num_of_vehicles = get_num_of_vehicles(vehicle_bin);
    int num_of_routes = get_num_of_routes(route_bin);
    
    for (int i = 0; i < num_of_vehicles; i++) {
        
        // Para cada veículo, le o registro 
        VEHICLE current_vehicle;
        read_vehicle_register(vehicle_bin, &current_vehicle, TRUE);

        // Verifica se não foi removido
        if (current_vehicle.is_removed == '0') {
            i--;
            free_vehicle_strings(&current_vehicle);
            continue;
        }   

        // E passa pelos registros de linha, até achar a interseção, se houver
        for (int j = 0; j < num_of_routes; j++) {

            // Leitura e verificação dos registros de linha
            ROUTE current_route;
            read_route_register(route_bin, &current_route, TRUE);
            if (current_route.is_removed == '0') {
                j--;
                free_route_strings(&current_route);
                continue;
            }
            
            // Procurando por codLinhas iguais
            if (current_route.route_code == current_vehicle.route_code) {
                found_matches = TRUE;
                print_vehicle_register(&current_vehicle, FALSE);
                print_route_register(&current_route);
                
                free_route_strings(&current_route);

                // Se achar, para de procurar pelas linhas e avança para o proximo veiculo
                break;
            }

            // Quando o codLinha é diferente, libera o registro da linha da memoria
            free_route_strings(&current_route);
        }

        // Libera o registro do veiculo da memoria
        free_vehicle_strings(&current_vehicle);

        // E volta para o inicio do arquivo de linhas para o novo codLinha
        fseek(route_bin, 82, SEEK_SET);
    }

    // Se nao acha intersecção, retorna um codigo de erro
    if (!found_matches)
        return NOT_FOUND;
    
    return SUCCESS;
}

// Busca otimizada usando arquivo de indices
int optimized_intersection(FILE *vehicle_bin, FILE *route_bin, btree *index_file){

    // Novamente, verifica se os arquivos estão consistentes
    bool vehicle_is_ok = check_bin_integrity(vehicle_bin);
    bool route_is_ok = check_bin_integrity(route_bin);
    if (!vehicle_is_ok || !route_is_ok)
        return FILE_FAILURE;
    
    // Controle de numero de veiculos e de intersecções
    bool found_matches = FALSE;
    int num_of_vehicles = get_num_of_vehicles(vehicle_bin);
    for (int i = 0; i < num_of_vehicles; i++) {
        
        // Para cada veículo, le o registro, verifica se existe 
        VEHICLE current_vehicle;
        read_vehicle_register(vehicle_bin, &current_vehicle, TRUE);
        if (current_vehicle.is_removed == '0') {
            i--;
            free_vehicle_strings(&current_vehicle);
            continue;
        }   
        
        // E busca pelo codLinha na arvore B
        int offset = search_key(index_file, current_vehicle.route_code);

        // Se não achou, libera o veiculo da memoria e avança para o proximo
        if (offset == -1) {
            free_vehicle_strings(&current_vehicle);
            continue;   
        }
        found_matches = TRUE;

        // Se achou, vai até o indice no arquivo de linha        
        fseek(route_bin, offset, SEEK_SET);
        
        // Le o registro 
        ROUTE final_route;
        read_route_register(route_bin, &final_route, TRUE);
        
        // E imprime conforme especificado
        print_vehicle_register(&current_vehicle, FALSE);
        print_route_register(&final_route);

        // Liberando da memória, logo após
        free_vehicle_strings(&current_vehicle);
        free_route_strings(&final_route);
    }

    // Se nao achou, retorna codigo de erro
    if (!found_matches)
        return NOT_FOUND;
    
    return SUCCESS;
}

// Função que atualiza o cabeçalho do arquivo ordenado
void update_header(FILE *bin_fp, char status, long long int next_reg){
    fseek(bin_fp, 1, SEEK_SET);
    fwrite(&next_reg, sizeof(long long int), 1, bin_fp);
    fseek(bin_fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, bin_fp);
}

// Função auxiliar de comparação entre duas rotas
bool route_compare(const void *v1, const void *v2){
    ROUTE *a = *(ROUTE **)v1;
    ROUTE *b = *(ROUTE **)v2;

    return (a->route_code > b->route_code);
}

// Função que retorna todos os registros do arquivo de linha
ROUTE **get_route_registers(FILE *bin_fp, int num_of_routes){
    ROUTE **routes = malloc(num_of_routes * sizeof(ROUTE*));
    for (int i = 0; i < num_of_routes; i++) {
        routes[i] = malloc(sizeof(ROUTE));
    }

    for (int i = 0; i < num_of_routes; i++) {
        read_route_register(bin_fp, routes[i], TRUE);

        if (routes[i]->is_removed == '0') {
            free_route_strings(routes[i]);
            i--;
        }
    }

    return routes;
}

// Função que escreve o cabeçalho do arquivo ordenado das linha
void write_route_header(FILE *bin_fp, ROUTE_HEADER *header){
    fseek(bin_fp, 0, SEEK_SET);
    // Setando o arquivo como inconsistente
    header->status = '0';
    // Atualizaremos o número do próximo registro após a escrita de todos os registros
    header->next_reg = 0;
    // Como não adicionaremos registros logicamente removidos podemos setar para 0
    header->num_of_removeds = 0; 
    fwrite(&header->status, sizeof(char), 1, bin_fp);
    fwrite(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fwrite(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fwrite(&header->num_of_removeds, sizeof(int), 1, bin_fp);
    fwrite(&header->code_description, sizeof(char), 15, bin_fp);
    fwrite(&header->card_description, sizeof(char), 13, bin_fp);
    fwrite(&header->name_description, sizeof(char), 13, bin_fp);
    fwrite(&header->color_description, sizeof(char), 24, bin_fp);
}

// Função que escreve um registro de linha no arquivo binário
void write_route(FILE *bin_fp, ROUTE *route){
    // Escrita dos campos de tamanho fixo:
    // removido, tamanhoRegistro, codLinha, aceitaCartao, respectivamente
    fwrite(&(route->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(route->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(route->route_code), sizeof(int), 1, bin_fp);
    fwrite(&(route->accepts_card), sizeof(char), 1, bin_fp);

    // Escrita do tamanho do nomeLinha: se for 0, nomeLinha não deve ser escrito 
    fwrite(&(route->name_length), sizeof(int), 1, bin_fp);
    if (route->name_length != 0)
        write_data_strings(bin_fp, route->route_name, route->name_length);

    // Escrita do tamanho do corLinha: se for 0, corLinha não deve ser escrito 
    fwrite(&(route->color_length), sizeof(int), 1, bin_fp);
    if (route->color_length != 0)
        write_data_strings(bin_fp, route->color, route->color_length);
}

// Função que ordena todos os registros de um arquivo de linha
ROUTE **sort_route_file(FILE *bin_fp){
    bool vehicle_is_ok = check_bin_integrity(bin_fp);
    if (!vehicle_is_ok) 
        return NULL;   

    ROUTE_HEADER header = get_route_header(bin_fp);

    ROUTE **routes = get_route_registers(bin_fp, header.num_of_regs);

    qsort(routes, header.num_of_regs, sizeof(ROUTE*), route_compare);

    return routes;
}

// Função que libera todos os registros de um arquivo de linha que estão em RAM
void free_route_registers(ROUTE **routes, int num_of_regs){
    for (int i = 0; i < num_of_regs; i++) {
        free_route_strings(routes[i]);
        free(routes[i]);
    }
    free(routes);
}

// Função que escreve um arquivo com os registros de linha de maneira ordenada
int write_sorted_route_file(FILE *bin_fp, char *new_file_name){
    ROUTE_HEADER header = get_route_header(bin_fp);
    
    ROUTE **routes = sort_route_file(bin_fp);
    if (routes == NULL) 
        return FILE_FAILURE; 

    FILE *new_fp = fopen(new_file_name, "wb");
    if (new_fp == NULL) 
        return FILE_FAILURE; 

    write_route_header(new_fp, &header);

    for (int i = 0; i < header.num_of_regs; i++) {
        write_route(new_fp, routes[i]);
    }

    free_route_registers(routes, header.num_of_regs);

    // Descobrindo o local do próximo registro
    header.next_reg = ftell(new_fp);
    // Setando o arquivo como consistente
    header.status = '1';
    
    update_header(new_fp, header.status, header.next_reg);

    fclose(new_fp);

    return SUCCESS;
}

// Função aulixiar de comparação entre dois veículos
bool vehicle_compare(const void *v1, const void *v2){
    VEHICLE *a = *(VEHICLE **)v1;
    VEHICLE *b = *(VEHICLE **)v2;

    return (a->route_code > b->route_code);
}

// Função que retorna todos os registros do arquivo de veículos
VEHICLE **get_vehicle_registers(FILE *bin_fp, int num_of_vehicles){
    VEHICLE **vehicles = malloc(num_of_vehicles * sizeof(VEHICLE*));
    for (int i = 0; i < num_of_vehicles; i++) {
        vehicles[i] = malloc(sizeof(VEHICLE));
    }

    for (int i = 0; i < num_of_vehicles; i++) {
        read_vehicle_register(bin_fp, vehicles[i], TRUE);

        if (vehicles[i]->is_removed == '0') {
            free_vehicle_strings(vehicles[i]);
            i--;
        }
    }

    return vehicles;
}

// Função que escreve o cabeçalho do arquivo ordenado dos veículos
void write_vehicle_header(FILE *bin_fp, VEHICLE_HEADER *header){
    fseek(bin_fp, 0, SEEK_SET);
    // Setando o arquivo como inconsistente
    header->status = '0';
    // Atualizremos o número do próximo registro após a escrita de todos os registros
    header->next_reg = 0;
    // Como não adicionaremos registros logicamente removidos podemos setar para 0
    header->num_of_removeds = 0;
    fwrite(&header->status, sizeof(char), 1, bin_fp);
    fwrite(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fwrite(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fwrite(&header->num_of_removeds, sizeof(int), 1, bin_fp);
    fwrite(&header->prefix_description, sizeof(char), 18, bin_fp);
    fwrite(&header->date_description, sizeof(char), 35, bin_fp);
    fwrite(&header->seats_description, sizeof(char), 42, bin_fp);
    fwrite(&header->route_description, sizeof(char), 26, bin_fp);
    fwrite(&header->model_description, sizeof(char), 17, bin_fp);
    fwrite(&header->category_description, sizeof(char), 20, bin_fp);
}

// Função que escreve um registro de veículo no arquivo binário
void write_vehicle(FILE *bin_fp, VEHICLE *vehicle){
    // Escrita dos campos de tamanho fixo:
    // removido, tamanhoRegistro, prefixo, data, quantidadeLugares, codigoLinha
    fwrite(&(vehicle->is_removed), sizeof(char), 1, bin_fp);
    fwrite(&(vehicle->register_length), sizeof(int), 1, bin_fp);
    fwrite(&(vehicle->prefix), sizeof(char), 5, bin_fp);
    fwrite(&(vehicle->date), sizeof(char), 10, bin_fp);
    fwrite(&(vehicle->num_of_seats), sizeof(int), 1, bin_fp);
    fwrite(&(vehicle->route_code), sizeof(int), 1, bin_fp);

    // Escrita do tamanho do modelo: se for 0, modelo não deve ser escrito 
    fwrite(&(vehicle->model_length), sizeof(int), 1, bin_fp);
    if (vehicle->model_length != 0)
        write_data_strings(bin_fp, vehicle->model, vehicle->model_length);

    // Escrita do tamanho da categoria: se for 0, categoria não deve ser escrito
    fwrite(&(vehicle->category_length), sizeof(int), 1, bin_fp);
    if (vehicle->category_length != 0)
        write_data_strings(bin_fp, vehicle->category, vehicle->category_length);
}

// Função que ordena todos os registros de um arquivo de veículos
VEHICLE **sort_vehicle_file(FILE *bin_fp){
    bool vehicle_is_ok = check_bin_integrity(bin_fp);
    if (!vehicle_is_ok) 
        return NULL;
    
    VEHICLE_HEADER header = get_vehicle_header(bin_fp);

    VEHICLE **vehicles = get_vehicle_registers(bin_fp, header.num_of_regs);

    qsort(vehicles, header.num_of_regs, sizeof(VEHICLE*), vehicle_compare);

    return vehicles;
}

// Função que libera todos os registros de um arquivo de veículos
void free_vehicle_registers(VEHICLE **vehicles, int num_of_regs){
    for (int i = 0; i < num_of_regs; i++) {
        free_vehicle_strings(vehicles[i]);
        free(vehicles[i]);
    }
    free(vehicles);
}

// Função que escreve um arquivo com os registros de veículo de maneira ordenada
int write_sorted_vehicle_file(FILE *bin_fp, char *new_file_name){
    VEHICLE_HEADER header = get_vehicle_header(bin_fp);
    
    VEHICLE **vehicles = sort_vehicle_file(bin_fp);
    if (vehicles == NULL) 
        return FILE_FAILURE;

    FILE *new_fp = fopen(new_file_name, "wb");
    if (new_fp == NULL) 
        return FILE_FAILURE;

    write_vehicle_header(new_fp, &header);

    for (int i = 0; i < header.num_of_regs; i++) {
        write_vehicle(new_fp, vehicles[i]);
    }

    free_vehicle_registers(vehicles, header.num_of_regs);

    // Descobrindo o local do próximo registro
    header.next_reg = ftell(new_fp);
    // Setando o arquivo como consistente
    header.status = '1';
    
    update_header(new_fp, header.status, header.next_reg);

    fclose(new_fp);

    return SUCCESS;
}

// Função que faz a comparação entre dois arquivos ordenados e printa os registros de veículo e
// os registros de linha que tem o mesmo código de linha
int merge_files(FILE *vehicle_fp, FILE *route_fp){
    // Abrindo e ordenado os registros de veículo
    VEHICLE **vehicles = sort_vehicle_file(vehicle_fp);
    if (vehicles == NULL)
        return FILE_FAILURE;
    int num_of_vehicles = get_num_of_vehicles(vehicle_fp);

    // Abrindo e ordenando os registros de linha
    ROUTE **routes = sort_route_file(route_fp);
    if (routes == NULL) {
        free_vehicle_registers(vehicles, num_of_vehicles);
        return FILE_FAILURE;
    }
    int num_of_routes = get_num_of_routes(route_fp);

    // Procurando pelos registros com mesmo código de linha
    int i = 0, j = 0;
    bool has_matches = FALSE;
    while (i < num_of_vehicles && j < num_of_routes) {
        if (vehicles[i]->route_code == routes[j]->route_code) {
            has_matches = TRUE;
            print_vehicle_register(vehicles[i], FALSE);
            print_route_register(routes[j]);
        }

        if (vehicles[i]->route_code <= routes[j]->route_code)
            i++;
        else
            j++;
    }

    // Liberando os registros que estão em RAM
    free_route_registers(routes, num_of_routes);
    free_vehicle_registers(vehicles, num_of_vehicles);

    if (!has_matches)
        return NOT_FOUND;

    return SUCCESS;
}
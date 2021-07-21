#include "../includes/intersect.h"

bool check_bin_integrity(FILE *bin_fp){
    if (bin_fp == NULL)
        return FALSE;
    fseek(bin_fp, 0, SEEK_SET);
    
    char status = '0';
    fread(&status, sizeof(char), 1, bin_fp);
    if (status == '0')
        return FALSE;
    
    return TRUE;
}

int brute_intersection(FILE *vehicle_bin, FILE *route_bin){
    bool vehicle_is_ok = check_bin_integrity(vehicle_bin);
    bool route_is_ok = check_bin_integrity(route_bin);
    if (!vehicle_is_ok || !route_is_ok)
        return FILE_FAILURE;

    bool found_matches = FALSE;
    int num_of_vehicles = get_num_of_vehicles(vehicle_bin);
    int num_of_routes = get_num_of_routes(route_bin);
    for (int i = 0; i < num_of_vehicles; i++) {
        VEHICLE current_vehicle;
        read_vehicle_register(vehicle_bin, &current_vehicle, TRUE);
        if (current_vehicle.is_removed == '0') {
            i--;
            free_vehicle_strings(&current_vehicle);
            continue;
        }   

        for (int j = 0; j < num_of_routes; j++) {
            ROUTE current_route;
            read_route_register(route_bin, &current_route, TRUE);
            if (current_route.is_removed == '0') {
                j--;
                free_route_strings(&current_route);
                continue;
            }
            
            if (current_route.route_code == current_vehicle.route_code) {
                found_matches = TRUE;
                print_vehicle_register(&current_vehicle, FALSE);
                print_route_register(&current_route);
                
                free_route_strings(&current_route);
                break;
            }

            // Quando nao acha, tambem remove da memoria para evitar leaks
            free_route_strings(&current_route);
        }
        free_vehicle_strings(&current_vehicle);
        fseek(route_bin, 82, SEEK_SET);
    }

    if (!found_matches)
        return NOT_FOUND;
    
    return SUCCESS;
}

int optimized_intersection(FILE *vehicle_bin, FILE *route_bin, btree *index_file){
    bool vehicle_is_ok = check_bin_integrity(vehicle_bin);
    bool route_is_ok = check_bin_integrity(route_bin);
    if (!vehicle_is_ok || !route_is_ok)
        return FILE_FAILURE;

    bool found_matches = FALSE;
    int num_of_vehicles = get_num_of_vehicles(vehicle_bin);
    for (int i = 0; i < num_of_vehicles; i++) {
        VEHICLE current_vehicle;
        read_vehicle_register(vehicle_bin, &current_vehicle, TRUE);
        if (current_vehicle.is_removed == '0') {
            i--;
            free_vehicle_strings(&current_vehicle);
            continue;
        }   

        int offset = search_key(index_file, current_vehicle.route_code);

        if (offset == -1) {
            free_vehicle_strings(&current_vehicle);
            continue;   
        }
        found_matches = TRUE;
        
        fseek(route_bin, offset, SEEK_SET);
        
        ROUTE final_route;
        read_route_register(route_bin, &final_route, TRUE);
        
        print_vehicle_register(&current_vehicle, FALSE);
        print_route_register(&final_route);

        free_vehicle_strings(&current_vehicle);
        free_route_strings(&final_route);

        fseek(route_bin, 82, SEEK_SET);
    }

    if (!found_matches)
        return NOT_FOUND;
    
    return SUCCESS;
}

void update_header(FILE *bin_fp, char status, long long int next_reg){
    fseek(bin_fp, 1, SEEK_SET);
    fwrite(&next_reg, sizeof(long long int), 1, bin_fp);
    fseek(bin_fp, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, bin_fp);
}

bool route_compare(const void *v1, const void *v2){
    ROUTE *a = *(ROUTE **)v1;
    ROUTE *b = *(ROUTE **)v2;

    return (a->route_code > b->route_code);
}

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

void write_route_header(FILE *bin_fp, ROUTE_HEADER *header){
    fseek(bin_fp, 0, SEEK_SET);
    header->status = '0'; // Set inconsistency
    header->next_reg = 0; // We're gonna update after writing all the vehicles
    header->num_of_removeds = 0; // Since we are'nt adding removed register to the new file we hard code it
    fwrite(&header->status, sizeof(char), 1, bin_fp);
    fwrite(&header->next_reg, sizeof(long long int), 1, bin_fp);
    fwrite(&header->num_of_regs, sizeof(int), 1, bin_fp);
    fwrite(&header->num_of_removeds, sizeof(int), 1, bin_fp);
    fwrite(&header->code_description, sizeof(char), 15, bin_fp);
    fwrite(&header->card_description, sizeof(char), 13, bin_fp);
    fwrite(&header->name_description, sizeof(char), 13, bin_fp);
    fwrite(&header->color_description, sizeof(char), 24, bin_fp);
}

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

ROUTE **sort_route_file(FILE *bin_fp){
    bool vehicle_is_ok = check_bin_integrity(bin_fp);
    if (!vehicle_is_ok) 
        return NULL;   

    ROUTE_HEADER header = get_route_header(bin_fp);

    ROUTE **routes = get_route_registers(bin_fp, header.num_of_regs);

    qsort(routes, header.num_of_regs, sizeof(ROUTE*), route_compare);

    return routes;
}

void free_route_registers(ROUTE **routes, int num_of_regs){
    for (int i = 0; i < num_of_regs; i++) {
        free_route_strings(routes[i]);
        free(routes[i]);
    }
    free(routes);
}

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

    header.next_reg = ftell(new_fp);
    header.status = '1'; //Set consistency
    
    update_header(new_fp, header.status, header.next_reg);

    fclose(new_fp);

    return SUCCESS;
}

bool vehicle_compare(const void *v1, const void *v2){
    VEHICLE *a = *(VEHICLE **)v1;
    VEHICLE *b = *(VEHICLE **)v2;

    return (a->route_code > b->route_code);
}

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

void write_vehicle_header(FILE *bin_fp, VEHICLE_HEADER *header){
    fseek(bin_fp, 0, SEEK_SET);
    header->status = '0'; // Set inconsistency
    header->next_reg = 0; // We're gonna update after writing all the vehicles
    header->num_of_removeds = 0; // Since we are'nt adding removed register to the new file we hard code it
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

VEHICLE **sort_vehicle_file(FILE *bin_fp){
    bool vehicle_is_ok = check_bin_integrity(bin_fp);
    if (!vehicle_is_ok) {
        return NULL;
    }

    VEHICLE_HEADER header = get_vehicle_header(bin_fp);

    VEHICLE **vehicles = get_vehicle_registers(bin_fp, header.num_of_regs);

    qsort(vehicles, header.num_of_regs, sizeof(VEHICLE*), vehicle_compare);

    return vehicles;
}

void free_vehicle_registers(VEHICLE **vehicles, int num_of_regs){
    for (int i = 0; i < num_of_regs; i++) {
        free_vehicle_strings(vehicles[i]);
        free(vehicles[i]);
    }
    free(vehicles);
}

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

    header.next_reg = ftell(new_fp);
    header.status = '1'; //Set consistency
    
    update_header(new_fp, header.status, header.next_reg);

    fclose(new_fp);

    return SUCCESS;
}

int merge_files(FILE *vehicle_fp, FILE *route_fp){
    VEHICLE **vehicles = sort_vehicle_file(vehicle_fp);
    ROUTE **routes = sort_route_file(route_fp);

    if (vehicles == NULL || routes == NULL)
        return FILE_FAILURE;

    int num_of_vehicles = get_num_of_vehicles(vehicle_fp);
    int num_of_routes = get_num_of_routes(route_fp);

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

    free_route_registers(routes, num_of_routes);
    free_vehicle_registers(vehicles, num_of_vehicles);

    if (!has_matches)
        return NOT_FOUND;

    return SUCCESS;
}
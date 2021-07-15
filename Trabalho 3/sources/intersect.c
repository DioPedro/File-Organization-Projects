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

// int main() {
//     FILE *vehicle = fopen("veiculo1.bin", "rb");
//     FILE *route = fopen("linha1.bin", "rb");

//     char fn[50] = "indiceLinha1.bin";
//     btree *new_tree = load_btree(fn);
//     optimized_intersection(vehicle, route, new_tree);
//     // brute_intersection(vehicle, route);

//     destroy_btree(new_tree);
//     fclose(vehicle);
//     fclose(route);

//     return 0;
// }
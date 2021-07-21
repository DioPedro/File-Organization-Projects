#ifndef INTERSECT_H
#define INTERSECT_H

#include <stdio.h>
#include <stdlib.h>
#include "vehicle.h"
#include "route.h"
#include "btree.h"

#define SUCCESS 0
#define FILE_FAILURE 1
#define NOT_FOUND 2

int brute_intersection(FILE *vehicle_bin, FILE *route_bin);
int optimized_intersection(FILE *vehicle_bin, FILE *route_bin, btree *index_file);
int write_sorted_vehicle_file(FILE *bin_fp, char *new_file_name);
int write_sorted_route_file(FILE *bin_fp, char *new_file_name);
int merge_files(FILE *vehicle_fp, FILE *route_fp);

#endif
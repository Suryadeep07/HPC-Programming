#ifndef UTILS_H
#define UTILS_H
#include <time.h>
#include "init.h"

// PIC operations
void interpolation_and_mover(double *mesh_value, Points *local_points, int local_num_points, double *phase_times);
void save_mesh(double *mesh_value);

#endif

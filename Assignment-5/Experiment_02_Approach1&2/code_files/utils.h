#ifndef UTILS_H
#define UTILS_H

#include "init.h"

void interpolation(double *mesh_value, Points *points);

void mover_immediate_serial(Points *points, double deltaX, double deltaY);
void mover_immediate_parallel(Points *points, double deltaX, double deltaY);

void mover_deferred_serial(Points *points, double deltaX, double deltaY);
void mover_deferred_parallel(Points *points, double deltaX, double deltaY);

void mover_no_insert_parallel(Points *points, double deltaX, double deltaY);

#endif

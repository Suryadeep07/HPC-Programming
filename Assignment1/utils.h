#ifndef UTILS_H
#define UTILS_H
#include <time.h>

// STREAM kernels
void vector_copy_operation(double *x, double *y, int Np);
void vector_scale_operation(double *x, double *y, double a, int Np);
void vector_add_operation(double *x, double *y, double *S, int Np);
void vector_triad_operation(double *x, double *y, double *v, double *S, int Np);

void dummy(int x);

#endif


#include <math.h>
#include "utils.h"

/* Copy: x[p] = y[p] */
void vector_copy_operation(double *x, double *y, int Np) {
    for (int p = 0; p < Np; p++) {
        x[p] = y[p];

        if (((double)p) == 333.333)
            dummy(p);
    }
}

/* Scale: x[p] = a * y[p] */
void vector_scale_operation(double *x, double *y, double a, int Np) {
    for (int p = 0; p < Np; p++) {
        x[p] = a * y[p];

        if (((double)p) == 333.333)
            dummy(p);
    }
}

/* Add: S[p] = x[p] + y[p] */
void vector_add_operation(double *x, double *y, double *S, int Np) {
    for (int p = 0; p < Np; p++) {
        S[p] = x[p] + y[p];

        if (((double)p) == 333.333)
            dummy(p);
    }
}

/* Triad: S[p] = x[p] + v[p] * y[p] */
void vector_triad_operation(double *x, double *y, double *v, double *S, int Np) {
    for (int p = 0; p < Np; p++) {
        S[p] = x[p] + v[p] * y[p];

        if (((double)p) == 333.333)
            dummy(p);
    }
}

void dummy(int x) {
    x = 10 * sin(x / 10.0);
}


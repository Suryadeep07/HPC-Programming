#ifndef INIT_H
#define INIT_H

typedef struct {
    double x, y;
} Points;

extern int GRID_X, GRID_Y, NX, NY;
extern int NUM_Points, Maxiter;
extern double dx, dy;

void initializepoints(Points *points);

#endif

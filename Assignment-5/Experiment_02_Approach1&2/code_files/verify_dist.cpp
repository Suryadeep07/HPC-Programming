#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "utils.h"

int GRID_X, GRID_Y, NX, NY, NUM_Points, Maxiter;
double dx, dy;

int main() {
    NX = 250; NY = 100;
    GRID_X = NX + 1; GRID_Y = NY + 1;
    dx = 1.0 / NX; dy = 1.0 / NY;
    NUM_Points = 100000; 
    Maxiter = 50;       
    
    Points *points = (Points *)malloc(NUM_Points * sizeof(Points));
    
    initializepoints(points);
    
    // Move particles multiple times to test the insertion/deletion distribution over time
    for(int i = 0; i < Maxiter; i++) {
        mover_immediate_serial(points, dx, dy); 
    }
    
    FILE *fp = fopen("particles_verification.csv", "w");
    fprintf(fp, "X,Y\n");
    for (int p = 0; p < NUM_Points; p++) {
        fprintf(fp, "%lf,%lf\n", points[p].x, points[p].y);
    }
    
    fclose(fp);
    free(points);
    
    printf("Verification data written to particles_verification.csv\n");
    return 0;
}
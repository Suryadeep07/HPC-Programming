#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "init.h"
#include "utils.h"

int GRID_X, GRID_Y, NX, NY, NUM_Points, Maxiter;
double dx, dy;

int main() {
    int nx_vals[] = {250, 500, 1000};
    int ny_vals[] = {100, 200, 400};
    int pts_vals[] = {100, 10000, 1000000, 100000000, 1000000000}; 
    int num_pts_tests = 5; 
    Maxiter = 10;
    
    FILE *fp = fopen("results_exp1.csv", "w");
    fprintf(fp, "Grid,Points,Imm_Interp,Imm_Mover,Imm_Total,Def_Interp,Def_Mover,Def_Total\n");

    for (int c = 0; c < 3; c++) {
        NX = nx_vals[c]; NY = ny_vals[c];
        GRID_X = NX + 1; GRID_Y = NY + 1;
        dx = 1.0 / NX; dy = 1.0 / NY;

        printf("\n--- Starting Grid %dx%d ---\n", NX, NY);

        for (int p = 0; p < num_pts_tests; p++) {
            NUM_Points = pts_vals[p];
            printf("Processing %d particles... ", NUM_Points);
            
            Points *points = (Points *)malloc(NUM_Points * sizeof(Points));
            double *mesh_value = (double *)malloc(GRID_X * GRID_Y * sizeof(double));

            if (!points || !mesh_value) {
                printf("Memory Allocation Failed! Skipping.\n");
                if(points) free(points);
                if(mesh_value) free(mesh_value);
                continue;
            }

            // Immediate 
            initializepoints(points);
            double imm_interp = 0.0, imm_mover = 0.0;
            for (int iter = 0; iter < Maxiter; iter++) {
                double t0 = omp_get_wtime();
                interpolation(mesh_value, points);
                double t1 = omp_get_wtime();
                mover_immediate_serial(points, dx, dy);
                double t2 = omp_get_wtime();
                imm_interp += (t1 - t0); imm_mover += (t2 - t1);
            }
            double imm_total = imm_interp + imm_mover;

            // Deferred
            initializepoints(points);
            double def_interp = 0.0, def_mover = 0.0;
            for (int iter = 0; iter < Maxiter; iter++) {
                double t0 = omp_get_wtime();
                interpolation(mesh_value, points);
                double t1 = omp_get_wtime();
                mover_deferred_serial(points, dx, dy);
                double t2 = omp_get_wtime();
                def_interp += (t1 - t0); def_mover += (t2 - t1);
            }
            double def_total = def_interp + def_mover;

            fprintf(fp, "%dx%d,%d,%e,%e,%e,%e,%e,%e\n", 
                   NX, NY, NUM_Points, imm_interp, imm_mover, imm_total, def_interp, def_mover, def_total);
            printf("Done!\n");

            free(points); free(mesh_value);
        }
    }
    fclose(fp);
    return 0;
}

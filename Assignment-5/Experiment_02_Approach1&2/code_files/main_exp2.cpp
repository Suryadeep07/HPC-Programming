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
    int threads[] = {2, 4, 8, 16};
    int num_threads = 4;
    NUM_Points = 14000000; 
    Maxiter = 10;
    
    FILE *fp = fopen("results_exp2.csv", "w");
    fprintf(fp, "Grid,Threads,Base_Interp,Base_Mover,Base_Total,Imm_Interp,Imm_Mover,Imm_Total,Def_Interp,Def_Mover,Def_Total\n");

    for (int c = 0; c < 3; c++) {
        NX = nx_vals[c]; NY = ny_vals[c];
        GRID_X = NX + 1; GRID_Y = NY + 1;
        dx = 1.0 / NX; dy = 1.0 / NY;

        Points *points = (Points *)malloc(NUM_Points * sizeof(Points));
        double *mesh_value = (double *)malloc(GRID_X * GRID_Y * sizeof(double));

        printf("\n--- Parallel Scalability: Grid %dx%d ---\n", NX, NY);

        for (int t = 0; t < num_threads; t++) {
            omp_set_num_threads(threads[t]);
            printf("Threads %d... ", threads[t]);
            
            // Baseline
            initializepoints(points);
            double base_i = 0.0, base_m = 0.0;
            for(int i=0; i<Maxiter; i++) {
                double t0 = omp_get_wtime(); interpolation(mesh_value, points);
                double t1 = omp_get_wtime(); mover_no_insert_parallel(points, dx, dy);
                double t2 = omp_get_wtime(); base_i += (t1-t0); base_m += (t2-t1);
            }
            
            // Immediate
            initializepoints(points);
            double imm_i = 0.0, imm_m = 0.0;
            for(int i=0; i<Maxiter; i++) {
                double t0 = omp_get_wtime(); interpolation(mesh_value, points);
                double t1 = omp_get_wtime(); mover_immediate_parallel(points, dx, dy);
                double t2 = omp_get_wtime(); imm_i += (t1-t0); imm_m += (t2-t1);
            }

            // Deferred
            initializepoints(points);
            double def_i = 0.0, def_m = 0.0;
            for(int i=0; i<Maxiter; i++) {
                double t0 = omp_get_wtime(); interpolation(mesh_value, points);
                double t1 = omp_get_wtime(); mover_deferred_parallel(points, dx, dy);
                double t2 = omp_get_wtime(); def_i += (t1-t0); def_m += (t2-t1);
            }

            fprintf(fp, "%dx%d,%d,%e,%e,%e,%e,%e,%e,%e,%e,%e\n", 
                   NX, NY, threads[t], 
                   base_i, base_m, base_i+base_m,
                   imm_i, imm_m, imm_i+imm_m, 
                   def_i, def_m, def_i+def_m);
            printf("Done!\n");
        }
        free(points); free(mesh_value);
    }
    fclose(fp);
    return 0;
}

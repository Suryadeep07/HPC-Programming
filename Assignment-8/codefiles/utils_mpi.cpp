#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void interpolation_and_mover(double *mesh_value, Points *local_points, int local_num_points, double *phase_times) {
    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;
    int grid_size = GRID_X * GRID_Y;

    double t_start = MPI_Wtime(); // Start Phase 1 timer

    // 1. Clear process-local mesh
    memset(mesh_value, 0, grid_size * sizeof(double));

    int max_threads = omp_get_max_threads();
    double **thread_meshes = (double **)malloc(max_threads * sizeof(double *));

    // --- PHASE 1: INTERPOLATION (Point to Mesh) ---
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        thread_meshes[tid] = (double *)calloc(grid_size, sizeof(double));
        double *local_mesh = thread_meshes[tid];

        #pragma omp for schedule(static)
        for (int p = 0; p < local_num_points; p++) {
            double x = local_points[p].x;
            double y = local_points[p].y;

            if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) continue;

            int i = (int)(x * inv_dx);
            int j = (int)(y * inv_dy);
            i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
            j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

            double d_x = (x * inv_dx) - i;
            double d_y = (y * inv_dy) - j;
            double inv_d_x = 1.0 - d_x;
            double inv_d_y = 1.0 - d_y;

            double w00 = inv_d_x * inv_d_y * cell_area;
            double w10 = d_x * inv_d_y * cell_area;
            double w01 = inv_d_x * d_y * cell_area;
            double w11 = d_x * d_y * cell_area;

            int base_idx = j * GRID_X + i;
            local_mesh[base_idx] += w00; 
            local_mesh[base_idx + 1] += w10;
            local_mesh[base_idx + GRID_X] += w01;
            local_mesh[base_idx + GRID_X + 1] += w11;
        }
    }

    #pragma omp parallel for schedule(static)
    for (int k = 0; k < grid_size; k++) {
        double sum = 0.0;
        for (int t = 0; t < max_threads; t++) sum += thread_meshes[t][k];
        mesh_value[k] = sum;
    }

    for (int t = 0; t < max_threads; t++) free(thread_meshes[t]);
    free(thread_meshes);

    double t_interp = MPI_Wtime(); 
    phase_times[0] += (t_interp - t_start); // Record Phase 1 time

    // --- PHASE 2 & 3: MPI GLOBAL REDUCTION & NORMALIZATION ---
    MPI_Allreduce(MPI_IN_PLACE, mesh_value, grid_size, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    double global_min = mesh_value[0], global_max = mesh_value[0];
    #pragma omp parallel for reduction(min:global_min) reduction(max:global_max)
    for (int k = 0; k < grid_size; k++) {
        if (mesh_value[k] < global_min) global_min = mesh_value[k];
        if (mesh_value[k] > global_max) global_max = mesh_value[k];
    }
    
    double range = global_max - global_min;
    #pragma omp parallel for schedule(static)
    for (int k = 0; k < grid_size; k++) {
        if (range > 0) mesh_value[k] = 2.0 * (mesh_value[k] - global_min) / range - 1.0;
        else mesh_value[k] = 0.0;
    }

    double t_overhead = MPI_Wtime();
    phase_times[1] += (t_overhead - t_interp); // Record Phase 2/3 time

    // --- PHASE 4 & 5: MOVER (Mesh to Point) & DENORMALIZATION ---
    #pragma omp parallel for schedule(static)
    for (int p = 0; p < local_num_points; p++) {
        double x = local_points[p].x;
        double y = local_points[p].y;

        if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) continue;

        int i = (int)(x * inv_dx);
        int j = (int)(y * inv_dy);
        i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
        j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

        double d_x = (x * inv_dx) - i;
        double d_y = (y * inv_dy) - j;
        double inv_d_x = 1.0 - d_x;
        double inv_d_y = 1.0 - d_y;

        double w00 = inv_d_x * inv_d_y * cell_area;
        double w10 = d_x * inv_d_y * cell_area;
        double w01 = inv_d_x * d_y * cell_area;
        double w11 = d_x * d_y * cell_area;

        int base_idx = j * GRID_X + i;
        double F_i = w00 * mesh_value[base_idx] + w10 * mesh_value[base_idx + 1] + w01 * mesh_value[base_idx + GRID_X] + w11 * mesh_value[base_idx + GRID_X + 1];

        local_points[p].x += F_i * dx;
        local_points[p].y += F_i * dy;
    }

    #pragma omp parallel for schedule(static)
    for (int k = 0; k < grid_size; k++) {
        if (range > 0) mesh_value[k] = (mesh_value[k] + 1.0) * range / 2.0 + global_min;
    }

    double t_mover = MPI_Wtime();
    phase_times[2] += (t_mover - t_overhead); // Record Phase 4/5 time
}

// Write mesh to file
void save_mesh(double *mesh_value) {
    FILE *fd = fopen("Mesh.out", "w");
    if (!fd) {
        printf("Error creating Mesh.out\n");
        exit(1);
    }

    for (int i = 0; i < GRID_Y; i++) {
        for (int j = 0; j < GRID_X; j++) {
            fprintf(fd, "%lf ", mesh_value[i * GRID_X + j]);
        }
        fprintf(fd, "\n");
    }
    fclose(fd);
}

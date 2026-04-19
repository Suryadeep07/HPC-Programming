#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include "utils.h"

double min_val, max_val;

// Interpolation (Scatter: Point -> Mesh)
void interpolation(double *mesh_value, Points *points) {
    int grid_size = GRID_X * GRID_Y;
    memset(mesh_value, 0, grid_size * sizeof(double));

    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    // Determine the maximum number of threads OpenMP might use
    int max_threads = omp_get_max_threads();
    
    // Allocate a massive 1D array to hold a private grid for EVERY thread.
    // This entirely removes the need for atomic locks during the scatter phase.
    double *local_meshes = (double*)calloc(max_threads * grid_size, sizeof(double));

    #pragma omp parallel 
    {
        int tid = omp_get_thread_num();
        // Point this thread to its own private section of the massive array
        double *my_grid = &local_meshes[tid * grid_size];

        // Because 'points' is now a Structure of Arrays, the CPU can vectorize this loop!
        #pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++) {
            if (points->is_void[p]) continue; 

            double x = points->x[p];
            double y = points->y[p];

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

            // Look! No #pragma omp atomic locks here. Pure, unthrottled memory writes.
            my_grid[base_idx] += w00;
            my_grid[base_idx + 1] += w10;
            my_grid[base_idx + GRID_X] += w01;
            my_grid[base_idx + GRID_X + 1] += w11;
        }
    } // Implicit barrier ensures all threads finish scattering

    // Fast Parallel Reduction: Merge all the private grids into the main global grid
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < grid_size; i++) {
        double sum = 0.0;
        for (int t = 0; t < max_threads; t++) {
            sum += local_meshes[t * grid_size + i];
        }
        mesh_value[i] += sum;
    }

    // Free the massive array
    free(local_meshes);
}

void normalization(double *mesh_value) {
    int grid_size = GRID_X * GRID_Y;
    min_val = mesh_value[0];
    max_val = mesh_value[0];
    
    #pragma omp parallel
    {
        double local_min = mesh_value[0];
        double local_max = mesh_value[0];
        
        #pragma omp for schedule(static)
        for (int i = 0; i < grid_size; i++) {
            if (mesh_value[i] < local_min) local_min = mesh_value[i];
            if (mesh_value[i] > local_max) local_max = mesh_value[i];
        }
        
        #pragma omp critical
        {
            if (local_min < min_val) min_val = local_min;
            if (local_max > max_val) max_val = local_max;
        }
    }

    double range = max_val - min_val;
    if (range == 0.0) range = 1.0; 

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < grid_size; i++) {
        mesh_value[i] = 2.0 * (mesh_value[i] - min_val) / range - 1.0;
    }
}

// Mover (Gather: Mesh -> Point)
void mover(double *mesh_value, Points *points) {
    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    #pragma omp parallel for schedule(static)
    for (int p = 0; p < NUM_Points; p++) {
        if (points->is_void[p]) continue;

        double x = points->x[p];
        double y = points->y[p];

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

        double F_i = w00 * mesh_value[base_idx] +
                     w10 * mesh_value[base_idx + 1] +
                     w01 * mesh_value[base_idx + GRID_X] +
                     w11 * mesh_value[base_idx + GRID_X + 1];

        // Update the separated arrays (SoA)
        points->x[p] += F_i * dx;
        points->y[p] += F_i * dy;

        if (points->x[p] < 0.0 || points->x[p] > 1.0 || 
            points->y[p] < 0.0 || points->y[p] > 1.0) {
            points->is_void[p] = true;
        }
    }
}

void denormalization(double *mesh_value) {
    int grid_size = GRID_X * GRID_Y;
    double range = max_val - min_val;
    if (range == 0.0) range = 1.0;

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < grid_size; i++) {
        mesh_value[i] = (mesh_value[i] + 1.0) * range / 2.0 + min_val;
    }
}

long long int void_count(Points *points) {
    long long int voids = 0;
    #pragma omp parallel for reduction(+:voids)
    for (int i = 0; i < NUM_Points; i++) {
        voids += (int)points->is_void[i];
    }
    return voids;
}

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

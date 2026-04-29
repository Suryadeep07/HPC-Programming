#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "utils.h"

// High-resolution timer for serial code (since we can't use MPI_Wtime here)
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

void interpolation_and_mover_serial(double *mesh_value, Points *points, double *phase_times) {
    double inv_dx = (double)NX;
    double inv_dy = (double)NY;
    double cell_area = dx * dy;
    int grid_size = GRID_X * GRID_Y;

    double t_start = get_time();

    // 1. Clear mesh
    memset(mesh_value, 0, grid_size * sizeof(double));

    // --- PHASE 1: INTERPOLATION ---
    for (int p = 0; p < NUM_Points; p++) {
        double x = points[p].x;
        double y = points[p].y;

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
        mesh_value[base_idx] += w00;
        mesh_value[base_idx + 1] += w10;
        mesh_value[base_idx + GRID_X] += w01;
        mesh_value[base_idx + GRID_X + 1] += w11;
    }

    double t_interp = get_time();
    phase_times[0] += (t_interp - t_start);

    // --- PHASE 2/3: NORMALIZATION ---
    double global_min = mesh_value[0], global_max = mesh_value[0];
    for (int k = 1; k < grid_size; k++) {
        if (mesh_value[k] < global_min) global_min = mesh_value[k];
        if (mesh_value[k] > global_max) global_max = mesh_value[k];
    }
    
    double range = global_max - global_min;
    for (int k = 0; k < grid_size; k++) {
        if (range > 0) mesh_value[k] = 2.0 * (mesh_value[k] - global_min) / range - 1.0;
        else mesh_value[k] = 0.0;
    }

    double t_overhead = get_time();
    phase_times[1] += (t_overhead - t_interp);

    // --- PHASE 4/5: MOVER & DENORMALIZATION ---
    for (int p = 0; p < NUM_Points; p++) {
        double x = points[p].x;
        double y = points[p].y;

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

        points[p].x += F_i * dx;
        points[p].y += F_i * dy;
    }

    for (int k = 0; k < grid_size; k++) {
        if (range > 0) mesh_value[k] = (mesh_value[k] + 1.0) * range / 2.0 + global_min;
    }

    double t_mover = get_time();
    phase_times[2] += (t_mover - t_overhead);
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

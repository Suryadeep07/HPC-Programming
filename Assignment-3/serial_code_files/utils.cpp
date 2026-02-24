#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

// Serial interpolation 
void interpolation(double *mesh_value, Points *points) {
    // Pre-calculate inverse spacing to change slow division into fast multiplication
    // Since dx = 1.0/NX, then 1/dx is just NX.
    double inv_dx = (double)NX; 
    double inv_dy = (double)NY;
    double cell_area = dx * dy;

    // Loop through each scattered point in a straight line 
    // This keeps the CPU memory reader moving fast without jumping around
    for (int p = 0; p < NUM_Points; p++) {
        double x = points[p].x;
        double y = points[p].y;

        // 1. Identify the nearest structured grid points using fast multiplication
        int i = (int)(x * inv_dx);
        int j = (int)(y * inv_dy);

        // Keep points inside the boundary using quick one-liners instead of "if" statements
        // This stops the CPU from having to stop and guess
        i = (i >= NX) ? NX - 1 : ((i < 0) ? 0 : i);
        j = (j >= NY) ? NY - 1 : ((j < 0) ? 0 : j);

        // 2. Compute the relative distances (no slow operation like division is used)
        double d_x = (x * inv_dx) - i;
        double d_y = (y * inv_dy) - j;
        
        // Save these subtractions so we don't have to do them twice
        double inv_d_x = 1.0 - d_x;
        double inv_d_y = 1.0 - d_y;

        // 3. Compute the weights scaled by the area of the cell
        double w00 = inv_d_x * inv_d_y * cell_area;
        double w10 = d_x * inv_d_y * cell_area;
        double w01 = inv_d_x * d_y * cell_area;
        double w11 = d_x * d_y * cell_area;

        // 4. Distribute the function value
        // Calculate the grid spot just once to save time
        int base_idx = j * GRID_X + i;

        mesh_value[base_idx] += w00;
        mesh_value[base_idx + 1] += w10;
        mesh_value[base_idx + GRID_X] += w01;
        mesh_value[base_idx + GRID_X + 1] += w11;
    }
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "utils.h"

// Interpolation
void interpolation(double *mesh_value, Points *points) {
    memset(mesh_value, 0, GRID_X * GRID_Y * sizeof(double));

    for (int p = 0; p < NUM_Points; p++) {
        double xp = points[p].x / dx;
        double yp = points[p].y / dy;
        
        int i = (int)xp;
        int j = (int)yp;
        
        if (i >= NX) i = NX - 1;
        if (j >= NY) j = NY - 1;
        
        double wx = xp - i;
        double wy = yp - j;
        
        mesh_value[j * GRID_X + i]           += (1.0 - wx) * (1.0 - wy);
        mesh_value[j * GRID_X + i + 1]       += wx * (1.0 - wy);
        mesh_value[(j + 1) * GRID_X + i]     += (1.0 - wx) * wy;
        mesh_value[(j + 1) * GRID_X + i + 1] += wx * wy;
    }
}

// IMMEDIATE REPLACEMENT
void mover_immediate_serial(Points *points, double deltaX, double deltaY) {
    unsigned int seed = 12345;
    for (int p = 0; p < NUM_Points; p++) {
        double rx = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double ry = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double nx = points[p].x + rx * deltaX;
        double ny = points[p].y + ry * deltaY;
        
        if (nx < 0.0 || nx > 1.0 || ny < 0.0 || ny > 1.0) {
            points[p].x = (double)rand_r(&seed) / RAND_MAX;
            points[p].y = (double)rand_r(&seed) / RAND_MAX;
        } else {
            points[p].x = nx;
            points[p].y = ny;
        }
    }
}

void mover_immediate_parallel(Points *points, double deltaX, double deltaY) {
    #pragma omp parallel
    {
        unsigned int seed = 12345 + omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++) {
            double rx = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
            double ry = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
            double nx = points[p].x + rx * deltaX;
            double ny = points[p].y + ry * deltaY;
            
            if (nx < 0.0 || nx > 1.0 || ny < 0.0 || ny > 1.0) {
                points[p].x = (double)rand_r(&seed) / RAND_MAX;
                points[p].y = (double)rand_r(&seed) / RAND_MAX;
            } else {
                points[p].x = nx;
                points[p].y = ny;
            }
        }
    }
}

// DEFERRED INSERTION
void mover_deferred_serial(Points *points, double deltaX, double deltaY) {
    unsigned int seed = 12345;
    int deleted_count = 0;

    for (int p = 0; p < NUM_Points; p++) {
        double rx = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double ry = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double nx = points[p].x + rx * deltaX;
        double ny = points[p].y + ry * deltaY;
        
        if (nx < 0.0 || nx > 1.0 || ny < 0.0 || ny > 1.0) {
            points[p].x = -1.0; 
            deleted_count++;
        } else {
            points[p].x = nx;
            points[p].y = ny;
        }
    }

    int left = 0, right = NUM_Points - 1;
    while (left < right) {
        if (points[left].x != -1.0) { left++; continue; }
        if (points[right].x == -1.0) { right--; continue; }
        points[left] = points[right];
        points[right].x = -1.0;
        left++; right--;
    }

    int valid_boundary = NUM_Points - deleted_count;
    for (int p = valid_boundary; p < NUM_Points; p++) {
        points[p].x = (double)rand_r(&seed) / RAND_MAX;
        points[p].y = (double)rand_r(&seed) / RAND_MAX;
    }
}

void mover_deferred_parallel(Points *points, double deltaX, double deltaY) {
    int deleted_count = 0;

    #pragma omp parallel
    {
        unsigned int local_seed = 12345 + omp_get_thread_num();
        #pragma omp for reduction(+:deleted_count) schedule(static)
        for (int p = 0; p < NUM_Points; p++) {
            double rx = ((double)rand_r(&local_seed) / RAND_MAX) * 2.0 - 1.0;
            double ry = ((double)rand_r(&local_seed) / RAND_MAX) * 2.0 - 1.0;
            double nx = points[p].x + rx * deltaX;
            double ny = points[p].y + ry * deltaY;
            
            if (nx < 0.0 || nx > 1.0 || ny < 0.0 || ny > 1.0) {
                points[p].x = -1.0; 
                deleted_count++;
            } else {
                points[p].x = nx;
                points[p].y = ny;
            }
        }
    }

    // Serial compaction avoids massive overhead and race conditions
    int left = 0, right = NUM_Points - 1;
    while (left < right) {
        if (points[left].x != -1.0) { left++; continue; }
        if (points[right].x == -1.0) { right--; continue; }
        points[left] = points[right];
        points[right].x = -1.0;
        left++; right--;
    }

    #pragma omp parallel
    {
        unsigned int local_seed = 9999 + omp_get_thread_num();
        int valid_boundary = NUM_Points - deleted_count;
        #pragma omp for schedule(static)
        for (int p = valid_boundary; p < NUM_Points; p++) {
            points[p].x = (double)rand_r(&local_seed) / RAND_MAX;
            points[p].y = (double)rand_r(&local_seed) / RAND_MAX;
        }
    }
}

// BASELINE (From Assignment 04 - No Insertion/Deletion)
void mover_no_insert_parallel(Points *points, double deltaX, double deltaY) {
    #pragma omp parallel
    {
        unsigned int seed = 12345 + omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int p = 0; p < NUM_Points; p++) {
            double new_x, new_y;
            do {
                double rx = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
                double ry = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
                new_x = points[p].x + rx * deltaX;
                new_y = points[p].y + ry * deltaY;
            } while (new_x < 0.0 || new_x > 1.0 || new_y < 0.0 || new_y > 1.0);
            
            points[p].x = new_x;
            points[p].y = new_y;
        }
    }
}
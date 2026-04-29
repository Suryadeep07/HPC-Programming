#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "init.h"
#include "utils.h"

int GRID_X, GRID_Y, NX, NY;
int NUM_Points, Maxiter;
double dx, dy;

void interpolation_and_mover_serial(double *mesh_value, Points *points, double *phase_times);

double get_time_main() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char **argv) {
    if (argc != 2) return 1;

    FILE *file = fopen(argv[1], "rb");
    if (!file) exit(1);

    fread(&NX, sizeof(int), 1, file);
    fread(&NY, sizeof(int), 1, file);
    fread(&NUM_Points, sizeof(int), 1, file);
    fread(&Maxiter, sizeof(int), 1, file);

    GRID_X = NX + 1;
    GRID_Y = NY + 1;
    dx = 1.0 / NX;
    dy = 1.0 / NY;

    double *mesh_value = (double *)calloc(GRID_X * GRID_Y, sizeof(double));
    Points *points = (Points *)calloc(NUM_Points, sizeof(Points));

    // Read points ONCE before the loop
    read_points(file, points);
    fclose(file);

    double total_time = 0.0;
    double phase_times[3] = {0.0, 0.0, 0.0};

    for (int iter = 0; iter < Maxiter; iter++) {
        double start = get_time_main();
        interpolation_and_mover_serial(mesh_value, points, phase_times);
        double end = get_time_main();
        total_time += (end - start);
    }

    // Print with identical formatting to the MPI version
    printf("Interpolation_Time = %lf\n", phase_times[0]);
    printf("Overhead_Time = %lf\n", phase_times[1]);
    printf("Mover_Time = %lf\n", phase_times[2]);
    printf("Total_Time = %lf\n", total_time);

    save_mesh(mesh_value);

    free(mesh_value);
    free(points);
    return 0;
}

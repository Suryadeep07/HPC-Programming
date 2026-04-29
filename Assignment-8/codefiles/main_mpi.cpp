#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "init.h"
#include "utils.h"

int GRID_X, GRID_Y, NX, NY;
int NUM_Points, Maxiter;
double dx, dy;

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) printf("Usage: %s <input_file>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    FILE *file = NULL;
    Points *global_points = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        file = fopen(argv[1], "rb");
        if (!file) {
            printf("Error opening input file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fread(&NX, sizeof(int), 1, file);
        fread(&NY, sizeof(int), 1, file);
        fread(&NUM_Points, sizeof(int), 1, file);
        fread(&Maxiter, sizeof(int), 1, file);
    }

    // Broadcast parameters to all processes
    MPI_Bcast(&NX, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&NY, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&NUM_Points, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Maxiter, 1, MPI_INT, 0, MPI_COMM_WORLD);

    GRID_X = NX + 1;
    GRID_Y = NY + 1;
    dx = 1.0 / NX;
    dy = 1.0 / NY;

    // Calculate load balance for particle decomposition
    int local_num_points = NUM_Points / size + ((rank < NUM_Points % size) ? 1 : 0);
    Points *local_points = (Points *)calloc(local_num_points, sizeof(Points));
    double *mesh_value = (double *)calloc(GRID_X * GRID_Y, sizeof(double));

    if (rank == 0) {
        global_points = (Points *)calloc(NUM_Points, sizeof(Points));
        read_points(file, global_points); // Read initial particles
        fclose(file);

        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        int offset = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = (NUM_Points / size + ((i < NUM_Points % size) ? 1 : 0)) * 2; // 2 doubles per Point
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }

    // Scatter particles as contiguous arrays of doubles
    MPI_Scatterv(global_points, sendcounts, displs, MPI_DOUBLE, 
                 local_points, local_num_points * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double total_time = 0.0;
    double phase_times[3] = {0.0, 0.0, 0.0}; // [0]=Interp, [1]=Overhead, [2]=Mover

    for (int iter = 0; iter < Maxiter; iter++) {
        double start_time = MPI_Wtime();

        // Pass the timing array into the function
        interpolation_and_mover(mesh_value, local_points, local_num_points, phase_times);

        double end_time = MPI_Wtime();
        total_time += (end_time - start_time);
    }

    if (rank == 0) {
        save_mesh(mesh_value);
        printf("Interpolation_Time = %lf\n", phase_times[0]);
        printf("Overhead_Time = %lf\n", phase_times[1]);
        printf("Mover_Time = %lf\n", phase_times[2]);
        printf("Total_Time = %lf\n", total_time);
        
        free(global_points);
        free(sendcounts);
        free(displs);
    }
    
    free(mesh_value);
    free(local_points);
    MPI_Finalize();
    return 0;
}

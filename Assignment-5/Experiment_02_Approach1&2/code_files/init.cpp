#include <stdlib.h>
#include "init.h"

void initializepoints(Points *points) {
    unsigned int seed = 42; 
    for (int i = 0; i < NUM_Points; i++) {
        points[i].x = (double)rand_r(&seed) / RAND_MAX;
        points[i].y = (double)rand_r(&seed) / RAND_MAX;
    }
}

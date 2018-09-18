/* Main solver routines for heat equation solver */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "heat.h"

/* Exchange the boundary values */
void exchange(field *temperature, parallel_data *parallel)
{

  /* send to the neighbor up, receive from the neighbor down */
  if (parallel->rank % 2) {
    MPI_Send(temperature->data[1], temperature->ny+2, MPI_DOUBLE,
	     parallel->nup, 11, MPI_COMM_WORLD);
    MPI_Recv(temperature->data[temperature->nx+1], temperature->ny+2,
	     MPI_DOUBLE, parallel->ndown, 11, MPI_COMM_WORLD,
	     MPI_STATUS_IGNORE);
  } else {
    MPI_Recv(temperature->data[temperature->nx+1], temperature->ny+2, 
             MPI_DOUBLE, parallel->ndown, 11, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Send(temperature->data[1], temperature->ny+2, MPI_DOUBLE,
             parallel->nup, 11, MPI_COMM_WORLD); 
  }

  /* send to the neighbor down, receive from the neighbor down */
  if (parallel->rank % 2) {
    MPI_Send(temperature->data[temperature->nx], temperature->ny+2, MPI_DOUBLE,
             parallel->ndown, 12, MPI_COMM_WORLD);
    MPI_Recv(temperature->data[0], temperature->ny+2,
             MPI_DOUBLE, parallel->nup, 12, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  } else { 
    MPI_Recv(temperature->data[0], temperature->ny+2,
             MPI_DOUBLE, parallel->nup, 12, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE); 
    MPI_Send(temperature->data[temperature->nx], temperature->ny+2, MPI_DOUBLE,
             parallel->ndown, 12, MPI_COMM_WORLD);
  }       
}


/* Update the temperature values using five-point stencil */

void evolve(field *curr, field *prev, double a, double dt)
{
#pragma _CRI inline_never
    
    int i, j;
    double dx2, dy2;

    /* Determine the temperature field at next time step
     * As we have fixed boundary conditions, the outermost gridpoints
     * are not updated. */
    dx2 = prev->dx * prev->dx;
    dy2 = prev->dy * prev->dy;
    for (i = 1; i < curr->nx + 1; i++) {
        for (j = 1; j < curr->ny + 1; j++) {
            curr->data[i][j] = prev->data[i][j] + a * dt *
                               ((prev->data[i + 1][j] -
                                 2.0 * prev->data[i][j] +
                                 prev->data[i - 1][j]) / dx2 +
                                (prev->data[i][j + 1] -
                                 2.0 * prev->data[i][j] +
                                 prev->data[i][j - 1]) / dy2);
        }
    }
}



#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>

#define N 4 /* number of rows and columns in matrix */
MPI_Status status;
double a[N][N],b[N][N],c[N][N];

int main(int argc, char** argv)
{
	int numtasks,taskid,numworkers,source,dest,rows,offset,i,j,k;
	clock_t start, stop;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	numworkers = numtasks-1;
/*---------------------------- master ----------------------------*/
	if (taskid == 0) {
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++) {
				a[i][j]= 1.0;
				b[i][j]= 2.0;
			}
		}
		start = clock();
/* send matrix data to the worker tasks */
		rows = N/numworkers;
		offset = 0;
		for (dest=1; dest<=numworkers; dest++){
			MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			MPI_Send(&a[offset][0], rows*N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
			MPI_Send(&b, N*N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
			offset = offset + rows;
		}
/* wait for results from all worker tasks */
		for (i=1; i<=numworkers; i++){
			source = i;
			MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
			MPI_Recv(&c[offset][0], rows*N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
		}
		stop = clock();
		printf("Here is the result matrix:\n");
		for (i=0; i<N; i++) {
			for (j=0; j<N; j++)
				printf("%6.2f ", c[i][j]);
			printf ("\n");
		}
		double time = (double)(stop-start)/(double)CLOCKS_PER_SEC;
		printf("Time = %.6f\n\n",time);
	}
/*---------------------------- worker----------------------------*/
	if (taskid > 0) {
		source = 0;
		MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&a, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&b, N*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
/* Matrix multiplication */
		for (k=0; k<N; k++) {
			for (i=0; i<rows; i++) {
				c[i][k] = 0.0;
				for (j=0; j<N; j++) {
					c[i][k] = c[i][k] + a[i][j] * b[j][k];
				}
			}
		}
		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&c, rows*N, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
	}
	MPI_Finalize();
}
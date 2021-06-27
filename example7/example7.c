#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<math.h>
#include <mpi.h>
#define length 8//0//0000

void swap(int* data, int i, int j) {
	int temp = data[i];
	data[i] = data[j];
	data[j] = temp;
}

int partition(int* data, int start, int end) {
	if (start >= end)
		return 0;
	int pivotValue = data[start];
	int low = start;
	int high = end - 1;
	while (low < high) {
		while (data[low] <= pivotValue && low < end) low++;
		while (data[high] > pivotValue && high > start) high--;
		if (low < high) swap(data, low, high);
	}
	swap(data, start, high);
	return high;
}

void quicksort(int* data, int start, int end) {
	if (end - start + 1 < 2) 
		return;
	int pivot = partition(data, start, end);
	quicksort(data, start, pivot);
	quicksort(data, pivot + 1, end);
}

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	printf("%d\n", size);//Display the current number of processes
	int* data = (int*)malloc(sizeof(int) * length);
	int* shou = (int*)malloc(sizeof(int) * 2);
	int i;
	if (rank == 0) {
		for (i = 0; i < length; i++)
			data[i] = i; //Assign values to data segments in different processes
	}
	int j;
	for (int j = 1; j < sqrt(length); j++){
		MPI_Scatter(data, length / size*j, MPI_INT, shou, length / size*j, MPI_INT, 0, MPI_COMM_WORLD);
		quicksort(shou, 0, length / size);//Quick sorting of data in different processes
		MPI_Gather(shou, length / size * j, MPI_INT, data, length / size * j, MPI_INT, 0, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0) {
		for (int i = 0; i < 10; i++) {
			printf("%d ", data[i]);//Print out the sorting results
		}
		printf("\n");
	}
	MPI_Finalize();
	return 0;
}

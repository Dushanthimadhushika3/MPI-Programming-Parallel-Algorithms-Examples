#define NUM 4
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>

void mergesort(int array[], int left, int right);
void merge(int array[], int left, int middle, int right);
int * mergeArrays(int a[], int b[], int n, int m);
void p2a(int a[], int * b, int size);
void l2g(int a[], int b[], int size);

int main(int argc, char * argv[]) {
  int i, a_size = NUM, local_size;
  int numtasks, rank, dest, source, rc, count, tag = 1, j;
  int a[NUM];
  int global[NUM];
  int * comp;
  MPI_Status Stat;
  MPI_Request req;
  MPI_Init( & argc, & argv);
  MPI_Comm_size(MPI_COMM_WORLD, & numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, & rank); //Local array for every process
  int local[(NUM / numtasks)];
  srand(time(NULL));
  //Setup array with random numbers
  for (i = 0; i < NUM; i++)
    a[i] = rand() % 100000;
  //Scatter and split array evenly for each process
  MPI_Scatter(a, NUM / numtasks, MPI_INT, local,
    NUM / numtasks, MPI_INT, 0, MPI_COMM_WORLD);
  local_size = NUM / numtasks;
  if (rank == 0) { //Parent Process
    clock_t begin, end;
    double time_spent;
    begin = clock();
    //Sequential merge sort
    mergesort(local, 0, local_size - 1);
    //Push sorted local array to global array
    l2g(global, local, local_size);
    int j, recv_size = local_size;
    int buff[recv_size];
    for (j = 0; j < numtasks - 1; j++) {
      //Receive sorted array from child process
      MPI_Recv(buff, recv_size, MPI_INT, MPI_ANY_SOURCE, 0,
        MPI_COMM_WORLD, & Stat);
      //Merge received array and global array together
      comp = mergeArrays(global, buff,
        local_size, recv_size);
      local_size += recv_size;
      //Pointer to Array
      p2a(global, comp, local_size);
    }
    end = clock();
    time_spent = (double)(end - begin)/ CLOCKS_PER_SEC;
    int k;
    //for (k=0; k<local_size;k++)
    //printf("%d\n", comp[k]);
    printf("Time spent (Parallel) : %f\n",
      time_spent);
    begin = clock();
    //Time sequential merge sort on same set of numbers
    mergesort(a, 0, NUM - 1);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time spent (Non-Parallel): %f\n",
      time_spent);
  } else { //Child process //Sequential mergesort the given array from scatter
    mergesort(local, 0, local_size - 1);
    //send the sorted array to the parent process
    MPI_Send(local, local_size, MPI_INT, 0, 0,
      MPI_COMM_WORLD);
  }
  MPI_Finalize();
}


//merge sort
void mergesort(int array[], int left, int right) {
  if (left < right) {
    int middle = (left + right) / 2;
    mergesort(array, left, middle);
    mergesort(array, middle + 1, right);
    merge(array, left, middle, right);
  }
}

//merge
void merge(int array[], int left, int middle, int right) {
  int temp[NUM];
  int i = left, j = middle + 1, k = 0;
  while (i <= middle && j <= right) {
    if (array[i] <= array[j])
      temp[k++] = array[i++];
    else
      temp[k++] = array[j++];
  }
  while (i <= middle)
    temp[k++] = array[i++];
  while (j <= right)
    temp[k++] = array[j++];
  k--;
  while (k >= 0) {
    array[left + k] = temp[k];
    k--;
  }
}

// merge arrays
int * mergeArrays(int a[], int b[], int n, int m) {
  int * c;
  int size = n + m;
  c = malloc(size * sizeof(int));
  int i = 0, j = 0, k = 0;
  while (i <= n - 1 && j <= m - 1) {
    if (a[i] <= b[j]) {
      c[k++] = a[i++];
    } else {
      c[k++] = b[j++];
    }
  }
  while (i <= n - 1) {
    c[k++] = a[i++];
  }
  while (j <= m - 1) {
    c[k++] = b[j++];
  }
  return c;
}

// helper - pointer to array
void p2a(int a[], int * b, int size) {
  int i;
  for (i = 0; i < size; i++) {
    a[i] = b[i];
  }
}

// helper - local to global
void l2g(int a[], int b[], int size) {
  int i;
  for (i = 0; i < size; i++)
    a[i] = b[i];
}

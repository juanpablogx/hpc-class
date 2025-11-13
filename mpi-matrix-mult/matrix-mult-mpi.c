#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <mpi.h>

// Generar una matriz cuadrada NxN con enteros aleatorios
void generate_matrix(int32_t *matrix, int n)
{
  for (int i = 0; i < n * n; i++)
  {
    matrix[i] = rand() % 100;
  }
}

// Multiplicar matrices: C_local = A_local * B
void multiply_matrices(int32_t *A_local, int32_t *B, int32_t *C_local, 
                       int rows_local, int n)
{
  for (int i = 0; i < rows_local; i++)
  {
    for (int j = 0; j < n; j++)
    {
      int32_t sum = 0;
      for (int k = 0; k < n; k++)
      {
        sum += A_local[i * n + k] * B[k * n + j];
      }
      C_local[i * n + j] = sum;
    }
  }
}

int main(int argc, char *argv[])
{
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (argc != 2)
  {
    if (rank == 0)
      printf("Uso: mpirun -np <procesos> %s <tamano_matriz>\n", argv[0]);
    MPI_Finalize();
    return 1;
  }

  int n = atoi(argv[1]);
  if (n <= 0)
  {
    if (rank == 0)
      printf("El tamaño debe ser positivo\n");
    MPI_Finalize();
    return 1;
  }

  // Calcular filas por proceso de forma balanceada
  int base_rows = n / size;
  int extra_rows = n % size;
  
  // Los primeros 'extra_rows' procesos reciben una fila adicional
  int rows_local = (rank < extra_rows) ? base_rows + 1 : base_rows;
  
  // Calcular desplazamientos para cada proceso
  int *sendcounts = NULL;
  int *displs = NULL;
  
  if (rank == 0)
  {
    sendcounts = (int *)malloc(size * sizeof(int));
    displs = (int *)malloc(size * sizeof(int));
    
    int offset = 0;
    for (int i = 0; i < size; i++)
    {
      int rows = (i < extra_rows) ? base_rows + 1 : base_rows;
      sendcounts[i] = rows * n;
      displs[i] = offset;
      offset += rows * n;
    }
  }
  
  // Matrices globales (solo el proceso 0 las necesita completas)
  int32_t *A = NULL;
  int32_t *B = NULL;
  int32_t *C = NULL;
  
  if (rank == 0)
  {
    srand(time(NULL));
    A = (int32_t *)malloc(n * n * sizeof(int32_t));
    B = (int32_t *)malloc(n * n * sizeof(int32_t));
    C = (int32_t *)malloc(n * n * sizeof(int32_t));
    
    if (!A || !B || !C)
    {
      printf("Error al asignar memoria\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    generate_matrix(A, n);
    generate_matrix(B, n);
  }
  else
  {
    // Los demás procesos solo necesitan B completa
    B = (int32_t *)malloc(n * n * sizeof(int32_t));
    if (!B)
    {
      printf("Error al asignar memoria en proceso %d\n", rank);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // Matrices locales para cada proceso
  int32_t *A_local = (int32_t *)malloc(rows_local * n * sizeof(int32_t));
  int32_t *C_local = (int32_t *)malloc(rows_local * n * sizeof(int32_t));
  
  if (!A_local || !C_local)
  {
    printf("Error al asignar memoria local en proceso %d\n", rank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  double start_time = MPI_Wtime();

  // Distribuir filas de A entre los procesos con Scatterv
  MPI_Scatterv(A, sendcounts, displs, MPI_INT32_T,
               A_local, rows_local * n, MPI_INT32_T,
               0, MPI_COMM_WORLD);

  // Broadcast de B a todos los procesos
  MPI_Bcast(B, n * n, MPI_INT32_T, 0, MPI_COMM_WORLD);

  // Cada proceso calcula su parte de C
  multiply_matrices(A_local, B, C_local, rows_local, n);

  // Recolectar resultados en el proceso 0 con Gatherv
  MPI_Gatherv(C_local, rows_local * n, MPI_INT32_T,
              C, sendcounts, displs, MPI_INT32_T,
              0, MPI_COMM_WORLD);

  double end_time = MPI_Wtime();

  // Solo el proceso 0 imprime resultados
  if (rank == 0)
  {
    if (n <= 5)
    {
      printf("Matriz A:\n");
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          printf("%d ", A[i * n + j]);
        }
        printf("\n");
      }

      printf("Matriz B:\n");
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          printf("%d ", B[i * n + j]);
        }
        printf("\n");
      }

      printf("Matriz C:\n");
      for (int i = 0; i < n; i++)
      {
        for (int j = 0; j < n; j++)
        {
          printf("%d ", C[i * n + j]);
        }
        printf("\n");
      }
    }

    printf("Tiempo de multiplicacion (MPI con %d procesos): %.6f segundos\n", 
           size, end_time - start_time);
    
    // Mostrar distribución de carga
    printf("Distribucion de filas: ");
    for (int i = 0; i < size; i++)
    {
      int rows = (i < extra_rows) ? base_rows + 1 : base_rows;
      printf("P%d:%d%s", i, rows, (i < size - 1) ? ", " : "\n");
    }
  }

  // Liberar memoria
  free(A_local);
  free(C_local);
  free(B);
  
  if (rank == 0)
  {
    free(A);
    free(C);
    free(sendcounts);
    free(displs);
  }

  MPI_Finalize();
  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <omp.h>

// Generar una matriz cuadrada NxN con enteros aleatorios
void generate_matrix(int32_t *matrix, int n)
{
  for (int i = 0; i < n * n; i++) {
    matrix[i] = rand() % 100;
  }
}

// Multiplicar matrices usando tasks de OpenMP
void multiply_matrices(int32_t *A, int32_t *B, int32_t *C, int n, int num_threads)
{
  omp_set_num_threads(num_threads);
  
  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int i = 0; i < n; i++) {
        #pragma omp task firstprivate(i)
        {
          for (int j = 0; j < n; j++) {
            int32_t sum = 0;
            for (int k = 0; k < n; k++) {
              sum += A[i * n + k] * B[j * n + k];
            }
            C[i * n + j] = sum;
          }
        }
      }
      #pragma omp taskwait
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Uso: %s <tamano_matriz> <num_hilos>\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);
  int num_threads = atoi(argv[2]);

  if (n <= 0 || num_threads <= 0)
  {
    printf("El tamaño y número de hilos deben ser positivos\n");
    return 1;
  }

  srand(time(NULL));

  // Reservar memoria dinámica
  int32_t *A = (int32_t *)malloc(n * n * sizeof(int32_t));
  int32_t *B = (int32_t *)malloc(n * n * sizeof(int32_t));
  int32_t *C = (int32_t *)malloc(n * n * sizeof(int32_t));

  if (!A || !B || !C)
  {
    printf("Error al asignar memoria\n");
    return 1;
  }

  // Llenar matrices con números aleatorios
  generate_matrix(A, n);
  generate_matrix(B, n);

  // Medir tiempo
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  multiply_matrices(A, B, C, n, num_threads);

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;

  // Si la matriz es pequeña, imprimirla
  if (n <= 5)
  {
    printf("Matriz A:\n");
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        printf("%d ", A[i * n + j]);
      }
      printf("\n");
    }

    printf("Matriz B:\n");
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        printf("%d ", B[j * n + i]);
      }
      printf("\n");
    }

    printf("Matriz C:\n");
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        printf("%d ", C[i * n + j]);
      }
      printf("\n");
    }
  }

  printf("Tiempo de multiplicacion con %d hilos (OpenMP Tasks): %.6f segundos\n",
         num_threads, elapsed);

  // Liberar memoria
  free(A);
  free(B);
  free(C);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// Generar una matriz cuadrada NxN con enteros aleatorios
void generate_matrix(int32_t *matrix, int n)
{
  for (int i = 0; i < n * n; i++) {
    matrix[i] = rand() % 100; // valores pequeños para evitar overflow
  }
}

// Multiplicar matrices cuadradas: C = A * B
void multiply_matrices(int32_t *A, int32_t *B, int32_t *C, int n)
{
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      int32_t sum = 0;
      for (int k = 0; k < n; k++) {
        sum += A[i * n + k] * B[j * n + k]; // Se asume que B esta transpuesta
      }
      C[i * n + j] = sum;
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Uso: %s <tamano_matriz>\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);
  if (n <= 0)
  {
    printf("El tamaño debe ser positivo\n");
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
  // La matriz B se asume que esta transpuesta
  generate_matrix(B, n);

  // Medir tiempo
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  multiply_matrices(A, B, C, n);

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

  // Mostrar tiempo de ejecución
  printf("Tiempo de multiplicacion: %.6f segundos\n", elapsed);

  // Liberar memoria
  free(A);
  free(B);
  free(C);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

typedef struct {
  int id;          // ID del hilo
  int n;           // tamaño de la matriz
  int num_threads; // número total de hilos
  int32_t *A, *B, *C;
} ThreadData;

// Generar una matriz cuadrada NxN con enteros aleatorios
void generate_matrix(int32_t *matrix, int n) {
  for (int i = 0; i < n * n; i++) {
    matrix[i] = rand() % 100;
  }
}

// Función que ejecutará cada hilo
void* thread_multiply(void* arg) {
  ThreadData *data = (ThreadData*)arg;
  int n = data->n;
  int id = data->id;
  int num_threads = data->num_threads;

  // Dividir las filas entre los hilos
  int filas_por_hilo = n / num_threads;
  int inicio = id * filas_por_hilo;
  int fin = (id == num_threads - 1) ? n : inicio + filas_por_hilo;

  for (int i = inicio; i < fin; i++) {
    for (int j = 0; j < n; j++) {
      int32_t sum = 0;
      for (int k = 0; k < n; k++) {
        sum += data->A[i * n + k] * data->B[k * n + j];
      }
      data->C[i * n + j] = sum;
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Uso: %s <tamano_matriz> <num_hilos>\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);
  int num_threads = atoi(argv[2]);

  if (n <= 0 || num_threads <= 0) {
    printf("El tamaño y número de hilos deben ser positivos\n");
    return 1;
  }

  srand(time(NULL));

  // Reservar memoria dinámica
  int32_t *A = (int32_t*)malloc(n * n * sizeof(int32_t));
  int32_t *B = (int32_t*)malloc(n * n * sizeof(int32_t));
  int32_t *C = (int32_t*)malloc(n * n * sizeof(int32_t));

  if (!A || !B || !C) {
    printf("Error al asignar memoria\n");
    return 1;
  }

  // Llenar matrices con números aleatorios
  generate_matrix(A, n);
  generate_matrix(B, n);

  pthread_t threads[num_threads];
  ThreadData thread_data[num_threads];

  // Medir tiempo
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // Crear hilos
  for (int i = 0; i < num_threads; i++) {
    thread_data[i].id = i;
    thread_data[i].n = n;
    thread_data[i].num_threads = num_threads;
    thread_data[i].A = A;
    thread_data[i].B = B;
    thread_data[i].C = C;
    pthread_create(&threads[i], NULL, thread_multiply, &thread_data[i]);
  }

  // Esperar a que terminen
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed = (end.tv_sec - start.tv_sec) +
                   (end.tv_nsec - start.tv_nsec) / 1e9;

  // Si la matriz es pequeña, imprimirla
  if (n <= 5) {
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
        printf("%d ", B[i * n + j]);
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
  printf("Tiempo de multiplicacion con %d hilos: %.6f segundos\n",
         num_threads, elapsed);

  // Liberar memoria
  free(A);
  free(B);
  free(C);

  return 0;
}

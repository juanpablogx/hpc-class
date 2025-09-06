#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

// Generar una matriz cuadrada NxN con enteros aleatorios
void generate_matrix(int32_t *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = rand() % 100;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <tamano_matriz> <num_procesos>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int num_procs = atoi(argv[2]);

    if (n <= 0 || num_procs <= 0) {
        printf("El tamaño y número de procesos deben ser positivos\n");
        return 1;
    }

    srand(time(NULL));

    // Crear memoria compartida para matrices
    int shmA = shmget(IPC_PRIVATE, n * n * sizeof(int32_t), IPC_CREAT | 0666);
    int shmB = shmget(IPC_PRIVATE, n * n * sizeof(int32_t), IPC_CREAT | 0666);
    int shmC = shmget(IPC_PRIVATE, n * n * sizeof(int32_t), IPC_CREAT | 0666);

    if (shmA < 0 || shmB < 0 || shmC < 0) {
        perror("Error al crear memoria compartida");
        return 1;
    }

    int32_t *A = (int32_t*)shmat(shmA, NULL, 0);
    int32_t *B = (int32_t*)shmat(shmB, NULL, 0);
    int32_t *C = (int32_t*)shmat(shmC, NULL, 0);

    if (!A || !B || !C) {
        perror("Error en shmat");
        return 1;
    }

    // Llenar matrices con números aleatorios
    generate_matrix(A, n);
    generate_matrix(B, n);

    // Medir tiempo
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Crear procesos
    for (int p = 0; p < num_procs; p++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Error en fork");
            return 1;
        }
        if (pid == 0) {
            // Proceso hijo
            int filas_por_proceso = n / num_procs;
            int inicio = p * filas_por_proceso;
            int fin = (p == num_procs - 1) ? n : inicio + filas_por_proceso;

            for (int i = inicio; i < fin; i++) {
                for (int j = 0; j < n; j++) {
                    int32_t sum = 0;
                    for (int k = 0; k < n; k++) {
                        sum += A[i * n + k] * B[k * n + j];
                    }
                    C[i * n + j] = sum;
                }
            }
            // Salir del hijo
            shmdt(A);
            shmdt(B);
            shmdt(C);
            exit(0);
        }
    }

    // Esperar a todos los hijos
    for (int i = 0; i < num_procs; i++) {
        wait(NULL);
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

    printf("Tiempo de multiplicacion con %d procesos: %.6f segundos\n",
           num_procs, elapsed);

    // Liberar memoria compartida
    shmdt(A);
    shmdt(B);
    shmdt(C);
    shmctl(shmA, IPC_RMID, NULL);
    shmctl(shmB, IPC_RMID, NULL);
    shmctl(shmC, IPC_RMID, NULL);

    return 0;
}

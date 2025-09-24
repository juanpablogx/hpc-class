// buffon_threads.c
// Simulación Monte Carlo del problema de Buffon usando hilos (pthread).
// Uso: ./buffon_threads <num_trials> <num_threads> [L] [D]
//   L y D opcionales (por defecto L=1.0, D=1.0). Se asume L <= D.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
  int id;
  int num_threads;
  unsigned long long num_trials;
  double L, D;
  unsigned long long local_crosses;
  unsigned int seed;
} ThreadData;

static double PI;

void* thread_func(void* arg) {
  ThreadData *td = (ThreadData*)arg;
  unsigned long long base = td->num_trials / td->num_threads;
  unsigned long long start = (unsigned long long)td->id * base;
  unsigned long long end = (td->id == td->num_threads - 1)
    ? td->num_trials
    : start + base;

  unsigned long long crosses = 0;
  unsigned int seed = td->seed;

  for (unsigned long long i = start; i < end; i++) {
    double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (td->D / 2.0);
    double theta = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (PI / 2.0);
    if (x <= (td->L / 2.0) * sin(theta)) crosses++;
  }

  td->local_crosses = crosses;
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Uso: %s <num_trials> <num_threads> [L] [D]\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  int num_threads = atoi(argv[2]);
  double L = (argc >= 4) ? atof(argv[3]) : 1.0;
  double D = (argc >= 5) ? atof(argv[4]) : 1.0;

  if (num_trials == 0 || num_threads <= 0 || L <= 0.0 || D <= 0.0 || L > D) {
    printf("Parámetros inválidos.\n");
    return 1;
  }

  PI = acos(-1.0);

  pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
  ThreadData *td = malloc(sizeof(ThreadData) * num_threads);
  if (!threads || !td) {
    perror("malloc");
    return 1;
  }

  unsigned int base_seed = (unsigned int)time(NULL);

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (int i = 0; i < num_threads; i++) {
    td[i].id = i;
    td[i].num_threads = num_threads;
    td[i].num_trials = num_trials;
    td[i].L = L;
    td[i].D = D;
    td[i].local_crosses = 0;
    td[i].seed = base_seed ^ (unsigned int)(i * 0x9e3779b9);
    if (pthread_create(&threads[i], NULL, thread_func, &td[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  unsigned long long total_crosses = 0;
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
    total_crosses += td[i].local_crosses;
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double p = (double)total_crosses / (double)num_trials;
  printf("Buffon hilos: trials=%llu threads=%d crosses=%llu P=%.10f tiempo=%.6f s\n",
         num_trials, num_threads, total_crosses, p, elapsed);

  free(threads);
  free(td);
  return 0;
}

// dartboard_threads.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

typedef struct {
  int id;
  int num_threads;
  unsigned long long num_trials;
  unsigned long long local_in;
  unsigned int seed;
} TData;

void* thread_func(void* arg) {
  TData *td = (TData*)arg;
  unsigned long long base = td->num_trials / td->num_threads;
  unsigned long long start = (unsigned long long)td->id * base;
  unsigned long long end = (td->id == td->num_threads - 1) ? td->num_trials : start + base;
  unsigned long long in_circle = 0;
  unsigned int seed = td->seed;

  for (unsigned long long i = start; i < end; i++) {
    double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
    double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
    if (x * x + y * y <= 1.0) in_circle++;
  }

  td->local_in = in_circle;
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Uso: %s <num_trials> <num_threads>\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  int num_threads = atoi(argv[2]);

  if (num_trials == 0 || num_threads <= 0) { printf("Parametros invalidos\n"); return 1; }

  pthread_t threads[num_threads];
  TData td[num_threads];
  unsigned int base_seed = (unsigned int)time(NULL);

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (int i = 0; i < num_threads; i++) {
    td[i].id = i;
    td[i].num_threads = num_threads;
    td[i].num_trials = num_trials;
    td[i].local_in = 0;
    td[i].seed = base_seed ^ (unsigned int)(i * 0x9e3779b9);
    pthread_create(&threads[i], NULL, thread_func, &td[i]);
  }

  unsigned long long total_in = 0;
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
    total_in += td[i].local_in;
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double pi_est = 4.0 * (double)total_in / (double)num_trials;
  printf("Dartboard hilos: trials=%llu threads=%d in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, num_threads, total_in, pi_est, elapsed);

  return 0;
}

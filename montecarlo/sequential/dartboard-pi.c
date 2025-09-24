// dartboard_seq.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

// Estimacion de pi lanzando dardos en el cuadrado [-1,1]x[-1,1]
// Uso: ./dartboard_seq <num_trials>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Uso: %s <num_trials>\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  if (num_trials == 0) { printf("num_trials debe ser > 0\n"); return 1; }

  unsigned long long in_circle = 0;
  unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (unsigned long long i = 0; i < num_trials; i++) {
    double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
    double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
    if (x * x + y * y <= 1.0) in_circle++;
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double pi_est = 4.0 * (double)in_circle / (double)num_trials;
  printf("Dartboard secuencial: trials=%llu in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, in_circle, pi_est, elapsed);

  return 0;
}

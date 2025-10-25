#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

// Optimizaciones:
// 1. Loop unrolling para mejor uso de pipeline
// 2. Reducir llamadas a rand_r
// 3. Evitar multiplicaciones redundantes

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Uso: %s <num_trials>\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  if (num_trials == 0) { printf("num_trials debe ser > 0\n"); return 1; }

  unsigned long long in_circle = 0;
  unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
  
  // Precalcular constantes
  const double scale = 2.0 / ((double)RAND_MAX + 1.0);
  
  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  // Loop unrolling (4x) para mejor rendimiento
  unsigned long long i;
  unsigned long long trials_unrolled = (num_trials / 4) * 4;
  
  for (i = 0; i < trials_unrolled; i += 4) {
    // Iteración 1
    double x1 = ((double)rand_r(&seed) * scale) - 1.0;
    double y1 = ((double)rand_r(&seed) * scale) - 1.0;
    if (x1 * x1 + y1 * y1 <= 1.0) in_circle++;
    
    // Iteración 2
    double x2 = ((double)rand_r(&seed) * scale) - 1.0;
    double y2 = ((double)rand_r(&seed) * scale) - 1.0;
    if (x2 * x2 + y2 * y2 <= 1.0) in_circle++;
    
    // Iteración 3
    double x3 = ((double)rand_r(&seed) * scale) - 1.0;
    double y3 = ((double)rand_r(&seed) * scale) - 1.0;
    if (x3 * x3 + y3 * y3 <= 1.0) in_circle++;
    
    // Iteración 4
    double x4 = ((double)rand_r(&seed) * scale) - 1.0;
    double y4 = ((double)rand_r(&seed) * scale) - 1.0;
    if (x4 * x4 + y4 * y4 <= 1.0) in_circle++;
  }
  
  // Procesar iteraciones restantes
  for (; i < num_trials; i++) {
    double x = ((double)rand_r(&seed) * scale) - 1.0;
    double y = ((double)rand_r(&seed) * scale) - 1.0;
    if (x * x + y * y <= 1.0) in_circle++;
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double pi_est = 4.0 * (double)in_circle / (double)num_trials;
  printf("Dartboard secuencial optimizado: trials=%llu in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, in_circle, pi_est, elapsed);

  return 0;
}
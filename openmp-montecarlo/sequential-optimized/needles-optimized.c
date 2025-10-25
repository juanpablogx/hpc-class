#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

// Optimizaciones:
// 1. Precalcular constantes
// 2. Loop unrolling
// 3. Reducir operaciones matemáticas

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Uso: %s <num_trials> [L] [D]\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  double L = (argc >= 3) ? atof(argv[2]) : 1.0;
  double D = (argc >= 4) ? atof(argv[3]) : 1.0;

  if (num_trials == 0 || L <= 0.0 || D <= 0.0 || L > D) {
    printf("Parámetros inválidos. Asegure: num_trials>0, L>0, D>0 y L<=D\n");
    return 1;
  }

  unsigned long long crosses = 0;
  unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
  const double PI = acos(-1.0);
  
  // Precalcular constantes
  const double scale_x = (D / 2.0) / ((double)RAND_MAX + 1.0);
  const double scale_theta = (PI / 2.0) / ((double)RAND_MAX + 1.0);
  const double half_L = L / 2.0;

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  // Loop unrolling (4x)
  unsigned long long i;
  unsigned long long trials_unrolled = (num_trials / 4) * 4;
  
  for (i = 0; i < trials_unrolled; i += 4) {
    // Iteración 1
    double x1 = (double)rand_r(&seed) * scale_x;
    double theta1 = (double)rand_r(&seed) * scale_theta;
    if (x1 <= half_L * sin(theta1)) crosses++;
    
    // Iteración 2
    double x2 = (double)rand_r(&seed) * scale_x;
    double theta2 = (double)rand_r(&seed) * scale_theta;
    if (x2 <= half_L * sin(theta2)) crosses++;
    
    // Iteración 3
    double x3 = (double)rand_r(&seed) * scale_x;
    double theta3 = (double)rand_r(&seed) * scale_theta;
    if (x3 <= half_L * sin(theta3)) crosses++;
    
    // Iteración 4
    double x4 = (double)rand_r(&seed) * scale_x;
    double theta4 = (double)rand_r(&seed) * scale_theta;
    if (x4 <= half_L * sin(theta4)) crosses++;
  }
  
  // Procesar iteraciones restantes
  for (; i < num_trials; i++) {
    double x = (double)rand_r(&seed) * scale_x;
    double theta = (double)rand_r(&seed) * scale_theta;
    if (x <= half_L * sin(theta)) crosses++;
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double p = (double)crosses / (double)num_trials;
  printf("Buffon secuencial optimizado: trials=%llu crosses=%llu P=%.10f tiempo=%.6f s\n",
         num_trials, crosses, p, elapsed);

  return 0;
}
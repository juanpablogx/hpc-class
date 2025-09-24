// buffon_seq.c
// Simulación Monte Carlo del problema de Buffon (secuencial).
// Uso: ./buffon_seq <num_trials> [L] [D]
//   num_trials: número de lanzamientos
//   L: longitud de la aguja (por defecto 1.0)
//   D: distancia entre líneas paralelas (por defecto 1.0)  (asume L <= D)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

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

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (unsigned long long i = 0; i < num_trials; i++) {
    // aprovechamos la simetría: x en [0, D/2), theta en [0, PI/2)
    double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (D / 2.0);
    double theta = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (PI / 2.0);

    if (x <= (L / 2.0) * sin(theta)) {
      crosses++;
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  double p = (double)crosses / (double)num_trials;
  printf("Buffon secuencial: trials=%llu crosses=%llu P=%.10f tiempo=%.6f s\n",
         num_trials, crosses, p, elapsed);

  return 0;
}

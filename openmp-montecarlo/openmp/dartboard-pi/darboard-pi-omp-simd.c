#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

// Versión OpenMP con SIMD - vectorización para mejor rendimiento

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Uso: %s <num_trials>\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);

  if (num_trials == 0) { 
    printf("Parametros invalidos\n"); 
    return 1; 
  }
  unsigned long long total_in = 0;
  
  double t0 = omp_get_wtime();

  #pragma omp parallel reduction(+:total_in)
  {
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)omp_get_thread_num() ^ (unsigned int)getpid();
    
    // for simd para vectorización automática
    #pragma omp for simd
    for (unsigned long long i = 0; i < num_trials; i++) {
      double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
      double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
      if (x * x + y * y <= 1.0) total_in++;
    }
  }

  double elapsed = omp_get_wtime() - t0;

  double pi_est = 4.0 * (double)total_in / (double)num_trials;
  printf("Dartboard OpenMP SIMD: trials=%llu in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, total_in, pi_est, elapsed);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

// Versión OpenMP con procesamiento por bloques - mejor localidad de cache

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
  
  // Tamaño de bloque óptimo para cache
  const unsigned long long BLOCK_SIZE = 10000;
  unsigned long long num_blocks = (num_trials + BLOCK_SIZE - 1) / BLOCK_SIZE;
  
  double t0 = omp_get_wtime();

  #pragma omp parallel reduction(+:total_in)
  {
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)omp_get_thread_num() ^ (unsigned int)getpid();
    
    #pragma omp for schedule(static)
    for (unsigned long long block = 0; block < num_blocks; block++) {
      unsigned long long start = block * BLOCK_SIZE;
      unsigned long long end = (start + BLOCK_SIZE < num_trials) ? start + BLOCK_SIZE : num_trials;
      
      for (unsigned long long i = start; i < end; i++) {
        double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
        double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) total_in++;
      }
    }
  }

  double elapsed = omp_get_wtime() - t0;

  double pi_est = 4.0 * (double)total_in / (double)num_trials;
  printf("Dartboard OpenMP blocked: trials=%llu in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, total_in, pi_est, elapsed);

  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>

// Versión OpenMP con tasks - paralelismo basado en tareas

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
  
  // Dividir trabajo en chunks para tasks
  const int NUM_TASKS = omp_get_num_threads() * 4;
  unsigned long long chunk_size = num_trials / NUM_TASKS;
  
  double t0 = omp_get_wtime();

  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int t = 0; t < NUM_TASKS; t++) {
        #pragma omp task
        {
          unsigned long long local_in = 0;
          unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)omp_get_thread_num() ^ (unsigned int)getpid() ^ (unsigned int)t;
          
          unsigned long long start = t * chunk_size;
          unsigned long long end = (t == NUM_TASKS - 1) ? num_trials : (t + 1) * chunk_size;
          
          for (unsigned long long i = start; i < end; i++) {
            double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
            double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
            if (x * x + y * y <= 1.0) local_in++;
          }
          
          #pragma omp atomic
          total_in += local_in;
        }
      }
    }
  }

  double elapsed = omp_get_wtime() - t0;

  double pi_est = 4.0 * (double)total_in / (double)num_trials;
  printf("Dartboard OpenMP tasks: trials=%llu in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, total_in, pi_est, elapsed);

  return 0;
}
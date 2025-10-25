#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>

// Versión OpenMP con tasks - paralelismo basado en tareas

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 4) {
    printf("Uso: %s <num_trials> [L] [D]\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  double L = (argc >= 3) ? atof(argv[2]) : 1.0;
  double D = (argc >= 4) ? atof(argv[3]) : 1.0;

  if (num_trials == 0 || L <= 0.0 || D <= 0.0 || L > D) {
    printf("Parámetros inválidos.\n");
    return 1;
  }

  const double PI = acos(-1.0);
  unsigned long long total_crosses = 0;
  
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
          unsigned long long local_crosses = 0;
          unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)omp_get_thread_num() ^ (unsigned int)getpid() ^ (unsigned int)t;
          
          unsigned long long start = t * chunk_size;
          unsigned long long end = (t == NUM_TASKS - 1) ? num_trials : (t + 1) * chunk_size;
          
          for (unsigned long long i = start; i < end; i++) {
            double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (D / 2.0);
            double theta = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * (PI / 2.0);
            if (x <= (L / 2.0) * sin(theta)) local_crosses++;
          }
          
          #pragma omp atomic
          total_crosses += local_crosses;
        }
      }
    }
  }

  double elapsed = omp_get_wtime() - t0;

  double p = (double)total_crosses / (double)num_trials;
  printf("Buffon OpenMP tasks: trials=%llu crosses=%llu P=%.10f tiempo=%.6f s\n",
         num_trials, total_crosses, p, elapsed);

  return 0;
}
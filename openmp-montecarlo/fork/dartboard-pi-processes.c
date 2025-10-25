// dartboard_procs.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Uso: %s <num_trials> <num_procs>\n", argv[0]);
    return 1;
  }

  unsigned long long num_trials = strtoull(argv[1], NULL, 10);
  int num_procs = atoi(argv[2]);

  if (num_trials == 0 || num_procs <= 0) { printf("Parametros invalidos\n"); return 1; }

  int shm_id = shmget(IPC_PRIVATE, num_procs * sizeof(unsigned long long),
                      IPC_CREAT | 0666);
  if (shm_id < 0) { perror("shmget"); return 1; }

  unsigned long long *shm_counts = (unsigned long long*)shmat(shm_id, NULL, 0);
  if (shm_counts == (void*)-1) { perror("shmat"); shmctl(shm_id, IPC_RMID, NULL); return 1; }
  for (int i = 0; i < num_procs; i++) shm_counts[i] = 0;

  struct timespec t0, t1;
  clock_gettime(CLOCK_MONOTONIC, &t0);

  for (int p = 0; p < num_procs; p++) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
      unsigned long long base = num_trials / num_procs;
      unsigned long long start = (unsigned long long)p * base;
      unsigned long long end = (p == num_procs - 1) ? num_trials : start + base;
      unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)getpid();
      unsigned long long local_in = 0;
      for (unsigned long long i = start; i < end; i++) {
        double x = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
        double y = ((double)rand_r(&seed) / ((double)RAND_MAX + 1.0)) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) local_in++;
      }
      shm_counts[p] = local_in;
      shmdt(shm_counts);
      _exit(0);
    }
  }

  for (int i = 0; i < num_procs; i++) wait(NULL);

  clock_gettime(CLOCK_MONOTONIC, &t1);
  double elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

  unsigned long long total_in = 0;
  for (int i = 0; i < num_procs; i++) total_in += shm_counts[i];

  double pi_est = 4.0 * (double)total_in / (double)num_trials;
  printf("Dartboard procesos: trials=%llu procs=%d in_circle=%llu pi_est=%.10f tiempo=%.6f s\n",
         num_trials, num_procs, total_in, pi_est, elapsed);

  shmdt(shm_counts);
  shmctl(shm_id, IPC_RMID, NULL);
  return 0;
}

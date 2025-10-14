#!/bin/bash
# run_dartboard_threads.sh

# 1. Compilar el programa
gcc dartboard-pi-threads.c -O3 -o dartboard-pi-threads -lpthread -lm

# 2. Lista de hilos
threads_list="8 16 32"

# 3. Exponentes de 10 (8 a 11 con paso 0.5)
exponents=$(seq 8 0.5 11)

# 4. Ejecutar
for threads in $threads_list; do
  outfile="resultados_dartboard_threads_${threads}.csv"
  echo "exp,trial,num_trials,threads,in_circle,pi_est,tiempo" > $outfile

  for exp in $exponents; do
    num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')

    for trial in $(seq 1 10); do
      output=$(./dartboard-pi-threads $num_trials $threads)

      in_circle=$(echo $output | sed -n 's/.*in_circle=\([0-9]*\).*/\1/p')
      pi_est=$(echo $output | sed -n 's/.*pi_est=\([0-9.]*\).*/\1/p')
      tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')

      echo "$exp,$trial,$num_trials,$threads,$in_circle,$pi_est,$tiempo" >> $outfile
    done
  done

  echo "Resultados guardados en $outfile"
done

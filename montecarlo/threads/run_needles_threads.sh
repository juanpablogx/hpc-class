#!/bin/bash
# run_buffon_threads.sh
# Automatiza Buffon con hilos para varios tamaños de prueba y número de hilos

# 1. Compilar
gcc needles-threads.c -o needles-threads -lm -lpthread

# 2. Valores de hilos a probar
threads_list=(4 8 16 32 2)

# 3. Exponentes de 10 desde 8 hasta 11 en pasos de 0.5
for threads in "${threads_list[@]}"; do
  outfile="needles_threads_${threads}.csv"
  echo "exp,trial,num_trials,threads,crosses,P,tiempo" > $outfile
  
  for exp in $(seq 8 0.5 11); do
    num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')
    
    for trial in $(seq 1 5); do
      output=$(./needles-threads $num_trials $threads)
      
      # Extraer datos del output
      crosses=$(echo $output | sed -n 's/.*crosses=\([0-9]*\).*/\1/p')
      P=$(echo $output | sed -n 's/.*P=\([0-9.]*\).*/\1/p')
      tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
      
      # Guardar en CSV
      echo "$exp,$trial,$num_trials,$threads,$crosses,$P,$tiempo" >> $outfile
    done
  done
  echo "Resultados guardados en $outfile"
done

#!/bin/bash
# run_buffon_procs.sh
# Automatiza Buffon con procesos para varios tamaños de prueba y número de procesos

# 1. Compilar
gcc needles-processes.c -o needles-processes -lm

# 2. Valores de procesos a probar
procs_list=(4 8 16 32 2)

# 3. Exponentes de 10 desde 8 hasta 11 en pasos de 0.5
for procs in "${procs_list[@]}"; do
  outfile="needles-processes_${procs}.csv"
  echo "exp,trial,num_trials,procs,crosses,P,tiempo" > $outfile
  
  for exp in $(seq 8 0.5 11); do
    num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')

    for trial in $(seq 1 5); do
      output=$(./needles-processes $num_trials $procs)
      
      # Extraer datos del output
      crosses=$(echo $output | sed -n 's/.*crosses=\([0-9]*\).*/\1/p')
      P=$(echo $output | sed -n 's/.*P=\([0-9.]*\).*/\1/p')
      tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
      
      # Guardar en CSV
      echo "$exp,$trial,$num_trials,$procs,$crosses,$P,$tiempo" >> $outfile
    done
  done
  echo "Resultados guardados en $outfile"
done

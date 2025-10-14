#!/bin/bash
# run_dartboard_procs.sh

# 1. Compilar el programa
gcc dartboard-pi-processes.c -O3 -o dartboard-pi-processes -lm

# 2. Lista de procesos a probar
for procs in 2; do
  outfile="dartboard_pi_processes_O3_${procs}.csv"
  echo "exp,trial,num_trials,in_circle,pi_est,tiempo" > $outfile

  # 3. Exponentes de 10 (de 8.0 a 11.0 con paso 0.5)
  for exp in $(seq 8 0.5 11); do
    num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')
    
    # 10 intentos
    for trial in $(seq 1 10); do
      output=$(./dartboard-pi-processes $num_trials $procs)

      # Extraer datos del output
      in_circle=$(echo $output | sed -n 's/.*in_circle=\([0-9]*\).*/\1/p')
      pi_est=$(echo $output | sed -n 's/.*pi_est=\([0-9.]*\).*/\1/p')
      tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')

      # Guardar en CSV
      echo "$exp,$trial,$num_trials,$in_circle,$pi_est,$tiempo" >> $outfile
    done
  done

  echo "Resultados guardados en $outfile"
done

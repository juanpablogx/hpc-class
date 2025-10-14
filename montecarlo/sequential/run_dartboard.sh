#!/bin/bash
# run_dartboard.sh

# 1. Compilar el programa
gcc dartboard-pi.c -O3 -o dartboard-pi -lm

# 2. Archivo de salida CSV
outfile="resultados_dartboard-O3.csv"
echo "exp,trial,num_trials,in_circle,pi_est,tiempo" > $outfile

# 3. Exponentes de 10 (8 a 11 con paso 0.5)
for exp in $(seq 8 0.5 11); do
  # Calcular num_trials = 10^exp
  num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')
  
  # 10 intentos
  for trial in $(seq 1 10); do
    output=$(./dartboard-pi $num_trials)
    
    # Extraer datos del output
    in_circle=$(echo $output | sed -n 's/.*in_circle=\([0-9]*\).*/\1/p')
    pi_est=$(echo $output | sed -n 's/.*pi_est=\([0-9.]*\).*/\1/p')
    tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
    
    # Guardar en CSV
    echo "$exp,$trial,$num_trials,$in_circle,$pi_est,$tiempo" >> $outfile
  done
done

echo "Resultados guardados en $outfile"

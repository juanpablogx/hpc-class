#!/bin/bash
# run_buffon.sh

# 1. Compilar el programa
gcc needles.c -O3 -o needles -lm

# 2. Archivo de salida CSV
outfile="resultados_needles03.csv"
echo "exp,trial,num_trials,crosses,P,tiempo" > $outfile

# 3. Exponentes de 10 (8 a 11 con paso 0.5)
for exp in $(seq 8 0.5 11); do
  # Calcular num_trials = 10^exp
  num_trials=$(awk -v e=$exp 'BEGIN {printf "%.0f", 10^e}')
  
  # 10 intentos
  for trial in $(seq 1 10); do
    output=$(./needles $num_trials)
    
    # Extraer datos del output
    crosses=$(echo $output | sed -n 's/.*crosses=\([0-9]*\).*/\1/p')
    P=$(echo $output | sed -n 's/.*P=\([0-9.]*\).*/\1/p')
    tiempo=$(echo $output | sed -n 's/.*tiempo=\([0-9.]*\).*/\1/p')
    
    # Guardar en CSV
    echo "$exp,$trial,$num_trials,$crosses,$P,$tiempo" >> $outfile
  done
done

echo "Resultados guardados en $outfile"

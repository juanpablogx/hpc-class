#!/bin/bash

# Configuración
EXECUTABLE="./cars-sequential"
NUM_RUNS=5
RESULTS_DIR="resultados_secuencial"

# Parámetros de la simulación
N_SIZES=(1000 2000 4000 8000 12000)  # Tamaños de carretera a probar
TIMESTEPS=5000       # Número de iteraciones
DENSITY=0.5          # Densidad de carros

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Script de Pruebas Secuencial con Perf ===${NC}"

# Verificar que perf está instalado
if ! command -v perf &> /dev/null; then
    echo -e "${RED}Error: perf no está instalado${NC}"
    echo "Instala con: sudo apt-get install linux-tools-common linux-tools-generic"
    exit 1
fi

# Verificar que el ejecutable existe
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${YELLOW}Ejecutable no encontrado. Compilando...${NC}"
    gcc -O3 -Wall -o cars-sequential cars-sequential.c -lm
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Error al compilar. Abortando.${NC}"
        exit 1
    fi
    echo -e "${GREEN}Compilación exitosa.${NC}"
fi

# Crear directorio de resultados
mkdir -p "$RESULTS_DIR"
echo -e "${GREEN}Directorio de resultados creado: $RESULTS_DIR${NC}"

# Crear subdirectorio para archivos de perf
PERF_DIR="$RESULTS_DIR/perf_outputs"
mkdir -p "$PERF_DIR"
echo -e "${GREEN}Subdirectorio para perf creado: $PERF_DIR${NC}"

# Crear archivo de log
LOG_FILE="$RESULTS_DIR/test_log.txt"
echo "Inicio de pruebas: $(date)" > "$LOG_FILE"
echo "Parámetros de simulación:" >> "$LOG_FILE"
echo "  N (tamaños carretera): ${N_SIZES[@]}" >> "$LOG_FILE"
echo "  Timesteps: $TIMESTEPS" >> "$LOG_FILE"
echo "  Densidad: $DENSITY" >> "$LOG_FILE"
echo "Número de ejecuciones por tamaño: $NUM_RUNS" >> "$LOG_FILE"
echo "----------------------------------------" >> "$LOG_FILE"

echo -e "\n${BLUE}=== Ejecutando pruebas con perf ===${NC}"
echo -e "${YELLOW}Parámetros: Timesteps=$TIMESTEPS, Density=$DENSITY${NC}\n"

# Iterar sobre cada tamaño de N
for N in "${N_SIZES[@]}"; do
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}    PRUEBAS CON N=$N${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    # Crear archivo CSV para este tamaño N
    CSV_FILE="$RESULTS_DIR/resultados_N${N}.csv"
    echo "Ejecucion,Tiempo_Codigo_Segundos,Tiempo_Perf_Segundos,Ciclos,Instrucciones,IPC,Cache_References,Cache_Misses,Cache_Miss_Rate" > "$CSV_FILE"
    
    echo -e "${YELLOW}Tamaño de carretera N=$N${NC}"

# Función para extraer el tiempo del código
extract_code_time() {
    local output="$1"
    echo "$output" | grep "Tiempo total de ejecución" | sed -n 's/.*: \([0-9.]*\) segundos/\1/p'
}

# Función para extraer métricas de perf
extract_perf_metrics() {
    local perf_file="$1"
    
    # Extraer time elapsed (en segundos)
    local time_elapsed=$(grep "time elapsed" "$perf_file" | awk '{print $1}')
    
    # Extraer ciclos
    local cycles=$(grep "cycles" "$perf_file" | head -1 | awk '{gsub(/,/, "", $1); print $1}')
    
    # Extraer instrucciones
    local instructions=$(grep "instructions" "$perf_file" | awk '{gsub(/,/, "", $1); print $1}')
    
    # Calcular IPC (instructions per cycle)
    local ipc=""
    if [ -n "$cycles" ] && [ -n "$instructions" ] && [ "$cycles" != "0" ]; then
        ipc=$(echo "scale=4; $instructions / $cycles" | bc)
    fi
    
    # Extraer cache references
    local cache_refs=$(grep "cache-references" "$perf_file" | awk '{gsub(/,/, "", $1); print $1}')
    
    # Extraer cache misses
    local cache_misses=$(grep "cache-misses" "$perf_file" | awk '{gsub(/,/, "", $1); print $1}')
    
    # Calcular cache miss rate
    local cache_miss_rate=""
    if [ -n "$cache_refs" ] && [ -n "$cache_misses" ] && [ "$cache_refs" != "0" ]; then
        cache_miss_rate=$(echo "scale=4; ($cache_misses / $cache_refs) * 100" | bc)
    fi
    
    # Retornar valores separados por comas
    echo "$time_elapsed,$cycles,$instructions,$ipc,$cache_refs,$cache_misses,$cache_miss_rate"
}

# Ejecutar pruebas
for run in $(seq 1 $NUM_RUNS); do
    echo -e "${YELLOW}=== Ejecución $run/$NUM_RUNS ===${NC}"
    
    PERF_OUTPUT="$PERF_DIR/perf_output_N${N}_run${run}.txt"
    APP_OUTPUT="$PERF_DIR/app_output_N${N}_run${run}.txt"
    
    # Ejecutar con perf
    echo -ne "  Ejecutando con perf... "
    
    perf stat -e cycles,instructions,cache-references,cache-misses \
        -o "$PERF_OUTPUT" \
        $EXECUTABLE $N $TIMESTEPS $DENSITY > "$APP_OUTPUT" 2>&1
    
    exit_code=$?
    
    if [ $exit_code -ne 0 ]; then
        echo -e "${RED}FALLO${NC}"
        echo "Error en ejecución $run, exit_code=$exit_code" >> "$LOG_FILE"
        cat "$APP_OUTPUT" >> "$LOG_FILE"
        continue
    fi
    
    # Extraer tiempo del código
    code_time=$(extract_code_time "$(cat "$APP_OUTPUT")")
    
    if [ -z "$code_time" ]; then
        echo -e "${RED}FALLO (no se pudo extraer tiempo del código)${NC}"
        echo "No se pudo extraer tiempo del código en run $run" >> "$LOG_FILE"
        continue
    fi
    
    # Extraer métricas de perf
    perf_metrics=$(extract_perf_metrics "$PERF_OUTPUT")
    
    if [ -z "$perf_metrics" ]; then
        echo -e "${RED}FALLO (no se pudo extraer métricas de perf)${NC}"
        echo "No se pudo extraer métricas de perf en run $run" >> "$LOG_FILE"
        continue
    fi
    
    # Guardar en CSV
    echo "$run,$code_time,$perf_metrics" >> "$CSV_FILE"
    
    # Extraer solo el tiempo de perf para mostrar
    perf_time=$(echo "$perf_metrics" | cut -d',' -f1)
    
    echo -e "${GREEN}OK${NC}"
    echo "  Tiempo código: ${code_time}s"
    echo "  Tiempo perf:   ${perf_time}s"
    
    # Guardar en log
    echo "=== Ejecución $run ===" >> "$LOG_FILE"
    echo "Tiempo código: $code_time" >> "$LOG_FILE"
    echo "Métricas perf: $perf_metrics" >> "$LOG_FILE"
    echo "--- Output aplicación ---" >> "$LOG_FILE"
    cat "$APP_OUTPUT" >> "$LOG_FILE"
    echo "--- Output perf ---" >> "$LOG_FILE"
    cat "$PERF_OUTPUT" >> "$LOG_FILE"
    echo "========================================" >> "$LOG_FILE"
done

echo -e "${GREEN}Resultados para N=$N guardados en: $CSV_FILE${NC}"

done  # Fin del loop de N_SIZES

# Generar estadísticas
echo -e "\n${YELLOW}=== Generando estadísticas ===${NC}"

STATS_FILE="$RESULTS_DIR/estadisticas.txt"

echo "Estadísticas de Ejecución Secuencial con Perf" > "$STATS_FILE"
echo "=============================================" >> "$STATS_FILE"
echo "" >> "$STATS_FILE"

# Iterar sobre cada tamaño N para generar estadísticas
for N in "${N_SIZES[@]}"; do
    CSV_FILE="$RESULTS_DIR/resultados_N${N}.csv"
    
    if [ ! -f "$CSV_FILE" ]; then
        continue
    fi
    
    echo "================================" >> "$STATS_FILE"
    echo "TAMAÑO N = $N" >> "$STATS_FILE"
    echo "================================" >> "$STATS_FILE"
    echo "" >> "$STATS_FILE"
    
    # Estadísticas de tiempo del código
    times_code=$(tail -n +2 "$CSV_FILE" | cut -d',' -f2)
    stats_code=$(echo "$times_code" | awk '
    {
        sum += $1
        sumsq += ($1)^2
        if (NR == 1 || $1 < min) min = $1
        if (NR == 1 || $1 > max) max = $1
        count++
    }
    END {
        avg = sum / count
        std = sqrt((sumsq / count) - (avg^2))
        printf "Promedio: %.6f s\nMínimo:   %.6f s\nMáximo:   %.6f s\nDesv Std: %.6f s\n", avg, min, max, std
    }')
    
    echo "TIEMPO DEL CÓDIGO (clock()):" >> "$STATS_FILE"
    echo "$stats_code" >> "$STATS_FILE"
    echo "" >> "$STATS_FILE"
    
    # Estadísticas de tiempo de perf
    times_perf=$(tail -n +2 "$CSV_FILE" | cut -d',' -f3)
    stats_perf=$(echo "$times_perf" | awk '
    {
        sum += $1
        sumsq += ($1)^2
        if (NR == 1 || $1 < min) min = $1
        if (NR == 1 || $1 > max) max = $1
        count++
    }
    END {
        avg = sum / count
        std = sqrt((sumsq / count) - (avg^2))
        printf "Promedio: %.6f s\nMínimo:   %.6f s\nMáximo:   %.6f s\nDesv Std: %.6f s\n", avg, min, max, std
    }')
    
    echo "TIEMPO DE PERF (time elapsed):" >> "$STATS_FILE"
    echo "$stats_perf" >> "$STATS_FILE"
    echo "" >> "$STATS_FILE"
    
    # Estadísticas de IPC
    ipc_values=$(tail -n +2 "$CSV_FILE" | cut -d',' -f6 | grep -v "^$")
    if [ -n "$ipc_values" ]; then
        stats_ipc=$(echo "$ipc_values" | awk '
        {
            sum += $1
            if (NR == 1 || $1 < min) min = $1
            if (NR == 1 || $1 > max) max = $1
            count++
        }
        END {
            avg = sum / count
            printf "Promedio: %.4f\nMínimo:   %.4f\nMáximo:   %.4f\n", avg, min, max
        }')
        
        echo "IPC (Instructions Per Cycle):" >> "$STATS_FILE"
        echo "$stats_ipc" >> "$STATS_FILE"
        echo "" >> "$STATS_FILE"
    fi
    
    # Estadísticas de Cache Miss Rate
    cache_miss_rates=$(tail -n +2 "$CSV_FILE" | cut -d',' -f9 | grep -v "^$")
    if [ -n "$cache_miss_rates" ]; then
        stats_cache=$(echo "$cache_miss_rates" | awk '
        {
            sum += $1
            if (NR == 1 || $1 < min) min = $1
            if (NR == 1 || $1 > max) max = $1
            count++
        }
        END {
            avg = sum / count
            printf "Promedio: %.4f %%\nMínimo:   %.4f %%\nMáximo:   %.4f %%\n", avg, min, max
        }')
        
        echo "CACHE MISS RATE:" >> "$STATS_FILE"
        echo "$stats_cache" >> "$STATS_FILE"
    fi
    
    echo "" >> "$STATS_FILE"
done

echo -e "${GREEN}Estadísticas guardadas en: $STATS_FILE${NC}"

# Resumen final
echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}          RESUMEN FINAL${NC}"
echo -e "${GREEN}========================================${NC}"
echo "Fin de pruebas: $(date)" >> "$LOG_FILE"
echo -e "${GREEN}Tamaños de carretera probados: ${N_SIZES[@]}${NC}"
echo -e "${GREEN}Total de ejecuciones por tamaño: $NUM_RUNS${NC}"
echo -e "${GREEN}Total de ejecuciones: $((${#N_SIZES[@]} * $NUM_RUNS))${NC}"
echo -e "${GREEN}Resultados guardados en: $RESULTS_DIR${NC}"
echo ""
echo "Archivos generados:"
echo "  - resultados_N<tamaño>.csv (tiempos y métricas por cada N)"
echo "  - estadisticas.txt (resumen estadístico de todos los tamaños)"
echo "  - test_log.txt (log detallado)"
echo "  - perf_outputs/ (directorio con salidas de perf y aplicación)"
echo "      * perf_output_N<tamaño>_runX.txt"
echo "      * app_output_N<tamaño>_runX.txt"
echo ""
echo -e "${YELLOW}Para ver las estadísticas:${NC}"
echo "  cat $STATS_FILE"
echo ""
echo -e "${YELLOW}Para ver un CSV específico:${NC}"
echo "  cat $RESULTS_DIR/resultados_N1000.csv"
echo "  # o formateado:"
echo "  column -t -s',' $RESULTS_DIR/resultados_N1000.csv"

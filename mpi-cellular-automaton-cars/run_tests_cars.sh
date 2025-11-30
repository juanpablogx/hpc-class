#!/bin/bash

# Configuración
EXECUTABLE="./cars-mpi"
NUM_PROCESSES=(2 4 6 8 10)
NUM_RUNS=5
RESULTS_DIR="resultados_mpi"

# Parámetros de la simulación
N=10000              # Tamaño de la carretera
TIMESTEPS=5000       # Número de iteraciones
DENSITY=0.5          # Densidad de carros

# Configuración de hosts del cluster
HOSTS="wn1,wn2,wn3"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Script de Pruebas para Autómata Celular de Tráfico MPI ===${NC}"

# Verificar que el ejecutable existe
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: No se encuentra el ejecutable $EXECUTABLE${NC}"
    echo "Compilando el programa..."
    mpicc -O3 -Wall -o cars-mpi cars-mpi.c -lm
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Error al compilar. Abortando.${NC}"
        exit 1
    fi
    echo -e "${GREEN}Compilación exitosa.${NC}"
fi

# Crear directorio de resultados
mkdir -p "$RESULTS_DIR"
echo -e "${GREEN}Directorio de resultados creado: $RESULTS_DIR${NC}"

# Crear archivo de log general
LOG_FILE="$RESULTS_DIR/test_log.txt"
echo "Inicio de pruebas: $(date)" > "$LOG_FILE"
echo "Hosts utilizados: $HOSTS" >> "$LOG_FILE"
echo "Parámetros de simulación:" >> "$LOG_FILE"
echo "  N (tamaño carretera): $N" >> "$LOG_FILE"
echo "  Timesteps: $TIMESTEPS" >> "$LOG_FILE"
echo "  Densidad: $DENSITY" >> "$LOG_FILE"
echo "Número de procesos: ${NUM_PROCESSES[@]}" >> "$LOG_FILE"
echo "Número de ejecuciones por configuración: $NUM_RUNS" >> "$LOG_FILE"
echo "----------------------------------------" >> "$LOG_FILE"

# Función para extraer el tiempo de ejecución del output
extract_time() {
    local output="$1"
    # Busca la línea "Tiempo total de ejecución: X.XXXXXX segundos"
    # y extrae el número antes de "segundos"
    echo "$output" | grep "Tiempo total de ejecución" | sed -n 's/.*: \([0-9.]*\) segundos/\1/p'
}

# ============================================================================
# PRUEBAS ASÍNCRONAS (sync_mode = 0)
# ============================================================================
echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}    PRUEBAS ASÍNCRONAS (MPI_Isend/Irecv)${NC}"
echo -e "${BLUE}========================================${NC}"

CSV_ASYNC="$RESULTS_DIR/resultados_asincrono.csv"
echo "Num_Procesos,Ejecucion,Tiempo_Segundos" > "$CSV_ASYNC"

for np in "${NUM_PROCESSES[@]}"; do
    echo -e "\n${YELLOW}=== Modo ASÍNCRONO con $np procesos ===${NC}"
    
    for run in $(seq 1 $NUM_RUNS); do
        echo -ne "  Ejecución $run/$NUM_RUNS... "
        
        # Ejecutar en modo asíncrono (último parámetro = 0)
        output=$(mpirun -np $np -hosts $HOSTS $EXECUTABLE $N $TIMESTEPS $DENSITY 0 2>&1)
        exit_code=$?
        
        # Guardar output completo para debugging
        echo "=== ASYNC: np=$np, run=$run ===" >> "$LOG_FILE"
        echo "$output" >> "$LOG_FILE"
        echo "===================================================" >> "$LOG_FILE"
        
        if [ $exit_code -ne 0 ]; then
            echo -e "${RED}FALLO${NC}"
            echo "Error en ejecución asíncrona: np=$np, run=$run, exit_code=$exit_code" >> "$LOG_FILE"
            continue
        fi
        
        # Extraer el tiempo
        time_value=$(extract_time "$output")
        
        if [ -z "$time_value" ]; then
            echo -e "${RED}FALLO (no se pudo extraer tiempo)${NC}"
            echo "No se pudo extraer tiempo asíncrono: np=$np, run=$run" >> "$LOG_FILE"
            continue
        fi
        
        # Guardar en CSV
        echo "$np,$run,$time_value" >> "$CSV_ASYNC"
        
        echo -e "${GREEN}OK${NC} (${time_value}s)"
        echo "EXITO ASYNC: np=$np, run=$run, time=$time_value" >> "$LOG_FILE"
    done
done

echo -e "${GREEN}Resultados asíncronos guardados en: $CSV_ASYNC${NC}"

# ============================================================================
# PRUEBAS SÍNCRONAS (sync_mode = 1)
# ============================================================================
echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}    PRUEBAS SÍNCRONAS (MPI_Send/Recv)${NC}"
echo -e "${BLUE}========================================${NC}"

CSV_SYNC="$RESULTS_DIR/resultados_sincrono.csv"
echo "Num_Procesos,Ejecucion,Tiempo_Segundos" > "$CSV_SYNC"

for np in "${NUM_PROCESSES[@]}"; do
    echo -e "\n${YELLOW}=== Modo SÍNCRONO con $np procesos ===${NC}"
    
    for run in $(seq 1 $NUM_RUNS); do
        echo -ne "  Ejecución $run/$NUM_RUNS... "
        
        # Ejecutar en modo síncrono (último parámetro = 1)
        output=$(mpirun -np $np -hosts $HOSTS $EXECUTABLE $N $TIMESTEPS $DENSITY 1 2>&1)
        exit_code=$?
        
        # Guardar output completo para debugging
        echo "=== SYNC: np=$np, run=$run ===" >> "$LOG_FILE"
        echo "$output" >> "$LOG_FILE"
        echo "===================================================" >> "$LOG_FILE"
        
        if [ $exit_code -ne 0 ]; then
            echo -e "${RED}FALLO${NC}"
            echo "Error en ejecución síncrona: np=$np, run=$run, exit_code=$exit_code" >> "$LOG_FILE"
            continue
        fi
        
        # Extraer el tiempo
        time_value=$(extract_time "$output")
        
        if [ -z "$time_value" ]; then
            echo -e "${RED}FALLO (no se pudo extraer tiempo)${NC}"
            echo "No se pudo extraer tiempo síncrono: np=$np, run=$run" >> "$LOG_FILE"
            continue
        fi
        
        # Guardar en CSV
        echo "$np,$run,$time_value" >> "$CSV_SYNC"
        
        echo -e "${GREEN}OK${NC} (${time_value}s)"
        echo "EXITO SYNC: np=$np, run=$run, time=$time_value" >> "$LOG_FILE"
    done
done

echo -e "${GREEN}Resultados síncronos guardados en: $CSV_SYNC${NC}"

# ============================================================================
# GENERAR ESTADÍSTICAS
# ============================================================================
echo -e "\n${YELLOW}=== Generando estadísticas resumidas ===${NC}"

STATS_FILE="$RESULTS_DIR/estadisticas.csv"
echo "Modo,Num_Procesos,Tiempo_Promedio,Tiempo_Min,Tiempo_Max,Desviacion_Std" > "$STATS_FILE"

# Estadísticas para modo asíncrono
for np in "${NUM_PROCESSES[@]}"; do
    times=$(grep "^$np," "$CSV_ASYNC" | cut -d',' -f3)
    
    if [ -z "$times" ]; then
        continue
    fi
    
    stats=$(echo "$times" | awk '
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
        printf "%.6f,%.6f,%.6f,%.6f", avg, min, max, std
    }')
    
    echo "ASINCRONO,$np,$stats" >> "$STATS_FILE"
done

# Estadísticas para modo síncrono
for np in "${NUM_PROCESSES[@]}"; do
    times=$(grep "^$np," "$CSV_SYNC" | cut -d',' -f3)
    
    if [ -z "$times" ]; then
        continue
    fi
    
    stats=$(echo "$times" | awk '
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
        printf "%.6f,%.6f,%.6f,%.6f", avg, min, max, std
    }')
    
    echo "SINCRONO,$np,$stats" >> "$STATS_FILE"
done

echo -e "${GREEN}Estadísticas guardadas en: $STATS_FILE${NC}"

# ============================================================================
# CREAR ARCHIVO CONSOLIDADO
# ============================================================================
echo -e "\n${YELLOW}=== Creando archivo CSV consolidado ===${NC}"
CONSOLIDATED_CSV="$RESULTS_DIR/resultados_consolidados.csv"
echo "Modo,Num_Procesos,Ejecucion,Tiempo_Segundos" > "$CONSOLIDATED_CSV"

# Agregar resultados asíncronos
while IFS=',' read -r np run time; do
    if [ "$np" != "Num_Procesos" ]; then
        echo "ASINCRONO,$np,$run,$time" >> "$CONSOLIDATED_CSV"
    fi
done < "$CSV_ASYNC"

# Agregar resultados síncronos
while IFS=',' read -r np run time; do
    if [ "$np" != "Num_Procesos" ]; then
        echo "SINCRONO,$np,$run,$time" >> "$CONSOLIDATED_CSV"
    fi
done < "$CSV_SYNC"

echo -e "${GREEN}Archivo consolidado creado: $CONSOLIDATED_CSV${NC}"

# ============================================================================
# RESUMEN FINAL
# ============================================================================
echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}          RESUMEN FINAL${NC}"
echo -e "${GREEN}========================================${NC}"
echo "Fin de pruebas: $(date)" >> "$LOG_FILE"
echo -e "${GREEN}Configuración de pruebas:${NC}"
echo -e "  N (carretera): $N"
echo -e "  Timesteps: $TIMESTEPS"
echo -e "  Densidad: $DENSITY"
echo -e "  Hosts: $HOSTS"
echo -e "\n${GREEN}Total de configuraciones probadas: $((${#NUM_PROCESSES[@]} * 2))${NC}"
echo -e "${GREEN}Total de ejecuciones: $((${#NUM_PROCESSES[@]} * 2 * $NUM_RUNS))${NC}"
echo -e "${GREEN}Resultados guardados en: $RESULTS_DIR${NC}"
echo ""
echo "Archivos generados:"
echo "  - resultados_asincrono.csv (tiempos modo asíncrono)"
echo "  - resultados_sincrono.csv (tiempos modo síncrono)"
echo "  - resultados_consolidados.csv (todos los datos)"
echo "  - estadisticas.csv (promedios y estadísticas por modo)"
echo "  - test_log.txt (log detallado de ejecución)"
echo ""
echo -e "${YELLOW}Para visualizar las estadísticas:${NC}"
echo "  cat $STATS_FILE"
echo ""
echo -e "${YELLOW}Para comparar modos:${NC}"
echo "  column -t -s',' $STATS_FILE"

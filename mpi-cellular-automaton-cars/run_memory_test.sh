#!/bin/bash

# Configuración
EXECUTABLE="./cars-sequential"
RESULTS_DIR="resultados_memoria"

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

echo -e "${GREEN}=== Script de Medición de Memoria con pmap ===${NC}"

# Verificar que pmap está instalado
if ! command -v pmap &> /dev/null; then
    echo -e "${RED}Error: pmap no está instalado${NC}"
    echo "Instala con: sudo apt-get install procps"
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

# Crear archivo CSV con encabezados
CSV_FILE="$RESULTS_DIR/memoria_por_tamanio.csv"
echo "N,Memoria_Total_KB,Memoria_RSS_KB,Memoria_Dirty_KB" > "$CSV_FILE"

# Crear archivo de log
LOG_FILE="$RESULTS_DIR/memory_log.txt"
echo "Inicio de pruebas de memoria: $(date)" > "$LOG_FILE"
echo "Parámetros de simulación:" >> "$LOG_FILE"
echo "  N (tamaños carretera): ${N_SIZES[@]}" >> "$LOG_FILE"
echo "  Timesteps: $TIMESTEPS" >> "$LOG_FILE"
echo "  Densidad: $DENSITY" >> "$LOG_FILE"
echo "----------------------------------------" >> "$LOG_FILE"

echo -e "\n${BLUE}=== Ejecutando pruebas de memoria ===${NC}"
echo -e "${YELLOW}Parámetros: Timesteps=$TIMESTEPS, Density=$DENSITY${NC}\n"

# Función para extraer uso de memoria de pmap
extract_memory() {
    local pmap_file="$1"
    
    # Extraer la línea total que contiene el resumen
    local total_line=$(grep "^total" "$pmap_file" | tail -1)
    
    if [ -z "$total_line" ]; then
        echo "0,0,0"
        return
    fi
    
    # Extraer memoria total (primera columna después de "total")
    local total_kb=$(echo "$total_line" | awk '{print $2}' | tr -d 'K')
    
    # Extraer RSS (segunda columna)
    local rss_kb=$(echo "$total_line" | awk '{print $3}' | tr -d 'K')
    
    # Extraer Dirty (tercera columna)
    local dirty_kb=$(echo "$total_line" | awk '{print $4}' | tr -d 'K')
    
    # Si algún valor está vacío, usar 0
    total_kb=${total_kb:-0}
    rss_kb=${rss_kb:-0}
    dirty_kb=${dirty_kb:-0}
    
    echo "$total_kb,$rss_kb,$dirty_kb"
}

# Iterar sobre cada tamaño de N
for N in "${N_SIZES[@]}"; do
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}    PRUEBA CON N=$N${NC}"
    echo -e "${BLUE}========================================${NC}"
    
    PMAP_OUTPUT="$RESULTS_DIR/pmap_N${N}.txt"
    APP_OUTPUT="$RESULTS_DIR/app_output_N${N}.txt"
    
    echo -ne "  Ejecutando aplicación en background... "
    
    # Ejecutar la aplicación en background
    $EXECUTABLE $N $TIMESTEPS $DENSITY > "$APP_OUTPUT" 2>&1 &
    APP_PID=$!
    
    echo -e "${GREEN}PID=$APP_PID${NC}"
    
    # Esperar un momento para que el proceso se estabilice
    sleep 2
    
    # Verificar que el proceso sigue corriendo
    if ! kill -0 $APP_PID 2>/dev/null; then
        echo -e "${RED}  El proceso terminó demasiado rápido${NC}"
        echo "Proceso terminó antes de tomar medición de memoria para N=$N" >> "$LOG_FILE"
        echo "$N,0,0,0" >> "$CSV_FILE"
        continue
    fi
    
    echo -ne "  Capturando uso de memoria con pmap... "
    
    # Ejecutar pmap para capturar el uso de memoria
    pmap -x $APP_PID > "$PMAP_OUTPUT" 2>&1
    pmap_exit=$?
    
    if [ $pmap_exit -ne 0 ]; then
        echo -e "${RED}FALLO${NC}"
        echo "Error al ejecutar pmap para N=$N" >> "$LOG_FILE"
        cat "$PMAP_OUTPUT" >> "$LOG_FILE"
        echo "$N,0,0,0" >> "$CSV_FILE"
    else
        echo -e "${GREEN}OK${NC}"
        
        # Extraer métricas de memoria
        memory_metrics=$(extract_memory "$PMAP_OUTPUT")
        
        # Guardar en CSV
        echo "$N,$memory_metrics" >> "$CSV_FILE"
        
        # Mostrar resultados
        total_mem=$(echo "$memory_metrics" | cut -d',' -f1)
        rss_mem=$(echo "$memory_metrics" | cut -d',' -f2)
        dirty_mem=$(echo "$memory_metrics" | cut -d',' -f3)
        
        echo "  Memoria Total: ${total_mem} KB"
        echo "  RSS:           ${rss_mem} KB"
        echo "  Dirty:         ${dirty_mem} KB"
        
        # Guardar en log
        echo "=== N=$N ===" >> "$LOG_FILE"
        echo "Memoria Total: $total_mem KB" >> "$LOG_FILE"
        echo "RSS: $rss_mem KB" >> "$LOG_FILE"
        echo "Dirty: $dirty_mem KB" >> "$LOG_FILE"
        echo "--- Output pmap ---" >> "$LOG_FILE"
        cat "$PMAP_OUTPUT" >> "$LOG_FILE"
        echo "========================================" >> "$LOG_FILE"
    fi
    
    # Esperar a que termine el proceso o matarlo si es necesario
    echo -ne "  Esperando a que termine el proceso... "
    
    # Esperar hasta 60 segundos para que termine
    wait_count=0
    while kill -0 $APP_PID 2>/dev/null && [ $wait_count -lt 60 ]; do
        sleep 1
        ((wait_count++))
    done
    
    # Si aún está corriendo, matarlo
    if kill -0 $APP_PID 2>/dev/null; then
        echo -e "${YELLOW}TIMEOUT, terminando proceso${NC}"
        kill -9 $APP_PID 2>/dev/null
    else
        echo -e "${GREEN}OK${NC}"
    fi
    
    # Guardar output de la aplicación en log
    echo "--- Output aplicación ---" >> "$LOG_FILE"
    cat "$APP_OUTPUT" >> "$LOG_FILE"
    echo "" >> "$LOG_FILE"
    
done

# Resumen final
echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}          RESUMEN FINAL${NC}"
echo -e "${GREEN}========================================${NC}"
echo "Fin de pruebas: $(date)" >> "$LOG_FILE"
echo -e "${GREEN}Tamaños de carretera probados: ${N_SIZES[@]}${NC}"
echo -e "${GREEN}Resultados guardados en: $RESULTS_DIR${NC}"
echo ""
echo "Archivos generados:"
echo "  - memoria_por_tamanio.csv (uso de memoria por cada N)"
echo "  - memory_log.txt (log detallado)"
echo "  - pmap_N<tamaño>.txt (output completo de pmap)"
echo "  - app_output_N<tamaño>.txt (output de la aplicación)"
echo ""
echo -e "${YELLOW}Para ver los resultados:${NC}"
echo "  cat $CSV_FILE"
echo "  # o formateado:"
echo "  column -t -s',' $CSV_FILE"
echo ""
echo -e "${GREEN}=== Tabla de Resultados ===${NC}"
column -t -s',' "$CSV_FILE"

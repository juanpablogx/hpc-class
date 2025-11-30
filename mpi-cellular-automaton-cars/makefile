# Makefile para Traffic Cellular Automaton

CC = gcc
MPICC = mpicc
CFLAGS = -O3 -Wall
LDFLAGS = -lm

# Ejecutables
SERIAL = traffic_serial
PARALLEL = traffic_mpi

all: $(SERIAL) $(PARALLEL)

$(SERIAL): traffic_serial.c
	$(CC) $(CFLAGS) -o $(SERIAL) traffic_serial.c $(LDFLAGS)

$(PARALLEL): traffic_mpi.c
	$(MPICC) $(CFLAGS) -o $(PARALLEL) traffic_mpi.c $(LDFLAGS)

clean:
	rm -f $(SERIAL) $(PARALLEL)

# Reglas para ejecutar
run_serial: $(SERIAL)
	./$(SERIAL) 10000 5000 0.5

run_parallel_async: $(PARALLEL)
	@echo "=== Modo ASÍNCRONO (MPI_Isend/Irecv) ==="
	mpirun -np 4 ./$(PARALLEL) 10000 5000 0.5 0

run_parallel_sync: $(PARALLEL)
	@echo "=== Modo SÍNCRONO (MPI_Send/Recv) ==="
	mpirun -np 4 ./$(PARALLEL) 10000 5000 0.5 1

# Tests de rendimiento
benchmark: $(SERIAL) $(PARALLEL)
	@echo "=== Benchmark Serial ==="
	./$(SERIAL) 50000 10000 0.5
	@echo "\n=== Benchmark Paralelo ASÍNCRONO (2 procesos) ==="
	mpirun -np 2 ./$(PARALLEL) 50000 10000 0.5 0
	@echo "\n=== Benchmark Paralelo ASÍNCRONO (4 procesos) ==="
	mpirun -np 4 ./$(PARALLEL) 50000 10000 0.5 0
	@echo "\n=== Benchmark Paralelo SÍNCRONO (2 procesos) ==="
	mpirun -np 2 ./$(PARALLEL) 50000 10000 0.5 1
	@echo "\n=== Benchmark Paralelo SÍNCRONO (4 procesos) ==="
	mpirun -np 4 ./$(PARALLEL) 50000 10000 0.5 1

.PHONY: all clean run_serial run_parallel_async run_parallel_sync benchmark


# ============================================================================
# COMPARACIÓN: COMUNICACIÓN SÍNCRONA vs ASÍNCRONA EN MPI
# ============================================================================
#
# El ejercicio pide implementar AMBOS modos de comunicación:
#
# 1. MODO ASÍNCRONO (sync_mode = 0)
# ----------------------------------
# Usa: MPI_Isend() + MPI_Irecv() + MPI_Waitall()
#
# Analogía: "Enviar una carta"
# - Pones la carta en el buzón y continúas con tu trabajo
# - No esperas a que llegue la respuesta inmediatamente
# - Luego verificas si llegaron las respuestas (MPI_Waitall)
#
# Ventajas:
# ✓ NO hay riesgo de deadlock
# ✓ Mejor rendimiento (comunicaciones se solapan)
# ✓ Todos los procesos envían/reciben simultáneamente
#
# Código:
#   MPI_Isend(&data, 1, MPI_INT, dest, tag, comm, &request);
#   MPI_Irecv(&buffer, 1, MPI_INT, src, tag, comm, &request);
#   MPI_Waitall(4, requests, statuses);
#
#
# 2. MODO SÍNCRONO (sync_mode = 1)
# ---------------------------------
# Usa: MPI_Send() + MPI_Recv()
#
# Analogía: "Llamada telefónica"
# - Llamas y te quedas esperando hasta que contesten
# - No puedes hacer nada más hasta que termine la llamada
# - Si todos llaman al mismo tiempo, nadie contesta → DEADLOCK!
#
# Desventajas:
# ✗ ALTO RIESGO DE DEADLOCK si no se programa con cuidado
# ✗ Menor rendimiento (comunicaciones secuenciales)
# ✗ Cada proceso debe esperar bloqueado
#
# Ventajas:
# ✓ Más simple de entender conceptualmente
# ✓ Comunicación garantizada cuando completa
#
# Código con PREVENCIÓN DE DEADLOCK:
#   if (rank % 2 == 0) {
#       MPI_Send(&data, ...);    // Pares envían primero
#       MPI_Recv(&buffer, ...);
#   } else {
#       MPI_Recv(&buffer, ...);  // Impares reciben primero
#       MPI_Send(&data, ...);
#   }
#
#
# ¿CÓMO EVITAMOS EL DEADLOCK EN MODO SÍNCRONO?
# ---------------------------------------------
# Problema: Si todos los procesos hacen Send primero:
#   Proceso 0: MPI_Send(...) → Bloqueado esperando que alguien reciba
#   Proceso 1: MPI_Send(...) → Bloqueado esperando que alguien reciba
#   Proceso 2: MPI_Send(...) → Bloqueado esperando que alguien reciba
#   → TODOS BLOQUEADOS → DEADLOCK! ☠️
#
# Solución: Alternar el orden según rank par/impar
#   Procesos PARES (0, 2, 4...):
#     Send derecha → Recv izquierda → Send izquierda → Recv derecha
#
#   Procesos IMPARES (1, 3, 5...):
#     Recv izquierda → Send derecha → Recv derecha → Send izquierda
#
# Así SIEMPRE hay alguien esperando recibir cuando otro envía.
#
#
# RENDIMIENTO ESPERADO
# --------------------
# El modo ASÍNCRONO debería ser más rápido porque:
# - Las comunicaciones se pueden solapar con el cómputo
# - No hay esperas innecesarias
# - Mejor uso del ancho de banda de red
#
# El modo SÍNCRONO será más lento porque:
# - Cada comunicación bloquea hasta completarse
# - No hay solapamiento
# - Serialización forzada de operaciones
#
# Ejecuta: make benchmark
# Para comparar los tiempos de ambos modos.
#
# ============================================================================
# GUÍA DE USO
# ============================================================================
#
# COMPILACIÓN:
# ------------
# make              # Compila ambas versiones
# make traffic_serial   # Solo versión serial
# make traffic_mpi      # Solo versión paralela
#
# EJECUCIÓN:
# ----------
# Versión Serial:
#   ./traffic_serial [N] [timesteps] [density]
#   Ejemplo: ./traffic_serial 10000 5000 0.5
#
# Versión Paralela:
#   mpirun -np <num_procesos> ./traffic_mpi [N] [timesteps] [density] [sync_mode]
#   
#   sync_mode: 0 = ASÍNCRONO (MPI_Isend/Irecv), 1 = SÍNCRONO (MPI_Send/Recv)
#   
#   Ejemplos:
#   mpirun -np 4 ./traffic_mpi 10000 5000 0.5 0  # Modo asíncrono
#   mpirun -np 4 ./traffic_mpi 10000 5000 0.5 1  # Modo síncrono
#
# Usando Makefile:
#   make run_parallel_async  # Ejecuta versión asíncrona
#   make run_parallel_sync   # Ejecuta versión síncrona
#
# PARÁMETROS:
# -----------
# N         : Longitud del camino (número de celdas). Default: 1000
# timesteps : Número de iteraciones. Default: 5000
# density   : Densidad inicial de carros (0.0 - 1.0). Default: 0.5
# sync_mode : Solo para versión paralela
#             0 = ASÍNCRONO (MPI_Isend/Irecv) - Default
#             1 = SÍNCRONO (MPI_Send/Recv)
#
# ANÁLISIS DE RENDIMIENTO:
# ------------------------
# make benchmark    # Ejecuta tests con diferentes configuraciones
#
# LIMPIEZA:
# ---------
# make clean        # Elimina ejecutables
#
# ============================================================================
# ANÁLISIS DE LA PARALELIZACIÓN MPI
# ============================================================================
#
# 1. DIVISIÓN DE DATOS:
#    - El camino de N celdas se divide entre los procesos
#    - Cada proceso maneja local_N = N/size celdas
#    - Se usa Scatterv para distribución inicial
#
# 2. GHOST CELLS (Celdas Fantasma):
#    - Cada proceso necesita conocer:
#      * La celda a su izquierda (del proceso anterior)
#      * La celda a su derecha (del proceso siguiente)
#    - Esto permite calcular las reglas del autómata en las fronteras
#
# 3. COMUNICACIÓN ASÍNCRONA:
#    - Se usa MPI_Isend/MPI_Irecv para comunicación no bloqueante
#    - Cada proceso envía sus fronteras a sus vecinos simultáneamente
#    - Topología circular: el último proceso conecta con el primero
#
# 4. SINCRONIZACIÓN:
#    - MPI_Waitall asegura que todas las comunicaciones terminen
#    - MPI_Reduce calcula la velocidad promedio global
#    - Se evita deadlock usando comunicación asíncrona
#
# 5. CONDICIONES DE FRONTERA PERIÓDICAS:
#    - Se implementan mediante topología circular de procesos
#    - rank 0 comunica con rank (size-1)
#    - Vecinos: left = (rank-1+size)%size, right = (rank+1)%size
#
# 6. BALANCE DE CARGA:
#    - Si N no es divisible por size, los primeros procesos
#      reciben una celda extra
#    - Esto minimiza el desbalance
#
# 7. ESCALABILIDAD:
#    - Speedup ideal: T_serial / T_parallel ≈ número de procesos
#    - Overhead: comunicación entre procesos
#    - Mejor rendimiento con N grande y pocas comunicaciones
#
# ============================================================================
# REGLAS DEL AUTÓMATA CELULAR
# ============================================================================
#
# Tabla 1: Si celda actual está VACÍA (Rt(i) = 0)
# --------------------------------------------------------
# Rt(i-1) | Rt(i) | Rt(i+1) | Rt+1(i) | Descripción
# --------------------------------------------------------
#    0    |   0   |    0    |    0    | Todo vacío
#    0    |   0   |    1    |    0    | Vacío permanece
#    1    |   0   |    0    |    1    | Carro entra desde atrás
#    1    |   0   |    1    |    1    | Carro entra (bloqueado)
#
# Tabla 2: Si celda actual tiene CARRO (Rt(i) = 1)
# --------------------------------------------------------
# Rt(i-1) | Rt(i) | Rt(i+1) | Rt+1(i) | Descripción
# --------------------------------------------------------
#    0    |   1   |    0    |    0    | Carro avanza
#    0    |   1   |    1    |    1    | Carro bloqueado
#    1    |   1   |    0    |    0    | Carro avanza
#    1    |   1   |    1    |    1    | Carro bloqueado
#
# Regla simplificada:
# - Si hay carro Y espacio adelante vacío → carro se mueve
# - Si hay carro Y espacio adelante ocupado → carro se queda
# ============================================================================

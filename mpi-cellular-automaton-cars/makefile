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

# Makefile para compilar todas las versiones de Dartboard y Buffon

CC = gcc
CFLAGS = -O3 -march=native
OMPFLAGS = -fopenmp
PTHREADFLAGS = -pthread
LIBS = -lm

# Directorios
BINDIR = bin
SEQDIR = sequential
SEQOPTDIR = sequential-optimized
OPENMPDIR = openmp
PTHREADDIR = pthreads
FORKDIR = fork

# Ejecutables secuenciales
SEQ_TARGETS = $(BINDIR)/dartboard-pi $(BINDIR)/dartboard-pi-optimized $(BINDIR)/needles $(BINDIR)/needles-optimized

# Ejecutables con OpenMP
OMP_TARGETS = $(BINDIR)/dartboard-pi-omp-basic $(BINDIR)/dartboard-pi-omp-reduction $(BINDIR)/dartboard-pi-omp-dynamic \
              $(BINDIR)/dartboard-pi-omp-simd $(BINDIR)/dartboard-pi-omp-blocked $(BINDIR)/dartboard-pi-omp-tasks \
              $(BINDIR)/needles-omp-basic $(BINDIR)/needles-omp-reduction $(BINDIR)/needles-omp-dynamic \
              $(BINDIR)/needles-omp-simd $(BINDIR)/needles-omp-blocked $(BINDIR)/needles-omp-tasks

# Ejecutables con Pthreads
PTHREAD_TARGETS = $(BINDIR)/dartboard-pi-threads $(BINDIR)/needles-threads

# Ejecutables con fork
FORK_TARGETS = $(BINDIR)/dartboard-pi-processes $(BINDIR)/needles-processes

ALL_TARGETS = $(SEQ_TARGETS) $(OMP_TARGETS) $(PTHREAD_TARGETS) $(FORK_TARGETS)

.PHONY: all clean seq pthread omp fork

all: $(ALL_TARGETS)

seq: $(SEQ_TARGETS)

pthread: $(PTHREAD_TARGETS)

omp: $(OMP_TARGETS)

fork: $(FORK_TARGETS)

# Versiones secuenciales
$(BINDIR)/dartboard-pi: $(SEQDIR)/dartboard-pi.c
	$(CC) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-optimized: $(SEQOPTDIR)/dartboard-pi-optimized.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles: $(SEQDIR)/needles.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-optimized: $(SEQOPTDIR)/needles-optimized.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

# Versiones OpenMP - Dartboard
$(BINDIR)/dartboard-pi-omp-basic: $(OPENMPDIR)/dartboard-pi/dartboard-pi-omp-basic.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-omp-reduction: $(OPENMPDIR)/dartboard-pi/dartboard-pi-omp-reduction.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-omp-dynamic: $(OPENMPDIR)/dartboard-pi/dartboard-pi-omp-dynamic.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-omp-simd: $(OPENMPDIR)/dartboard-pi/darboard-pi-omp-simd.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-omp-blocked: $(OPENMPDIR)/dartboard-pi/dartboard-pi-omp-blocked.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/dartboard-pi-omp-tasks: $(OPENMPDIR)/dartboard-pi/dartboard-pi-omp-tasks.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

# Versiones OpenMP - Needles
$(BINDIR)/needles-omp-basic: $(OPENMPDIR)/needles/needles-omp-basic.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-omp-reduction: $(OPENMPDIR)/needles/needles-omp-reduction.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-omp-dynamic: $(OPENMPDIR)/needles/needles-omp-dynamic.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-omp-simd: $(OPENMPDIR)/needles/needles-omp-simd.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-omp-blocked: $(OPENMPDIR)/needles/needles-omp-blocked.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-omp-tasks: $(OPENMPDIR)/needles/needles-omp-tasks.c
	$(CC) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

# Versiones Pthreads
$(BINDIR)/dartboard-pi-threads: $(PTHREADDIR)/dartboard-pi-threads.c
	$(CC) $(CFLAGS) $(PTHREADFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-threads: $(PTHREADDIR)/needles-threads.c
	$(CC) $(CFLAGS) $(PTHREADFLAGS) -o $@ $< $(LIBS)

# Versiones con fork
$(BINDIR)/dartboard-pi-processes: $(FORKDIR)/dartboard-pi-processes.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

$(BINDIR)/needles-processes: $(FORKDIR)/needles-processes.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

# Crear directorio bin si no existe
$(BINDIR):
	mkdir -p $(BINDIR)

# Todos los targets dependen de que exista el directorio bin
$(ALL_TARGETS): | $(BINDIR)

clean:
	rm -rf $(BINDIR)

# Ayuda
help:
	@echo "Targets disponibles:"
	@echo "  all      - Compila todas las versiones"
	@echo "  seq      - Compila solo versiones secuenciales"
	@echo "  omp      - Compila solo versiones con OpenMP"
	@echo "  pthread  - Compila solo versiones con Pthreads"
	@echo "  fork     - Compila solo versiones con fork"
	@echo "  clean    - Elimina todos los ejecutables"
	@echo ""
	@echo "Versiones secuenciales:"
	@echo "  $(SEQ_TARGETS)"
	@echo ""
	@echo "Versiones OpenMP:"
	@echo "  $(OMP_TARGETS)"
	@echo ""
	@echo "Versiones Pthreads:"
	@echo "  $(PTHREAD_TARGETS)"
	@echo ""
	@echo "Versiones fork:"
	@echo "  $(FORK_TARGETS)"
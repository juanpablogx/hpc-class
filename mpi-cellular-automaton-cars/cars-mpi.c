#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void initialize_road(int *road, int N, double density, int seed) {
    srand(seed);
    for (int i = 0; i < N; i++) {
        road[i] = (rand() / (double)RAND_MAX < density) ? 1 : 0;
    }
}

void update_road_local(int *road_old, int *road_new, int local_N, 
                       int left_ghost, int right_ghost, int *moved_cars) {
    *moved_cars = 0;
    
    // Inicializar road_new como copia de road_old
    for (int i = 0; i < local_N; i++) {
        road_new[i] = road_old[i];
    }
    
    // Aplicar reglas del autómata
    for (int i = 0; i < local_N; i++) {
        int current = road_old[i];
        int next;
        
        // Determinar el valor de la siguiente celda
        if (i == local_N - 1) {
            next = right_ghost;
        } else {
            next = road_old[i + 1];
        }
        
        // Si hay carro y espacio adelante vacío -> el carro se mueve
        if (current == 1 && next == 0) {
            road_new[i] = 0;  // La celda actual queda vacía
            if (i < local_N - 1) {
                road_new[i + 1] = 1;  // El carro se mueve a la siguiente celda local
            }
            // Si i == local_N - 1, el carro cruza la frontera (se maneja después)
            (*moved_cars)++;
        }
    }
    
    // Manejar carros que cruzan fronteras
    // Si la última celda tenía un carro y se movió, cruza al siguiente proceso
    if (road_old[local_N - 1] == 1 && right_ghost == 0) {
        road_new[local_N - 1] = 0;
    }
    
    // Si la primera celda está vacía y viene un carro de atrás
    if (road_old[0] == 0 && left_ghost == 1) {
        int prev_next = road_old[0];
        if (prev_next == 0) {
            road_new[0] = 1;
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int N = 1000;
    int timesteps = 5000;
    double density = 0.5;
    int sync_mode = 0;  // 0 = asíncrono, 1 = síncrono
    
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) timesteps = atoi(argv[2]);
    if (argc > 3) density = atof(argv[3]);
    if (argc > 4) sync_mode = atoi(argv[4]);
    
    // Dividir el camino entre los procesos
    int local_N = N / size;
    int remainder = N % size;
    if (rank < remainder) local_N++;
    
    int *road_old = (int*)calloc(local_N, sizeof(int));
    int *road_new = (int*)calloc(local_N, sizeof(int));
    int *full_road = NULL;
    
    // Proceso 0 inicializa el camino completo
    if (rank == 0) {
        full_road = (int*)calloc(N, sizeof(int));
        initialize_road(full_road, N, density, time(NULL));
    }
    
    // Calcular desplazamientos para Scatterv
    int *sendcounts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = N / size;
            if (i < remainder) sendcounts[i]++;
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    MPI_Scatterv(full_road, sendcounts, displs, MPI_INT,
                 road_old, local_N, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Contar carros totales
    int local_cars = 0;
    for (int i = 0; i < local_N; i++) {
        local_cars += road_old[i];
    }
    int total_cars = 0;
    MPI_Reduce(&local_cars, &total_cars, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Simulación de Tráfico - Versión Paralela MPI (%s)\n", 
               sync_mode ? "SÍNCRONA" : "ASÍNCRONA");
        printf("N = %d, Timesteps = %d, Densidad = %.2f\n", N, timesteps, density);
        printf("Procesos = %d, Total de carros: %d\n\n", size, total_cars);
    }
    
    // Determinar vecinos (topología circular)
    int left_neighbor = (rank - 1 + size) % size;
    int right_neighbor = (rank + 1) % size;
    
    double start_time = MPI_Wtime();
    
    for (int t = 0; t < timesteps; t++) {
        int left_ghost, right_ghost;
        
        if (sync_mode == 0) {
            // ========================================================
            // COMUNICACIÓN ASÍNCRONA (MPI_Isend/Irecv)
            // ========================================================
            // Todos los procesos envían y reciben simultáneamente
            // Las comunicaciones se completan en cualquier orden
            
            MPI_Request requests[4];
            
            // Enviar última celda al vecino derecho, recibir del izquierdo
            MPI_Isend(&road_old[local_N - 1], 1, MPI_INT, right_neighbor, 0, 
                      MPI_COMM_WORLD, &requests[0]);
            MPI_Irecv(&left_ghost, 1, MPI_INT, left_neighbor, 0, 
                      MPI_COMM_WORLD, &requests[1]);
            
            // Enviar primera celda al vecino izquierdo, recibir del derecho
            MPI_Isend(&road_old[0], 1, MPI_INT, left_neighbor, 1, 
                      MPI_COMM_WORLD, &requests[2]);
            MPI_Irecv(&right_ghost, 1, MPI_INT, right_neighbor, 1, 
                      MPI_COMM_WORLD, &requests[3]);
            
            // Esperar a que TODAS las comunicaciones terminen
            MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
            
        } else {
            // ========================================================
            // COMUNICACIÓN SÍNCRONA (MPI_Send/Recv)
            // ========================================================
            // PELIGRO DE DEADLOCK: Si todos hacen Send primero, nadie
            // estará disponible para Recv!
            //
            // SOLUCIÓN: Procesos pares e impares usan orden diferente
            
            if (rank % 2 == 0) {
                // Procesos PARES: primero envían a la derecha
                MPI_Send(&road_old[local_N - 1], 1, MPI_INT, right_neighbor, 0, 
                         MPI_COMM_WORLD);
                MPI_Recv(&left_ghost, 1, MPI_INT, left_neighbor, 0, 
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                // Luego envían a la izquierda
                MPI_Send(&road_old[0], 1, MPI_INT, left_neighbor, 1, 
                         MPI_COMM_WORLD);
                MPI_Recv(&right_ghost, 1, MPI_INT, right_neighbor, 1, 
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
            } else {
                // Procesos IMPARES: primero reciben de la izquierda
                MPI_Recv(&left_ghost, 1, MPI_INT, left_neighbor, 0, 
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&road_old[local_N - 1], 1, MPI_INT, right_neighbor, 0, 
                         MPI_COMM_WORLD);
                
                // Luego reciben de la derecha
                MPI_Recv(&right_ghost, 1, MPI_INT, right_neighbor, 1, 
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&road_old[0], 1, MPI_INT, left_neighbor, 1, 
                         MPI_COMM_WORLD);
            }
        }
        
        // Actualizar celdas locales
        int local_moved = 0;
        update_road_local(road_old, road_new, local_N, 
                         left_ghost, right_ghost, &local_moved);
        
        // Calcular velocidad promedio
        int total_moved = 0;
        MPI_Reduce(&local_moved, &total_moved, 1, MPI_INT, MPI_SUM, 0, 
                   MPI_COMM_WORLD);
        
        if (rank == 0 && (t % 1000 == 0 || t == timesteps - 1)) {
            double velocity = (total_cars > 0) ? 
                             (double)total_moved / total_cars : 0.0;
            printf("Timestep %d: Velocidad promedio = %.4f\n", t, velocity);
        }
        
        // Intercambiar arrays
        int *temp = road_old;
        road_old = road_new;
        road_new = temp;
    }
    
    double end_time = MPI_Wtime();
    
    if (rank == 0) {
        printf("\nTiempo total de ejecución: %.6f segundos\n", 
               end_time - start_time);
    }
    
    free(road_old);
    free(road_new);
    if (rank == 0) {
        free(full_road);
        free(sendcounts);
        free(displs);
    }
    
    MPI_Finalize();
    return 0;
}

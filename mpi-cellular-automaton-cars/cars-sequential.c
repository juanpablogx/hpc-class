#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initialize_road(int *road, int N, double density) {
    for (int i = 0; i < N; i++) {
        road[i] = (rand() / (double)RAND_MAX < density) ? 1 : 0;
    }
}

void update_road(int *road_old, int *road_new, int N, int *moved_cars) {
    *moved_cars = 0;
    
    for (int i = 0; i < N; i++) {
        int next = (i + 1) % N;
        
        if (road_old[i] == 1) {
            // Si hay un carro y el espacio adelante está vacío, se mueve
            if (road_old[next] == 0) {
                road_new[next] = 1;
                road_new[i] = 0;
                (*moved_cars)++;
            } else {
                // Si el espacio adelante está ocupado, se queda
                road_new[i] = 1;
            }
        } else if (road_old[i] == 0) {
            // Si la celda está vacía, verificar que no venga un carro de atrás
            int prev = (i - 1 + N) % N;
            if (road_old[prev] != 1 || road_old[i] == 1) {
                road_new[i] = road_old[i];
            }
        }
    }
}

int count_cars(int *road, int N) {
    int total = 0;
    for (int i = 0; i < N; i++) {
        total += road[i];
    }
    return total;
}

int main(int argc, char *argv[]) {
    int N = 1000;              // Longitud del camino
    int timesteps = 5000;      // Número de iteraciones
    double density = 0.5;      // Densidad inicial de carros
    
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) timesteps = atoi(argv[2]);
    if (argc > 3) density = atof(argv[3]);
    
    srand(time(NULL));
    
    int *road_old = (int*)calloc(N, sizeof(int));
    int *road_new = (int*)calloc(N, sizeof(int));
    
    initialize_road(road_old, N, density);
    int total_cars = count_cars(road_old, N);
    
    printf("Simulación de Tráfico - Versión Serial\n");
    printf("N = %d, Timesteps = %d, Densidad = %.2f\n", N, timesteps, density);
    printf("Total de carros: %d\n\n", total_cars);
    
    // Medición de tiempo
    clock_t start = clock();
    
    for (int t = 0; t < timesteps; t++) {
        int moved_cars = 0;
        update_road(road_old, road_new, N, &moved_cars);
        
        double velocity = (total_cars > 0) ? (double)moved_cars / total_cars : 0.0;
        
        if (t % 1000 == 0 || t == timesteps - 1) {
            printf("Timestep %d: Velocidad promedio = %.4f\n", t, velocity);
        }
        
        // Intercambiar arrays para la siguiente iteración
        int *temp = road_old;
        road_old = road_new;
        road_new = temp;
    }
    
    clock_t end = clock();
    double time_elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nTiempo total de ejecución: %.6f segundos\n", time_elapsed);
    
    free(road_old);
    free(road_new);
    
    return 0;
}

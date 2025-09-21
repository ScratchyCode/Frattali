// graficare con: gnuplot -p -e "plot 'sierpinski_points.dat' with points pt 7 ps 0.1 notitle"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

// --- Parametri ---
#define NUM_THREADS 8
#define TEMP_FILENAME_BUFFER 64
#define TRANSIENT_ITERATIONS 100 // per non avere punti spuri

// Struttura per un punto 2D, long doule per alta precision
typedef struct {
    long double x;
    long double y;
} Point;

// Struttura per passare i dati a ogni thread
typedef struct {
    long thread_id;
    long long num_points_to_generate;
} ThreadData;

Point vertices[3];

// Funzione eseguita da ogni thread
void* generate_points(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    unsigned int seed = time(NULL) + data->thread_id;
    char temp_filename[TEMP_FILENAME_BUFFER];

    snprintf(temp_filename, sizeof(temp_filename), "sierpinski_thread_%ld.tmp", data->thread_id);

    FILE* tempFile = fopen(temp_filename, "w");
    if (tempFile == NULL) {
        perror("Errore creazione file temporaneo");
        return NULL;
    }

    Point current_point = {0.25L + data->thread_id * 0.1L, 0.25L};

    // --- FASE DEL TRANSIENTE ---
    // Eseguiamo N iterazioni "a vuoto" per superare il comportamento transiente iniziale.
    // Questo assicura che il punto si trovi sull'attrattore prima di iniziare la registrazione.
    for (int i = 0; i < TRANSIENT_ITERATIONS; ++i) {
        int vertex_index = rand_r(&seed) % 3;
        current_point.x = (current_point.x + vertices[vertex_index].x) / 2.0L;
        current_point.y = (current_point.y + vertices[vertex_index].y) / 2.0L;
    }

    // --- FASE DI REGISTRAZIONE (POST-TRANSIENTE) ---
    // Ora che il transiente è superato, generiamo e scriviamo i punti richiesti.
    for (long long i = 0; i < data->num_points_to_generate; ++i) {
        int vertex_index = rand_r(&seed) % 3;
        current_point.x = (current_point.x + vertices[vertex_index].x) / 2.0L;
        current_point.y = (current_point.y + vertices[vertex_index].y) / 2.0L;
        fprintf(tempFile, "%.18Lf %.18Lf\n", current_point.x, current_point.y);
    }
    
    fclose(tempFile);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <numero_totale_di_punti>\n", argv[0]);
        return 1;
    }

    long long total_points = atoll(argv[1]);
    if (total_points <= 0) return 1;

    vertices[0] = (Point){0.0L, 0.0L};
    vertices[1] = (Point){1.0L, 0.0L};
    vertices[2] = (Point){0.5L, sqrtl(3.0L) / 2.0L};

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    long long points_per_thread = total_points / NUM_THREADS;

    printf("FASE 1: Avvio di %d thread per generare %lld punti...\n", NUM_THREADS, total_points);

    for (long i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].num_points_to_generate = points_per_thread;
        if (i == NUM_THREADS - 1) {
            thread_data[i].num_points_to_generate += total_points % NUM_THREADS;
        }
        pthread_create(&threads[i], NULL, generate_points, &thread_data[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Generazione parallela completata.\n");
    printf("FASE 2: Unione (merge) dei file temporanei...\n");
    
    FILE *finalFile = fopen("sierpinski_points.dat", "w");
    if (finalFile == NULL) {
        perror("Errore creazione file finale");
        return 1;
    }

    for (long i = 0; i < NUM_THREADS; ++i) {
        char temp_filename[TEMP_FILENAME_BUFFER];
        snprintf(temp_filename, sizeof(temp_filename), "sierpinski_thread_%ld.tmp", i);
        
        FILE *tempFile = fopen(temp_filename, "r");
        if (tempFile == NULL) continue;

        int ch;
        while ((ch = fgetc(tempFile)) != EOF) {
            fputc(ch, finalFile);
        }
        
        fclose(tempFile);
        remove(temp_filename);
    }

    fclose(finalFile);

    printf("Fatto! File 'sierpinski_points.dat' creato con successo.\n");
    return 0;
}

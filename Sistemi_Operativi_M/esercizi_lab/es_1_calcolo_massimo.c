#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define M 5
#define N 100

int vector[N];


void *Calcolo(void *t) {
    long tid;
    int max = 0;

    tid = (int) t;
    printf("Thread %ld è partito...\n", tid);

    for(long i = ((N / M) * tid); i < ((N / M) * (tid + 1)); i++) {
        if(vector[i] > max) {
            max = vector[i];
        }
    }

    printf("Thread %ld ha finito. Ris = %ld\n", tid, max);
    pthread_exit((void*) max);
}

int main(int argc, char *argv[]) {
    pthread_t threads[M];
    long t;
    long status;
    int rc;
    int valori_max[M];
    int max = 0;

    // inizializzazione vettore con N valori random
    srand(time(0));
    
    for(int i = 0; i < N; i++) {
        int num = (rand() % (300 - 1 + 1)) + 1;
        vector[i] = num;
    }

    // creazione threads
    for(t = 0; t < M; t++) {
        printf("Main: creazione del thread %ld\n", t);

        rc = pthread_create(&threads[t], NULL, Calcolo, (void *)t);

        if(rc) {
            printf("ERRORE: %d\n", rc);
            exit(-1);
        }
    }

    // join threads
    for(t = 0; t < M; t++) {
        rc = pthread_join(threads[t], (void *)&status);

        if(rc) {
            printf("ERRORE join thread %ld codice %d\n", t, rc);
        } else {
            printf("Finito thread %ld con ris: %ld\n", t, status);
            valori_max[t] = status;
        }
    }

    // calcolo valore finale
    for(int i = 0; i < M; i++) {
        if(valori_max[i] > max) {
            max = valori_max[i];
        }
    }

    if(max == 0) {
        printf("ERRORE conteggio valore massimo...\n");
        exit(-2);
    } else {
        printf("Il valore massimo del vettore è: %d\n", max);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10
#define K 5

typedef struct {
    char film[K][40];
    int voti[K][N];
    int partecipazioni;
    pthread_mutex_t mutex;
} Sondaggio;

void *sondaggio_process(void *arg) {
    Sondaggio *s;
    int temp;
    int sum = 0;
    float media;
    s = (Sondaggio *) arg;
    srand(time(0));

    // inizio sezione critica
    pthread_mutex_lock(&s->mutex);

    s->partecipazioni++;
    temp = s->partecipazioni;

    for(int i = 0; i < K; i++) {
        int voto = (rand() % 10 + 1 - 1) + 1;
        s->voti[i][temp] = voto;

        for(int j = 0; j < temp; j++) {
            sum = sum + s->voti[i][j];
        }

        media = sum / temp;
        sum = 0;
        printf("Il punteggio parziale di %s è: %.2f\n", s->film[i], media);
    }

    // fine sezione critica
    pthread_mutex_unlock(&s->mutex);

    pthread_exit(0);
}

void setupFilm(Sondaggio *s) {
    strcpy(s->film[0], "Taxi Driver");
    strcpy(s->film[1], "Il Padrino");
    strcpy(s->film[2], "2001: Odissea nello spazio");
    strcpy(s->film[3], "Barry Lyndon");
    strcpy(s->film[4], "Shrek 2");
    s->partecipazioni = 0;
}

int main(int argc, char *argv[]) {
    pthread_t threads[N];
    Sondaggio *s;
    float voto_medio, best = 0;
    int sum = 0, best_i = 0;

    // inizializzazione struttura dati del sondaggio con nomi dei film
    setupFilm(s);

    // creazione thread che rappresentano le persone che svolgono il sondaggio
    for(int i = 0; i < N; i++) {
        printf("Creazione del thread: %d\n", i);

        if(pthread_create(&threads[i], NULL, sondaggio_process, (void *) s) < 0) {
            fprintf(stderr, "pthread_create error for thread %d\n", i);
            exit(1);
        }
    }

    // joid dei threads
    for(int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // termine sondaggio e conteggio finale
    printf("\nSONDAGGIO TERMINATO...\n");

    for(int i = 0; i < K; i++) {
        for(int j = 0; i < N; i++) {
            sum = sum + s->voti[i][j];
        }

        voto_medio = sum / N;
        sum = 0;

        if(voto_medio > best) {
            best = voto_medio;
            best_i = i;
        }

        printf("Il punteggio totale di %s è: %.2f\n", s->film[i], voto_medio);
    }

    printf("\nIl film con il punteggio medio più alto è %s con %.2f\n", s->film[best_i], best);
}
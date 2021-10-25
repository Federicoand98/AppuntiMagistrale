#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 30
#define MaxP 20
#define MaxA 5
#define MaxQ 8

typedef struct{
    int posti_liberi;
    int quad_liberi;
    int auto_libere;
    int sospesi[NUM_THREADS];
    sem_t S;
    int ordine_accesso; // mi serve per verifica
    pthread_mutex_t m;
} parco;

parco P;

void init(parco *p) {
	int i;

	p->posti_liberi=MaxP;
    p->quad_liberi=MaxQ;
    p->auto_libere=MaxA;
    p->ordine_accesso=0;

    sem_init(&(p->S),0,0);

    for(i=0; i<NUM_THREADS; i++)
        p->sospesi[i]=0;
        
    pthread_mutex_init(&p->m, NULL); 
}

// verifica che vi siano processi in attesa prima di pos-1
int davanti(parco *p, int pos) { 
    int i;

    for (i=pos-2; i>=0; i--)
        if (p->sospesi[i]!=0)
            return 1;

    return 0;
}

int da_risvegliare(parco *p) {   
    int i, ris=0;

    for (i=0; i<NUM_THREADS; i++)
        ris+=p->sospesi[i];

    return ris;
}

void entra(parco *p, int num) {
    pthread_mutex_lock(&p->m);
  
    int myplace;
    myplace= ++p->ordine_accesso;
  
    if (num < 3) {
        while(p->posti_liberi<num || p->quad_liberi==0 || davanti(p,myplace)) {   
            p->sospesi[myplace-1]++;
            pthread_mutex_unlock(&p->m);
            sem_wait(&(p->S)); // sospensione sul semaforo condizione
            pthread_mutex_lock(&p->m);
            p->sospesi[myplace-1]--;
            
        }

        p->posti_liberi-=num;
        p->quad_liberi--;
    } else {
        //gruppo di 3-5 persone     
        while(p->posti_liberi<num || p->auto_libere==0 || davanti(p,myplace)) {   
            p->sospesi[myplace-1]++;
            pthread_mutex_unlock(&p->m);
            sem_wait(&(p->S)); // sospensione sul semaforo condizione
            pthread_mutex_lock(&p->m);
            p->sospesi[myplace-1]--;
           
        }

        p->posti_liberi-=num;
        p->auto_libere--;
    }
   
    printf("entra il gruppo %d-simo di %d persone\n\n", myplace, num);
    pthread_mutex_unlock (&p->m);
}

void esci(parco *p, int num) {   
    int i,k;
    
    pthread_mutex_lock(&p->m);
    p->posti_liberi+=num;
    
    if (num<3)
        p->quad_liberi++;
    else
        p->auto_libere++;
    
    k=da_risvegliare(p);
    
    for (i=0; i<k; i++)   
        sem_post(&p->S); // risveglio tutti
    
    pthread_mutex_unlock (&p->m);
}

// gruppo visitatore-> t è il numero di componenti
void *visitatore(void *t) {	
    int N;
	N = (int)t;
    entra(&P, N); // richiesta entrata di un gruppo di N persone
    sleep(rand()%3);
    esci(&P,N);
}


int main (int argc, char *argv[]) {
    pthread_t thread[NUM_THREADS];
    int rc, num;
    long t;
    float media, max;
    void *status;
   
    srand(time(NULL));
    init(&P);
  
    for(t=0; t<NUM_THREADS; t++) {
        num=(rand()%5)+1;
        rc = pthread_create(&thread[t], NULL, visitatore, (void *)num);

        if (rc) {
            printf("ERRORE: %d\n", rc);
            exit(-1);   
        }
    }

	for(t=0; t<NUM_THREADS; t++) {
        rc = pthread_join(thread[t], &status);

        if (rc) 
            printf("ERRORE join thread %ld codice %d\n", t, rc);
    }

    return 0;
}
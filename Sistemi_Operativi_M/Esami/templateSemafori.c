#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define N 10
#define MAX 5


typedef struct{
    //variabili relative alla struttura
    pthread_mutex_t m;
}struttura;

struttura s;

typedef struct{
sem_t mb;
sem_t sb; 
int count;
} barriera;

barriera b;

void init(struttura *s, barriera *b) {
        int i;
        //inizializzazione variabili struttura
        pthread_mutex_init(&s->m, NULL);
        // inizializzazione barriera:
        sem_init(&b->mb,0,1);
        sem_init(&b->sb,0,0);
        b->count=0;
}


void sync_barriera(barriera *b, struttura *s)
{       sem_wait(&b->mb);
        b->count++; 
        if (b->count==NUM_THREADS)
        {       
                sem_post(&b->sb);
        }
        sem_post(&b->mb);
        sem_wait(&b->sb); 
        sem_post(&b->sb);
        return;
}


void engine(sondaggio *s,int id) {
 pthread_mutex_lock(&s->m);

//operazioni necessarie sulla struttura
 pthread_mutex_unlock (&s->m);
}



void *funzione(void *arg) // codice spettatore
{       int id,result = 0;
        id = (int) arg;
        engine(&s,id);
        sync_barriera(&b, &s);
        //printf("Spettatore %ld vede il film vincitore: %s\n",tid, F.vincitore); // visione film
        pthread_exit((void*) result);
}













int main (int argc, char *argv[])
{  pthread_t thread[NUM_THREADS];
   int rc;
   int t;
   
   void *status;
   
   init(&s, &b);
  
   for(t=0; t<NUM_THREADS; t++) {
      rc = pthread_create(&thread[t], NULL, funzione, (void *)t); 
      if (rc) {
         printf("ERRORE: %d\n", rc);
         exit(-1);   }
  }
        for(t=0; t<NUM_THREADS; t++) {
           rc = pthread_join(thread[t], &status);
                if (rc) 
                   printf("ERRORE join thread %ld codice %d\n", t, rc);
   }
    
  return 0;
}

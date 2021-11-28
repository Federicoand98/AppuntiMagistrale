#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define NUM_THREAD 10


typedef struct
{ 
  pthread_cond_t coda; 
  int sospCodaEntrata;
  int sospCodaUscita;
  pthread_mutex_t mutex;
}risorsa;

risorsa r;


void init_risorsa(risorsa *r){
 pthread_cond_init(&r->coda,NULL);
 //inizializzazione variabili della struttura
 r->sospCoda=0;
 pthread_mutex_init(&p->mutex,NULL); 
}


void entra(risorsa *r,int id){
pthread_mutex_lock(&r->mutex);//PROLOGO


//corpo della funzione
while( verifica_invarianza(r,id) || piu_prioritari(r,id)){

     r->sospCodaEntrata++;
     pthread_cond_wait(&r->codaEntrata,&r->mutex);
     r->sospCodaEntrata--;

}
r->countRisorsa++; 

//eventuali signal(r)

pthread_mutex_unlock(&r->mutex);//EPILOGO
}


void esce(risorsa *r,int id){
pthread_mutex_lock(&r->mutex);//PROLOGO


//corpo della funzione
while( verifica_invarianza(r,id) || piu_prioritari(r,id)) 
//deve restare nel while fino a che almeno una delle condizioni è verificata
{ 

     r->sospCodaUscita++;
     pthread_cond_wait(&r->codaUscita,&r->mutex);
     r->sospCodaUscita--;

}
r->countRisorsa--; 

//eventuali signal(r)
pthread_cond_broadcast(&r->coda);
pthread_mutex_unlock(&r->mutex);//EPILOGO
}


void *funzione(void *arg){
int id = (int) arg;
sleep(1);
entra(&r,id);
sleep(1);
esce(&r,id);
pthread_exit(NULL);
}



main(){

pthread_t NomeThread[NUM_THREAD];
init_risorsa(&r);
srand(time(NULL));

for(i=0;i<NUM_THREAD;i++){
   pthread_create(&NomeThread[i],NULL,NomeFunzione,(void*) i);
 }


for(i=0;i<NUM_THREAD;i++){
  pthread_join(NomeThread[i],NULL);
}

} //fine main



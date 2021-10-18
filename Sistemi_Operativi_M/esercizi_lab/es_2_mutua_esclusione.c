#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define K 5
#define N 20

typedef struct {
    char film[K][40];
    int voti[K];
    int pareri;
    pthread_mutex_t m;
} Sondaggio;

void *process(void *arg) {

}

int main(int argc, char *argv[]) {
    pthread_t threads[N];
    Sondaggio *sondaggio;
    
    
}
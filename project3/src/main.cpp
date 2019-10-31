#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <vector>
#include <set>
using namespace std;
int N;

void* ThreadFunc(void* arg){
    long tid = ((long)arg+1);

    return NULL;
}

int main(int argc, char* argv[]){
    if (argc < 4) {
        printf("usage: ./run N\n");
        printf("N: Number of worker threads\n");
        return 0;
	}
    srand(time(NULL));

    N = atoi(argv[1]);

    lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_t* threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t) * N);

    // Create threads to work.
    for( long i = 0; i<(long)N; i++){
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i)<0){
            printf("pthread_create error!\n");
            return 0;
        }
    }

    // Wait threads end.
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
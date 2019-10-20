#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <vector>
#include "Records.h"
using namespace std;


int N, R, E;
volatile int commit_id = 1;
record* records;

void* ThreadFunc(void* arg){
    long tid = (long)arg+1;
    char log_file_name[20];
    sprintf(log_file_name, "thread%ld.txt", tid);
    FILE* log_file = fopen(log_file_name, "w");
    while(commit_id < E){
        int i = rand()%R;
        int j = i, k = i;
        while(j==i){
            j = rand()%R;
        }
        while(k==j || k==i){
            k = rand()%R;
        }

   
        // read lock i
        long t = records[i].val;
        // writer lock j
        // chk cycle
        if(DetectCycle()){
            // unlock i
        }
        records[j].val += (t+1);
        // writer lock k
        // chk cycle
        if(DetectCycle()){
            // rollback j
            // unlock j
            // unlock i
        }
        records[k].val -= t;
        // Commit log
        int present_commit = __sync_fetch_and_add(&commit_id, 1);
        if(present_commit>=E){
            break;
        }
        fprintf(log_file, "%d %d %d %d %ld %ld %ld\n", present_commit, i,j,k,records[i].val, records[j].val, records[k].val );
        // unlock writer k
        // unlock writer j
        // unlock writer i


    }
    fclose(log_file);
    return NULL;
}

int main(int argc, char* argv[]){
    if (argc < 4) {
        printf("usage: ./run N R E\n");
        printf("N: Number of worker threads\n");
        printf("R: Number of records\n");
        printf("E: Last global execution order which threads will be used to decide termination\n");
        return 0;
	}
    srand(time(NULL));

    N = atoi(argv[1]);
    R = atoi(argv[2]);
    E = atoi(argv[3]);
    
    pthread_t* threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t) * N);
    records = (record *)malloc(sizeof(record)*R);

    // Initialize records
    for(int i = 0;i<R;i++){
        records[i].val = 100;
        records[i].read_active = 0;
        records[i].write_active = 0;
        records[i].read_cv = PTHREAD_COND_INITIALIZER;
        records[i].write_cv = PTHREAD_COND_INITIALIZER;
    }

    // Create threads to work.
    for( long i = 0; i<(long)N;i++){
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i)<0){
            printf("pthread_create error!\n");
            return 0;
        }
    }

    // Wait threads end.
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    return 0;
}
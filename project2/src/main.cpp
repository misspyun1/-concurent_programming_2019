#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <vector>
using namespace std;

int N, R, E;
int* records;
pthread_mutex_t* record_lock;
vector<deque<int> > lock_wait;
int commit_id;

bool DetectCycle(){

}

void* ThreadFunc(void* arg){
    long tid = (long)arg+1;
    char log_file_name[15];
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

        pthread_mutex_lock(&record_lock[i]);
        commit_id ++;
        int present_commit = commit_id;
        int t = records[i];
        pthread_mutex_lock(&record_lock[j]);
        records[j] += (t+1);
        pthread_mutex_lock(&record_lock[k]);
        records[k] -= t;
        // Commit log
        fprintf(log_file, "%d %d %d %d %d %d %d\n", present_commit, i,j,k,records[i], records[j], records[k] );
        pthread_mutex_unlock(&record_lock[k]);
        pthread_mutex_unlock(&record_lock[j]);
        pthread_mutex_unlock(&record_lock[i]);


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
    records = (int *)malloc(sizeof(int)*R);
    record_lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*R);
    // Initialize records as 100.
    for(int i = 0;i<R;i++){
        records[i] = 100;
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
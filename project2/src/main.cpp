#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <vector>
#include <set>
#include "Records.h"
using namespace std;

int N, R, E;
int commit_id = 1;
record* records;
pthread_mutex_t lock_table_mutex;

/* pair<lock_type, thread id>
    * lock_type 0 : reader lock
    * lock_type 1 : writer lock 
*/
vector<deque<pair<int, int> > >lock_table;
vector<unordered_set<int> > worker_chk;

void* ThreadFunc(void* arg){
    long tid = ((long)arg+1);
    char log_file_name[20];
    sprintf(log_file_name, "thread%ld.txt", tid);
    FILE* log_file = fopen(log_file_name, "w");
    while(commit_id <= E){
        int i = (rand()%R + 1);
        int j = i, k = i;
        while(j==i){
            j = (rand()%R + 1);
        }
        while(k==j || k==i){
            k = (rand()%R + 1);
        }
        printf("Thread %ld, i : %d j : %d k : %d\n",tid, i, j, k);
   
        // Acquire read lock for records[i].
        ReadLock(i, tid, records, lock_table, worker_chk, lock_table_mutex);
        long t = records[i].val;        
        
        // Chk cycle before acquire lock for records[j]
        if(DetectCycle(j, tid, lock_table, worker_chk, lock_table_mutex)){
            printf("Detect Cycle before j%ld : %d %d %d\n",tid, i, j, k);
            ReadUnlock(i, tid, records, lock_table, worker_chk, lock_table_mutex);
            continue;
        }
        // Acquire write lock for records[j].
        WriteLock(j, tid, records, lock_table, worker_chk, lock_table_mutex);
        records[j].val += (t+1);

        // Chk cycle before acquire lock for records[k]
        if(DetectCycle(k, tid, lock_table, worker_chk, lock_table_mutex)){
            printf("Detect Cycle before k %ld : %d %d %d\n",tid, i, j, k);
            records[j].val -= (t+1);
            WriteUnlock(j, tid, records, lock_table, worker_chk, lock_table_mutex);
            ReadUnlock(i, tid, records, lock_table, worker_chk, lock_table_mutex);
            continue;
        }
        // Acquire write lock for records[k].
        WriteLock(k, tid, records, lock_table, worker_chk, lock_table_mutex);
        records[k].val -= t;

        // Commit log
        int present_commit = __sync_fetch_and_add(&commit_id, 1);
        if(present_commit>E){
            records[k].val += t;
            records[j].val -= (t+1);
            WriteUnlock(k, tid, records, lock_table, worker_chk, lock_table_mutex);
            WriteUnlock(j, tid, records, lock_table, worker_chk, lock_table_mutex);
            ReadUnlock(i, tid, records, lock_table, worker_chk, lock_table_mutex);
            break;
        }
        printf("*****Commit log : %d %d %d %d %ld %ld %ld\n", present_commit, i,j,k,records[i].val, records[j].val, records[k].val );
        fprintf(log_file, "%d %d %d %d %ld %ld %ld\n", present_commit, i,j,k,records[i].val, records[j].val, records[k].val );

        // Release write lock for records[k].
        WriteUnlock(k, tid, records, lock_table, worker_chk, lock_table_mutex);
        // Release write lock for records[j].
        WriteUnlock(j, tid, records, lock_table, worker_chk, lock_table_mutex);
        // Release read lock for records[i].
        ReadUnlock(i, tid, records, lock_table, worker_chk, lock_table_mutex);


    }
    printf("Thread %ld END!!!!!\n", tid);
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

    lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_t* threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t) * N);
    worker_chk = vector<unordered_set<int> > (N+1);
    records = (record *)malloc(sizeof(record)*(R+1));
    lock_table = vector<deque<pair<int, int> > > (R+1);

    // Initialize records
    for(int i = 0; i < R; i++){
        records[i].val = 100;
        records[i].read_active = 0;
        records[i].write_active = 0;
        records[i].read_wait = 0;
        records[i].write_wait = 0;
        records[i].read_cv = PTHREAD_COND_INITIALIZER;
        records[i].write_cv = PTHREAD_COND_INITIALIZER;
        records[i].mutex = PTHREAD_MUTEX_INITIALIZER;
    }

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

    long records_sum = 0;
    for(int i = 1;i<=R;i++){
        printf("%ld ",records[i].val);
        records_sum += records[i].val;
    }

    printf("\nSUM : %ld\n", records_sum);
    free(threads);
    free(records);
    return 0;
}
#include <thread>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "wait_free_snapshot.h"
using namespace std;
int N;
bool is_1min = false;
int* update_cnt;
void* ThreadFunc(int arg){
    long tid = (arg+1);

    
    while(!is_1min){
        int rand_val = rand(); // get random value

        update_cnt[tid]++;
    }

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
    
    thread* threads;
    threads = (thread *)malloc(sizeof(thread) * N);
    update_cnt = (int *)malloc(sizeof(int) * (N+1));

    // Create threads to work.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
    for( long i = 0; i<(long)N; i++){
        threads[i] = thread(ThreadFunc, i);
    }

    this_thread::sleep_for(chrono::duration<int>(1));

    is_1min = true;
    int total_update_cnt = 0;

    // Wait threads end.
    for (int i = 0; i < N; i++) {
        threads[i].join();
    }

    for (int i = 1; i <= N; i++){
        total_update_cnt += update_cnt[i];
    }
    printf("Total updates count for all threads is %d\n", total_update_cnt);

    return 0;
}
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <vector>
using namespace std;

typedef struct Record{
    long val;

    /* pair<lock_type, thread id>
     * lock_type 0 : reader lock
     * lock_type 1 : writer lock 
     */
    deque<pair<int, int> > lock_table;
    int read_active;
    int write_active;
    pthread_mutex_t mutex;
    pthread_cond_t read_cv;
    pthread_cond_t write_cv;
}record;

int ReadLock(int num, int tid, record* records);
int ReadUnlock(int num, int tid, record* records);
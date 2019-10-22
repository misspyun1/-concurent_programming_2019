#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <unordered_set>
#include <vector>
using namespace std;

typedef struct Record{
    long val;

    int read_active;
    int write_active;    
    int read_wait;
    int write_wait;
    pthread_mutex_t mutex;
    pthread_cond_t read_cv;
    pthread_cond_t write_cv;
}record;

/* num : Records Number (1 <= num <= R)
 * tid : Threads Id (1 <= tid <= N)
 * lock_table : locktable for [num]th Record
 */
int ReadLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk,  pthread_mutex_t & lock_table_mutex);
int ReadUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk,  pthread_mutex_t & lock_table_mutex);
int WriteLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk, pthread_mutex_t & lock_table_mutex);
int WriteUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk, pthread_mutex_t & lock_table_mutex);

bool DetectCycle(int num, int tid, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> > worker_chk, pthread_mutex_t & lock_table_mutex);
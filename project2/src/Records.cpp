#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <deque>
#include <queue>
#include <vector>
#include <unordered_set>
#include "Records.h"
using namespace std;

/* num : Records Number (1 <= num <= R)
 * tid : Threads Id (1 <= tid <= N)
 * lock_table : locktable for [num]th Record
 */
int ReadLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk,  pthread_mutex_t & lock_table_mutex){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    records[num].read_wait ++;
    while(records[num].write_active){
        status = pthread_cond_wait(&records[num].read_cv, &records[num].mutex);
        if(status!=0){
            break;
        }
    }
    records[num].read_wait --;

    // Add tid to lock table
    pthread_mutex_lock(&lock_table_mutex);
    lock_table[num].push_back(make_pair(0, tid));
    pthread_mutex_unlock(&lock_table_mutex);

    if(status==0){
        records[num].read_active++;
    }
    worker_chk[tid].insert(num);
    printf("Thread %d, read lock %d\n",tid,num);
    pthread_mutex_unlock(&records[num].mutex);
    return status;
}

int ReadUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk,  pthread_mutex_t & lock_table_mutex){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status!=0){
        return status;
    }
    records[num].read_active --;

    pthread_mutex_lock(&lock_table_mutex);
    // Mark [num]th lock on lock table as unlocked
    for(unsigned int i = 0;i<lock_table[num].size();i++){
        if(lock_table[num][i].second==tid){
            printf("UNLOCK thread %d from record %d\n", tid, num);
            lock_table[num][i].second = -1;
            break;
        }
    }

    // Pop deactivated locks.
    while(lock_table[num].size()>0 && lock_table[num][0].second==-1){
        lock_table[num].pop_front();
    }
    pthread_mutex_unlock(&lock_table_mutex);

    // Send signal to write lock
    if(records[num].read_active == 0 && records[num].write_wait>0){
        status  = pthread_cond_signal(&records[num].write_cv);
    }
    worker_chk[tid].erase(num);
    pthread_mutex_unlock(&records[num].mutex);

    return status;
}

int WriteLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk, pthread_mutex_t & lock_table_mutex){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    records[num].write_wait ++;
    while(records[num].write_active || records[num].read_active>0){
        status = pthread_cond_wait(&records[num].write_cv, &records[num].mutex);
        if(status!=0){
            break;
        }
    }
    records[num].write_wait --;

    pthread_mutex_lock(&lock_table_mutex);
    lock_table[num].push_back(make_pair(1, tid));
    pthread_mutex_unlock(&lock_table_mutex);

    if(status==0){
        records[num].write_active = 1;
    }
    worker_chk[tid].insert(num);
    printf("Thread %d, write lock %d\n",tid,num);
    pthread_mutex_unlock(&records[num].mutex);
    return status;  
}

int WriteUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk, pthread_mutex_t & lock_table_mutex){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status!=0){
        return status;
    }
    records[num].write_active = 0;

    pthread_mutex_lock(&lock_table_mutex);
    // Mark [num]th lock on lock table as unlocked
    for(unsigned int i = 0;i<lock_table[num].size();i++){
        if(lock_table[num][i].second==tid){
            lock_table[num][i].second = -1;
            break;
        }
    }

    // Pop deactivated locks.
    while(lock_table[num].size()>0 && lock_table[num][0].second==-1){
        lock_table[num].pop_front();
    }
    pthread_mutex_unlock(&lock_table_mutex);
    
    // Broadcast to read lock
    if(records[num].read_wait > 0 ){
        status = pthread_cond_broadcast(&records[num].read_cv);
        if(status!=0){
            worker_chk[tid].erase(num);
            pthread_mutex_unlock(&records[num].mutex);
            return status;
        }
    }else if (records[num].write_wait>0){
        // Send signal to write lock
        status  = pthread_cond_signal(&records[num].write_cv);
        if(status!=0){
            worker_chk[tid].erase(num);
            pthread_mutex_unlock(&records[num].mutex);
            return status;
        }
    }
    worker_chk[tid].erase(num);
    pthread_mutex_unlock(&records[num].mutex);

    return status;
}

/* num : Records Number (1 <= num <= R)
 * tid : Threads Id (1 <= tid <= N)
 * lock_table : locktable for [num]th Record
 */
bool DetectCycle(int num, int tid, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> > worker_chk, pthread_mutex_t & lock_table_mutex){
    // 나를 부모로 갖고 있는 부모가 있ㄴ느지 보기
    queue<int>q;
    pthread_mutex_lock(&lock_table_mutex);
    for(unsigned int i = 0; i < lock_table[num].size();i++){
        if(lock_table[num][i].second != -1){
            q.push(lock_table[num][i].second);
            worker_chk[lock_table[num][i].second].erase(num);
        }
    }
    printf("Start to Detect Cycle Record %d at Thread %d : %lu\n", num, tid, q.size());
    while(!q.empty()){
        // now : Thread ID
        int now = q.front();
        q.pop();
        printf("Thread %d, Record %d, now : %d\n",tid, num, now);
        if(now==tid){
            return true;
        }

        for(int ijk : worker_chk[now]){
            // ijk : rest of numbers [now]th thread has
            printf("Record %d at Thread %d size -> %lu\n",ijk, now, lock_table[ijk].size());
            for(unsigned int i = 0; i< lock_table[ijk].size();i++){
                printf("Parent of Record %d at Thread %d : %d\n",ijk, now, lock_table[ijk][i].second);
                if(lock_table[ijk][i].second == now){
                    break;
                }
                if(lock_table[ijk][i].second == -1){
                    continue;
                }
                q.push(lock_table[ijk][i].second);
            }
        }
        worker_chk[now].clear();

    }
    
    pthread_mutex_unlock(&lock_table_mutex);
    return false;
}
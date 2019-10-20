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
int ReadLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    while(records[num].write_active){
        status = pthread_cond_wait(&records[num].read_cv, &records[num].mutex);
        if(status){
            break;
        }
    }

    // Add tid to lock table
    lock_table[num].push_back(make_pair(0, tid));

    if(status==0){
        records[num].read_active ++;
    }
    worker_chk[tid].insert(num);
    pthread_mutex_unlock(&records[num].mutex);
    return status;
}

int ReadUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status){
        return status;
    }
    records[num].read_active --;

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

    // Send signal to write lock
    if(records[num].read_active == 0 && records[num].write_active>0){
        status  = pthread_cond_signal(&records[num].write_cv);
    }
    worker_chk[tid].erase(num);
    pthread_mutex_unlock(&records[num].mutex);

    return status;
}

int WriteLock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    while(records[num].write_active || records[num].read_active){
        status = pthread_cond_wait(&records[num].write_cv, &records[num].mutex);
        if(status){
            break;
        }
    }

    lock_table[num].push_back(make_pair(1, tid));

    if(status==0){
        records[num].write_active = 1;
    }
    worker_chk[tid].insert(num);
    pthread_mutex_unlock(&records[num].mutex);
    return status;  
}

int WriteUnlock(int num, int tid, record* records, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> >& worker_chk){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status){
        return status;
    }
    records[num].write_active = 0;

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
    
    // Broadcast to read lock
    if(records[num].read_active > 0 ){
        status = pthread_cond_broadcast(&records[num].read_cv);
        if(status){
            pthread_mutex_unlock(&records[num].mutex);
            return status;
        }
    }
    
    // Send signal to write lock
    if (records[num].write_active>0){
        status  = pthread_cond_signal(&records[num].write_cv);
        if(status){
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
bool DetectCycle(int num, int tid, vector<deque<pair<int, int> > > & lock_table, vector<unordered_set<int> > worker_chk){
    // 나를 부모로 갖고 있는 부모가 있ㄴ느지 보기
    queue<int>q;
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
    return false;
}
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

int ReadLock(int num, int tid, record* records){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    while(records[num].write_active){
        status = pthread_cond_wait(&records[num].read_cv, &records[num].mutex);
        if(status){
            break;
        }
    }

    // Add tid to lock table
    records[num].lock_table.push_back(make_pair(0, tid));

    if(status==0){
        records[num].read_active ++;
    }
    pthread_mutex_unlock(&records[num].mutex);
    return status;
}

int ReadUnlock(int num, int tid, record* records){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status){
        return status;
    }
    records[num].read_active --;

    // Mark [num]th lock on lock table as unlocked
    for(unsigned int i = 0;i<records[num].lock_table.size();i++){
        if(records[num].lock_table[i].second==tid){
            records[num].lock_table[i].second = -1;
            break;
        }
    }

    // Pop deactivated locks.
    while(records[num].lock_table.size()>0 && records[num].lock_table[0].second==-1){
        records[num].lock_table.pop_front();
    }

    // Send signal to write lock
    if(records[num].read_active == 0 && records[num].write_active>0){
        status  = pthread_cond_signal(&records[num].write_cv);
    }
    pthread_mutex_unlock(&records[num].mutex);

    return status;
}

int WriteLock(int num, int tid, record* records){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    while(records[num].write_active || records[num].read_active){
        status = pthread_cond_wait(&records[num].write_cv, &records[num].mutex);
        if(status){
            break;
        }
    }

    records[num].lock_table.push_back(make_pair(1, tid));

    if(status==0){
        records[num].write_active = 1;
    }
    pthread_mutex_unlock(&records[num].mutex);
    return status;  
}

int WriteUnlock(int num, int tid, record* records){
    int status = 0;
    status = pthread_mutex_lock(&records[num].mutex);
    if(status){
        return status;
    }
    records[num].write_active = 0;

    // Mark [num]th lock on lock table as unlocked
    for(unsigned int i = 0;i<records[num].lock_table.size();i++){
        if(records[num].lock_table[i].second==tid){
            records[num].lock_table[i].second = -1;
            break;
        }
    }

    // Pop deactivated locks.
    while(records[num].lock_table.size()>0 && records[num].lock_table[0].second==-1){
        records[num].lock_table.pop_front();
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
    pthread_mutex_unlock(&records[num].mutex);

    return status;
}

bool DetectCycle(){
    return false;
}
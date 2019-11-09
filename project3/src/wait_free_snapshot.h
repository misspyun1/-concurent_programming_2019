#ifndef WAITFREESNAPSHOT_H
#define WAITFREESNAPSHOT_H

#include <vector>
#include <cstdlib>
using namespace std;

typedef struct Snapshot{
    int* value;
    int* stamp;
    int length;

    Snapshot(int N){
        length = N;
        value = (int*)malloc(sizeof(int)*N);
        stamp = (int*)malloc(sizeof(int)*N);
    }

}Snapshot;

class WaitFreeSnapshot{
public:
    Snapshot* snaps;
    int snap_size;

    WaitFreeSnapshot(){
        snap_size = 0;
    }

    void init(int N){
        snap_size = N;
        snaps = new Snapshot(N);
    }

    void update(int value, int tid){
        Snapshot* snapshot = scan();
        snaps->stamp[tid] = snapshot->stamp[tid]+1;
        snaps->value[tid] = value;
    }

    Snapshot* scan(){
        Snapshot* old_snaps;
        Snapshot* new_snaps;
        vector<bool>moved(snap_size, false);

        old_snaps = collect();

        while(true){
            new_snaps = collect();
            bool is_moved = false;
            for(int i = 0; i < snap_size; i++){
                if(old_snaps->stamp[i] != new_snaps->stamp[i]){
                    if(moved[i]){
                        return old_snaps;
                    }else{
                        is_moved = true;
                        moved[i] = true;
                        old_snaps = new_snaps;
                        break;
                    }
                }
            }
            if(!is_moved){
                break;
            }
        }
        return new_snaps;
    }

    // 
    Snapshot* collect(){
        Snapshot* res = new Snapshot(snap_size);
        for(int i = 0; i < snap_size; i++){
            res->value[i] = snaps->value[i];
            res->stamp[i] = snaps->stamp[i];
        }
        return res;
    }

};

#endif
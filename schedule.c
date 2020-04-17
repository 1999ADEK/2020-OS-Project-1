#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>
#include "process.h"
#include "schedule.h"

// initialization
int cur_t = 0; // current time
int num_fin = 0; // number of finish process
int last_p = -1; // index of last executed process, -1 implies none
int last_t; // the last time at which context switch happens, used for RR policy

int cmp(const void *a, const void *b){
    process *p1 = (process *)a;
    process *p2 = (process *)b;
    if (p1->ready_t < p2->ready_t)
        return -1;
    else if (p1->ready_t > p2->ready_t)
        return 1;
    else
        return 0;
}

// find next process to execute
int sched_next(process *P, int num_p, int policy){
    // can't choose different process if policy is non-preemptive
    if (last_p != -1 && (policy == FIFO || policy == SJF)){
        return last_p;
    }

    int cur_p = -1;
    // SJF and PSJF
    if (policy == SJF || policy == PSJF){
        for (int i = 0; i < num_p; i++){
            // skip processes that are not ready or are finished
            if (P[i].pid == -1 || P[i].exec_t == 0){
                continue;
            }
            if (cur_p == -1 || P[i].exec_t < P[cur_p].exec_t){
                cur_p = i;
            }
        }
    }
    // FIFO
    else if (policy == FIFO){
        for (int i = 0; i < num_p; i++){
            // since P is sorted by ready time, the first ready process
            // that is not finshed will be the one to choose
            if (P[i].pid != -1 && P[i].exec_t != 0){
                cur_p = i;
                break;
            }
        }
    }

    // RR
    else{
        // choose one process to execute if none is being execute
        if (last_p == -1){
            for (int i = 0; i < num_p; i++){
                if (P[i].pid != -1 && P[i].exec_t != 0){
                    cur_p = i;
                    break;
                }
            }
        }
        // switch process if one round of execution is finished
        else if ((cur_t - last_t) % TIME_QUAN == 0){
            for (int i = 1; i < num_p + 1; i++){
                if (P[(last_p+i) % num_p].pid != -1 && P[(last_p+i) % num_p].exec_t != 0){
                    cur_p = (last_p+i) % num_p;
                    break;
                }
            }
        }
        else{
            cur_p = last_p;
        }
    }

    return cur_p;
}

// schedule processes based on given policy
void scheduler(process *P, int num_p, int policy){
    // assign scheduler to specific core and set it to
    // have high priority
    proc_assign_cpu(getpid(), SCHED_CPU);
    proc_wake(getpid());

    // initialize pid to -1, which implies the process
    // is not ready
    for (int i = 0; i < num_p; i++){
        P[i].pid = -1;
    }
    // sort the process list based on ready time
    qsort(P, num_p, sizeof(process), cmp);

    // start schedulling
    while (1){
        // put ready process into waiting line
        for (int i = 0; i < num_p; i++){
            if (P[i].ready_t == cur_t){
                P[i].pid = proc_exec(&P[i]);
                proc_blck(P[i].pid);
#ifdef DEBUG
                fprintf(stderr, "%s ready at time %d\n", P[i].name, cur_t);
#endif
            }
        }

        // find next process to execute
        int cur_p = sched_next(P, num_p, policy);
        // context switch if neccessary
        if (last_p != cur_p){
            if (last_p != -1){
                proc_blck(P[last_p].pid);
            }
            if (cur_p != -1){
                proc_wake(P[cur_p].pid);
                last_t = cur_t;
            }
        }

        // execute for a unit of time
        unit_t();

        if (cur_p != -1){
            P[cur_p].exec_t--;
            // check if the current process is finished
            if (P[cur_p].exec_t == 0){
                // output process info
                waitpid(P[cur_p].pid, NULL, 0);
#ifdef DEBUG
                fprintf(stderr, "%s is finished at time %d\n", P[cur_p].name, cur_t);
#endif
                printf("%s %d\n", P[cur_p].name, P[cur_p].pid);
                cur_p = -1;
                num_fin++;
                // end scheduling if all processes are done
                if (num_fin == num_p){
                    return;
                }
            }
        }

        // update information
        cur_t++;
        last_p = cur_p;
    }
}
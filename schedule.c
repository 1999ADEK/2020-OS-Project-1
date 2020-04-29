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
int ready_idx = 0; // index of process list to check if the process is ready
process *last_p = NULL; // last executed process
int last_t; // the last time at which context switch happens, used for RR policy

// compare 2 processes based on ready time and execution time
// so that process with earlier ready time comes first in a list
// if 2 processes have the same ready time, choose the one with
// shorter execution time
int cmp(const void *a, const void *b){
    process *p1 = (process *)a;
    process *p2 = (process *)b;
    if (p1->ready_t < p2->ready_t)
        return -1;
    else if (p1->ready_t > p2->ready_t)
        return 1;
    else if (p1->exec_t < p2->exec_t)
        return -1;
    else if (p1->exec_t > p2->exec_t)
        return 1;
    else
        return 0;
}

// insert a process to ready queue based on different policies
void insert_to_list(process_list *queue, process_node *new, int policy){
    if (queue->head == NULL){
        queue->head = new;
        queue->tail = new;
        return;
        }
    if (policy == FIFO || policy == RR){
        queue->tail->next = new;
        queue->tail = new;
        return;
    }
    else{
        if (policy == PSJF && new->p->exec_t < queue->head->p->exec_t){
            new->next = queue->head;
            queue->head = new;
            return;
        }
        process_node *pos = queue->head;
        while(pos->next != NULL && pos->next->p->exec_t <= new->p->exec_t){
            pos = pos->next;
        }
        if (pos->next == NULL){
            pos->next = new;
            queue->tail = new;
        }
        else{
            new->next = pos->next;
            pos->next = new;
        } 
    }
}

// remove the first node in ready queue and return the node
void remove_from_list(process_list *queue){
    queue->head = queue->head->next;
}

// find next process to execute
process *sched_next(process_list *queue, int policy){
    if (queue->head == NULL){
        return NULL;
    }
    // non-preemptive
    if (policy == FIFO || policy == SJF){
        // can't choose different process if policy is non-preemptive
        if (last_p != NULL){
            return last_p;
        }
        else{
            return queue->head->p;
        }
    }
    // PSJF
    else if (policy == PSJF){
        return queue->head->p;
    }
    // RR
    else{
        // choose one process to execute if none is being execute
        if (last_p == NULL){
            return queue->head->p;
        }
        // switch process if one round of execution is finished
        else if ((cur_t - last_t) % TIME_QUAN == 0){
            if (queue->head == queue->tail){
                return queue->head->p;
            }
            return queue->head->next->p;
        }
        else{
            return last_p;
        }
    }
}

void print_queue(process_list *queue){
    process_node *pos = queue->head;
    while(pos != NULL){
        fprintf(stderr, "%s ->", pos->p->name);
        pos = pos->next;
    }
    fprintf(stderr, "\n");
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

    // create a process queue
    process_list *ready_queue = malloc(sizeof(process_list));
    ready_queue->head = NULL;
    ready_queue->tail = NULL;

    // start schedulling
    while (1){
        // put ready process into waiting line
        while (ready_idx < num_p && P[ready_idx].ready_t == cur_t){
            process_node *new = malloc(sizeof(process_node));
            new->p = &P[ready_idx];
            new->next = NULL;
            insert_to_list(ready_queue, new, policy);
            P[ready_idx].pid = proc_exec(&P[ready_idx]);
            proc_blck(P[ready_idx].pid);
#ifdef DEBUG
                fprintf(stderr, "%s ready at time %d\n", P[ready_idx].name, cur_t);
                print_queue(ready_queue);
#endif
        ready_idx++;
        }

        // find next process to execute
        process *cur_p = sched_next(ready_queue, policy);
        // context switch if neccessary
        if (last_p != cur_p){
            if (last_p != NULL){
                proc_blck(last_p->pid);
                if (policy == RR){
                    process_node *new = malloc(sizeof(process_node));
                    new->p = ready_queue->head->p;
                    new->next = NULL;
                    remove_from_list(ready_queue);
                    insert_to_list(ready_queue, new, policy);
                }
            }
            if (cur_p != NULL){
#ifdef DEBUG
                fprintf(stderr, "cur_p is %s at time %d\n", cur_p->name, cur_t);
#endif
                proc_wake(cur_p->pid);
                last_t = cur_t;
            }
        }

        // execute for a unit of time
        unit_t();

        if (cur_p != NULL){
            cur_p->exec_t--;
            // check if the current process is finished
            if (cur_p->exec_t == 0){
                // remove it from the ready queue
                remove_from_list(ready_queue);
                // output process info
                waitpid(cur_p->pid, NULL, 0);
#ifdef DEBUG
                fprintf(stderr, "%s is finished at time %d\n", cur_p->name, cur_t);
                print_queue(ready_queue);
#endif
                printf("%s %d\n", cur_p->name, cur_p->pid);
                cur_p = NULL;
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
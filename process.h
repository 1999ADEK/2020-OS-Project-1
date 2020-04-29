#ifndef _PROCESS_H_
#define _PROCESS_H_
#include <sys/types.h>
#define PROC_CPU 1
#define SCHED_CPU 0
#define GET_TIME 333
#define SHOW_INFO 334

typedef struct {
    pid_t pid; // pid of the process
    char name[32]; // name of the process
    int exec_t; // the amount of time the process still need to be executed
    int ready_t; // the time the process is ready
} process;

typedef struct node {
    process *p;
    struct node *next;
} process_node;

typedef struct{
    process_node *head;
    process_node *tail;
} process_list;

void unit_t();
void proc_assign_cpu(pid_t pid, int cpu);
pid_t proc_exec(process *p);
void proc_blck(pid_t pid);
void proc_wake(pid_t pid);
#endif
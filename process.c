#define _GNU_SOURCE
#include <sched.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "process.h"

void unit_t(){
    volatile unsigned long i; 
    for(i = 0; i < 1000000UL; i++);
}

// assign a process to specific cpu
void proc_assign_cpu(pid_t pid, int cpu){
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    sched_setaffinity(pid, sizeof(cpu_set_t), &set);
}

// execute a process
pid_t proc_exec(process *p){
    pid_t pid;
    pid = fork();

    // child process
    if (pid == 0){
        // assign to specific cpu
        proc_assign_cpu(getpid(), PROC_CPU);
        
        char output[50]; // output from dmesg
        unsigned long st_sec, st_nsec; // start time
        unsigned long ft_sec, ft_nsec; // end time

        // get start time of this process
        syscall(GET_TIME, &st_sec, &st_nsec);

        // execute this process
        for (int t = 0; t < p->exec_t; t++){
            unit_t();
#ifdef DEBUG
            if (t % 100 == 0){
                fprintf(stderr, "%s: %d/%d\n", p->name, t, p->exec_t);
            }
#endif
        }

        // get end time of this process
        syscall(GET_TIME, &ft_sec, &ft_nsec);

        // output from dmesg
        sprintf(output, "[Project1] %d %lu.%09lu %lu.%09lu\n", getpid(), st_sec, st_nsec, ft_sec,ft_nsec);
        syscall(SHOW_INFO, output);

        exit(0);
    }
    // parent process
    else if (pid > 0){
        return pid; // child's pid
    }
    else{
        perror("fork()");
        return -1;
        // exit(-1);
    }
}

void proc_blck(pid_t pid){
    // block a process
    struct sched_param param;
    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_IDLE, &param);
    if (ret < 0){
        perror("proc_blck()");
        exit(-1);
    }
    return;
}

void proc_wake(pid_t pid){
    // make a process of top priority to execute
    struct sched_param param;
    param.sched_priority = 0;
    int ret = sched_setscheduler(pid, SCHED_OTHER, &param);
    if (ret < 0){
        perror("proc_wake()");
        exit(-1);
    }
    return;
}
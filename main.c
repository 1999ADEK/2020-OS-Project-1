#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "schedule.h"

int main(){
    char sched_policy[5];
    int policy;

    scanf("%s", sched_policy);

    // determine scheduling policy
    if (strcmp(sched_policy, "FIFO") == 0)
        policy = FIFO;
    else if (strcmp(sched_policy, "RR") == 0)
        policy = RR;
    else if (strcmp(sched_policy, "SJF") == 0)
        policy = SJF;
    else if (strcmp(sched_policy, "PSJF") == 0)
        policy = PSJF;
    else{
        printf("Invalid policy.\n");
        return 0;
    }

    // construct process list
    int num_p;
    scanf("%d", &num_p);
    process P[num_p];
    for (int i = 0; i < num_p; i++){
        scanf("%s%d%d", P[i].name, &P[i].ready_t, &P[i].exec_t);
    }

    // start scheduling
    scheduler(P, num_p, policy);

    return 0;
}
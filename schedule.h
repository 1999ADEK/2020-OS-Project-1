#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_
#include "process.h"

#define FIFO 0
#define RR   1
#define SJF  2
#define PSJF 3
#define TIME_QUAN 500

int cmp(const void *a, const void *b);
void insert_to_list(process_list *queue, process_node *new, int policy);
void remove_from_list(process_list *queue);
process *sched_next(process_list *queue, int policy);
void scheduler(process *P, int num_p, int policy);
#endif
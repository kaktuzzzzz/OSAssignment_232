
#include "queue.h"
#include "sched.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
// static unsigned long prev_prio;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
	// prev_prio = 0;
	int i;

	for (i = 0; i < MAX_PRIO; i++){
		mlq_ready_queue[i].size = 0;
		mlq_ready_queue[i].slot = MAX_PRIO - i;
	}
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t *get_mlq_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 */

    pthread_mutex_lock(&queue_lock);
	// if(!empty(&mlq_ready_queue[prev_prio])&&(mlq_ready_queue[prev_prio].slot>0)){
	// 	proc = dequeue(&mlq_ready_queue[prev_prio]);
	// 	mlq_ready_queue[prev_prio].slot -= 1;
	// }
	// else{
    unsigned long prio;
	// bool flag = false;
    for (prio = 0; prio < MAX_PRIO; prio++) {
        if (empty(&mlq_ready_queue[prio])||mlq_ready_queue[prio].slot<=0) {
            continue;
        }
        proc = dequeue(&mlq_ready_queue[prio]);
        if (proc != NULL) {
			mlq_ready_queue[prio].slot -= 1;
			// prev_prio = prio;
			// flag = true;
			// printf("get process %u from queue %ld - curr_slot of queue: %d\n",proc->pid,prio,mlq_ready_queue[prio].slot);
            break;
        }

		
    }
	//if proc not found, allocate the slot and find again
	if(proc == NULL){
		unsigned long p;
		for(int i = 0; i < MAX_PRIO; i++){
			mlq_ready_queue[i].slot = MAX_PRIO - i;
		}
	 	for(p =0 ; p<MAX_PRIO; p++){
			if(empty(&mlq_ready_queue[p])||mlq_ready_queue[p].slot <= 0){
				continue;
			}
			proc = dequeue(&mlq_ready_queue[p]);
			if (proc != NULL) {
				mlq_ready_queue[p].slot -= 1;
				// printf("get process %u from queue %ld - curr_slot of queue: %d\n",proc->pid,p,mlq_ready_queue[p].slot);
			// prev_prio = prio;
            	break;
        	}
		}
	}
	// }
    pthread_mutex_unlock(&queue_lock);

    return proc;
}

void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	return add_mlq_proc(proc);
}
#else
struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if(!empty(&ready_queue)){
		proc = dequeue(&ready_queue);
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif

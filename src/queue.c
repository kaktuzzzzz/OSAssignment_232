#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if(q->size>=MAX_QUEUE_SIZE){
                return;
        }
        // q->size++;
        // for(int i =q->size-1; i>0; i++){
        //         q->proc[i] = q->proc[i-1];
        // }
        // q->proc[0] = proc;

        int t = q->size;
        q->proc[t] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if(q->size == 0){
                return NULL;
        }
        int highest_prio_index = 0;
        for(int i = 1; i < q->size; i++){
                if(q->proc[i]->priority < q->proc[highest_prio_index]->priority){
                        highest_prio_index = i;
                }
        }
        struct pcb_t * cur = q->proc[highest_prio_index];
        for(int i = highest_prio_index; i< (q->size - 1); i++){
                q->proc[i] = q->proc[i+1];
        }
        // struct pcb_t * cur = q->proc[0];
        // for(int i = 0; i< (q->size-1); i++){
        //         q->proc[i]= q->proc[i+1];
        // }
        q->size--;
        return cur;
        
}


#ifndef _RT_H
#define _RT_H

#include "sched.h"

#define RR_TIMESLICE        1

/* task의 priority가 valid 한지 확인*/
static inline int rt_prio(int prio)
{
	if (prio < MAX_RT_RPIO)
		return 1;
	return 0;
}

struct rq *rq_of_rt_se(struct sched_rt_entity *rt_se);
struct rt_rq *rt_rq_of_se(struct sched_rt_entity *rt_se);
void update_curr_rt(struct rq *rq);
void enqueue_rt_entity(struct sched_rt_entity *rt_se, int head);
struct sched_rt_entity *pick_next_rt_entity(struct rt_rq *rt_rq);
struct task_struct *_pick_next_task_rt(struct rq *rq);
struct task_struct *pick_next_task_rt(struct rq *rq, struct task_struct *prev);
void requeue_task_rt(struct rq *rq, struct task_struct *p, int head);
void task_tick_rt(struct rq *rq, struct task_struct *p);
void __delist_rt_entity(struct sched_rt_entity *rt_se, struct rt_prio_array *array);
void dequeue_rt_entity(struct sched_rt_entity *rt_se);


#endif
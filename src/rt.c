/***********************************************************
 * rt.c 파일의 함수들은 linux kernel의 kernel/sched/rt.c 파일의 함수들을 이용 
 * **********************************************************/

#include "ops.h"
#include "rt.h"
#include "list.h"

/* sched_rt_entity를 이용하여 task_struct를 return */
static inline struct task_struct *rt_task_of(struct sched_rt_entity *rt_se)
{
    return container_of(rt_se, struct task_struct, rt);
}

/* rt_rq를 이용하여 rq를 return */
static inline struct rq *rq_of_rt_rq(struct rt_rq *rt_rq)
{
	return container_of(rt_rq, struct rq, rt);
}

/* sched_rt_entity를 이용하여 rq를 return */
struct rq *rq_of_rt_se(struct sched_rt_entity *rt_se)
{
	struct rt_rq *rt_rq = rt_se->rt_rq;

	return rq_of_rt_rq(rt_rq);
}

/* sched_rt_entity를 이용하여 rt_rq를 return */
struct rt_rq *rt_rq_of_se(struct sched_rt_entity *rt_se) 
{
   return rt_se->rt_rq;
}

/* rt_se entity의 priority를 return */
static inline int rt_se_prio(struct sched_rt_entity *rt_se) 
{
    return rt_task_of(rt_se)->priority;
}

static inline void inc_rt_tasks(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
    int prio = rt_se_prio(rt_se);

    if(!rt_prio(prio))
        return;

    rt_rq->rt_nr_running += 1;
}

/* 실행중인 task 수를 감소 */
static inline void dec_rt_tasks(struct sched_rt_entity *rt_se, struct rt_rq *rt_rq)
{
    if(!rt_prio(rt_se_prio(rt_se)))
        return;

    if(!rt_rq->rt_nr_running)
        return;

    rt_rq->rt_nr_running -= 1;
}

/* 현재 실행 중인 task의 실행시간을 update */
void update_curr_rt(struct rq *rq) 
{
    struct task_struct *curr = rq->curr;
    unsigned long long delta_exec;
    unsigned long long now = rq->clock_task;
    struct rt_rq *rt_rq = &rq->rt;

    delta_exec = now - curr->se.exec_start;             //실행시간 계산
    if((signed long long)delta_exec <= 0)              
        return;

    curr->se.sum_exec_runtime += delta_exec;            //전체 실행시간에 더함

    curr->se.exec_start = now;                          //실행 시작 시간을 현재 시간으로 변경

    rt_rq->rt_time += delta_exec;                       //rt task의 실행시간을 누적 

    return;
}

/* sched_rt_entity를 해당 priority의 queue에 enqueue함 */
void enqueue_rt_entity(struct sched_rt_entity *rt_se, int head) 
{
    struct rt_rq *rt_rq = rt_rq_of_se(rt_se);
    struct rt_prio_array *array = &rt_rq->active;
    struct list_head *queue = array->queue + rt_se_prio(rt_se);

    if(rt_se->on_list)
        return;

    if(head)    
        list_add(&rt_se->run_list, queue);
    else 
        list_add_tail(&rt_se->run_list, queue);

    __set_bit(rt_se_prio(rt_se), array->bitmap);    //priority의 bit를 1로 set
    
    rt_se->on_list = 1;
    rt_se->on_rq = 1;

    inc_rt_tasks(rt_se, rt_rq);
}

void __delist_rt_entity(struct sched_rt_entity *rt_se, struct rt_prio_array *array)
{
    list_del_init(&rt_se->run_list);

    if(list_empty(array->queue + rt_se_prio(rt_se)))
        __clear_bit(rt_se_prio(rt_se), array->bitmap);

    rt_se->on_list = 0;
}

/* sched_rt_entity를 해당 priority의 queue에서 dequeue함 */
void dequeue_rt_entity(struct sched_rt_entity *rt_se)
{
    struct rt_rq *rt_rq = rt_rq_of_se(rt_se);
    struct rt_prio_array *array = &rt_rq->active;

    if(!rt_se->on_list) 
        return;

    __delist_rt_entity(rt_se, array);
    rt_se->on_rq = 0;

    dec_rt_tasks(rt_se, rt_rq);
}

/* 다음으로 수행할 rt_entity를 pick */
struct sched_rt_entity *pick_next_rt_entity(struct rt_rq *rt_rq) 
{
    struct rt_prio_array *array = &rt_rq->active;
    struct sched_rt_entity *next = NULL;
    struct list_head *queue;
    int idx;

    idx = sched_find_first_bit(array->bitmap);
    if(idx >= MAX_RT_RPIO) 
        return NULL;

    queue = array->queue + idx;
    next = list_entry(queue->next, struct sched_rt_entity, run_list);

    return next;
}

/* 다음으로 수행할 task를 pick */
struct task_struct *_pick_next_task_rt(struct rq *rq) 
{
    struct sched_rt_entity *rt_se;
    struct task_struct *p;
    struct rt_rq *rt_rq = &rq->rt;
  
    rt_se = pick_next_rt_entity(rt_rq);
    if(!rt_se)
        return NULL;

    p = rt_task_of(rt_se);
   
    p->se.exec_start = rq->clock_task;

    return p;
}

struct task_struct *pick_next_task_rt(struct rq *rq, struct task_struct *prev)
{
    struct task_struct *p;

    update_rq_clock(rq);
    update_curr_rt(rq);

    p = _pick_next_task_rt(rq);

    return p;
}

/* p에 해당하는 task를 list의 head or tail로 옮김 */
void requeue_task_rt(struct rq *rq, struct task_struct *p, int head)
{
    struct sched_rt_entity *rt_se = &p->rt;
    struct rt_rq *rt_rq = &rq->rt;

    if(rt_se->on_rq) {
        struct rt_prio_array *array = &rt_rq->active;
        struct list_head *queue = array->queue + rt_se_prio(rt_se);


        //task의 entry를 list의 head/tail로 이동
        if(head)
            list_move(&rt_se->run_list, queue);
        else 
            list_move_tail(&rt_se->run_list, queue);
    }
}

/* timer_tick 발생시마다 수행하는 함수 */
void task_tick_rt(struct rq *rq, struct task_struct *p)
{
    struct sched_rt_entity *rt_se = &p->rt;

    update_rq_clock(rq);
    update_curr_rt(rq);

    if(p->policy != SCHED_RR)       //SCHED_FIFO인 경우 리턴
        return;

    if(--p->rt.time_slice)           //time_slice를 1감소시키고 0보다 큰 경우 리턴 
        return;

    p->rt.time_slice = RR_TIMESLICE * p->weight;      //timeslice를 setting

    /* 같은 우선순위 리스트에 복수개의 sched_entity가 있는 경우 */
    if(rt_se->run_list.prev != rt_se->run_list.next) {       
        requeue_task_rt(rq, p, 0);      //p를 list의 tail로 move
        resched_curr(rq);               //current task에 리스케쥴 flag를 set
        return;
    }
}
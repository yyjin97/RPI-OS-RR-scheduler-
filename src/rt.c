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

void enqueue_rt_entity(struct sched_rt_entity *rt_se, int head) 
{
    struct rt_rq *rt_rq = rt_rq_of_se(rt_se);
    struct rt_prio_array *array = &rt_rq->active;
    struct list_head *queue = array->queue + rt_se_prio(rt_se);

    if(!test_bit(rt_se_prio(rt_se), array->bitmap))
        INIT_LIST_HEAD(queue);

    if(head)    
        list_add(&rt_se->run_list, queue);
    else 
        list_add_tail(&rt_se->run_list, queue);

    __set_bit(rt_se_prio(rt_se), array->bitmap);    //priority의 bit를 1로 set
    rt_se->on_rq = 1;
}

struct sched_rt_entity *pick_next_rt_entity(struct rt_rq *rt_rq) 
{
    struct rt_prio_array *array = &rt_rq->active;
    struct sched_rt_entity *next = NULL;
    struct list_head *queue;
    int idx;

    idx = sched_find_first_bit(array->bitmap);
    if(idx >= MAX_RT_RPIO) 
        return NULL;

    queue = array->queue + idx; //////////////
    next = list_entry(queue->next, struct sched_rt_entity, run_list);

    return next;
}

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

    //put_prev_task!!!!!!!!

    p = _pick_next_task_rt(rq);

    return p;
}

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
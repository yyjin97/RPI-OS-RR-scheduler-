#ifndef _SCHED_H
#define _SCHED_H

#define THREAD_CPU_CONTEXT			0 		// offset of cpu_context in task_struct 

#ifndef __ASSEMBLER__

#include "list.h"

#define THREAD_SIZE				4096

#define NR_TASKS				64 

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#define TASK_RUNNING			0X00
#define TASK_DEAD 				0X08

#define MAX_RT_RPIO     		100

/*
 * scheduling policy 
 */
#define SCHED_NORMAL	0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_BATCH		3

/*
 * thread flags
 */
#define TIF_NEED_RESCHED		0x01	

#define current_thread_info() ((struct thread_info *)current)

extern struct task_struct *current;
extern struct task_struct * task[NR_TASKS];
extern int nr_tasks;
struct task_struct;

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

/*************schdeuler entity 구조체 ********************/
struct sched_entity {
	unsigned long long 		exec_start;
	unsigned long long 		sum_exec_runtime;
	unsigned long long 		prev_sum_exec_runtime;

	struct cfs_rq 			*cfs_rq;
};

struct sched_rt_entity {
    struct list_head        run_list;
	unsigned int 			time_slice;
	unsigned short 			on_rq;
	unsigned short 			on_list;

	struct rt_rq 			*rt_rq;
};

struct sched_dl_entity {
	//Deadline scheduler 구현 X
	int dummy;
};
/******************************************************/

struct thread_info {
    struct cpu_context  cpu_context;        //linux kernel에서는 thread_struct 구조체에 존재
    unsigned long       flags;
    int                 preempt_count;
};

struct task_struct {
	struct thread_info thread_info;
	long state;	
	long priority;
	long weight;

	unsigned int policy;
	unsigned int pid;

	struct sched_entity 		se;
	struct sched_rt_entity 		rt;
	struct sched_dl_entity 		dl;
};

//RT scheduler 
struct rt_prio_array {
    DECLARE_BITMAP(bitmap, MAX_RT_RPIO+1);
    struct list_head queue[MAX_RT_RPIO];
};


/*******************scheduler runqueue 구조체 *******************/
struct cfs_rq {
	//CFS scheduler 구현 X
	int dummy;
};

struct rt_rq {
    struct rt_prio_array active;
    unsigned int rt_nr_running;

	unsigned long long rt_time;
};

struct dl_rq {
	//Deadline scheduler 구현 X
	int dummy;
};

struct rq {
	struct cfs_rq cfs;
	struct rt_rq rt;
	struct dl_rq dl;

	struct task_struct *curr;

	unsigned long long clock_task;
};
/*********************************************************/

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct task_struct* next);
extern void cpu_switch_to(struct task_struct* prev, struct task_struct* next);
extern void resched_curr(struct rq *rq);
extern void update_rq_clock(struct rq *rq);
extern void exit_process();

/* thread_info */
static inline void set_ti_thread_flag(struct thread_info *ti, int flag)
{
    ti->flags = (unsigned long)flag;
}

static inline void clear_ti_thread_flag(struct thread_info *ti, int flag)
{
    ti->flags = 0;
}

static inline int test_ti_thread_flag(struct thread_info *ti, int flag)
{
    return ti->flags == (unsigned long)flag ? 1 : 0;
}

static inline struct thread_info *task_thread_info(struct task_struct *task)
{
	return &task->thread_info;
}

/* task_struct */
static inline int test_tsk_need_resched(struct task_struct *tsk)
{
	return test_ti_thread_flag(task_thread_info(tsk), TIF_NEED_RESCHED);
}

static inline void set_tsk_need_resched(struct task_struct *tsk)
{
	set_ti_thread_flag(task_thread_info(tsk), TIF_NEED_RESCHED);
}

static inline void clear_tsk_need_resched(struct task_struct *tsk)
{
	clear_ti_thread_flag(task_thread_info(tsk), TIF_NEED_RESCHED);
}

#define INIT_TASK \
/*cpu_context*/	{{ {0,0,0,0,0,0,0,0,0,0,0,0,0},0,0}, \
/* state etc */	0,20,0,SCHED_RR,0, {0,0,0,0},{{0,0},0,0,0},{0} \
}


#endif
#endif

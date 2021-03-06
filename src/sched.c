#include "irq.h"
#include "printf.h"
#include "rt.h"
#include "timer.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
//struct task_struct * task[NR_TASKS] = {&(init_task), };
static struct rq rq;
int nr_tasks = 1;

bool need_resched(void) 
{
	return test_ti_thread_flag(current_thread_info(), TIF_NEED_RESCHED);
}

/* rq 구조체의 clock_task 값을 현재 clock으로 update */
void update_rq_clock(struct rq *rq)
{
	long long delta;

	delta = timer_clock() - rq->clock_task;
	if(delta < 0)
		return;
	rq->clock_task += delta;
}

/* 현재 실행 중인 task의 thread_info구조체의 flag를 TIF_NEED_RESCHED로 set 
 * flag가 TIF_NEED_RESCHED로 set되어있는 경우에민 reschedule 됨 */
void resched_curr(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	
	if(test_tsk_need_resched(curr))
		return;

	set_tsk_need_resched(curr);

	return;
}

void sched_init(void)
{
	struct rt_prio_array *array;

	current->rt.rt_rq = &rq.rt;
	current->rt.time_slice = RR_TIMESLICE;

	array = &rq.rt.active;
	for(int i = 0; i < MAX_RT_RPIO; i++) {
		INIT_LIST_HEAD(array->queue + i);
		__clear_bit(i, array->bitmap);
	}
	
	rq.clock_task = timer_clock();

	rq.curr = current;
	rq.rt.rt_nr_running = 0;
	rq.rt.rt_time = 0;

	enqueue_rt_entity(&current->rt, 1);
}

void preempt_disable(void)
{
	current->thread_info.preempt_count++;
}

void preempt_enable(void)
{
	current->thread_info.preempt_count--;
}

void _schedule(void)
{
	preempt_disable();
	struct task_struct *next;
	struct task_struct *prev;

	prev = current;

	next = pick_next_task_rt(&rq, prev);
	clear_tsk_need_resched(prev);

	switch_to(next);
	preempt_enable();	
}

void schedule(void)
{
	requeue_task_rt(&rq, current, 0); 
	_schedule();
}

void switch_to(struct task_struct * next) 
{
	if (current == next) 
		return;
	struct task_struct * prev = current;
	current = next;
	rq.curr = current;
	cpu_switch_to(prev, next);
}

void schedule_tail(void) {
	preempt_enable();
}

static int p[4];

void timer_tick()
{
	if(current->thread_info.preempt_count > 0)
		return;

	task_tick_rt(rq_of_rt_se(&current->rt), current);
	enable_irq();
	if(need_resched()) {		//TIF_NEED_RESCHED가 set된 경우 reschedule됨

		int total = timer_clock();
		int pid = current->pid;
		p[pid - 1] = current->se.sum_exec_runtime;

		//process별 실행 시간 정보 출력 
		printf("\n\r p1: %d(%d%%)   p2: %d(%d%%)   p3: %d(%d%%)   p4: %d(%d%%)   total: %d \n\r",
			p[0], (p[0]*100)/total, p[1], (p[1]*100)/total, p[2], (p[2]*100)/total, p[3], (p[3]*100)/total, total);
		_schedule();
	}
	disable_irq();
}

void exit_process(){				
	preempt_disable();
	struct task_struct *tsk = current;
	unsigned int pid = current->pid;

	dequeue_rt_entity(&current->rt);

	tsk->state = TASK_DEAD;
	printf("\r\nexit process : %d\n\r", pid);

	preempt_enable();
	schedule();
}

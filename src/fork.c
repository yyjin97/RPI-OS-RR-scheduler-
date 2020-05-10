#include "mm.h"
#include "sched.h"
#include "entry.h"
#include "rt.h"
#include "list.h"

int copy_process(unsigned long fn, unsigned long arg, long weight)
{
	preempt_disable();
	struct task_struct *p;

	p = (struct task_struct *) get_free_page();
	if (!p)
		return 1;
	p->priority = 20;
	p->weight = weight;
	p->pid = nr_tasks++;
	p->state = TASK_RUNNING;
	p->thread_info.preempt_count = 1; //disable preemtion until schedule_tail

	p->thread_info.cpu_context.x19 = fn;
	p->thread_info.cpu_context.x20 = arg;
	p->thread_info.cpu_context.pc = (unsigned long)ret_from_fork;
	p->thread_info.cpu_context.sp = (unsigned long)p + THREAD_SIZE;
	p->policy = SCHED_RR;

	p->rt.rt_rq = rt_rq_of_se(&current->rt);
	p->rt.time_slice = RR_TIMESLICE * p->weight;
	p->rt.on_rq = 0;

	p->se.exec_start = 0;
	p->se.sum_exec_runtime = 0;
	p->se.prev_sum_exec_runtime = 0;
	
	clear_tsk_need_resched(p);

	INIT_LIST_HEAD(&p->rt.run_list);

	enqueue_rt_entity(&p->rt, 1);

	preempt_enable();
	return 0;
}

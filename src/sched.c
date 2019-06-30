#include "sched.h"
#include "irq.h"
#include "printf.h"
#include "timer.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct * task[NR_TASKS] = {&(init_task), };
int nr_tasks = 1;

void preempt_disable(void)
{
	current->preempt_count++;
}

void preempt_enable(void)
{
	current->preempt_count--;
}


void _schedule(void)
{
	preempt_disable();
	int next,c;
	bool flag;
	struct task_struct * p;
	while (1) {

		next = 0;
		flag = 0;
		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if (p && p->state == TASK_RUNNING &&  p->runned != true) {
				next = i;
				flag = 1;
				break;
			}
		}
		if (flag) {
			break;
		}
		for (int i = 0; i < NR_TASKS; i++) {   // Si no se encuentras procesos a correr significa que ya se corrieron todos y asi debemos resetear la condicion de corrido.
			p = task[i];	
			p->runned = false;
		}
	}
	p = task[next];
	p->runned = true;    //marcamos al proceso elegido como corrido para que no vuelva a ser ocupado hasta el siguiente loop.
	switch_to(task[next]);

	preempt_enable();
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
}

void switch_to(struct task_struct * next)
{
	if (current == next)
		return;
	struct task_struct * prev = current;
	current = next;
	cpu_switch_to(prev, next);
}

void schedule_tail(void) {
	preempt_enable();
}


void timer_tick()
{
	--current->counter;
	if (current->counter>0 || current->preempt_count >0) {
		return;
	}
	current->counter=0;
	enable_irq();
	_schedule();
	disable_irq();
}

#include "sched.h"
#include "irq.h"
#include "printf.h"
#include "peripherals/timer.h" // para el uso de TIMER_C1
#include "utils.h" // para el uso de get32

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
	int next;
	bool flag; //utilizada para ver si encontramos un proceso que correr
	struct task_struct * p;
	while (1) {
		next = 0;
		for (int i = 0; i < NR_TASKS; i++){
			p = task[i];
			if (p && p->state == TASK_RUNNING && p->runned != true) { // se selecciona el proceso que es corrible y si no ha sido corrido aun
				flag = true; //se cambia el flag ya que encontramos un proceso
				next = i; //se guarda el lugar de lista
				break; //para salir del ciclo for cuando ya tenemos el proceso a ejecutar
			}
		}
		if (flag) { //si encontramos el proceso rompemos el while
			break;
		}
		for (int i = 0; i < NR_TASKS; i++) { //si no se encuentra ningun proceso a correr (significa que ya se corrieron todos),se reinicia la condicion de corrido
			p = task[i];
			if (p) {
				p->runned = false;
			}
		}
	}
	p = task[next]; //el proceso que sera corrido
	p->start_time = get32(TIMER_C1); //se inicia el tiempo donde empieza a correr el proceso
	switch_to(p); //cambiamos a ese proceso
	preempt_enable();
}

void schedule(void)
{
	current->runned = true; // se actualiza el estado del proceso actual a corrido
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
	unsigned int t = get32(TIMER_C1);
	if ( (t - current->start_time) < 5000000) { // provamos que el programa no se ejecute por mas de 5 millones de ticks, que son aproximadamente 5 segundos
		return;
	}
	current->runned = true; //si ya se corrio por mas de 5 segundos, setiamos su estado de runned a true
	enable_irq();
	_schedule();
	disable_irq();
}

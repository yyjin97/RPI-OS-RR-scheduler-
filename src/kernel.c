#include "printf.h"
#include "utils.h"
#include "timer.h"
#include "irq.h"
#include "fork.h"
#include "sched.h"
#include "mini_uart.h"

void process(char *array)
{
	int k = 0;
	while (1){
		for (int i = 0; i < 5; i++){
			uart_send(array[i]);
			delay(100000);
		}
		k++;
		if(array[0] == '0' && k == 300)
			exit_process();
		if(array[0] == 'a' && k == 300)
			exit_process();

	}
}

void kernel_main(void)
{
	uart_init();
	init_printf(0, putc);
	irq_vector_init();
	timer_init();
	enable_interrupt_controller();
	sched_init();
	enable_irq();

	int res = copy_process((unsigned long)&process, (unsigned long)"00000", (long)1);
	if (res != 0) {
		printf("error while starting process 1");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"aaaaa", (long)2);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"*****", (long)3);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"ppppp", (long)4);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}

	while (1){
		schedule();
	}	
}


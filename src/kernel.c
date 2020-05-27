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
		if(k == 150) {
			if(array[0] == '1' || array[0] == '4')
				exit_process();
		}
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

	int res = copy_process((unsigned long)&process, (unsigned long)"11111", (long)1);
	if (res != 0) {
		printf("error while starting process 1");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"22222", (long)1);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"33333", (long)2);
	if (res != 0) {
		printf("error while starting process 2");
		return;
	}
	

	while (1){
		schedule();
		if(timer_clock() > 5) {
			res = copy_process((unsigned long)&process, (unsigned long)"44444", (long)2);
			if (res != 0) {
				printf("error while starting process 2");
				return;
			}
			exit_process();
		}
	}	
}


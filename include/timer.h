#ifndef	_TIMER_H
#define	_TIMER_H

void timer_init ( void );
void handle_timer_irq ( void );
unsigned int timer_clock( void );

#endif  /*_TIMER_H */

#ifndef TIMERAPI
#define TIMERAPI

/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <avr/io.h>
#include <stdint.h>

#include "eecs_388_lib.h"
#include "wiring_private.h"
#include "pins_arduino.h"

/******************************************************************************
 *  Timer Interrupt Functions
 *******************************************************************************/

uint16_t get_cycles(uint8_t timer);

void set_cycles(uint16_t cycle, uint8_t timer);

void enable_interrupt(void);

void disable_interrupt(void);

void enable_timer_interrupt(uint8_t shift, uint8_t timer);

void disable_timer_interrupt(int timer);

uint32_t pow(uint32_t n, uint32_t e);

/* Timer 0: 8bits, 	maximum time 0.016384s or 16.384ms
 * Timer 1: 16bits, maximum time 4.194304s or 4194.304ms
 * Timer 2: 8bits, 	maximum time 0.016384s or 16.384ms
 *
 * Returns 0 if successful, or -1 if bad arguments.
 */
int set_interrupt_timer(uint32_t period_s, uint8_t timer);



#endif
/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <avr/io.h>
#include <stdint.h>

#include "eecs_388_lib.h"
#include "wiring_private.h"
#include "pins_arduino.h"
#include "timer.h"

/******************************************************************************
 *  Timer Interrupt Functions
 *******************************************************************************/

uint16_t get_cycles(uint8_t timer) {
	if (timer == 0)
		return TCNT0;
	if (timer == 1)
		return TCNT1;
	return TCNT2;
}

void set_cycles(uint16_t cycle, uint8_t timer) {
	if (timer == 0)
	{
		TCNT0 = 0;
		OCR0A = cycle;
		return;
	}
	if (timer == 1)
	{
		TCNT1 = 0;
		OCR1A = cycle;
		return;
	}
	TCNT2 = 0;
	OCR2A = cycle;
}

void enable_interrupt(void) {
	sei(); // set SREG I-bit
}

void disable_interrupt(void) {
	cli(); // clear SREG I-bit
}

void enable_timer_interrupt(uint8_t shift, uint8_t timer)
{
	// Setup CTC mode and prescalar value
	if (timer == 0)
	{
		TCCR0A = (0x1 << WGM01);
		TCCR0B = shift;
		TIMSK0 |= (0x1 << OCIE0A);
		return;
	}
	if (timer == 1)
	{
		TCCR1A = 0;
		TCCR1B = (0x1 << WGM12) | shift;
		TIMSK1 |= (0x1 << OCIE1A);
		return;
	}

	TCCR2A = (0x1 << WGM21);
	TCCR2B = shift;
	TIMSK2 |= (0x1 << OCIE2A);
}

void disable_timer_interrupt(int timer) {
	if (timer == 0)
	{
		TIMSK0 &= ~(0x1 << OCIE0A);
		return;
	}
	if (timer == 1)
	{
		TIMSK1 &= ~(0x1 << OCIE1A);
		return;
	}
	TIMSK2 &= ~(0x1 << OCIE2A);
}

uint32_t pow(uint32_t n, uint32_t e)
{
	uint32_t out = 1;

	for (uint32_t i = 0; i < e; i++)
	{
		out *= n;
	}

	return out;
}

/* Timer 0: 8bits, 	maximum time 0.016384s or 16.384ms
 * Timer 1: 16bits, maximum time 4.194304s or 4194.304ms
 * Timer 2: 8bits, 	maximum time 0.016384s or 16.384ms
 *
 * Returns 0 if successful, or -1 if bad arguments.
 */
int set_interrupt_timer(uint32_t period_s, uint8_t timer)
{
	if (timer > 2) return -1;

	// 1) Find knowns from datasheet
	static const uint32_t ARDUINO_CLOCKSPEED_HZ = 16.0e6;
	const uint32_t prescalar_values[5] = {1, 8, 64, 256, 1024};
	const uint8_t prescale_shift_0_1[7] = {
		(0x1 << CS10), 
		(0x1 << CS11), 
		(0x1 << CS10) | (0x1 << CS11),
		(0x1 << CS12),
		(0x1 << CS12) | (0x1 << CS10),
		0,
		0
	};
	const uint8_t prescale_shift_2[7] = {
		(0x1 << CS20), 									// 1 
		(0x1 << CS21),  								// 8
		(0x1 << CS20) | (0x1 << CS21),  				// 32
		(0x1 << CS22),									// 64
		(0x1 << CS22) | (0x1 << CS20),					// 128
		(0x1 << CS22) | (0x1 << CS21),  				// 256
		(0x1 << CS22) | (0x1 << CS21) | (0x1 << CS20) 	// 1024
	};

	// Timer 0: 8 bits, timer 1: 16 bits, timer 2: 8 bits.
	uint32_t timer_width_bits[3] = {8, 16, 8};

	// 2) Find minimum prescalar shift value and cycle count
	uint32_t cycles;
	uint8_t shift;
	int good = 0;

	for (int i = 0; i < (timer == 0 || timer == 1) ? 5 : 7; i++)
	{
		uint32_t p = prescalar_values[i];
		uint64_t max_cycles = pow(2, timer_width_bits[timer]);
		uint64_t request_cycles = ((uint64_t) period_s * ARDUINO_CLOCKSPEED_HZ) / p;
		if (request_cycles > max_cycles)
			continue;

		if (request_cycles == 0) return -1;
		cycles = request_cycles - 1;
		if (timer == 1 || timer == 0)
			shift = prescale_shift_0_1[i];
		if (timer == 2)
			shift = prescale_shift_2[i];
		good = 1;
		break;
	}

	// 3) Prevent bad timer values from happening.
	if (good == 0)
		return -1;

	ser_printf("Setup: shift: %d, timer: %d", shift, timer);

	// 4) Use them to configure the timer.
	enable_timer_interrupt(shift, timer);
	set_cycles(cycles, timer);
	enable_interrupt();
	return 0;
}
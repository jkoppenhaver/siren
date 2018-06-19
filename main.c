/*
 * main.c
 *		Description: This is the c file that contains main() and runs when the chip first powers on.
 *      Author: jkoppenhaver
 */

#include "lookup_table.h"
#include "hw_addrs.h"
#include "setup.h"
#include "isr.h"

//Global Variables
volatile unsigned char siren_enable = 0;
volatile unsigned char siren_last = 0;
volatile unsigned int *freq_ptr = (unsigned int *)LOOKUP_VALUE;

void main(void) {
	setupRuntimeClock();
	setupPWMPin();
	setupPWMTimer();
	setupIntTimer();
	setupButtonPin();
	setupButtonTimer();
	while(1){
	}
}


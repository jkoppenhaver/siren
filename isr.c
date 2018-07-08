/*
 * isr.c
 *		Description: This file contains all of the Interrupt Service Routines that this project requires.
 *      Author: jkoppenhaver
 */

#include "isr.h"
#include "hw_addrs.h"
#include "setup.h"

/************************************************
 * Arguments: None
 * Returns: None
 * This ISR runs when Timer1 expires.  This means
 * that the current siren mode is ready to move
 * to the next frequency.  This ISR controls the
 * changing tone of the siren as well as turning
 * the siren off if it has been disabled and has
 * reached the lowest frequency.
 ***********************************************/
void timer1ISR(void){
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	//Check to see if the siren is at the highest frequency and rising
	if((freq_ptr == &LOOKUP_VALUE[LOOKUP_LENGTH-1]) && (siren_enable & 1)){
		//Set the siren type to the same type but falling instead of rising
		siren_enable++;
	TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
	} //Check to see if the siren is at the lowest frequency and falling and not zero
	else if((freq_ptr == LOOKUP_VALUE) && !(siren_enable & 1) && siren_enable){
		//Set the siren type to the same type but rising instead of falling
		siren_enable--;
	TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
	}//Check to see if the siren is at the lowest frequency and siren is disabled
	else if((freq_ptr == LOOKUP_VALUE) && (siren_enable == SIREN_TYPE_OFF)){
		//Set the siren type to the same type but rising instead of falling
		TimerDisable(TIMER0_BASE, TIMER_A);
		TimerDisable(TIMER1_BASE, TIMER_A);
		return;
	}
	if(siren_enable & 1){
		freq_ptr++;

	}
	else{
		freq_ptr--;
	}
	//Load the lower 16 bits into the Load Register
	TimerLoadSet(TIMER0_BASE, TIMER_A, *freq_ptr);
	//In PWM mode, the prescaler register acts as a timer extension so load the upper 16 bits into PR
	TimerPrescaleSet(TIMER0_BASE, TIMER_A, (*freq_ptr) >> 16);
	//Set the duty cycle to 50% by setting the match value to half the frequency
	TimerMatchSet(TIMER0_BASE, TIMER_A, *freq_ptr>>1);
}

/************************************************
 * Arguments: None
 * Returns: None
 * This ISR is triggered when either button is
 * pressed or released.  This is in control of
 * changing siren modes and starting the button
 * timer to determine if a button was pressed or
 *  held.
 ***********************************************/
void buttonISR(void){
	//Read which interrupts triggered and use bit banding to read the current state of portf
	unsigned long masked_ints = GPIOPinIntStatus(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	unsigned long current = GPIOPinRead(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	if(masked_ints & 1 << 4){
		if(current & 1 << 4){
			//Button1 released (RISING EDGE)
			//If the button is in a hold mode return to the last non hold mode
			if((siren_enable == SIREN_TYPE_PHASER) || (siren_enable == SIREN_TYPE_PHASER_FALL)){
				siren_enable = siren_last;
				if(siren_enable){
					TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
					TimerEnable(TIMER0_BASE, TIMER_A);
					TimerEnable(TIMER1_BASE, TIMER_A);
				}
			}
		}
		else{
			//Button1 pressed (FALLING EDGE)
			TimerEnable(WTIMER0_BASE, TIMER_A);

		}
	}
	if(masked_ints & BUTTON2_PIN){
		if(current & BUTTON2_PIN){
			//Button2 released (RISING EDGE)
			//If the button is in a hold mode return to the last non hold mode
			if(siren_enable == SIREN_TYPE_HORN){
				siren_enable = siren_last;
				if(siren_enable){
					TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
					TimerEnable(TIMER0_BASE, TIMER_A);
					TimerEnable(TIMER1_BASE, TIMER_A);
				}
				else{
					TimerDisable(TIMER0_BASE, TIMER_A);
				}
			}
		}
		else{
			//Button2 pressed (FALLING EDGE)
			TimerEnable(WTIMER0_BASE, TIMER_B);
		}
	}
	GPIOPinIntClear(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
}

/************************************************
 * Arguments: None
 * Returns: None
 * This ISR is triggered when wide timer0 A
 * expires.  This happens when
 * BUTTON_TIMER_HOLD_TIME has passed since the
 * falling edge of the button.  By reading the
 * current state of the button it can determine
 * if it was a press or a hold.
 ***********************************************/
void wtimer0AISR(void){
	TimerIntClear(WTIMER0_BASE, TIMER_TIMA_TIMEOUT);
	if(GPIOPinRead(GPIO_PORTF_BASE, BUTTON1_PIN)){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL || siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_last = SIREN_TYPE_OFF;
			siren_enable = SIREN_TYPE_OFF;
		}
		else{
			if(siren_enable != SIREN_TYPE_WAIL){
				siren_enable = SIREN_TYPE_WAIL;
				TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
				TimerEnable(TIMER0_BASE, TIMER_A);
				TimerEnable(TIMER1_BASE, TIMER_A);
			}
		}
	}
	else if((siren_enable != SIREN_TYPE_PHASER) && (siren_enable != SIREN_TYPE_PHASER_FALL)){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_PHASER;
		TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
		TimerEnable(TIMER0_BASE, TIMER_A);
		TimerEnable(TIMER1_BASE, TIMER_A);
	}
}
/************************************************
 * Arguments: None
 * Returns: None
 * This ISR is triggered when wide timer0 B
 * expires.  This happens when
 * BUTTON_TIMER_HOLD_TIME has passed since the
 * falling edge of the button.  By reading the
 * current state of the button it can determine
 * if it was a press or a hold.
 ***********************************************/
void wtimer0BISR(void){
	TimerIntClear(WTIMER0_BASE, TIMER_TIMB_TIMEOUT);
	if(GPIOPinRead(GPIO_PORTF_BASE, BUTTON2_PIN)){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL){
			siren_enable = SIREN_TYPE_YELP;
			TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
			TimerEnable(TIMER0_BASE, TIMER_A);
			TimerEnable(TIMER1_BASE, TIMER_A);
		}
		else if(siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_enable = SIREN_TYPE_WAIL;
			TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
			TimerEnable(TIMER0_BASE, TIMER_A);
			TimerEnable(TIMER1_BASE, TIMER_A);
		}
	}
	else if(siren_enable != SIREN_TYPE_HORN){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_HORN;
		INT_TIMER_DISABLE;
		TimerLoadSet(TIMER0_BASE, TIMER_A, HORN_VALUE);
		//In PWM mode, the prescaler register acts as a timer extension so load the upper 16 bits into PR
		TimerPrescaleSet(TIMER0_BASE, TIMER_A, (HORN_VALUE) >> 16);
		//Load the lower 16 bits into the Load Register
		//Set the duty cycle to 50% by setting the match value to half the frequency
		TimerMatchSet(TIMER0_BASE, TIMER_A, HORN_VALUE>>1);
		TimerEnable(TIMER0_BASE, TIMER_A);
	}
}




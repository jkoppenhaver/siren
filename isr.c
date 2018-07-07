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
//  TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_ICR) |= 1;
	//Check to see if the siren is at the highest frequency and rising
	if((freq_ptr == &LOOKUP_VALUE[LOOKUP_LENGTH-1]) && (siren_enable & 1)){
		//Set the siren type to the same type but falling instead of rising
		siren_enable++;
//  TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
    LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
	} //Check to see if the siren is at the lowest frequency and falling and not zero
	else if((freq_ptr == LOOKUP_VALUE) && !(siren_enable & 1) && siren_enable){
		//Set the siren type to the same type but rising instead of falling
		siren_enable--;
//  TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
		LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
	}//Check to see if the siren is at the lowest frequency and siren is disabled
	else if((freq_ptr == LOOKUP_VALUE) && (siren_enable == SIREN_TYPE_OFF)){
		//Set the siren type to the same type but rising instead of falling
//	TimerDisable(TIMER0_BASE, TIMER_A);
		PWM_TIMER_DISABLE;
//	TimerDisable(TIMER1_BASE, TIMER_A);
		INT_TIMER_DISABLE;
		return;
	}
	if(siren_enable & 1){
		freq_ptr++;

	}
	else{
		freq_ptr--;
	}
	//Load the lower 16 bits into the Load Register
//TimerLoadSet(TIMER0_BASE, TIMER_A, *freq_ptr);
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAILR) = *freq_ptr & 0xFFFF;
	//In PWM mode, the prescaler register acts as a timer extension so load the upper 16 bits into PR
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAPR) = *freq_ptr>>16;
	//Set the duty cycle to 50% by setting the match value to half the frequency
//TimerMatchSet(TIMER0_BASE, TIMER_A, *freq_ptr>>1);
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMATCHR) = *freq_ptr>>1;
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
//unsigned log masked_ints = GPIOPinIntStatus(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN)
	unsigned long masked_ints = HW_ADDR(GPIO_PORTF_BASE, GPIO_MIS);
//unsigned long current = GPIOPinRead(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	unsigned long current =  HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2));
	if(masked_ints & BUTTON1_PIN){
		if(current & BUTTON1_PIN){
			//Button1 released (RISING EDGE)
			//If the button is in a hold mode return to the last non hold mode
			if((siren_enable == SIREN_TYPE_PHASER) || (siren_enable == SIREN_TYPE_PHASER_FALL)){
				siren_enable = siren_last;
				if(siren_enable){
//				TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
					LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//				TimerEnable(TIMER0_BASE, TIMER_A);
					PWM_TIMER_ENABLE;
//				TimerEnable(TIMER1_BASE, TIMER_A);
					INT_TIMER_ENABLE;
				}
			}
		}
		else{
			//Button1 pressed (FALLING EDGE)
//		TimerEnable(TIMER_ADDR, TIMER_A);
			HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) |= 1;
		}
	}
	if(masked_ints & BUTTON2_PIN){
		if(current & BUTTON2_PIN){
			//Button2 released (RISING EDGE)
			//If the button is in a hold mode return to the last non hold mode
			if(siren_enable == SIREN_TYPE_HORN){
				siren_enable = siren_last;
				if(siren_enable){
//				TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
					LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//				TimerEnable(TIMER0_BASE, TIMER_A);
					PWM_TIMER_ENABLE;
//				TimerEnable(TIMER1_BASE, TIMER_A);
					INT_TIMER_ENABLE;
				}
				else{
//				TimerDisable(TIMER0_BASE, TIMER_A);
					PWM_TIMER_DISABLE;
				}
			}
		}
		else{
			//Button2 pressed (FALLING EDGE)
//		TimerEnable(TIMER_ADDR, TIMER_B);
			HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) |= 1<<8;
		}
	}
//GPIOPinIntClear(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	HW_ADDR(GPIO_PORTF_BASE, GPIO_ICR) |= BUTTON1_PIN | BUTTON2_PIN;
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
//TimerIntClear(TIMER_ADDR, TIMER_TIMA_TIMEOUT);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 1;
//if(GPIOPinRead(GPIO_PORT_F_BASE, BUTTON1_PIN)){
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2)) & BUTTON1_PIN){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL || siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_last = SIREN_TYPE_OFF;
			siren_enable = SIREN_TYPE_OFF;
		}
		else{
			if(siren_enable != SIREN_TYPE_WAIL){
				siren_enable = SIREN_TYPE_WAIL;
//			TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
				LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//			TimerEnable(TIMER0_BASE, TIMER_A);
				PWM_TIMER_ENABLE;
//			TimerEnable(TIMER1_BASE, TIMER_A);
				INT_TIMER_ENABLE;
			}
		}
	}
	else if((siren_enable != SIREN_TYPE_PHASER) && (siren_enable != SIREN_TYPE_PHASER_FALL)){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_PHASER;
//	TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
		LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//	TimerEnable(TIMER0_BASE, TIMER_A);
		PWM_TIMER_ENABLE;
//	TimerEnable(TIMER1_BASE, TIMER_A);
		INT_TIMER_ENABLE;
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
//TimerIntClear(TIMER_ADDR, TIMER_TIMB_TIMEOUT);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 1<<8;
//if(GPIOPinRead(GPIO_PORT_F_BASE, BUTTON2_PIN)){
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2)) & BUTTON2_PIN){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL){
			siren_enable = SIREN_TYPE_YELP;
//		TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
			LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//		TimerEnable(TIMER0_BASE, TIMER_A);
			PWM_TIMER_ENABLE;
//		TimerEnable(TIMER1_BASE, TIMER_A);
			INT_TIMER_ENABLE;
		}
		else if(siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_enable = SIREN_TYPE_WAIL;
//		TimerLoadSet(TIMER1_BASE, TIMER_A, RISE_FALL_TIMES[siren_enable]);
			LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
//		TimerEnable(TIMER0_BASE, TIMER_A);
			PWM_TIMER_ENABLE;
//		TimerEnable(TIMER1_BASE, TIMER_A);
			INT_TIMER_ENABLE;
		}
	}
	else if(siren_enable != SIREN_TYPE_HORN){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_HORN;
		INT_TIMER_DISABLE;
//	TimerLoadSet(TIMER0_BASE, TIMER_A, HORN_VALUE);
		LOAD_PWM_TIMER(HORN_VALUE);
//	TimerEnable(TIMER0_BASE, TIMER_A);
		PWM_TIMER_ENABLE;
	}
}




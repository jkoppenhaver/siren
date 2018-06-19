/*
 * main.c
 *
 *      Author: user
 */

#include "lookup_table.h"
#include "hw_addrs.h"
#include "setup.h"


//Siren Type Definitions
#define SIREN_TYPE_OFF			0
#define SIREN_TYPE_WAIL			1
#define SIREN_TYPE_WAIL_FALL	2
#define SIREN_TYPE_YELP			3
#define SIREN_TYPE_YELP_FALL	4
#define SIREN_TYPE_PHASER		5
#define SIREN_TYPE_PHASER_FALL	6
#define SIREN_TYPE_HORN			7
#define SIREN_TYPE_HORN_FILL	8

//Timer and PWM Manipulation Macros
#define LOAD_PWM_TIMER(value) (HW_ADDR(GPTM_TIMER0_BASE,GPTM_TAILR) = value)
#define PWM_TIMER_ENABLE (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) |= 0x1)
#define PWM_TIMER_DISABLE (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) &= ~(0x1))
#define LOAD_INT_TIMER(value) (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAILR) = value)
#define INT_TIMER_ENABLE (HW_ADDR(GPTM_TIMER1_BASE, GPTM_CTL) |= 0x1)
#define INT_TIMER_DISABLE (HW_ADDR(GPTM_TIMER1_BASE, GPTM_CTL) &= ~(0x1))

//Global Variables
volatile unsigned char siren_enable = 0;
volatile unsigned char siren_last = 0;
volatile unsigned int *freq_ptr = (unsigned int *)LOOKUP_VALUE;

//ISR Function Declarations
void timer1ISR(void);
void wtimer0AISR(void);
void wtimer0BISR(void);

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
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_ICR) |= 1;
	//Check to see if the siren is at the highest frequency and rising
	if((freq_ptr == &LOOKUP_VALUE[LOOKUP_LENGTH-1]) && (siren_enable & 1)){
		//Set the siren type to the same type but falling instead of rising
		siren_enable++;
    LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
	} //Check to see if the siren is at the lowest frequency and falling and not zero
	else if((freq_ptr == LOOKUP_VALUE) && !(siren_enable & 1) && siren_enable){
		//Set the siren type to the same type but rising instead of falling
		siren_enable--;
		LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
	}//Check to see if the siren is at the lowest frequency and siren is disabled
	else if((freq_ptr == LOOKUP_VALUE) && (siren_enable == SIREN_TYPE_OFF)){
		//Set the siren type to the same type but rising instead of falling
		PWM_TIMER_DISABLE;
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
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAILR) = *freq_ptr & 0xFFFF;
	//In PWM mode, the prescaler register acts as a timer extension so load the upper 16 bits into PR
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAPR) = *freq_ptr>>16;
	//Set the duty cycle to 50% by setting the match value to half the frequency
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
	unsigned long masked_ints = HW_ADDR(GPIO_PORTF_BASE, GPIO_MIS);
	unsigned long current =  HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2));
	if(masked_ints & BUTTON1_PIN){
		if(current & BUTTON1_PIN){
			//Button1 released (RISING EDGE)
			//If the button is in a hold mode return to the last non hold mode
			if((siren_enable == SIREN_TYPE_PHASER) || (siren_enable == SIREN_TYPE_PHASER_FALL)){
				siren_enable = siren_last;
				if(siren_enable){
					LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
					PWM_TIMER_ENABLE;
					INT_TIMER_ENABLE;
				}
			}
		}
		else{
			//Button1 pressed (FALLING EDGE)
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
					LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
					PWM_TIMER_ENABLE;
					INT_TIMER_ENABLE;
				}
				else{
					PWM_TIMER_DISABLE;
				}
			}
		}
		else{
			//Button2 pressed (FALLING EDGE)
			HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) |= 1<<8;
		}
	}
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
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 1;
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2)) & BUTTON1_PIN){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL || siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_last = SIREN_TYPE_OFF;
			siren_enable = SIREN_TYPE_OFF;
		}
		else{
			if(siren_enable != SIREN_TYPE_WAIL){
				siren_enable = SIREN_TYPE_WAIL;
				LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
				PWM_TIMER_ENABLE;
				INT_TIMER_ENABLE;
			}
		}
	}
	else if((siren_enable != SIREN_TYPE_PHASER) && (siren_enable != SIREN_TYPE_PHASER_FALL)){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_PHASER;
		LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
		PWM_TIMER_ENABLE;
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
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 1<<8;
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN)<<2)) & BUTTON2_PIN){
		//Button has been released so it was just a button press
		if(siren_enable == SIREN_TYPE_WAIL || siren_enable == SIREN_TYPE_WAIL_FALL){
			siren_enable = SIREN_TYPE_YELP;
			LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
			PWM_TIMER_ENABLE;
			INT_TIMER_ENABLE;
		}
		else if(siren_enable == SIREN_TYPE_YELP || siren_enable == SIREN_TYPE_YELP_FALL){
			siren_enable = SIREN_TYPE_WAIL;
			LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
			PWM_TIMER_ENABLE;
			INT_TIMER_ENABLE;
		}
	}
	else if(siren_enable != SIREN_TYPE_HORN){
		//Button is being held and hold function is not active
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_HORN;
		INT_TIMER_DISABLE;
		LOAD_PWM_TIMER(HORN_VALUE);
		PWM_TIMER_ENABLE;
	}
}


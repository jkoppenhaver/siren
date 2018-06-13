/*
 * main.c
 */
 
#include "lookup_table.h"

//General Purpose Timer Module Base Addresses
#define GPTM_TIMER0_BASE		  0x40030000
#define GPTM_TIMER1_BASE		  0x40031000
#define GPTM_WIDE_TIMER0_BASE 0x40036000
//General Purpose Timer Module Register Offsets
#define GPTM_CTL		  0x00C
#define GPTM_CFG		  0x000
#define GPTM_TAMR		  0x004
#define GPTM_TBMR		  0x008
#define GPTM_IMR		  0x018
#define GPTM_ICR		  0x024
#define GPTM_TAILR		0x028
#define GPTM_TBILR		0x02C
#define GPTM_TAMATCHR	0x030
#define GPTM_TAPR		  0x038
//General Purpose Input/Output Base Addresses
#define GPIO_PORTB_BASE			0x40005000
#define GPIO_PORTF_BASE			0x40025000
//General Purpose Input/Output Register Offsets
#define GPIO_DIR  0x400
#define GPIO_IBE  0x408
#define GPIO_IM   0x410
#define GPIO_ICR  0x41C
#define GPIO_MIS  0x418
#define GPIO_AF   0x420
#define GPIO_PUR  0x510
#define GPIO_DEN  0x51C
#define GPIO_LOCK 0x520
#define GPIO_CR   0x524
#define GPIO_CTL  0x52C
//System Control Clock Configuration Register Addresses
#define SYSCTL_RCGCGPIO			0x400FE608
#define SYSCTL_RCGCTIMER		0x400FE604
#define SYSCTL_RCGCWTIMER		0x400FE65C
#define SYSCTL_RCC				  0x400FE060
//NIVC Enable Register Addresses
#define NIVC_EN0				0xE000E100
#define NIVC_EN1				0xE000E104
#define NIVC_EN2				0xE000E108
//Pin Definitions
#define PWM_PIN 1<<6;
#define BUTTON1_PIN 1<<4
#define BUTTON2_PIN 1<<0
//Siren Type Definitions
#define SIREN_TYPE_OFF			0
#define SIREN_TYPE_WAIL			1
#define SIREN_TYPE_WAIL_FALL	2
#define SIREN_TYPE_YELP			3
#define SIREN_TYPE_YELP_FALL	4
#define SIREN_TYPE_PHASER		5
#define SIREN_TYPE_PHASER_FALL	6
//Timer Manipulation Defines and Macros
#define LOAD_INT_TIMER(value) (*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAILR)) = value)
#define PWM_TIMER_ENABLE (*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) |= 0x1)
#define PWM_TIMER_DISABLE (*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) &= ~(0x1))
#define INT_TIMER_ENABLE (*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) |= 0x1)
#define INT_TIMER_DISABLE (*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(0x1))

//Configuration Values
//Minimum time needed to trigger a button hold
#define BUTTON_TIMER_HOLD_TIME 40000000/2

//Function Declarations
void setupRuntimeClock(void);
void setupPWMPin(void);
void setupPWMTimer(void);
void setupIntTimer(void);
void timer1ISR(void);
void setupButtonPin(void);
void setupButtonTimer(void);
void wtimer0AISR(void);
void wtimer0BISR(void);

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

void setupIntTimer(void){
	//Enable the Timer1 peripheral
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1 << 1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(0x1);
	//Write a value of 0x000.0000 to GPTMCFG to put the timer in 32 bit config for 16/32
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_CFG)) &= ~(0xF);
	//In GPTMTnMR set TnILD to 1, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAMR)) = (*((unsigned long*)GPTM_TIMER1_BASE + GPTM_TAMR) & ~(0xF)) | (1<<0x8) | (0x2);
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAPR)) = (*((unsigned long*)GPTM_TIMER1_BASE + GPTM_TAPR) & ~(0xFF)) | 2;
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_IMR)) |= 1;
	//Enable the Timer1 interrupt in the NIVC enable registers
	*((unsigned long*)(NIVC_EN0)) |= 1<<21;
}

void setupPWMTimer(void){
	//Enable the Timer0 peripheral
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1 << 0;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) &= ~(0x1);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in 16 bit config for 16/32 and 32 bit config for 32/64
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_CFG)) = (*((unsigned long*)GPTM_TIMER0_BASE + GPTM_CFG) & ~(0xF)) | 0x4;
	//In GPTMTnMR set TnILD to 1, TnAMS(Bit 3) to 0x01, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	//This is using timer A so n=A
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAMR)) = (*((unsigned long*)GPTM_TIMER0_BASE + GPTM_TAMR) & ~(0xF)) | (1<<0x8) | (1 << 0x3) | (0x2);
}

void setupButtonTimer(void){
	//Enable the Wide Timer0 peripheral
	*((unsigned long*)SYSCTL_RCGCWTIMER) |= 0x1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_CTL)) &= ~(0x1);
	*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_CTL)) &= ~(0x1<<0x8);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in split mode
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_CFG)) = (*((unsigned long*)GPTM_WIDE_TIMER0_BASE + GPTM_CFG) & ~(0xF)) | 0x4;
	//In GPTMTnMR set TnMR(Bits 1:0) to 0x01
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_TAMR)) = (*((unsigned long*)GPTM_WIDE_TIMER0_BASE + GPTM_TAMR) & ~(0xFFF)) | (0x1);
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_TBMR)) = (*((unsigned long*)GPTM_WIDE_TIMER0_BASE + GPTM_TBMR) & ~(0xFFF)) | (0x1);
	//Enable interrupts for A and B timers in the interrupt mask register
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_IMR)) |= 0x1 | (0x1<<0x8);
	//Enable interrupts for A and B timers in the NIVC
	//Interrupts #94 and #95
	*((unsigned long*)(NIVC_EN2)) |= 0x1<<(94-64) | 0x1<<(95-64);
	//Load the timeout value into both timers A and B
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_TAILR)) = BUTTON_TIMER_HOLD_TIME;
	*((unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_TBILR)) = BUTTON_TIMER_HOLD_TIME;
}

void setupPWMPin(void){
	//Enable the clock to the GPIO pins
	*((unsigned long*)SYSCTL_RCGCGPIO) |= 1<<1;
	//Write 0x7 to the lowest nibble of PCTL register to MUX the timer to this pin
	*((unsigned long*)(GPIO_PORTB_BASE + GPIO_CTL)) = (*((unsigned long*)(GPIO_PORTB_BASE + GPIO_CTL)) & ~(0xF << 24)) | (0x7 << 24);
	//Set to output by writing a 1 to the GPIODIR reg and enable the pins
	*((unsigned long*)(GPIO_PORTB_BASE + GPIO_DIR)) |= PWM_PIN;
	//Enable the alternate function
	*((unsigned long*)(GPIO_PORTB_BASE + GPIO_AF)) |= PWM_PIN;
	//Enable the digital functions of the pin
	*((unsigned long*)(GPIO_PORTB_BASE + GPIO_DEN)) |= PWM_PIN;
	return;
}

void setupButtonPin(void){
	//Enable the clock to the GPIO pins
	*((unsigned long*)SYSCTL_RCGCGPIO) |= 1<<5;
	//Set to output by writing a 1 to the GPIODIR reg and enable the pins
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_DIR)) &= ~(BUTTON1_PIN);
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_DIR)) &= ~(BUTTON2_PIN);
	//Enable digital functions by writing a 1 to the GPIODEN reg for the pins
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_DEN)) |= BUTTON1_PIN;
	//Because PF0 is an NMI, the port must be unlocked before write access is possible
	//First write a value of 0x4C4F434B to GPIOLOCK and then PF0 must be set to R/W in GPIOCR
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_LOCK)) = 0x4C4F434B;
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_CR)) |= BUTTON2_PIN;
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_DEN)) |= BUTTON2_PIN;
	//Enable pullups by setting the pin bits in GPIOPUR
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_PUR)) |= BUTTON1_PIN;
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_PUR)) |= BUTTON2_PIN;
	//Set the pins to detect both rising and falling edges
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_IBE)) |= BUTTON1_PIN;
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_IBE)) |= BUTTON2_PIN;
	//Enable interupts on the button pins
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_IM)) |= BUTTON1_PIN;
	*((unsigned long*)(GPIO_PORTF_BASE + GPIO_IM)) |= BUTTON2_PIN;
	//Clear any pending interrupts
	*((volatile unsigned long*)(GPIO_PORTF_BASE + GPIO_ICR)) |= BUTTON1_PIN | BUTTON2_PIN;
	//Enable GPIOF(#30) interrupts in the NVIC
	*((unsigned long*)(NIVC_EN0)) |= 1<<30;
	return;
}

void setupRuntimeClock(void){
    //Read in the current RCC value to modify
	unsigned long rcc = *((unsigned long*)SYSCTL_RCC);
	//Set SYS_DIV(Bits 26:23) to 0x04 for /5 divisor(40MHz) or to 0x03 for a /4 divisor(50MHz)
	unsigned long mask = 0xF << 23;
	rcc = (rcc & ~mask) | (0x4 << 23);
	//Set bit 22 to use the system clock divider (required to use the PLL)
	rcc |= 0x1<<22;
	//CLear bit 11 to disable the PLL bypass
	rcc &= ~(0x1<<11);
	//Set the crystal frequency to 16MHz Bits (10:6)
	mask = 0x1F << 6;
	rcc = (rcc & ~mask) | (0x15 << 6);
	//Set the oscillator source to main by clearing bits (5:4)
	rcc &= ~(0x1<<5 | 0x1<<4);
	//Clear bit 0 to enable the main oscillator
	rcc &= ~(0x1);
	//Write the RCC configuration
	*((unsigned long*)SYSCTL_RCC) = rcc;
	//Power on the PLL by setting bit 13 BYPASS must be set first
	*((unsigned long*)SYSCTL_RCC) &= ~(0x1<<13);
	return;
}

void timer1ISR(void){
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_ICR)) |= 0x1;
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
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAILR)) = *freq_ptr & 0xFFFF;
	//In PWM mode, the prescaler register acts as a timer extension so load the upper 16 bits into PR
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAPR)) = *freq_ptr >> 16;
	//Set the duty cycle to 50% by setting the match value to half the frequency
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAMATCHR)) = *freq_ptr >> 1;
}

void buttonISR(void){
	//Read which interrupts triggered and use bit banding to read the current state of portf
	unsigned long masked_ints = *((volatile unsigned long*)(GPIO_PORTF_BASE + GPIO_MIS));
	unsigned long current =  *((volatile unsigned long*)(GPIO_PORTF_BASE + ((BUTTON1_PIN | BUTTON2_PIN) << 2)));
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
			*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_CTL)) |= 0x1;
		}
	}
	if(masked_ints & BUTTON2_PIN){
		if(current & BUTTON2_PIN){
			//Button2 released (RISING EDGE)
		}
		else{
			//Button2 pressed (FALLING EDGE)
			*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_CTL)) |= 0x1 << 8;
		}
	}
	*((volatile unsigned long*)(GPIO_PORTF_BASE + GPIO_ICR)) |= BUTTON1_PIN | BUTTON2_PIN;
}
void wtimer0AISR(void){
	*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_ICR)) |= 0x1;
	if(*((volatile unsigned long*)(GPIO_PORTF_BASE + ((BUTTON1_PIN | BUTTON2_PIN) << 2))) & BUTTON1_PIN){
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
		siren_last = siren_enable;
		siren_enable = SIREN_TYPE_PHASER;
		LOAD_INT_TIMER(RISE_FALL_TIMES[siren_enable]);
		PWM_TIMER_ENABLE;
		INT_TIMER_ENABLE;
	}
}

void wtimer0BISR(void){
	*((volatile unsigned long*)(GPTM_WIDE_TIMER0_BASE + GPTM_ICR)) |= 0x1 << 8;
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

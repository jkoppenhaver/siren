/*
 * main.c
 */
//#include "inc/hw_memmap.h"
//#include "inc/hw_sysctl.h"
//#include "inc/hw_gpio.h"
//#include "inc/hw_types.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/gpio.h"
//#include "driverlib/timer.h"
//#include "driverlib/pin_map.h"

#include "lookup_table.h"



#define GPTM_CTL		0x00C
#define GPTM_CFG		0x000
#define GPTM_TAMR		0x004
#define GPTM_IMR		0x018
#define GPTM_ICR		0x024
#define GPTM_TAILR		0x028
#define GPTM_TAMATCHR	0x030
#define GPTM_TAPR		0x038


#define GPTM_TIMER0_BASE	0x40030000
#define GPTM_TIMER1_BASE	0x40031000
#define GPIO_PORTB_BASE		0x40005000
#define GPIO_PORTF_BASE		0x40025000
#define SYSCTL_RCGCGPIO		0x400FE608
#define SYSCTL_RCGCTIMER	0x400FE604
#define SYSCTL_RCC			0x400FE060
#define NIVC_EN0			0xE000E100

#define GPIO_PORTF_DIR  (GPIO_PORTF_BASE+0x400)
#define GPIO_PORTF_AF (GPIO_PORTF_BASE+0x420)
#define GPIO_PORTF_DEN (GPIO_PORTF_BASE+0x51C)
#define GPIO_PORTF_CTL (GPIO_PORTF_BASE+0x52C)
#define GPIO_DIR  0x400
#define GPIO_AF 0x420
#define GPIO_DEN 0x51C
#define GPIO_CTL 0x52C
#define PWM_PIN 1<<6;

#define SIREN_TYPE_WAIL			1
#define SIREN_TYPE_WAIL_FALL	2
#define SIREN_TYPE_YELP			3
#define SIREN_TYPE_YELP_FALL	4
#define SIREN_TYPE_PHASER		5
#define SIREN_TYPE_PHASER_FALL	6

void setupRuntimeClock(void);
void setupPWMPin(void);
void setupPWMTimer(void);
void PWMTimerEnable(void);
void loadFrequency(unsigned long);
void setupIntTimer(void);
void timer1ISR(void);
void PWMTimerDisable(void);
void loadIntTimer(unsigned long value);
void intTimerEnable(void);
void intTimerDisable(void);
void startSiren(unsigned char type);

volatile unsigned char siren_enable = 0;
volatile unsigned int *freq_ptr = (unsigned int *)LOOKUP_VALUE;

void main(void) {
	setupRuntimeClock();
	setupPWMPin();
	setupPWMTimer();
	setupIntTimer();
	startSiren(SIREN_TYPE_WAIL);
	while(1){

	}
}

void setupIntTimer(void){
	//Enable the Timer1 peripheral
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1 << 1;
	//Ensure the timer is disabled TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(0x1);
	//Write a value of 0x000.0000 to GPTMCFG to put the timer in 32 bit config for 16/32
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_CFG)) &= ~(0xF);
	//In GPTMTnMR set TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAMR)) = (*((unsigned long*)GPTM_TIMER1_BASE + GPTM_TAMR) & ~(0xF)) | (0x2);
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAPR)) = (*((unsigned long*)GPTM_TIMER1_BASE + GPTM_TAPR) & ~(0xFF)) | 2;
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_IMR)) |= 1;
	*((unsigned long*)(NIVC_EN0)) |= 1<<21;
}
void loadIntTimer(unsigned long value){
	//Load the 32 bits into the Load Register
	*((unsigned long*)(GPTM_TIMER1_BASE + GPTM_TAILR)) = value;
}

void setupPWMTimer(void){
	//Enable the Timer0 peripheral
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1 << 0;
	//Ensure the timer is disabled TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) &= ~(0x1);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in 16 bit config for 16/32 and 32 bit config for 32/64
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_CFG)) = (*((unsigned long*)GPTM_TIMER0_BASE + GPTM_CFG) & ~(0xF)) | 0x4;
	//In GPTMTnMR set TnAMS(Bit 3) to 0x01, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	//This is using timer A so n=A
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAMR)) = (*((unsigned long*)GPTM_TIMER0_BASE + GPTM_TAMR) & ~(0xF)) | (1 << 0x3) | (0x2);
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAPR)) = (*((unsigned long*)GPTM_TIMER0_BASE + GPTM_TAPR) & ~(0xFF)) | 2;
}

void PWMTimerEnable(void){
	*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) |= 0x1;
}

void PWMTimerDisable(void){
	*((volatile unsigned long*)(GPTM_TIMER0_BASE + GPTM_CTL)) &= ~(0x1);
}

void intTimerEnable(void){
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) |= 0x1;
}

void intTimerDisable(void){
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(0x1);
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
	*((unsigned long*)(GPIO_PORTB_BASE + GPIO_DEN)) |= PWM_PIN;
	return;
}

void setupRuntimeClock(void){
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

void loadFrequency(unsigned long freq){
	//Load the lower 16 bits into the Load Register
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAILR)) = freq & 0xFFFF;
	//In PWM mode, the rpescaler register acts as a timer extension so load the upper 16 bits into PR
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAPR)) = freq >> 16;
	//Set the duty cycle to 50% by setting the match value to half the frequency
	*((unsigned long*)(GPTM_TIMER0_BASE + GPTM_TAMATCHR)) = freq >> 1;

}

void timer1ISR(void){
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_ICR)) |= 0x1;
	//Check to see if the siren is at the highest frequency and rising
	if((freq_ptr == &LOOKUP_VALUE[LOOKUP_LENGTH-1]) && (siren_enable & 1)){
		//Set the siren type to the same type but falling instead of rising
		siren_enable++;
		loadIntTimer(RISE_FALL_TIMES[siren_enable]);
	} //Check to see if the siren is at the lowest frequency and falling
	else if((freq_ptr == LOOKUP_VALUE) && !(siren_enable & 1)){
		//Set the siren type to the same type but rising instead of falling
		siren_enable--;
		loadIntTimer(RISE_FALL_TIMES[siren_enable]);
	}
	if(siren_enable & 1){
		loadFrequency(*(++freq_ptr));
	}
	else{
		loadFrequency(*(--freq_ptr));
	}
}

void startSiren(unsigned char type){
	siren_enable = type;
	loadIntTimer(RISE_FALL_TIMES[siren_enable]);
	loadFrequency(*freq_ptr);
	PWMTimerEnable();
	intTimerEnable();

}

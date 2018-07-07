/*
 * setup.c
 *		Description: This file contains the configuration routines for the different hardware peripherals.
 *      Author: jkoppenhaver
 */
#include "setup.h"
#include "hw_addrs.h"

/************************************************
 * Arguments: None
 * Returns: None
 * This function sets up the interrupt timer.
 * Timer1 is used for this timer in 32 bit mode,
 * configured as a periodic timer with interrupt
 * on timeout.
 ***********************************************/
void setupIntTimer(void){
	//Enable the Timer1 peripheral
//SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1<<1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
//TimerDisable(TIMER1_BASE, TIMER_A)
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(1);
	//Write a value of 0x0000.0000 to GPTMCFG to put the timer in 32 bit config
//TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC)
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_CFG) &= ~(0xF);
	//In GPTMTnMR set TnILD to 1, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAMR) & ~(0xF)) | (1<<8) | (2);
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAPR) = (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAPR) & ~(0xFF)) | 2;
//TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_IMR) |= 1;
	//Enable the Timer1 interrupt in the NIVC enable registers
//IntEnable(TIMER_A);
	HW_ADDR(NIVC_EN0,0) |= 1<<21;
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function sets up the PWM timer.
 * Timer0 is used for this timer in 16 bit mode,
 * configured as a periodic timer with the
 * alternate function(PWM) enabled.
 ***********************************************/
void setupPWMTimer(void){
	//Enable the Timer0 peripheral
//SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	HW_ADDR(SYSCTL_RCGCTIMER,0) |= 1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
//TimerDisable(TIMER0_BASE, TIMER_A)
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) &= ~(1);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in 16 bit config for 16/32 and 32 bit config for 32/64
//TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_CFG) = (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CFG) & ~(0xF)) | 4;
	//In GPTMTnMR set TnILD to 1, TnAMS(Bit 3) to 0x01, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	//This is using timer A so n=A
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMR) & ~(0xF)) | (1<<8) | (1 << 3) | (2);
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function sets up the button timer.
 * Wide Timer0 is used for this timer in 32 bit
 * mode, configured as a one shot timer.  Timer A
 * is used for one button and timer B is used for
 * the other.  Both A and B have interrupts on
 * timeout.
 ***********************************************/
void setupButtonTimer(void){
	//Enable the Wide Timer0 peripheral
	HW_ADDR(SYSCTL_RCGCWTIMER,0) |= 1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) &= ~(1);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) &= ~(1<<8);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in split mode
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CFG) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CFG) & ~(0xF)) | 4;
	//In GPTMTnMR set TnMR(Bits 1:0) to 0x01
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TAMR) & ~(0xFFF)) | (1);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TBMR) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TBMR) & ~(0xFFF)) | (1);
	//Enable interrupts for A and B timers in the interrupt mask register
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_IMR) |= 1 | (1<<8);
	//Enable interrupts for A and B timers in the NIVC
	//Interrupts #94 and #95
	HW_ADDR(NIVC_EN2,0) |= 1<<(94-64) | 1<<(95-64);
	//Load the timeout value into both timers A and B
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TAILR) = BUTTON_TIMER_HOLD_TIME;
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TBILR) = BUTTON_TIMER_HOLD_TIME;
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function configures the PWM pin.  PB6 is
 * used because it is tied to the PWM mode on
 * Timer0.
 ***********************************************/
void setupPWMPin(void){
	//Enable the clock to the GPIO pins
	HW_ADDR(SYSCTL_RCGCGPIO, 0) |= 1<<1;
	//Write 0x7 to the lowest nibble of PCTL register to MUX the timer to this pin
	HW_ADDR(GPIO_PORTB_BASE, GPIO_CTL) = (HW_ADDR(GPIO_PORTB_BASE, GPIO_CTL) & ~(0xF<<24)) | (7<<24);
	//Set to output by writing a 1 to the GPIODIR reg and enable the pins
	HW_ADDR(GPIO_PORTB_BASE, GPIO_DIR) |= PWM_PIN;
	//Enable the alternate function
	HW_ADDR(GPIO_PORTB_BASE, GPIO_AF) |= PWM_PIN;
	//Enable the digital functions of the pin
	HW_ADDR(GPIO_PORTB_BASE, GPIO_DEN) |= PWM_PIN;
	return;
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function configures the Button pins.  The
 * stellaris board has two buttons built in.
 * They are connected to PF0 and PF4.  The
 * buttons are connected to ground so pull-ups
 * must be enabled.  Also, because PF0 is a NMI,
 * the register must be unlocked before certain
 * values are changed.  Interrupts are also
 * enabled for both pins.
 ***********************************************/
void setupButtonPin(void){
	//Enable the clock to the GPIO pins
	HW_ADDR(SYSCTL_RCGCGPIO, 0) |= 1<<5;
	//Set to output by writing a 1 to the GPIODIR reg and enable the pins
	HW_ADDR(GPIO_PORTF_BASE, GPIO_DIR) &= ~(BUTTON1_PIN);
	HW_ADDR(GPIO_PORTF_BASE, GPIO_DIR) &= ~(BUTTON2_PIN);
	//Enable digital functions by writing a 1 to the GPIODEN reg for the pins
	HW_ADDR(GPIO_PORTF_BASE, GPIO_DEN) |= BUTTON1_PIN;
	//Because PF0 is an NMI, the port must be unlocked before write access is possible
	//First write a value of 0x4C4F434B to GPIOLOCK and then PF0 must be set to R/W in GPIOCR
	HW_ADDR(GPIO_PORTF_BASE, GPIO_LOCK) = 0x4C4F434B;
	HW_ADDR(GPIO_PORTF_BASE, GPIO_CR) |= BUTTON2_PIN;
	HW_ADDR(GPIO_PORTF_BASE, GPIO_DEN) |= BUTTON2_PIN;
	//Enable pullups by setting the pin bits in GPIOPUR
	HW_ADDR(GPIO_PORTF_BASE, GPIO_PUR) |= BUTTON1_PIN;
	HW_ADDR(GPIO_PORTF_BASE, GPIO_PUR) |= BUTTON2_PIN;
	//Set the pins to detect both rising and falling edges
	HW_ADDR(GPIO_PORTF_BASE, GPIO_IBE) |= BUTTON1_PIN;
	HW_ADDR(GPIO_PORTF_BASE, GPIO_IBE) |= BUTTON2_PIN;
	//Enable interupts on the button pins
	HW_ADDR(GPIO_PORTF_BASE, GPIO_IM) |= BUTTON1_PIN;
	HW_ADDR(GPIO_PORTF_BASE, GPIO_IM) |= BUTTON2_PIN;
	//Clear any pending interrupts
	*((volatile unsigned long*)(GPIO_PORTF_BASE + GPIO_ICR)) |= BUTTON1_PIN | BUTTON2_PIN;
	//Enable GPIOF(#30) interrupts in the NVIC
	HW_ADDR(NIVC_EN0, 0) |= 1<<30;
	return;
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function configures the system clock to
 * run at 40MHz using the PLL.
 ***********************************************/
void setupRuntimeClock(void){
    //Read in the current RCC value to modify
	unsigned long rcc = HW_ADDR(SYSCTL_RCC, 0);
	//Set SYS_DIV(Bits 26:23) to 0x04 for /5 divisor(40MHz) or to 0x03 for a /4 divisor(50MHz)
	unsigned long mask = 0xF<<23;
	rcc = (rcc & ~mask) | (4<<23);
	//Set bit 22 to use the system clock divider (required to use the PLL)
	rcc |= 1<<22;
	//Clear bit 11 to disable the PLL bypass
	rcc &= ~(1<<11);
	//Set the crystal frequency to 16MHz Bits (10:6)
	mask = 0x1F<<6;
	rcc = (rcc & ~mask) | (0x15<<6);
	//Set the oscillator source to main by clearing bits (5:4)
	rcc &= ~(1<<5 | 1<<4);
	//Clear bit 0 to enable the main oscillator
	rcc &= ~(1);
	//Write the RCC configuration
	HW_ADDR(SYSCTL_RCC, 0) = rcc;
	//Power on the PLL by setting bit 13 BYPASS must be set first
	HW_ADDR(SYSCTL_RCC, 0) &= ~(1<<13);
	return;
}

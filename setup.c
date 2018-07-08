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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	//Write a value of 0x0000.0000 to GPTMCFG to put the timer in 32 bit config
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	//In GPTMTnMR set TnILD to 1, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	//Enable the Timer1 interrupt in the NIVC enable registers
	IntEnable(INT_TIMER1A);
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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in 16 bit config for 16/32 and 32 bit config for 32/64
	TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM);
	//In GPTMTnMR set TnILD to 1
	//Couldn't find a way to configure this in driverlib
	//This is important for a clean sound
	(*((volatile unsigned long*)(TIMER0_BASE + 0x004))) |= 1<<8;
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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in split mode
	TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_ONE_SHOT|TIMER_CFG_B_ONE_SHOT);
	//In GPTMTnMR set TnMR(Bits 1:0) to 0x01
	//Enable interrupts for A and B timers in the interrupt mask register
	TimerIntEnable(WTIMER0_BASE, TIMER_TIMA_TIMEOUT|TIMER_TIMB_TIMEOUT);
	//Enable interrupts for A and B timers in the NIVC
	//Interrupts #94 and #95
	IntEnable(INT_WTIMER0A);
	IntEnable(INT_WTIMER0B);
	//Load the timeout value into both timers A and B
	TimerLoadSet(WTIMER0_BASE,TIMER_BOTH,BUTTON_TIMER_HOLD_TIME);
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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB6_T0CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_6);
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
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//Because PF0 is a NMI, the register must be unlocked first
	(*((volatile unsigned long*)(GPIO_PORTF_BASE + 0x520))) = 0x4C4F434B;
	(*((volatile unsigned long*)(GPIO_PORTF_BASE + 0x524))) |= 1;
	//Set to input by writing a 0 to the GPIODIR reg and enable the pins
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	//Enable digital functions by writing a 1 to the GPIODEN reg for the pins
	//Because PF0 is an NMI, the port must be unlocked before write access is possible
	//First write a value of 0x4C4F434B to GPIOLOCK and then PF0 must be set to R/W in GPIOCR
	//Enable pullups by setting the pin bits in GPIOPUR
	GPIOPadConfigSet(GPIO_PORTF_BASE,BUTTON1_PIN|BUTTON2_PIN,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
	//Set the pins to detect both rising and falling edges
	GPIOIntTypeSet(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN, GPIO_BOTH_EDGES);
	//Enable interrupts on the button pins
	GPIOPinIntEnable(GPIO_PORTF_BASE, BUTTON1_PIN|BUTTON2_PIN);
	//Clear any pending interrupts
	GPIOPinIntClear(GPIO_PORTF_BASE, BUTTON2_PIN|BUTTON1_PIN);
	//Enable GPIOF(#30) interrupts in the NVIC
	IntEnable(INT_GPIOF);
	return;
}

/************************************************
 * Arguments: None
 * Returns: None
 * This function configures the system clock to
 * run at 40MHz using the PLL.
 ***********************************************/
void setupRuntimeClock(void){
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	return;
}

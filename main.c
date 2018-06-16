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
#define SIREN_TYPE_HORN			7
#define SIREN_TYPE_HORN_FILL	8
//Hardware Manipulation Defines and Macros
#define HW_ADDR(addr,offset) (*((volatile unsigned long*)(addr + offset)))
#define LOAD_PWM_TIMER(value) (HW_ADDR(GPTM_TIMER0_BASE,GPTM_TAILR) = value)
#define PWM_TIMER_ENABLE (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) |= 0x1)
#define PWM_TIMER_DISABLE (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) &= ~(0x1))
#define LOAD_INT_TIMER(value) (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAILR) = value)
#define INT_TIMER_ENABLE (HW_ADDR(GPTM_TIMER1_BASE, GPTM_CTL) |= 0x1)
#define INT_TIMER_DISABLE (HW_ADDR(GPTM_TIMER1_BASE, GPTM_CTL) &= ~(0x1))

//Configuration Values
//Minimum time needed to trigger a button hold
#define BUTTON_TIMER_HOLD_TIME 40000000/4

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
	*((unsigned long*)SYSCTL_RCGCTIMER) |= 1 << 1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	*((volatile unsigned long*)(GPTM_TIMER1_BASE + GPTM_CTL)) &= ~(0x1);
	//Write a value of 0x000.0000 to GPTMCFG to put the timer in 32 bit config for 16/32
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_CFG) &= ~(0xF);
	//In GPTMTnMR set TnILD to 1, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAMR) & ~(0xF)) | (1<<0x8) | (0x2);
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAPR) = (HW_ADDR(GPTM_TIMER1_BASE, GPTM_TAPR) & ~(0xFF)) | 2;
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_IMR) |= 1;
	//Enable the Timer1 interrupt in the NIVC enable registers
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
	HW_ADDR(SYSCTL_RCGCTIMER,0) |= 1 << 0;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_CTL) &= ~(0x1);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in 16 bit config for 16/32 and 32 bit config for 32/64
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_CFG) = (HW_ADDR(GPTM_TIMER0_BASE, GPTM_CFG) & ~(0xF)) | 0x4;
	//In GPTMTnMR set TnILD to 1, TnAMS(Bit 3) to 0x01, TnCMR(Bit 2) to 0x00, and TnMR(Bits 1:0) to 0x02
	//This is using timer A so n=A
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMR) & ~(0xF)) | (1<<0x8) | (1 << 0x3) | (0x2);
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
	HW_ADDR(SYSCTL_RCGCWTIMER,0) |= 0x1;
	//Ensure the timer is disabled, TnEN in the GPTMCTL reg should be cleared.
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) &= ~(0x1);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) &= ~(0x1<<0x8);
	//Write a value of 0x000.0004 to GPTMCFG to put the timer in split mode
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CFG) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CFG) & ~(0xF)) | 0x4;
	//In GPTMTnMR set TnMR(Bits 1:0) to 0x01
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TAMR) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TAMR) & ~(0xFFF)) | (0x1);
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TBMR) = (HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_TBMR) & ~(0xFFF)) | (0x1);
	//Enable interrupts for A and B timers in the interrupt mask register
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_IMR) |= 0x1 | (0x1<<0x8);
	//Enable interrupts for A and B timers in the NIVC
	//Interrupts #94 and #95
	HW_ADDR(NIVC_EN2,0) |= 0x1<<(94-64) | 0x1<<(95-64);
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
	HW_ADDR(GPIO_PORTB_BASE, GPIO_CTL) = (HW_ADDR(GPIO_PORTB_BASE, GPIO_CTL) & ~(0xF << 24)) | (0x7 << 24);
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
	unsigned long mask = 0xF << 23;
	rcc = (rcc & ~mask) | (0x4 << 23);
	//Set bit 22 to use the system clock divider (required to use the PLL)
	rcc |= 0x1<<22;
	//Clear bit 11 to disable the PLL bypass
	rcc &= ~(0x1<<11);
	//Set the crystal frequency to 16MHz Bits (10:6)
	mask = 0x1F << 6;
	rcc = (rcc & ~mask) | (0x15 << 6);
	//Set the oscillator source to main by clearing bits (5:4)
	rcc &= ~(0x1<<5 | 0x1<<4);
	//Clear bit 0 to enable the main oscillator
	rcc &= ~(0x1);
	//Write the RCC configuration
	HW_ADDR(SYSCTL_RCC, 0) = rcc;
	//Power on the PLL by setting bit 13 BYPASS must be set first
	HW_ADDR(SYSCTL_RCC, 0) &= ~(0x1<<13);
	return;
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
	HW_ADDR(GPTM_TIMER1_BASE, GPTM_ICR) |= 0x1;
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
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAPR) = *freq_ptr >> 16;
	//Set the duty cycle to 50% by setting the match value to half the frequency
	HW_ADDR(GPTM_TIMER0_BASE, GPTM_TAMATCHR) = *freq_ptr >> 1;
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
	unsigned long current =  HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN) << 2));
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
			HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) |= 0x1;
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
			HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_CTL) |= 0x1 << 8;
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
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 0x1;
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN) << 2)) & BUTTON1_PIN){
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
	HW_ADDR(GPTM_WIDE_TIMER0_BASE, GPTM_ICR) |= 0x1 << 8;
	if(HW_ADDR(GPIO_PORTF_BASE, ((BUTTON1_PIN | BUTTON2_PIN) << 2)) & BUTTON2_PIN){
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

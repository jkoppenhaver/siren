/*
 * frequencyDivider.c
 *
 * Created: 7/4/2018 9:31:50 PM
 * Author : user
 */ 

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define INPUT_PIN PB0
#define QA_PIN PB1
#define QB_PIN PB2
#define LED_PIN PB5

uint8_t divider = 0;
volatile uint8_t counter = 0;

ISR(PCINT_vect){
	PORTB = PINB ^ _BV(QA_PIN);
	if(counter == divider){
		counter = 0;
		PORTB = PINB ^ _BV(QB_PIN);
	}
	else{
		counter++;
	}
}

int main(void){
	DDRB |= _BV(QA_PIN) | _BV(QB_PIN);
	DDRD &= ~(0xF);
	PORTD |= 0x0F;
	divider = (~PIND) & 0x0F;
	PCMSK |= _BV(PCINT0);
	GIMSK |= _BV(PCIE);
	sei();
	while(1){
		divider = (~PIND) & 0x0F;
	}	
}
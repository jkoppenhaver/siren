/*
 * isr.h
 *      Author: jkoppenhaver
 */

#ifndef ISR_H_
#define ISR_H_

//ISR Function Declarations
void timer1ISR(void);
void wtimer0AISR(void);
void wtimer0BISR(void);

//Global Variables
extern volatile unsigned char siren_enable;
extern volatile unsigned char siren_last;
extern volatile unsigned int *freq_ptr;

//Values from lookup.h
extern const unsigned int LOOKUP_LENGTH;
extern const unsigned int HORN_VALUE;
extern const unsigned long RISE_FALL_TIMES[];
extern const unsigned int LOOKUP_VALUE[];

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

#endif /* ISR_H_ */

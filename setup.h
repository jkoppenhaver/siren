/*
 * setup.h
 *      Author: jkoppenhaver
 */

#ifndef SETUP_H_
#define SETUP_H_

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

//Pin Definitions
#define BUTTON1_PIN GPIO_PIN_4
#define BUTTON2_PIN GPIO_PIN_0

//Configuration Values
//Minimum time needed to trigger a button hold
#define BUTTON_TIMER_HOLD_TIME 40000000/4

//Function Declarations
void setupIntTimer(void);
void setupPWMTimer(void);
void setupButtonTimer(void);
void setupPWMPin(void);
void setupButtonPin(void);
void setupRuntimeClock(void);

#endif /* SETUP_H_ */

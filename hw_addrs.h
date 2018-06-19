#ifndef HW_ADDRS_H_
#define HW_ADDRS_H_

//Hardware Manipulation Defines and Macros
#define HW_ADDR(addr,offset) (*((volatile unsigned long*)(addr + offset)))

//General Purpose Timer Module Base Addresses
#define GPTM_TIMER0_BASE		0x40030000
#define GPTM_TIMER1_BASE		0x40031000
#define GPTM_WIDE_TIMER0_BASE	0x40036000
//General Purpose Timer Module Register Offsets
#define GPTM_CTL		0x00C
#define GPTM_CFG		0x000
#define GPTM_TAMR		0x004
#define GPTM_TBMR		0x008
#define GPTM_IMR		0x018
#define GPTM_ICR		0x024
#define GPTM_TAILR		0x028
#define GPTM_TBILR		0x02C
#define GPTM_TAMATCHR	0x030
#define GPTM_TAPR		0x038
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
#define SYSCTL_RCC				0x400FE060
//NIVC Enable Register Addresses
#define NIVC_EN0				0xE000E100
#define NIVC_EN1				0xE000E104
#define NIVC_EN2				0xE000E108

#endif

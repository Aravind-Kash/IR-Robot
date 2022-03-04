// RGB PWM Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   M1PWM5 (PF1) drives an NPN transistor that powers the red LED
// Green LED:
//   M1PWM7 (PF3) drives an NPN transistor that powers the green LED
// Blue LED:
//   M1PWM6 (PF2) drives an NPN transistor that powers the blue LED

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "wait.h"
#include "tm4c123gh6pm.h"
#include "rgb_led.h"
#include "uart0.h"
uint32_t Left;
uint32_t Right;

//Port C Mask
#define Left_Mask 16

//Port D Mask
#define Right_Mask 1

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
/*

void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

        SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0 | SYSCTL_RCGCWTIMER_R2;
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R3;
        _delay_cycles(3);

    // Configure PC4 and PD0 for Hall effect sensor
        GPIO_PORTC_DIR_R &= ~Left_Mask;
        GPIO_PORTC_DEN_R |= Left_Mask;
        GPIO_PORTC_PUR_R |= Left_Mask;
        GPIO_PORTC_AFSEL_R |= Left_Mask;
        GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC4_M;
        GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_WT0CCP0;


        GPIO_PORTD_DIR_R &= ~Right_Mask;
        GPIO_PORTD_DEN_R |= Right_Mask;
        GPIO_PORTD_PUR_R |= Right_Mask;
        GPIO_PORTD_AFSEL_R |=Right_Mask;
        GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD0_M;
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_WT2CCP0;


        //Cofigure WTIMER0 and WTIMER2

        WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;
        WTIMER0_CFG_R = 4;
        WTIMER0_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR;
        WTIMER0_CTL_R = 0;
        WTIMER0_TAV_R = Left;                               // zero counter for first period
        WTIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter

        WTIMER2_CTL_R &= ~TIMER_CTL_TAEN;
        WTIMER2_CFG_R = 4;
        WTIMER2_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR;
        WTIMER2_CTL_R = 0;                               //
        WTIMER2_TAV_R = Right;                               // zero counter for first period
        WTIMER2_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter


}

*/




//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
/*
void forward(uint16_t dist)
{
    setRawPwm(1000, 0, 0, 950);
    dist=30;
    char buf[128];
    while(WTIMER2_TAV_R < (46*dist)/21);
    {
        sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
                putsUart0(buf);
    }

    setRawPwm(0,0,0,0);
}

void reverse(uint16_t dist)
{
    setRawPwm(0, 1000, 950, 0);
    dist=30;
    char buf[128];
    while(WTIMER2_TAV_R < (46*dist)/21);
    {
           sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
                   putsUart0(buf);
    }
    setRawPwm(0,0,0,0);
}
void right(uint8_t angle)
{
    setRawPwm(1000, 0, 975, 0);
    angle=6.75;
    char buf[128];
    while(WTIMER2_TAV_R < (46*angle)/21);
    {
        sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        putsUart0(buf);
    }
    setRawPwm(0, 0, 0, 0);
}

void left(uint8_t angle)
{
    setRawPwm(0, 1000, 0, 975);
    angle=6.75;
    char buf[128];
    while(WTIMER2_TAV_R < (46*angle)/21);
    {
        sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        putsUart0(buf);
    }
    setRawPwm(0, 0, 0, 0);
}


int main(void)
{
	// Initialize hardware
    initHw();
	initPwm();
	initUart0();





    //forward(10);
	//reverse(10);
	//right(10);
	left(10);



	while(true);


}

*/

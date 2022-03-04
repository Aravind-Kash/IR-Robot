// RGB LED Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red Backlight LED:
//   M1PWM5 (PF1) drives an NPN transistor that powers the red LED
// Green Backlight LED:
//   M1PWM7 (PF3) drives an NPN transistor that powers the green LED
// Blue Backlight LED:
//   M1PWM6 (PF2) drives an NPN transistor that powers the blue LED

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <rgb_led.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"

// Port Bidband
uint32_t Left;
uint32_t Right;

// PortF masks
#define PWM_2A_MASK 1           //PF0
#define PWM_2B_MASK 2           //PF1
#define PWM_3A_MASK 4           //PF2
#define PWM_3B_MASK 8           //PF3
#define PWM_1B_MASK 32          //PE5

//Port C Mask
//#define Left_Mask  16

//Port D Mask
//#define Right_Mask 1
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize PWM
/*

 void initPwm()
{
    // Enable clocks
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5 | SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);


    GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R |= 1;

    // Configure Port F for Motor PWM
    GPIO_PORTF_DIR_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3A_MASK | PWM_3B_MASK;
    GPIO_PORTF_DEN_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3A_MASK | PWM_3B_MASK;
    GPIO_PORTF_AFSEL_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3A_MASK | PWM_3B_MASK;
    GPIO_PORTF_PCTL_R &= ~(GPIO_PCTL_PF0_M | GPIO_PCTL_PF1_M | GPIO_PCTL_PF2_M | GPIO_PCTL_PF3_M);
    GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF0_M1PWM4 | GPIO_PCTL_PF1_M1PWM5 | GPIO_PCTL_PF2_M1PWM6 | GPIO_PCTL_PF3_M1PWM7;

    //Configure Port E for Motor PWM
    GPIO_PORTE_DIR_R |= PWM_1B_MASK;
    GPIO_PORTE_DEN_R |= PWM_1B_MASK;
    GPIO_PORTE_AFSEL_R |= PWM_1B_MASK;
    GPIO_PORTE_PCTL_R &= ~GPIO_PCTL_PE5_M;
    GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE5_M1PWM3;




    // Configure PWM module 0 to drive PWM backlight
   
    SYSCTL_SRPWM_R = SYSCTL_SRPWM_R1;                // reset PWM1 module
    SYSCTL_SRPWM_R = 0;                              // leave reset state
    PWM1_1_CTL_R = 0; //PE5                          // turn-off PWM1 generator 1 (drives outs 2 and 3)
    PWM1_2_CTL_R = 0;                                // turn-off PWM1 generator 2 (drives outs 4 and 5)
    PWM1_3_CTL_R = 0;                                // turn-off PWM1 generator 3 (drives outs 6 and 7)


    PWM1_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;      // output 3 on PWM1, gen 1b, cmpb PE5
    PWM1_2_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;		// output 4 on PWM1, gen 2a, cmpa
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;		// output 5 on PWM1, gen 2b, cmpb
    PWM1_3_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;		// output 6 on PWM1, gen 3a, cmpa
    PWM1_3_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;		// output 7 on PWM1, gen 3b, cmpb

    PWM1_1_LOAD_R = 1023;//PE5
    PWM1_2_LOAD_R = 1023;
    PWM1_3_LOAD_R = 1023;
                                                     // invert outputs so duty cycle increases with increasing compare values
    PWM1_1_CMPB_R = 0; // PE5
    PWM1_2_CMPA_R = 0;
    PWM1_2_CMPB_R = 0;                               // Compare values for PWM
    PWM1_3_CMPA_R = 0;                               
    PWM1_3_CMPB_R = 0;                               

    PWM1_1_CTL_R = PWM_0_CTL_ENABLE;
    PWM1_2_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_3_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 3
    PWM1_ENABLE_R = PWM_ENABLE_PWM3EN | PWM_ENABLE_PWM4EN | PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM6EN | PWM_ENABLE_PWM7EN;
                                                     // enable outputs
                                                     
    //Configure PC4 and PD0 for Hall effect sensor
	//GPIO_PORTC_DIR_R &= ~Left_Mask;
	//GPIO_PORTC_DEN_R |= Left_Mask;
	//GPIO_PORTC_PUR_R |= Left_Mask;
	//GPIO_PORTC_AFSEL_R |= Left_Mask;
	//GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC4_M;
	//GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_WT0CCP0;
	
	
	//GPIO_PORTD_DIR_R &= ~Right_Mask;
	//GPIO_PORTD_DEN_R |= Right_Mask;
	//GPIO_PORTD_PUR_R |= Right_Mask;
	//GPIO_PORTD_AFSEL_R |=Right_Mask;
	//GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD0_M;
	//GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_WT2CCP0;
	
	
	//Cofigure WTIMER0 and WTIMER2
	
	//WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;
	//WTIMER0_CFG_R = 4;
	//WTIMER0_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR;
	//WTIMER0_CTL_R = 0;
	//WTIMER0_TAV_R = Left;                               // zero counter for first period
    //WTIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter

	//WTIMER2_CTL_R &= ~TIMER_CTL_TAEN;
	//WTIMER2_CFG_R = 4;
	//WTIMER2_TAMR_R = TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR;
	//WTIMER2_CTL_R = 0;                               //
	//WTIMER2_TAV_R = Right;                               // zero counter for first period
    //WTIMER2_CTL_R |= TIMER_CTL_TAEN;                 // turn-on counter
	
}
*/
/*
void setRawPwm(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
{

    PWM1_2_CMPA_R = m0;
    PWM1_2_CMPB_R = m1;
    //PWM1_3_CMPA_R = m2; //PF2

    PWM1_1_CMPB_R = m2; // PE5

    PWM1_3_CMPB_R = m3;
}
*/

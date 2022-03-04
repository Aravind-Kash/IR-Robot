// ARAVIND RAVI KIRAN KASHYAP
// IR ROBOT CODE

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Blue LED:
//   PF2 drives an NPN transistor that powers the blue LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// 4x4 Keyboard
//   Column 0-3 open drain outputs on PB0, PB1, PB4, PA6
//   Rows 0-3 inputs with pull-ups on PE1, PE2, PE3, PA7
//   To locate a key (r, c), the column c is driven low so the row r reads as low

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "kb.h"
#include "tm4c123gh6pm.h"
#include <rgb_led.h>
#include <stdint.h>
#ifndef RGB_LED_H_
#define RGB_LED_H_
#endif


#define pin1         (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))  //PF2
#define pin1_MASK 4

//Port C Mask
#define Left_Mask 16

//Port D Mask
#define Right_Mask 1



// PortF masks
#define PWM_2A_MASK 1           //PF0
#define PWM_2B_MASK 2           //PF1
//#define PWM_3A_MASK 4           //PF2
#define PWM_3B_MASK 8           //PF3
#define PWM_1B_MASK 32          //PE5


uint32_t time[50];
uint8_t count = 0;
uint32_t bc = 0;
char code[32];
uint8_t i = 0;
bool valid = false;
uint8_t key = 0;
uint32_t Left;
uint32_t Right;
uint16_t dist = 0;

char codes[48][32] =
{
 "00100000110111110001000011101111",
 "00100000110111111101000000101111",
 "00100000110111111001110001100011",
 "00100000110111110101111010100001",
 "00100000110111110001111011100001",
 "00100000110111111111000000001111",
 "00100000110111111000100001110111",
 "00100000110111110100100010110111",
 "00100000110111111100100000110111",
 "00100000110111110010100011010111",
 "00100000110111111010100001010111",
 "00100000110111110110100010010111",
 "00100000110111111110100000010111",
 "00100000110111110001100011100111",
 "00100000110111111001100001100111",
 "00100000110111110011001011001101",
 "00100000110111110000100011110111",
 "00100000110111110101100010100111",
 "00100000110111110100000010111111",
 "00100000110111110111100010000111",
 "00100000110111110000000011111111",
 "00100000110111110101010110101010",
 "00100000110111111100000000111111",
 "00100000110111111001000001101111",
 "00100000110111111000000001111111",
 "00100000110111110110101010010101",
 "00100000110111110011111011000001",
 "00100000110111110011101011000101",
 "00100000110111111100001000111101",
 "00100000110111110000001011111101",
 "00100000110111111111010100001010",
 "00100000110111111110000000011111",
 "00100000110111110010001011011101",
 "00100000110111110110000010011111",
 "00100000110111110001010011101011",
 "00100000110111111000001001111101",
 "00100000110111111101101000100101",
 "00100000110111111000100101110110",
 "00100000110111110111000010001111",
 "00100000110111111000110101110010",
 "00100000110111111111000100001110",
 "00100000110111110000110111110010",
 "00100000110111110101110110100010",
 "00100000110111110111000110001110",
 "00100000110111110100111010110001",
 "00100000110111111000111001110001",
 "00100000110111111100011000111001",
 "00100000110111111000011001111001"
};

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    //GPIO Configuration
    GPIO_PORTF_DIR_R &= ~pin1_MASK;
    GPIO_PORTF_DR2R_R |= pin1_MASK;
    GPIO_PORTF_DEN_R |= pin1_MASK;

    // Configure falling edge interrupts
    GPIO_PORTF_IS_R &= ~pin1_MASK;
    GPIO_PORTF_IBE_R &= ~pin1_MASK;
    GPIO_PORTF_IEV_R &= ~pin1_MASK;
    GPIO_PORTF_PUR_R |= pin1_MASK;
    GPIO_PORTF_IM_R |= pin1_MASK;
    NVIC_EN0_R |= 1 << (INT_GPIOF-16);


    // Configure Timer 1 as the time base
    WTIMER1_CTL_R &= ~TIMER_CTL_TAEN;                                   // turn-off timer before reconfiguring
    WTIMER1_CFG_R = 4 ;                                                 // configure as 32-bit timer (A+B)
    WTIMER1_TAMR_R |= TIMER_TAMR_TAMR_PERIOD;                           // configure for periodic mode (count down)
    WTIMER1_TAMR_R &= ~TIMER_TAMR_TACDIR;
    WTIMER1_TAPR_R = 40;                                                // set load value to 40 for 1 MHz interrupt rate
    WTIMER1_TAV_R = 0xFFFFFFFF;
    WTIMER1_CTL_R |= TIMER_CTL_TAEN;                                    // turn-on timer

}

void gpioIsr()
{
    GPIO_PORTF_ICR_R |= pin1_MASK;
    if(count == 0)
    WTIMER1_TAV_R = 0xFFFFFFFF;

    time[count] = WTIMER1_TAV_R;

    if(count == 0)
    {
        count++;
    }
    else if(count == 1)
    {
        if(time[0]-time[1] > 13000 && time[0]-time[1] < 14000)
        {
            count++;
        }
        else
        {
            count=0;
        }
    }
    else if(count >= 33)
    {
        if(time[count-1]-time[count] > 844 && time[count-1]-time[count] < 1406)
        {
            code[bc]='0';
            bc++;
            count++;
        }
        else if(time[count-1]-time[count] > 1969 && time[count-1]-time[count] < 2600)
        {
            code[bc]='1';
            bc++;
            count++;
        }
        else
        {
            count=0;
            bc=0;
            return;
        }

        bc = 0;

        // Check the signal
        uint8_t i = 0, j = 0;
        for(; i < 48; i++)
        {
            for(j = 0; j < 32; j++)
            {
                if(codes[i][j] != code[j])
                {

                    break;
                }
                if(j == 31)

                {
                    key = i;
                    valid = true;

                }

            }
        }

        // If the signal is correct

        count = 0;
        bc = 0;
        return;
    }
    else if(count > 1)
    {
        if(time[count-1]-time[count] > 844 && time[count-1]-time[count] < 1406)
        {
            code[bc]='0';
            bc++;
            count++;
        }
        else if(time[count-1]-time[count] > 1969 && time[count-1]-time[count] < 2600)
        {
            code[bc]='1';
            bc++;
            count++;
        }
        else
        {
            count=0;
            bc=0;
        }
    return;
    }
}


void inithw()
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

void initPwm()
{
    // Enable clocks
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5 | SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);


    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= 1;

    // Configure Port F for Motor PWM
    GPIO_PORTF_DIR_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3B_MASK;
    GPIO_PORTF_DEN_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3B_MASK;
    GPIO_PORTF_AFSEL_R |= PWM_2A_MASK | PWM_2B_MASK | PWM_3B_MASK;
    GPIO_PORTF_PCTL_R &= ~(GPIO_PCTL_PF0_M | GPIO_PCTL_PF1_M | GPIO_PCTL_PF3_M);
    GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF0_M1PWM4 | GPIO_PCTL_PF1_M1PWM5 | GPIO_PCTL_PF3_M1PWM7;

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
    PWM1_2_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;      // output 4 on PWM1, gen 2a, cmpa
    PWM1_2_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;      // output 5 on PWM1, gen 2b, cmpb
    //PWM1_3_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO;      // output 6 on PWM1, gen 3a, cmpa
    PWM1_3_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO;      // output 7 on PWM1, gen 3b, cmpb

    PWM1_1_LOAD_R = 1023;//PE5
    PWM1_2_LOAD_R = 1023;
    PWM1_3_LOAD_R = 1023;
                                                     // invert outputs so duty cycle increases with increasing compare values
    PWM1_1_CMPB_R = 0; // PE5
    PWM1_2_CMPA_R = 0;
    PWM1_2_CMPB_R = 0;                               // Compare values for PWM
    //PWM1_3_CMPA_R = 0;
    PWM1_3_CMPB_R = 0;

    PWM1_1_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 1
    PWM1_2_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 2
    PWM1_3_CTL_R = PWM_0_CTL_ENABLE;                 // turn-on PWM1 generator 3
    PWM1_ENABLE_R = PWM_ENABLE_PWM3EN | PWM_ENABLE_PWM4EN | PWM_ENABLE_PWM5EN | PWM_ENABLE_PWM7EN;
                                                     // enable outputs

}



void setRawPwm(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
{

    PWM1_2_CMPA_R = m0;
    PWM1_2_CMPB_R = m1;
    //PWM1_3_CMPA_R = m2; //PF2

    PWM1_1_CMPB_R = m2; // PE5

    PWM1_3_CMPB_R = m3;
}


void forward(uint8_t dist)
{
    setRawPwm(1000, 0, 0, 950);
    //dist=30;
    //char buf[128];
    while(WTIMER2_TAV_R < (46*dist)/21);
    {
        //sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        //putsUart0(buf);
    }
    setRawPwm(0,0,0,0);
}

void reverse(uint8_t dist)
{
    setRawPwm(0, 1000, 950, 0);
    //dist=30;
    //char buf[128];
    while(WTIMER2_TAV_R < (46*dist)/21);
    {
        //sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        //putsUart0(buf);
    }
   setRawPwm(0,0,0,0);
}

void right(uint8_t angle)
{
    setRawPwm(1000, 0, 975, 0);
    angle=6.75;
    //char buf[128];
    while(WTIMER2_TAV_R < (46*angle)/21);
    {
        //sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        //putsUart0(buf);
    }
    setRawPwm(0, 0, 0, 0);
}

void left(uint8_t angle)
{
    setRawPwm(0, 1000, 0, 975);
    angle=6.75;
    //char buf[128];
    while(WTIMER2_TAV_R < (46*angle)/21);
    {
        //sprintf(buf,"%u %u\n",WTIMER0_TAV_R, WTIMER2_TAV_R);
        //putsUart0(buf);
    }
    setRawPwm(0, 0, 0, 0);
}

/*void stop( )
{
    setRawPwm(0, 0, 0, 0);
}*/



//---------------------------------------------------------------------
//Main
//---------------------------------------------------------------------


int main(void)
{
    //initPwm();
    initHw();
    //inithw();
    initUart0();
    putsUart0("\nInitialized\n");



    //forward(10);
    //reverse(10);
    //right(10);
    //left(10);

    // Endless loop
    while (true)
    {
        initPwm();
        inithw();

        if(!valid)
            continue;
        valid = false;

        switch(key)
        {

        case 0 :
            putsUart0("\nOn/Off button is pressed.\n");
            break;
        case 1 :
            putsUart0("\nMouse button is pressed.\n");
            break;
        case 2 :
            putsUart0("\nCap button is pressed.\n");
            break;
        case 3 :
            putsUart0("\n[?] button is pressed.\n");
            break;
        case 4 :
            putsUart0("\nSearch button is pressed.\n");
            break;
        case 5 :
            putsUart0("\nTV button is pressed.\n");
            break;
        case 6 :
            putsUart0("\n1 button is pressed.\n");
            dist = dist*10 + 1;
            dist++;
            break;
        case 7 :
            putsUart0("\n2 button is pressed.\n");
            dist = dist*10 + 2;
            dist++;
            break;
        case 8 :
            putsUart0("\n3 button is pressed.\n");
            dist = dist*10 + 3;
            dist++;
            break;
        case 9 :
            putsUart0("\n4 button is pressed.\n");
            dist = dist*10 + 4;
            dist++;
            break;
        case 10 :
            putsUart0("\n5 button is pressed.\n");
            dist = dist*10 + 5;
            dist++;
            break;
        case 11 :
            putsUart0("\n6 button is pressed.\n");
            dist = dist*10 + 6;
            dist++;
            break;
        case 12 :
            putsUart0("\n7 button is pressed.\n");
            dist = dist*10 + 7;
            dist++;
            break;
        case 13 :
            putsUart0("\n8 button is pressed.\n");
            dist = dist*10 + 8;
            dist++;
            break;
        case 14 :
            putsUart0("\n9 button is pressed.\n");
            dist = dist*10 + 9;
            dist++;
            break;
        case 15 :
            putsUart0("\nList button is pressed.\n");
            break;
        case 16 :
            putsUart0("\n0 button is pressed.\n");
            dist = dist*10 + 0;
            dist++;
            break;
        case 17 :
            putsUart0("\nFLASHBK button is pressed.\n");
            break;
        case 18 :
            putsUart0("\nVolumn Up button is pressed.\n");
            break;
        case 19 :
            putsUart0("\nFAV button is pressed.\n");
            break;
        case 20 :
            putsUart0("\nChannel Up button is pressed.\n");
            break;
        case 21 :
            putsUart0("\nINFO button is pressed.\n");
            break;
        case 22 :
            putsUart0("\nVolumn Down button is pressed.\n");
            break;
        case 23 :
            putsUart0("\nMUTE button is pressed.\n");
            break;
        case 24 :
            putsUart0("\nChannel Down button is pressed.\n");
            break;
        case 25 :
            putsUart0("\nNETFLIX button is pressed.\n");
            break;
        case 26 :
            putsUart0("\nHOME button is pressed.\n");
            break;
        case 27 :
            putsUart0("\nAmazon button is pressed.\n");
            break;
        case 28 :
            putsUart0("\nSetting button is pressed.\n");
            break;
        case 29 :
            putsUart0("\nUP button is pressed.\n");
            forward(dist); //Forward
            dist = 0;
            break;
        case 30 :
            putsUart0("\nLIVE ZOOM button is pressed.\n");
            break;
        case 31 :
            putsUart0("\nLEFT button is pressed.\n");
            left(10);  //Left
            break;
        case 32 :
            putsUart0("\nOK button is pressed.\n");
            break;
        case 33 :
            putsUart0("\nRIGHT button is pressed.\n");
            right(10); //Right
            break;
        case 34 :
            putsUart0("\nBACK button is pressed.\n");
            break;
        case 35 :
            putsUart0("\nDOWN button is pressed.\n");
            reverse(dist); //Reverse
            dist = 0;
            break;
        case 36 :
            putsUart0("\nEXIT button is pressed.\n");
            break;
        case 37 :
            putsUart0("\nSAP/* button is pressed.\n");
            break;
        case 38 :
            putsUart0("\nSLEEP button is pressed.\n");
            break;
        case 39 :
            putsUart0("\nSTOP button is pressed.\n");
            //stop(10);
            break;
        case 40 :
            putsUart0("\nREWIND button is pressed.\n");
            break;
        case 41 :
            putsUart0("\nPLAY button is pressed.\n");
            break;
        case 42 :
            putsUart0("\nPAUSE button is pressed.\n");
            break;
        case 43 :
            putsUart0("\nFORWARD button is pressed.\n");
            break;
        case 44 :
            putsUart0("\nRED1 button is pressed.\n");
            break;
        case 45 :
            putsUart0("\nGREEN2 button is pressed.\n");
            break;
        case 46 :
            putsUart0("\nYELLOW3 button is pressed.\n");
            break;
        case 47 :
            putsUart0("\nBLUE4 button is pressed.\n");
            break;

        default :
            putsUart0("Unlisted button pressed.");

        }
    }
}









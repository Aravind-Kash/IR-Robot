// Keyboard Library
// Jason Losh

// Hook in debounceIsr to TIMER1A IVT entry
// Hook in keyPressIsr to GPIOA and GPIOE IVT entries

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// 4x4 Keyboard
//   Column 0-3 open drain outputs on PB0, PB1, PB4, PA6
//   Rows 0-3 inputs with pull-ups on PE1, PE2, PE3, PA7
//   To locate a key (r, c), the column c is driven low so the row r reads low

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <kb.h>
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// Bitband aliases
#define COL0 (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 0*4)))
#define COL1 (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 1*4)))
#define COL2 (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 4*4)))
#define COL3 (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 6*4)))
#define ROW0 (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 1*4)))
#define ROW1 (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 2*4)))
#define ROW2 (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4)))
#define ROW3 (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 7*4)))

// PortA Masks
#define COL3_MASK 64
#define ROW3_MASK 128

// PortB Masks
#define COL0_MASK 1
#define COL1_MASK 2
#define COL2_MASK 16

// PortE Masks
#define ROW0_MASK 2
#define ROW1_MASK 4
#define ROW2_MASK 8

// Keyboard variables
#define KB_BUFFER_LENGTH 16
#define KB_NO_KEY -1
char keyboardBuffer[KB_BUFFER_LENGTH];
uint8_t keyboardReadIndex = 0;
uint8_t keyboardWriteIndex = 0;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initKb()
{
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0 | SYSCTL_RCGCGPIO_R1 | SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Configure keyboard
    // Columns 0-3 with open-drain outpus connected to PB0, PB1, PB4, PA6
    // Rows 0-3 with pull-ups connected to PE1, PE2, PE3, PA7
    GPIO_PORTA_DEN_R |= COL3_MASK | ROW3_MASK;
    GPIO_PORTB_DEN_R |= COL0_MASK | COL1_MASK | COL2_MASK;
    GPIO_PORTE_DEN_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;

    GPIO_PORTA_DIR_R |= COL3_MASK;
    GPIO_PORTB_DIR_R |= COL0_MASK | COL1_MASK | COL2_MASK;
    GPIO_PORTA_DIR_R &= ~ROW3_MASK;
    GPIO_PORTE_DIR_R &= ~ROW0_MASK & ~ROW1_MASK & ~ROW2_MASK;

    GPIO_PORTA_ODR_R |= COL3_MASK;
    GPIO_PORTB_ODR_R |= COL0_MASK | COL1_MASK | COL2_MASK;
    GPIO_PORTA_PUR_R |= ROW3_MASK;
    GPIO_PORTE_PUR_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;

    // Configure falling edge interrupts on row inputs
    // (edge mode, single edge, falling edge, clear any interrupts, turn on)
    GPIO_PORTA_IS_R &= ~ROW3_MASK;
    GPIO_PORTE_IS_R &= ~(ROW0_MASK | ROW1_MASK | ROW2_MASK);
    GPIO_PORTA_IBE_R &= ~ROW3_MASK;
    GPIO_PORTE_IBE_R &= ~(ROW0_MASK | ROW1_MASK | ROW2_MASK);
    GPIO_PORTA_IEV_R &= ~ROW3_MASK;
    GPIO_PORTE_IEV_R &= ~(ROW0_MASK | ROW1_MASK | ROW2_MASK);
    GPIO_PORTA_ICR_R |= ROW3_MASK;
    GPIO_PORTE_ICR_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;
    NVIC_EN0_R |= 1 << (INT_GPIOA-16);               // turn-on interrupt 16 (GPIOA)
    NVIC_EN0_R |= 1 << (INT_GPIOE-16);               // turn-on interrupt 20 (GPIOE)

    // Configure Timer 1 for keyboard service
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 200000;                         // set load value to 2e5 for 200 Hz interrupt rate
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    NVIC_EN0_R |= 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)

    // Configure NVIC to require debounce before key press detection
    GPIO_PORTA_IM_R &= ~ROW3_MASK;                    // turn-off key press interrupts
    GPIO_PORTE_IM_R &= ~(ROW0_MASK | ROW1_MASK | ROW2_MASK);
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on debounce interrupt
}

// Non-blocking function called to drive a selected column low for readout
void setKeyboardColumn(int8_t col)
{
    COL0 = col != 0;
    COL1 = col != 1;
    COL2 = col != 2;
    COL3 = col != 3;
    __asm(" NOP \n NOP \n NOP \n NOP \n");
}

// Non-blocking function called to drive all selected column low for readout
void setKeyboardAllColumns()
{
    //COL0 = COL1 = COL2 = COL3 = 0;
    COL0 = 0;
    COL1 = 0;
    COL2 = 0;
    COL3 = 0;
    __asm(" NOP \n NOP \n NOP \n NOP \n");
}

// Non-blocking function called to determine is a key is pressed in the selected column
int8_t getKeyboardRow()
{
    int8_t row = KB_NO_KEY;
    if (!ROW0) row = 0;
    if (!ROW1) row = 1;
    if (!ROW2) row = 2;
    if (!ROW3) row = 3;
    return row;
}

// Non-blocking function called by the keyboard ISR to determine if a key is pressed
int8_t getKeyboardScanCode()
{
    uint8_t col = 0;
    int8_t row;
    int8_t code = KB_NO_KEY;
    bool found = false;
    while (!found && (col < 4))
    {
        setKeyboardColumn(col);
        row = getKeyboardRow();
        found = row != KB_NO_KEY;
        if (found)
            code = row << 2 | col;
        else
            col++;
    }
    return code;
}

// Key press detection interrupt
void keyPressIsr()
{
    // Handle key press
    bool full;
    int8_t code;
    code = getKeyboardScanCode();
    if (code != KB_NO_KEY)
    {
        full = ((keyboardWriteIndex+1) % KB_BUFFER_LENGTH) == keyboardReadIndex;
        if (!full)
        {
            keyboardBuffer[keyboardWriteIndex] = code;
            keyboardWriteIndex = (keyboardWriteIndex + 1) % KB_BUFFER_LENGTH;
        }
        GPIO_PORTA_IM_R &= ~ROW3_MASK;                    // turn-off key press interrupts
        GPIO_PORTE_IM_R &= ~(ROW0_MASK | ROW1_MASK | ROW2_MASK);
        TIMER1_IMR_R = TIMER_IMR_TATOIM;                  // turn-on debounce interrupt
    }
    GPIO_PORTA_ICR_R |= ROW3_MASK;
    GPIO_PORTE_ICR_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;
}

// 5ms timer interrupt used for debouncing
void debounceIsr()
{
    static uint8_t debounceCount = 0;
    setKeyboardAllColumns();
    if (getKeyboardRow() != KB_NO_KEY)
        debounceCount = 0;
    else
    {
        debounceCount ++;
        if (debounceCount == 10)
        {
            debounceCount = 0;                                  // zero for next debounce
            GPIO_PORTA_ICR_R |= ROW3_MASK;                      // clear out any key presses during debounce
            GPIO_PORTE_ICR_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;
            GPIO_PORTA_IM_R |= ROW3_MASK;                       // turn-on key press interrupts
            GPIO_PORTE_IM_R |= ROW0_MASK | ROW1_MASK | ROW2_MASK;
            TIMER1_IMR_R &= ~TIMER_IMR_TATOIM;                  // turn-off debounce interrupt
        }
    }
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;
}

// Non-blocking function called by the user to determine if a key is present in the buffer
bool kbhit()
{
    return (keyboardReadIndex != keyboardWriteIndex);
}

// Blocking function called by the user to get a keyboard character
char getKey()
{
    const char keyCap[17] = {"123A456B789C*0#D"};
    while (!kbhit());
    uint8_t code = keyboardBuffer[keyboardReadIndex];
    keyboardReadIndex = (keyboardReadIndex + 1) % KB_BUFFER_LENGTH;
    return (char)keyCap[code];
}


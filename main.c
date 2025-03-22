#include "msp430g2553.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>

bool Switch1State = false;
bool Switch2State = false;
bool HazardMode = false;  // New: Hazard mode flag

#define LED1 BIT1  // P2.1 - PWM (TA1.1)
#define LED2 BIT6  // P1.6 - PWM (TA0.1)

#define SWITCH1 BIT3  // P2.3 - Switch 1
#define SWITCH2 BIT3  // P1.3 - Switch 2

#define UART_TX BIT1  // P1.1 - UART TX
#define UART_RX BIT2  // P1.2 - UART RX

volatile unsigned int dutyCycle = 750;
volatile int fadeAmount = 10;  // Smooth fading

void delay_ms(unsigned int ms) {
    while (ms--)
        __delay_cycles(1000);  // Assuming 1MHz clock
}

// UART Initialization (9600 Baud Rate)
void uart_init(void) {
    P1SEL |= UART_TX + UART_RX;
    P1SEL2 |= UART_TX + UART_RX;
    UCA0CTL1 |= UCSSEL_2;  // SMCLK
    UCA0BR0 = 104;         // 1MHz/9600 = 104
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;  // Modulation
    UCA0CTL1 &= ~UCSWRST;
}

// Function to send a single character via UART
void uart_send_char(char c) {
    while (!(IFG2 & UCA0TXIFG));  // Wait for TX buffer to be ready
    UCA0TXBUF = c;
}

// Function to send a string via UART
void uart_send_string(const char *str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

// Function to check if button is pressed for 1 second
bool isButtonPressedFor1Sec(volatile unsigned char *port, unsigned char pin) {
    int count = 0;
    while (!(*port & pin)) {  // Wait while button is pressed
        delay_ms(10);  // Check every 10ms
        count += 10;
        if (count >= 1000) {  // 1-second hold
            return true;
        }
    }
    return false;
}

// Function to check if both buttons are pressed together for 1 second
bool isBothButtonsPressedFor1Sec() {
    int count = 0;
    while (!(P2IN & SWITCH1) && !(P1IN & SWITCH2)) { // Both buttons pressed
        delay_ms(10);
        count += 10;
        if (count >= 1000) {
            return true;
        }
    }
    return false;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer

    uart_init();  // Initialize UART

    // Configure P2.1 (PWM output)
    P2DIR |= LED1;
    P2SEL |= LED1;

    // Configure P1.6 (PWM output)
    P1DIR |= LED2;
    P1SEL |= LED2;

    // Configure P2.3 (Switch 1)
    P2DIR &= ~SWITCH1;
    P2REN |= SWITCH1;
    P2OUT |= SWITCH1;
    P2IE |= SWITCH1;
    P2IFG &= ~SWITCH1;

    // Configure P1.3 (Switch 2)
    P1DIR &= ~SWITCH2;
    P1REN |= SWITCH2;
    P1OUT |= SWITCH2;
    P1IE |= SWITCH2;
    P1IFG &= ~SWITCH2;

    __bis_SR_register(GIE);  // Enable global interrupts

    // Set PWM Period
    TA1CCR0 = 1000 - 1;
    TA0CCR0 = 1000 - 1;

    TA1CCTL1 = OUTMOD_7;  // P2.1 (TA1.1) - LED1
    TA1CCR1 = 0;

    TA0CCTL1 = OUTMOD_7;  // P1.6 (TA0.1) - LED2
    TA0CCR1 = 0;

    TA1CTL = TASSEL_2 + MC_1;  // SMCLK, Up mode
    TA0CTL = TASSEL_2 + MC_1;

    uart_send_string("System Initialized.\r\n");

    while (1) {
        if (HazardMode) {
            TA1CCR1 = (TA1CCR1 == 750) ? 0 : 750;
            TA0CCR1 = (TA0CCR1 == 750) ? 0 : 750;
        } else {
            if (Switch1State) {
                TA1CCR1 = (TA1CCR1 == 750) ? 0 : 750;  // Toggle PWM duty cycle
            } else {
                TA1CCR1 = 0;
            }
            if (Switch2State) {
                TA0CCR1 = (TA0CCR1 == 750) ? 0 : 750;  // Toggle PWM duty cycle
            } else {
                TA0CCR1 = 0;
            }
        }
        delay_ms(300);
    }
}

// Port 2 ISR (Handles P2.3 - Left Indicator Button)
#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void) {
    if (P2IFG & SWITCH1) {
        if (isBothButtonsPressedFor1Sec()) {
            HazardMode = !HazardMode;  // Toggle hazard mode
            Switch1State = false;
            Switch2State = false;
            uart_send_string(HazardMode ? "Hazard Mode ON\r\n" : "Hazard Mode OFF\r\n");
        } else if (!HazardMode && isButtonPressedFor1Sec(&P2IN, SWITCH1)) {
            if (Switch2State) {
                Switch2State = false;  // Turn OFF Right Indicator
                uart_send_string("Right Indicator OFF\r\n");
            }
            Switch1State = !Switch1State;  // Toggle Left Indicator
            uart_send_string(Switch1State ? "Left Indicator ON\r\n" : "Left Indicator OFF\r\n");
        } else if (HazardMode) {
            HazardMode = false; // Exit hazard mode on single button press
            uart_send_string("Hazard Mode OFF\r\n");
        }
        P2IFG &= ~SWITCH1;
    }
}

// Port 1 ISR (Handles P1.3 - Right Indicator Button)
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
    if (P1IFG & SWITCH2) {
        if (isBothButtonsPressedFor1Sec()) {
            HazardMode = !HazardMode;  // Toggle hazard mode
            Switch1State = false;
            Switch2State = false;
            uart_send_string(HazardMode ? "Hazard Mode ON\r\n" : "Hazard Mode OFF\r\n");
        } else if (!HazardMode && isButtonPressedFor1Sec(&P1IN, SWITCH2)) {
            if (Switch1State) {
                Switch1State = false;  // Turn OFF Left Indicator
                uart_send_string("Left Indicator OFF\r\n");
            }
            Switch2State = !Switch2State;  // Toggle Right Indicator
            uart_send_string(Switch2State ? "Right Indicator ON\r\n" : "Right Indicator OFF\r\n");
        } else if (HazardMode) {
            HazardMode = false; // Exit hazard mode on single button press
            uart_send_string("Hazard Mode OFF\r\n");
        }
        P1IFG &= ~SWITCH2;
    }
}

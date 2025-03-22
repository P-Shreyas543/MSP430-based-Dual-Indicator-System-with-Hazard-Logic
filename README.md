# MSP430 Dual Indicator System with Hazard Logic  

This project implements a dual indicator system using the **MSP430G2553** microcontroller. The system supports left and right indicators controlled by switches, PWM-based brightness control, and UART-based status updates. Additionally, it includes hazard logic that enables both indicators when both switches are pressed.  

## Features  
- Left and right turn indicators controlled via **interrupt-based switches**  
- **PWM-based LED brightness control**  
- **Hazard logic:** Both indicators blink together when both switches are pressed  
- **UART communication:** Sends status messages (ON/OFF) for indicators  

## Components Used  
- **Microcontroller:** MSP430G2553  
- **LEDs:** Two LEDs for left and right indicators  
- **Switches:** Two push-buttons to control indicators  
- **UART:** Serial communication for logging status messages  

## Circuit Connections  
| Component  | Pin  | Description |
|------------|------|-------------|
| Left Indicator LED | P2.1 | PWM output (TA1.1) |
| Right Indicator LED | P1.6 | PWM output (TA0.1) |
| Switch 1 (Left) | P2.3 | Interrupt-based switch |
| Switch 2 (Right) | P1.3 | Interrupt-based switch |
| UART TX | P1.1 | Serial communication |
| UART RX | P1.2 | Serial communication |

![20250322_153913](https://github.com/user-attachments/assets/13098a03-f20d-4a43-a1e3-163a0ae5206b)

## Installation & Usage  
1. **Download main.c**  
2. **Compile & Upload Code**
3. **Use Code Composer Studio (CCS) or MSP430-GCC**
4. **Flash the code onto the MSP430G2553**

## Testing the System
- Press Switch 1 to toggle the Left Indicator
- Press Switch 2 to toggle the Right Indicator
- Press both switches together to activate the hazard lights
- View status messages via UART (9600 Baud Rate)

## Demo
**Connect the MSP430 to a serial monitor (e.g., PuTTY, Arduino Serial Monitor) to see status messages like:**

```sh
    System Initialized.
    Left Indicator ON
    Left Indicator OFF
    Right Indicator ON
    Right Indicator OFF
    Hazard Mode ON
    Hazard Mode OFF
```

## Thank You

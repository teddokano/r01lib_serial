/*
 *  main_serial_test.cpp
 *
 *  Serial class test for FRDM-MCXA153
 *  Sends "Hello, world!" over LPUART0 (P0_3=TX / P0_2=RX)
 *
 *  Prerequisites:
 *    - r01lib project configured for semihosting (SEMIHOST_OPERATION defined,
 *      i.e. SDK_DEBUGCONSOLE is NOT set to DEBUGCONSOLE_REDIRECT_TO_SDK)
 *    - Serial.h / Serial.cpp placed in source/r01lib/
 *    - "#include "Serial.h"" added to r01lib.h
 *
 *  Wiring:
 *    FRDM-MCXA153 P0_3 (TX) --> USB-Serial adapter RX
 *    FRDM-MCXA153 P0_2 (RX) --> USB-Serial adapter TX
 *    GND                    --> USB-Serial adapter GND
 *
 *  Terminal settings: 115200 baud, 8N1
 */

#include "r01lib.h"

// LPUART0: P0_3 = TX, P0_2 = RX
Serial uart(P0_3, P0_2, 115200);

int main(void)
{
    // Single message
    uart.printf("Hello, world!\r\n");

    // Blink LED and echo received characters to confirm RX also works
    DigitalOut led(GREEN);

    while (true)
    {

        if (uart.readable())
        {
            int c = uart.getc();
            uart.putc(c);   // echo back
            led = !led;
        }
    }
}

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

//Serial		uart( USBTX, USBRX, 115200);
Serial		uart( D1, D0, 115200);
//Serial		uart( MB_TX, MB_RX, 115200);
DigitalOut	led(GREEN);

int main(void)
{
#if 1
    uart.printf("Hello, world!\r\n");

    while (true)
    {
        if ( uart.readable() )
        {
            int	c	= uart.getc();
            uart.putc(c);   // echo back
            led = !led;
        }
    }
#else
    DigitalOut	pin( D1 );
    while (true)
     {
         pin	= !pin;
         wait_us( 1 );
     }


#endif
}

/*
 *  main_serial_test.cpp
 *
 *  Serial class test for FRDM-MCXA153
 *  Sends "Hello, world!" over LPUART0 (P0_3=TX / P0_2=RX)
 *  and echoes received characters back using an RX interrupt callback.
 *
 *  Prerequisites:
 *    - r01lib project configured for semihosting (SEMIHOST_OPERATION defined)
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

//Serial		uart( USBTX, USBRX, 115200 );
Serial  	uart( D1,    D0,    115200 );   // Target
DigitalOut	led( GREEN );

// RX interrupt callback: echo each received character back
void on_rx( void )
{
    int	c	= uart.getc();
    if ( c != -1 )
        uart.putc( c );
}

int main(void)
{
    // Send greeting once at startup
    uart.printf( "Hello, world!\r\n" );

    // Attach RX interrupt -- from here getc() is non-blocking
    uart.attach( on_rx, Serial::RxIrq );

    while ( true )
    {
        led	= !led;
        wait(0.5);
    }
}

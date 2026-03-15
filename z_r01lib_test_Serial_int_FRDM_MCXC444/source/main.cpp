/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include "r01lib.h"

Serial		uart( USBTX, USBRX, 115200 );
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

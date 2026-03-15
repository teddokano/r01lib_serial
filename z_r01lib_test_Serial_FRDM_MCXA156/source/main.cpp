/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
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

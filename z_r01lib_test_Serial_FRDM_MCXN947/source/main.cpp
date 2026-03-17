/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include "r01lib.h"

DigitalOut	pin( D1 );
//Serial		uart( USBTX, USBRX, 115200);
//Serial		uart( D1, D0, 115200);
//Serial		uart( MB_TX, MB_RX, 115200);
DigitalOut	led(GREEN);

int main(void)
{
	Serial		uart( D1, D0, 115200);
	int	count	= 0;

	pin.pin_mux( 0 );

	while ( true )
	{
		pin	= count & 1;
		count++;
	}

//	DigitalOut	pin( D1 );

    while (true)
    	pin	= !pin;

    while (true)
    {
        if ( uart.readable() )
        {
            int	c	= uart.getc();
            uart.putc(c);   // echo back
            led = !led;
        }
    }
}

/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

DigitalOut	led( BLUE );
DigitalIn	pin( D2 );
DigitalOut	od( D0, 1, DigitalOut::OpenDrain );

int main( void )
{
	printf( "Hello, world!\r\n" );

	while ( true )
	{
		led	= !led;
		od	= !od;
		pin.mode( (int)led ? DigitalIn::PullUp : DigitalIn::PullDown );
		wait( 0.001 );
	}
}

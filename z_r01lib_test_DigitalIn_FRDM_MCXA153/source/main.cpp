/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

DigitalOut	out( D0  );
DigitalIn	in(  D2  );
DigitalIn	sw(  SW2 );

int main( void )
{
	printf( "short D0 and D2 pins to monitor the input toggling\r\n" );
	printf( "get \"0\" by pressing SW2\r\n" );
	bool	v	= false;

	while ( true )
	{
		printf( "%d %d %d\r\n", out & 1, in & 1, sw & 1 );
		v	= !v;
		out	= v;
		wait( 0.1 );
	}
}

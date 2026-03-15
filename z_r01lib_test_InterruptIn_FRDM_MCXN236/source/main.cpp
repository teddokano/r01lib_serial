/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

bool		flag_sw;
bool		flag_pin;

InterruptIn	sw( SW2 );
InterruptIn	pin( D2 );

void callback_sw( void )
{
	flag_sw	= true;
}

void callback_pin( void )
{
	flag_pin	= true;
}

int main( void )
{
	sw.rise(  callback_sw  );
	pin.fall( callback_pin );

	while ( true )
	{
		if ( flag_sw )
		{
			flag_sw	= false;
			PRINTF( "!!! Switch event detected\r\n" );
		}
		if ( flag_pin )
		{
			flag_pin	= false;
			PRINTF( "!!! Pin event detected\r\n" );
		}
		PRINTF( "%d %d\r\n", sw & 1, pin & 1 );
		wait( 0.1 );
	}
}

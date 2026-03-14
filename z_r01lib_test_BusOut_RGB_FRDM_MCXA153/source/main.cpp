/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

BusOut	leds( RED, GREEN, BLUE );

int main( void )
{

	while ( true )
	{
		for( int i = 0; i < 3; i++ )
		{
			leds	= ~(1 << i);
			wait( 0.2 );
		}
	}
}

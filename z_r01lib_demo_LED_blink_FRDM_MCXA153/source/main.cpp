/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

DigitalOut	led( BLUE );

int main( void )
{
	led	= true;

	while ( true )
	{
		led	= !led;
		wait( 0.5 );
	}
}

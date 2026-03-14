/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

DigitalOut	r( RED   );
DigitalOut	g( GREEN );
DigitalOut	b( BLUE  );

int main( void )
{
	r = 1;
	g = 1;
	b = 1;

	while ( true )
	{
		r	= 0;
		wait( 0.2 );
		r	= 1;
		wait( 0.2 );

		g	= 0;
		wait( 0.2 );
		g	= 1;
		wait( 0.2 );

		b	= 0;
		wait( 0.2 );
		b	= 1;
		wait( 0.2 );
	}
}

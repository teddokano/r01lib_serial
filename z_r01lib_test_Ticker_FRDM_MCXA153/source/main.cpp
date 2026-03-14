/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

#ifdef	TARGET_C444
#error	"r01lib on C444 is not supporting Ticker"
#endif

Ticker		t;
DigitalOut	led( GREEN );

void callback( void )
{
	led	= !led;
}

int main( void )
{
	PRINTF( "Check LED is blinking\r\n" );
	t.attach( callback, 0.1 );

	while ( true )
		;
}

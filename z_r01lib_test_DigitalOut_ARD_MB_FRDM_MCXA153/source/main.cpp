/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

#if 0
DigitalOut	pins[]	= {
		DigitalOut( D0 ),
		DigitalOut( D1 ),
		DigitalOut( D2 ),
		DigitalOut( D3 ),
		DigitalOut( D4 ),
		DigitalOut( D5 ),
		DigitalOut( D6 ),
		DigitalOut( D7 ),
		DigitalOut( D8 ),
		DigitalOut( D9 ),
		DigitalOut( D10 ),
		DigitalOut( D11 ),
		DigitalOut( D12 ),
		DigitalOut( D13 ),
		DigitalOut( D18 ),
		DigitalOut( D19 ),
		DigitalOut( A0 ),
		DigitalOut( A1 ),
		DigitalOut( A2 ),
		DigitalOut( A3 ),
		DigitalOut( A4 ),
		DigitalOut( A5 )
};

#else
DigitalOut	pins[]	= {
		DigitalOut( D0 ),
		DigitalOut( MB_RST ),
		DigitalOut( MB_CS ),
		DigitalOut( MB_SCK ),
		DigitalOut( MB_MISO ),
		DigitalOut( MB_MOSI ),
		DigitalOut( MB_PWM ),
		DigitalOut( MB_INT ),
		DigitalOut( MB_RX ),
		DigitalOut( MB_TX ),
		DigitalOut( MB_SCL ),
		DigitalOut( MB_SDA ),
};
#endif


int main( void )
{
	printf( "monitor pulses on pins\r\n" );

	bool	pol	= 0;
	while ( true )
	{
		for ( int i = 0; i < (int)(sizeof( pins ) / sizeof( DigitalOut )); i++ )
			pins[ i ]	= pol;

		for ( int i = 0; i < (int)(sizeof( pins ) / sizeof( DigitalOut )); i++ )
		{
			for ( int n = 0; n < i; n++ )
			{
				pins[ i ]	= !pol;
				pins[ i ]	= pol;
			}
		}

		pol	= !pol;
	}
}

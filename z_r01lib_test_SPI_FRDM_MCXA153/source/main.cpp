/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

SPI			spi( D11, D12, D13, D10 );	//	MOSI, MISO, SCLK, CS

int main( void )
{
	spi.frequency( 2000000 );
	spi.mode( 0 );

	uint8_t	write_data[ 8 ]	= { 0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC };
	uint8_t	read_data[ 8  ]	= { 0x00 };

	PRINTF( "Watch the SPI signal on D10:CS, D11:MOSI, D12:MISO and D13:SCK\r\n" );
	PRINTF( "Loop back data will be shown when D11:MOSI and D12:MISO are shorted\r\n" );

	while ( true )
	{
		spi.write( write_data, read_data, sizeof( write_data ) );

		for ( int i = 0; i < (int)sizeof( read_data ); i++ )
			PRINTF( " 0x%02X", read_data[ i ] );

		PRINTF( "\r\n" );
	}
}


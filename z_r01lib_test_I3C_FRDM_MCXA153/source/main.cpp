/*
 *  @author Tedd OKANO
 *
 *  Released under the MIT license
 */

#include	"r01lib.h"

#if	defined( TARGET_N236 ) || defined( TARGET_C444 )
#error	"r01lib on N236 and C444 are not supporting I3C"
#endif

I3C		i3c( I3C_SDA, I3C_SCL );	//	SDA, SCL

constexpr	uint8_t	static_address	= 0x48;
constexpr	uint8_t	dynamic_address	= 0x08;
uint8_t				w_data[]	= { 0 };
uint8_t				r_data[ 2 ];


int main( void )
{
	printf( "r01lib I3C test\r\n");

	i3c.ccc_broadcast( CCC::BROADCAST_RSTDAA, NULL, 0 ); // Reset DAA
	i3c.ccc_set( CCC::DIRECT_SETDASA, static_address, dynamic_address << 1 ); // Set Dynamic Address from Static Address

	while ( true )
	{
		i3c.write( dynamic_address, w_data, sizeof( w_data ), I2C::NO_STOP );
		i3c.read(  dynamic_address, r_data, sizeof( r_data ) );

		PRINTF( "0x %02X %02X\r\n", r_data[ 0 ], r_data[ 1 ] );
		wait( 1 );
	}
}

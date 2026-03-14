/*
 *  Serial.cpp - Mbed-compatible Serial class for r01lib
 *
 *  Supports FRDM-MCXA153 (CPU_MCXA153VLH)
 *
 *  LPUART instance / pin / mux mapping for MCXA153:
 *
 *    LPUART0  P0_3(TX) P0_2(RX)  Alt2  -- free when semihost debug is used
 *    LPUART1  P1_9(TX) P1_8(RX)  Alt2  -- Arduino D19/D18 (shared with I2C)
 *    LPUART2  P3_15(TX) P3_14(RX) Alt2  -- MikroBus MB_TX/MB_RX (D8/D9)
 *    LPUART2  P1_5(TX) P1_4(RX)  Alt4  -- Arduino D1/D0
 *
 *  Clock source: FRO_HF_DIV (attached for all LPUART instances by
 *  BOARD_InitBootClocks() which is called from init_mcu()).
 *  Clock frequency is obtained via CLOCK_GetLpuartClkFreq(instance).
 *
 *  NOTE on LPUART0:
 *    When r01lib uses semihosting for debug output (SEMIHOST_OPERATION is
 *    defined, i.e. SDK_DEBUGCONSOLE is NOT set to DEBUGCONSOLE_REDIRECT_TO_SDK),
 *    BOARD_InitDebugConsole() does NOT configure LPUART0.
 *    Therefore LPUART0 (P0_3 TX / P0_2 RX) is freely available for Serial.
 *    If the project is configured to redirect printf to LPUART0 via the SDK
 *    debug console, do NOT use LPUART0 with this Serial class.
 *
 *  @author  (based on r01lib pattern by Tedd OKANO)
 *
 *  Released under the MIT license License
 */

extern "C" {
#include "fsl_lpuart.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "fsl_port.h"
}

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "io.h"
#include "mcu.h"
#include "Serial.h"

#ifdef CPU_MCXA153VLH

// ---------------------------------------------------------------------------
//  Internal pin-to-LPUART mapping table
// ---------------------------------------------------------------------------

struct lpuart_pin_map_t {
    int              tx_pin;   ///< r01lib pin enum value for TX
    int              rx_pin;   ///< r01lib pin enum value for RX
    LPUART_Type     *base;     ///< LPUART peripheral base pointer
    uint32_t         instance; ///< LPUART instance index (for clock query)
    port_mux_t       mux;      ///< PORT mux value applied to both TX and RX
    reset_ip_name_t  rst;      ///< Reset control name
};

//  All practical pin combinations on FRDM-MCXA153.
//  LPUART0 is included here because r01lib uses semihosting for debug output
//  (not the SDK UART debug console), so LPUART0 hardware is free to use.
static const lpuart_pin_map_t s_pinMap[] = {
    //  TX       RX       base      inst   mux              rst
    { P0_3, P0_2, LPUART0, 0U, kPORT_MuxAlt2, kLPUART0_RST_SHIFT_RSTn }, // LPUART0 (semihost mode only)
    { MB_TX, MB_RX, LPUART2, 2U, kPORT_MuxAlt2, kLPUART2_RST_SHIFT_RSTn }, // MikroBus MB_TX/MB_RX
    { D1,    D0,    LPUART2, 2U, kPORT_MuxAlt4, kLPUART2_RST_SHIFT_RSTn }, // Arduino D0/D1
    { D19,   D18,   LPUART1, 1U, kPORT_MuxAlt2, kLPUART1_RST_SHIFT_RSTn }, // Arduino D18/D19
};
static constexpr size_t s_pinMapSize = sizeof(s_pinMap) / sizeof(s_pinMap[0]);

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

Serial::Serial( int tx, int rx, int baud )
    : Obj( true ), _base( nullptr ), _mux( kPORT_MuxAlt2 ),
      _tx_pin( tx ), _rx_pin( rx )
{
    resolve_pins( tx, rx );

    if ( !_base )
    {
        panic( "Serial: unsupported TX/RX pin combination for MCXA153" );
        return;
    }

    //  Release peripheral from reset
    RESET_ReleasePeripheralReset( _rst );

    //  Configure LPUART
    LPUART_GetDefaultConfig( &_config );
    _config.baudRate_Bps = (uint32_t)baud;
    _config.enableTx     = true;
    _config.enableRx     = true;

    _clk_freq = CLOCK_GetLpuartClkFreq( _instance );

    LPUART_Init( _base, &_config, _clk_freq );

    //  Set pin mux for TX and RX
    {
        DigitalInOut _tx_io( (uint8_t)tx );
        DigitalInOut _rx_io( (uint8_t)rx );
        _tx_io.pin_mux( (int)_mux );
        _rx_io.pin_mux( (int)_mux );
    }
}

// ---------------------------------------------------------------------------
//  Destructor
// ---------------------------------------------------------------------------

Serial::~Serial()
{
    if ( _base )
        LPUART_Deinit( _base );
}

// ---------------------------------------------------------------------------
//  Private: resolve pin combination to LPUART instance
// ---------------------------------------------------------------------------

void Serial::resolve_pins( int tx, int rx )
{
    _base = nullptr;
    for ( size_t i = 0; i < s_pinMapSize; i++ )
    {
        if ( s_pinMap[i].tx_pin == tx && s_pinMap[i].rx_pin == rx )
        {
            _base     = s_pinMap[i].base;
            _instance = s_pinMap[i].instance;
            _mux      = s_pinMap[i].mux;
            _rst      = s_pinMap[i].rst;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
//  baud rate change
// ---------------------------------------------------------------------------

void Serial::baud( int baudrate )
{
    _config.baudRate_Bps = (uint32_t)baudrate;
    LPUART_Deinit( _base );
    LPUART_Init( _base, &_config, _clk_freq );
}

// ---------------------------------------------------------------------------
//  putc / getc
// ---------------------------------------------------------------------------

int Serial::putc( int c )
{
    uint8_t b = (uint8_t)c;
    LPUART_WriteBlocking( _base, &b, 1 );
    return c;
}

int Serial::getc( void )
{
    uint8_t b;
    LPUART_ReadBlocking( _base, &b, 1 );
    return (int)b;
}

// ---------------------------------------------------------------------------
//  printf
// ---------------------------------------------------------------------------

int Serial::printf( const char *fmt, ... )
{
    char    buf[ 256 ];
    va_list ap;
    va_start( ap, fmt );
    int n = vsnprintf( buf, sizeof(buf), fmt, ap );
    va_end( ap );

    if ( n > 0 )
        LPUART_WriteBlocking( _base, (const uint8_t *)buf, (size_t)n );

    return n;
}

// ---------------------------------------------------------------------------
//  readable / writable
// ---------------------------------------------------------------------------

bool Serial::readable( void )
{
    return ( LPUART_GetStatusFlags( _base ) & kLPUART_RxDataRegFullFlag ) != 0U;
}

bool Serial::writable( void )
{
    return ( LPUART_GetStatusFlags( _base ) & kLPUART_TxDataRegEmptyFlag ) != 0U;
}

// ---------------------------------------------------------------------------
//  Bulk read / write
// ---------------------------------------------------------------------------

status_t Serial::write( const uint8_t *data, size_t length )
{
    return LPUART_WriteBlocking( _base, data, length );
}

status_t Serial::read( uint8_t *data, size_t length )
{
    return LPUART_ReadBlocking( _base, data, length );
}

#else
    #error "Serial.cpp: CPU_MCXA153VLH is required. Add support for other targets as needed."
#endif // CPU_MCXA153VLH

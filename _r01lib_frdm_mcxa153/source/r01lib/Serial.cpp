/*
 *  Serial.cpp - Mbed-compatible Serial class for r01lib
 *
 *  Supports FRDM-MCXA153 (CPU_MCXA153VLH)
 *
 *  Design note:
 *    Both TX and RX use software ring buffers driven directly by the LPUART
 *    interrupt, without the SDK Transfer API.
 *
 *    RX path:
 *      kLPUART_RxDataRegFullInterruptEnable fires →
 *        ISR reads one byte → pushes into _rx_buf → calls _rx_callback.
 *
 *    TX path:
 *      putc/write/printf push bytes into _tx_buf and enable
 *      kLPUART_TxDataRegEmptyInterruptEnable →
 *        ISR pops one byte → writes to hardware.
 *        When _tx_buf is empty the ISR disables TX interrupt and calls
 *        _tx_callback (if attached).
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
#include "fsl_common.h"
}

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "io.h"
#include "mcu.h"
#include "Serial.h"

#ifdef CPU_MCXA153VLH

// ---------------------------------------------------------------------------
//  Instance registry — lets global IRQ handlers find the right Serial object
// ---------------------------------------------------------------------------

static Serial *s_serial_instances[ 3 ] = { nullptr, nullptr, nullptr };

// ---------------------------------------------------------------------------
//  Global LPUART IRQ handlers  (same pattern as irq.c for GPIO)
// ---------------------------------------------------------------------------

extern "C"
{
    void LPUART0_IRQHandler( void )
    {
        if ( s_serial_instances[ 0 ] )
            s_serial_instances[ 0 ]->_irq_handler();
        SDK_ISR_EXIT_BARRIER;
    }

    void LPUART1_IRQHandler( void )
    {
        if ( s_serial_instances[ 1 ] )
            s_serial_instances[ 1 ]->_irq_handler();
        SDK_ISR_EXIT_BARRIER;
    }

    void LPUART2_IRQHandler( void )
    {
        if ( s_serial_instances[ 2 ] )
            s_serial_instances[ 2 ]->_irq_handler();
        SDK_ISR_EXIT_BARRIER;
    }
}

// ---------------------------------------------------------------------------
//  Internal pin-to-LPUART mapping table
// ---------------------------------------------------------------------------

struct lpuart_pin_map_t {
    int               tx_pin;
    int               rx_pin;
    LPUART_Type      *base;
    uint32_t          instance;
    port_mux_t        mux;
    reset_ip_name_t   rst;
    IRQn_Type         irqn;
    clock_attach_id_t clk_attach;
    clock_div_name_t  clk_div;
};

static const lpuart_pin_map_t s_pinMap[] = {
    //  TX      RX      base     inst   mux              rst                       irqn           clk_attach              clk_div
    { P0_3,  P0_2,  LPUART0, 0U, kPORT_MuxAlt2, kLPUART0_RST_SHIFT_RSTn, LPUART0_IRQn, kFRO12M_to_LPUART0, kCLOCK_DivLPUART0 }, // LPUART0 = USBTX/USBRX
    { MB_TX, MB_RX, LPUART2, 2U, kPORT_MuxAlt2, kLPUART2_RST_SHIFT_RSTn, LPUART2_IRQn, kFRO12M_to_LPUART2, kCLOCK_DivLPUART2 }, // MikroBus
    { D1,    D0,    LPUART2, 2U, kPORT_MuxAlt3, kLPUART2_RST_SHIFT_RSTn, LPUART2_IRQn, kFRO12M_to_LPUART2, kCLOCK_DivLPUART2 }, // Arduino D0/D1  (RM Table 61: Alt3)
    { D19,   D18,   LPUART1, 1U, kPORT_MuxAlt2, kLPUART1_RST_SHIFT_RSTn, LPUART1_IRQn, kFRO12M_to_LPUART1, kCLOCK_DivLPUART1 }, // Arduino D18/D19
};
static constexpr size_t s_pinMapSize = sizeof(s_pinMap) / sizeof(s_pinMap[0]);

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

Serial::Serial( int tx, int rx, int baud )
    : Obj( true ),
      _base( nullptr ), _mux( kPORT_MuxAlt2 ), _irqn( NotAvail_IRQn ),
      _clk_attach( kFRO12M_to_LPUART0 ), _clk_div( kCLOCK_DivLPUART0 ),
      _tx_pin( tx ), _rx_pin( rx ),
      _rx_head( 0 ), _rx_tail( 0 ),
      _tx_head( 0 ), _tx_tail( 0 ),
      _rx_callback( nullptr ), _tx_callback( nullptr )
{
    resolve_pins( tx, rx );

    if ( !_base )
    {
        panic( "Serial: unsupported TX/RX pin combination for MCXA153" );
        return;
    }

    s_serial_instances[ _instance ] = this;

    //  Clock: attach source and set divider (div=1 → no division)
    //  Must be done before RESET_ReleasePeripheralReset and LPUART_Init.
    //  BOARD_InitBootClocks() does NOT configure LPUART1/2 clocks.
    CLOCK_SetClockDiv( _clk_div, 1U );
    CLOCK_AttachClk( _clk_attach );

    RESET_ReleasePeripheralReset( _rst );

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
    if ( !_base )
        return;

    LPUART_DisableInterrupts( _base,
        kLPUART_RxDataRegFullInterruptEnable |
        kLPUART_TxDataRegEmptyInterruptEnable );
    DisableIRQ( _irqn );

    LPUART_Deinit( _base );
    s_serial_instances[ _instance ] = nullptr;
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
            _base       = s_pinMap[i].base;
            _instance   = s_pinMap[i].instance;
            _mux        = s_pinMap[i].mux;
            _rst        = s_pinMap[i].rst;
            _irqn       = s_pinMap[i].irqn;
            _clk_attach = s_pinMap[i].clk_attach;
            _clk_div    = s_pinMap[i].clk_div;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
//  Private: push one byte into the TX ring buffer and arm TX interrupt
// ---------------------------------------------------------------------------

void Serial::tx_enqueue( uint8_t b )
{
    //  If the TX buffer is full, wait for the ISR to drain one slot.
    uint16_t next;
    do {
        next = (uint16_t)(( _tx_head + 1U ) & ( TX_RING_BUF_SIZE - 1U ));
    } while ( next == _tx_tail );   // spin until space available

    _tx_buf[ _tx_head ] = b;
    _tx_head = next;

    //  Arm the TX-empty interrupt so the ISR picks up the byte.
    EnableIRQ( _irqn );
    LPUART_EnableInterrupts( _base, kLPUART_TxDataRegEmptyInterruptEnable );
}

// ---------------------------------------------------------------------------
//  Private: reflect RX callback state to RX interrupt enable
// ---------------------------------------------------------------------------

void Serial::update_irq_enables( void )
{
    if ( _rx_callback )
    {
        EnableIRQ( _irqn );
        LPUART_EnableInterrupts( _base, kLPUART_RxDataRegFullInterruptEnable );
    }
    else
    {
        LPUART_DisableInterrupts( _base, kLPUART_RxDataRegFullInterruptEnable );
        //  Disable NVIC only when TX buffer is also idle
        if ( _tx_head == _tx_tail )
            DisableIRQ( _irqn );
    }
}

// ---------------------------------------------------------------------------
//  attach
// ---------------------------------------------------------------------------

void Serial::attach( func_ptr callback, IrqType type )
{
    if ( type == RxIrq )
    {
        _rx_callback = callback;
        update_irq_enables();
    }
    else
    {
        _tx_callback = callback;
        //  TxIrq callback fires from the ISR when TX buffer drains to empty.
        //  No immediate interrupt enable needed here.
    }
}

// ---------------------------------------------------------------------------
//  _irq_handler  (called from LPUART*_IRQHandler)
// ---------------------------------------------------------------------------

void Serial::_irq_handler( void )
{
    uint32_t flags = LPUART_GetStatusFlags( _base );

    //  ---- RX: one byte available ----
    if ( flags & kLPUART_RxDataRegFullFlag )
    {
        uint8_t  byte = LPUART_ReadByte( _base );
        uint16_t next = (uint16_t)(( _rx_head + 1U ) & ( RX_RING_BUF_SIZE - 1U ));

        if ( next != _rx_tail )         // drop silently if full
        {
            _rx_buf[ _rx_head ] = byte;
            _rx_head = next;
        }

        if ( _rx_callback )
            _rx_callback();
    }

    //  ---- TX: register empty — pop one byte from TX ring buffer ----
    if ( flags & kLPUART_TxDataRegEmptyFlag )
    {
        if ( _tx_head != _tx_tail )
        {
            //  Send next byte
            LPUART_WriteByte( _base, _tx_buf[ _tx_tail ] );
            _tx_tail = (uint16_t)(( _tx_tail + 1U ) & ( TX_RING_BUF_SIZE - 1U ));
        }
        else
        {
            //  Buffer drained: disable TX interrupt
            LPUART_DisableInterrupts( _base, kLPUART_TxDataRegEmptyInterruptEnable );

            if ( _tx_callback )
                _tx_callback();
        }
    }

    //  ---- Overrun: clear flag ----
    if ( flags & kLPUART_RxOverrunFlag )
        LPUART_ClearStatusFlags( _base, kLPUART_RxOverrunFlag );
}

// ---------------------------------------------------------------------------
//  baud rate change
// ---------------------------------------------------------------------------

void Serial::baud( int baudrate )
{
    LPUART_DisableInterrupts( _base,
        kLPUART_RxDataRegFullInterruptEnable |
        kLPUART_TxDataRegEmptyInterruptEnable );

    _config.baudRate_Bps = (uint32_t)baudrate;
    LPUART_Deinit( _base );
    LPUART_Init( _base, &_config, _clk_freq );

    update_irq_enables();

    //  Re-arm TX interrupt if buffer still has data
    if ( _tx_head != _tx_tail )
        LPUART_EnableInterrupts( _base, kLPUART_TxDataRegEmptyInterruptEnable );
}

// ---------------------------------------------------------------------------
//  putc  (non-blocking via TX ring buffer)
// ---------------------------------------------------------------------------

int Serial::putc( int c )
{
    tx_enqueue( (uint8_t)c );
    return c;
}

// ---------------------------------------------------------------------------
//  getc
// ---------------------------------------------------------------------------

int Serial::getc( void )
{
    if ( _rx_callback )
    {
        //  Non-blocking: pop from RX ring buffer
        if ( _rx_head == _rx_tail )
            return -1;

        uint8_t b = _rx_buf[ _rx_tail ];
        _rx_tail  = (uint16_t)(( _rx_tail + 1U ) & ( RX_RING_BUF_SIZE - 1U ));
        return (int)b;
    }
    else
    {
        //  Blocking: wait directly from hardware
        uint8_t b;
        LPUART_ReadBlocking( _base, &b, 1 );
        return (int)b;
    }
}

// ---------------------------------------------------------------------------
//  printf  (non-blocking via TX ring buffer)
// ---------------------------------------------------------------------------

int Serial::printf( const char *fmt, ... )
{
    char    buf[ 256 ];
    va_list ap;
    va_start( ap, fmt );
    int n = vsnprintf( buf, sizeof(buf), fmt, ap );
    va_end( ap );

    for ( int i = 0; i < n; i++ )
        tx_enqueue( (uint8_t)buf[ i ] );

    return n;
}

// ---------------------------------------------------------------------------
//  readable / writable
// ---------------------------------------------------------------------------

bool Serial::readable( void )
{
    if ( _rx_callback )
        return ( _rx_head != _rx_tail );

    return ( LPUART_GetStatusFlags( _base ) & kLPUART_RxDataRegFullFlag ) != 0U;
}

bool Serial::writable( void )
{
    //  True when the TX ring buffer has at least one free slot
    uint16_t next = (uint16_t)(( _tx_head + 1U ) & ( TX_RING_BUF_SIZE - 1U ));
    return ( next != _tx_tail );
}

// ---------------------------------------------------------------------------
//  write  (non-blocking via TX ring buffer)
// ---------------------------------------------------------------------------

status_t Serial::write( const uint8_t *data, size_t length )
{
    for ( size_t i = 0; i < length; i++ )
        tx_enqueue( data[ i ] );

    return kStatus_Success;
}

// ---------------------------------------------------------------------------
//  read  (blocking)
// ---------------------------------------------------------------------------

status_t Serial::read( uint8_t *data, size_t length )
{
    return LPUART_ReadBlocking( _base, data, length );
}

#else
    #error "Serial.cpp: CPU_MCXA153VLH is required."
#endif // CPU_MCXA153VLH

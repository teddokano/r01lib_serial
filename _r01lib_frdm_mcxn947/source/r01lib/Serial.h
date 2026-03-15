/*
 *  Serial.h - Mbed-compatible Serial class for r01lib
 *
 *  Supports:
 *    FRDM-MCXA153  (CPU_MCXA153VLH)   LPUART0/1/2
 *    FRDM-MCXN236  (CPU_MCXN236VDF)   LP_FLEXCOMM2/5 (LPUART2/5)
 *    FRDM-MCXN947  (CPU_MCXN947VDF)   LP_FLEXCOMM4 (LPUART4)
 *    FRDM-MCXC444  (CPU_MCXC444VLH)   LPUART0/1
 *
 *  Usage (same API on all targets):
 *    Serial uart( MB_TX, MB_RX, 115200 );
 *    uart.printf( "Hello\r\n" );
 *    uart.attach( on_rx, Serial::RxIrq );
 *
 *  Supported pin pairs per target:
 *
 *   MCXA153:
 *     USBTX/USBRX (P0_3/P0_2)   -> LPUART0  Alt2
 *     MB_TX/MB_RX (P3_15/P3_14) -> LPUART2  Alt2
 *     D1/D0       (P1_5/P1_4)   -> LPUART2  Alt3
 *     D19/D18     (P1_9/P1_8)   -> LPUART1  Alt2
 *
 *   MCXN236:
 *     USBTX/USBRX (P1_9/P1_8)   -> LPUART4 (FC4)  Alt2
 *
 *   MCXC444:
 *     MB_TX/MB_RX (PTE0/PTE1)   -> LPUART1  Alt3
 *     D1/D0       (PTA2/PTA1)   -> LPUART0  Alt2
 *
 *  @author  (based on r01lib pattern by Tedd OKANO)
 *  Released under the MIT license License
 */

#ifndef R01LIB_SERIAL_H
#define R01LIB_SERIAL_H

extern "C" {
#include "fsl_lpuart.h"
#include "fsl_clock.h"
#include "fsl_port.h"
}

#include <stdarg.h>
#include "obj.h"
#include "io.h"

typedef void (*func_ptr)( void );

class Serial : public Obj
{
public:
    enum IrqType {
        RxIrq = 0,
        TxIrq = 1,
    };

    static constexpr size_t RX_RING_BUF_SIZE = 64U;
    static constexpr size_t TX_RING_BUF_SIZE = 256U;

    Serial( int tx, int rx, int baud = 9600 );
    virtual ~Serial();

    void     baud( int baudrate );
    int      putc( int c );
    int      getc( void );
    int      printf( const char *fmt, ... );
    bool     readable( void );
    bool     writable( void );
    status_t write( const uint8_t *data, size_t length );
    status_t read( uint8_t *data, size_t length );
    void     attach( func_ptr callback, IrqType type = RxIrq );

    // Internal: called from global IRQ handlers
    void _irq_handler( void );

private:
    // ---- target-specific helpers (implemented per #ifdef in Serial.cpp) ----
    void     resolve_pins( int tx, int rx );
    void     _setup_clock( void );
    uint32_t _get_clk_freq( void );
    void     _release_reset( void );
    void     _register_instance( void );
    void     _unregister_instance( void );

    // ---- common helpers ----
    void tx_enqueue( uint8_t b );
    void update_irq_enables( void );

    // ---- hardware ----
    LPUART_Type    *_base;
    lpuart_config_t _config;
    uint32_t        _clk_freq;
    uint32_t        _instance;
    port_mux_t      _mux;
    IRQn_Type       _irqn;
    int             _tx_pin;
    int             _rx_pin;

    // ---- target-specific clock / reset fields ----
#if defined( CPU_MCXA153VLH )
    reset_ip_name_t   _rst;
    clock_attach_id_t _clk_attach;
    clock_div_name_t  _clk_div;
#elif defined( CPU_MCXN236VDF ) || defined( CPU_MCXN947VDF )
    reset_ip_name_t   _rst;
    clock_attach_id_t _clk_attach;
#elif defined( CPU_MCXC444VLH )
    clock_ip_name_t   _clk_gate;
#endif

    // ---- RX ring buffer (ISR writes, getc reads) ----
    volatile uint8_t  _rx_buf[ RX_RING_BUF_SIZE ];
    volatile uint16_t _rx_head;
    volatile uint16_t _rx_tail;

    // ---- TX ring buffer (putc/write writes, ISR reads) ----
    volatile uint8_t  _tx_buf[ TX_RING_BUF_SIZE ];
    volatile uint16_t _tx_head;
    volatile uint16_t _tx_tail;

    // ---- user callbacks ----
    func_ptr _rx_callback;
    func_ptr _tx_callback;
};

#endif // R01LIB_SERIAL_H

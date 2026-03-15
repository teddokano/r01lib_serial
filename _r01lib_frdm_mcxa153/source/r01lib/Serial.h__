/*
 *  Serial.h - Mbed-compatible Serial class for r01lib
 *
 *  Supports FRDM-MCXA153 (CPU_MCXA153VLH)
 *
 *  @author  (based on r01lib pattern by Tedd OKANO)
 *
 *  Released under the MIT license License
 */

#ifndef R01LIB_SERIAL_H
#define R01LIB_SERIAL_H

extern "C" {
#include "fsl_lpuart.h"
#include "fsl_clock.h"
#include "fsl_reset.h"
#include "fsl_port.h"
}

#include <stdarg.h>
#include "obj.h"
#include "io.h"

/** Callback function pointer type (same as InterruptIn / Mbed style) */
typedef void (*func_ptr)( void );

/** Serial class
 *
 *  @class Serial
 *
 *  Mbed SDK compatible UART (serial) class using LPUART peripheral.
 *
 *  Both TX and RX are fully interrupt-driven with software ring buffers.
 *  putc() / write() / printf() are non-blocking: bytes are queued into the
 *  TX ring buffer and sent from the TX-empty ISR.  getc() is non-blocking
 *  after attach(RxIrq), returning -1 when the RX buffer is empty.
 *
 *  Supported pin pairs on FRDM-MCXA153 (CPU_MCXA153VLH):
 *
 *    TX=USBTX(P0_3), RX=USBRX(P0_2)  ->  LPUART0  Alt2  (semihost mode)
 *    TX=MB_TX(P3_15), RX=MB_RX(P3_14) ->  LPUART2  Alt2  (MikroBus)
 *    TX=D1  (P1_5),  RX=D0  (P1_4)   ->  LPUART2  Alt3  (Arduino D0/D1)
 *    TX=D19 (P1_9),  RX=D18 (P1_8)   ->  LPUART1  Alt2  (Arduino D18/D19)
 *
 *  Usage example:
 *    Serial uart(USBTX, USBRX, 115200);
 *
 *    // polling TX, polling RX
 *    uart.printf("Hello\r\n");
 *    if (uart.readable()) { int c = uart.getc(); }
 *
 *    // interrupt-driven RX with non-blocking TX
 *    void on_rx() {
 *        int c = uart.getc();   // non-blocking, reads from RX ring buffer
 *        if (c != -1) uart.putc(c);  // non-blocking, queues into TX ring buffer
 *    }
 *    uart.attach(on_rx, Serial::RxIrq);
 */

class Serial : public Obj
{
public:
    /** IrqType: select which interrupt event to attach a callback to */
    enum IrqType {
        RxIrq = 0,  ///< Called each time a character is received
        TxIrq = 1,  ///< Called each time the TX buffer becomes empty
    };

    /** RX/TX ring buffer sizes (bytes) — must be powers of 2 */
    static constexpr size_t RX_RING_BUF_SIZE = 64U;
    static constexpr size_t TX_RING_BUF_SIZE = 256U;

    /** Create a Serial instance
     *
     * @param tx    TX pin number (USBTX, MB_TX, D1, D19 ...)
     * @param rx    RX pin number (USBRX, MB_RX, D0, D18 ...)
     * @param baud  baud rate in bps (default: 9600)
     */
    Serial( int tx, int rx, int baud = 9600 );

    /** Destructor */
    virtual ~Serial();

    /** Change baud rate
     *
     * @param baudrate  new baud rate in bps
     */
    void baud( int baudrate );

    /** Queue a single character for transmission (non-blocking)
     *  The byte is placed in the TX ring buffer and sent from the ISR.
     *  Blocks only if the TX ring buffer is full (waits for space).
     *
     * @param c  character to send
     * @return   the character sent
     */
    int putc( int c );

    /** Receive a single character
     *  Blocking in polling mode (no attach).
     *  Non-blocking after attach(RxIrq): returns -1 when the RX buffer
     *  is empty.
     *
     * @return  received character, or -1 if no data (IRQ mode)
     */
    int getc( void );

    /** Printf-style formatted output (non-blocking via TX ring buffer)
     *
     * @param fmt  printf format string
     * @return     number of characters written
     */
    int printf( const char *fmt, ... );

    /** Check if at least one byte is available to read
     *
     * @return  true if data is ready
     */
    bool readable( void );

    /** Check if the TX ring buffer has space for at least one byte
     *
     * @return  true if space is available
     */
    bool writable( void );

    /** Write a buffer of bytes via TX ring buffer (non-blocking)
     *  Each byte is queued; blocks per-byte only if the TX buffer is full.
     *
     * @param data    pointer to data buffer
     * @param length  number of bytes
     * @return        kStatus_Success
     */
    status_t write( const uint8_t *data, size_t length );

    /** Read a buffer of bytes (blocking)
     *
     * @param data    pointer to receive buffer
     * @param length  number of bytes to read
     * @return        kStatus_Success on success
     */
    status_t read( uint8_t *data, size_t length );

    /** Attach a callback for RX or TX interrupt events (Mbed compatible)
     *
     *  RxIrq: callback invoked each time a byte is received.
     *         getc() becomes non-blocking (returns -1 when buffer empty).
     *  TxIrq: callback invoked each time the TX ring buffer becomes empty.
     *
     *  Pass nullptr to detach.
     *
     * @param callback  function pointer, or nullptr to detach
     * @param type      RxIrq or TxIrq (default: RxIrq)
     */
    void attach( func_ptr callback, IrqType type = RxIrq );

    // -----------------------------------------------------------------------
    //  Internal: called from global LPUART IRQ handlers — do not call directly
    // -----------------------------------------------------------------------
    void _irq_handler( void );

private:
    void resolve_pins( int tx, int rx );
    void update_irq_enables( void );
    void tx_enqueue( uint8_t b );   ///< push one byte into TX ring buffer

    // ---- hardware ----
    LPUART_Type        *_base;
    lpuart_config_t     _config;
    uint32_t            _clk_freq;
    uint32_t            _instance;
    port_mux_t          _mux;
    reset_ip_name_t     _rst;
    IRQn_Type           _irqn;
    clock_attach_id_t   _clk_attach;
    clock_div_name_t    _clk_div;
    int                 _tx_pin;
    int                 _rx_pin;

    // ---- RX software ring buffer (written by ISR, read by getc) ----
    volatile uint8_t    _rx_buf[ RX_RING_BUF_SIZE ];
    volatile uint16_t   _rx_head;
    volatile uint16_t   _rx_tail;

    // ---- TX software ring buffer (written by putc/write, read by ISR) ----
    volatile uint8_t    _tx_buf[ TX_RING_BUF_SIZE ];
    volatile uint16_t   _tx_head;   ///< written by putc / write (foreground)
    volatile uint16_t   _tx_tail;   ///< read    by TX ISR       (background)

    // ---- user callbacks ----
    func_ptr            _rx_callback;
    func_ptr            _tx_callback;
};

#endif // R01LIB_SERIAL_H

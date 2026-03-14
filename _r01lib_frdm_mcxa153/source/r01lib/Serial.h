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

/** Serial class
 *
 *  @class Serial
 *
 *  Mbed SDK compatible UART (serial) class using LPUART peripheral.
 *
 *  Supported pin pairs on FRDM-MCXA153 (CPU_MCXA153VLH):
 *
 *    TX=MB_TX (P3_15), RX=MB_RX (P3_14)  ->  LPUART2  Alt2  (MikroBus)
 *    TX=D1    (P1_5),  RX=D0    (P1_4)   ->  LPUART2  Alt4  (Arduino D0/D1)
 *    TX=D19   (P1_9),  RX=D18   (P1_8)   ->  LPUART1  Alt2  (Arduino D18/D19)
 *
 *  LPUART0 (P0_2/P0_3) is reserved for the debug console and cannot be used.
 *
 *  Usage example:
 *    #include "r01lib.h"
 *    #include "Serial.h"
 *
 *    Serial uart(MB_TX, MB_RX);           // 9600 baud default
 *    Serial uart(MB_TX, MB_RX, 115200);   // specify baud rate
 *
 *    uart.printf("Hello %d\r\n", 42);
 *    uart.putc('A');
 *    int c = uart.getc();                 // blocking receive
 *    if (uart.readable()) { ... }
 */

class Serial : public Obj
{
public:
    /** Create a Serial instance
     *
     * @param tx    TX pin number (use pin name defines: MB_TX, D1, D19, ...)
     * @param rx    RX pin number (use pin name defines: MB_RX, D0, D18, ...)
     * @param baud  baud rate in bps (default: 9600)
     */
    Serial( int tx, int rx, int baud = 9600 );

    /** Destructor - releases LPUART peripheral */
    virtual ~Serial();

    /** Change baud rate
     *
     * @param baudrate  new baud rate in bps
     */
    void baud( int baudrate );

    /** Send a single character (blocking)
     *
     * @param c  character to send (as int, Mbed compatible)
     * @return   the character sent
     */
    int putc( int c );

    /** Receive a single character (blocking)
     *
     * @return  received character
     */
    int getc( void );

    /** Printf-style formatted output (blocking)
     *
     * @param fmt  printf format string
     * @return     number of characters written
     */
    int printf( const char *fmt, ... );

    /** Check if at least one byte is available to read (non-blocking)
     *
     * @return  true if data is ready in the receive register
     */
    bool readable( void );

    /** Check if the transmit register is empty and ready for next byte (non-blocking)
     *
     * @return  true if transmitter is ready
     */
    bool writable( void );

    /** Write a buffer of bytes (blocking)
     *
     * @param data    pointer to data buffer
     * @param length  number of bytes to write
     * @return        kStatus_Success on success
     */
    status_t write( const uint8_t *data, size_t length );

    /** Read a buffer of bytes (blocking)
     *
     * @param data    pointer to receive buffer
     * @param length  number of bytes to read
     * @return        kStatus_Success on success
     */
    status_t read( uint8_t *data, size_t length );

private:
    /** Resolve TX/RX pin combination to LPUART hardware resources */
    void resolve_pins( int tx, int rx );

    LPUART_Type        *_base;      ///< LPUART peripheral base pointer
    lpuart_config_t     _config;    ///< LPUART SDK configuration struct
    uint32_t            _clk_freq;  ///< Source clock frequency [Hz]
    uint32_t            _instance;  ///< LPUART instance index (0/1/2)
    port_mux_t          _mux;       ///< PORT pin mux value for TX and RX
    reset_ip_name_t     _rst;       ///< Reset control identifier
    int                 _tx_pin;    ///< TX pin number (r01lib enum)
    int                 _rx_pin;    ///< RX pin number (r01lib enum)
};

#endif // R01LIB_SERIAL_H

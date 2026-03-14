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
 *  Supported pin pairs on FRDM-MCXA153 (CPU_MCXA153VLH):
 *
 *    TX=P0_3,  RX=P0_2   ->  LPUART0  Alt2  (semihost mode only)
 *    TX=MB_TX, RX=MB_RX  ->  LPUART2  Alt2  (MikroBus)
 *    TX=D1,    RX=D0     ->  LPUART2  Alt4  (Arduino D0/D1)
 *    TX=D19,   RX=D18    ->  LPUART1  Alt2  (Arduino D18/D19)
 *
 *  Usage example:
 *    Serial uart(MB_TX, MB_RX, 115200);
 *
 *    // polling
 *    uart.printf("Hello\r\n");
 *    if (uart.readable()) { int c = uart.getc(); }
 *
 *    // interrupt-driven RX
 *    void on_rx() { int c = uart.getc(); uart.putc(c); }
 *    uart.attach(on_rx, Serial::RxIrq);
 */

class Serial : public Obj
{
public:
	/** IrqType: select which interrupt event to attach a callback to */
	enum IrqType {
		RxIrq = 0,  ///< Called each time a character is received
		TxIrq = 1,  ///< Called when the TX register becomes empty
	};

	/** RX ring buffer size (bytes) — must be a power of 2 */
	static constexpr size_t RX_RING_BUF_SIZE = 64U;

	/** Create a Serial instance
	 *
	 * @param tx    TX pin number (pin name defines: P0_3, MB_TX, D1, D19 ...)
	 * @param rx    RX pin number (pin name defines: P0_2, MB_RX, D0, D18 ...)
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

	/** Send a single character (blocking)
	 *
	 * @param c  character to send
	 * @return   the character sent
	 */
	int putc( int c );

	/** Receive a single character
	 *  Blocking in polling mode (no attach).
	 *  Non-blocking after attach(RxIrq): returns -1 when the ring buffer
	 *  is empty.
	 *
	 * @return  received character, or -1 if no data (IRQ mode)
	 */
	int getc( void );

	/** Printf-style formatted output (blocking)
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

	/** Check if the transmitter is ready for the next byte
	 *
	 * @return  true if TX register is empty
	 */
	bool writable( void );

	/** Write a buffer of bytes (blocking)
	 *
	 * @param data    pointer to data buffer
	 * @param length  number of bytes
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

	/** Attach a callback for RX or TX interrupt events (Mbed compatible)
	 *
	 *  RxIrq: callback invoked each time a byte is received.
	 *         getc() becomes non-blocking (returns -1 when buffer empty).
	 *  TxIrq: callback invoked each time the TX register becomes empty.
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

	// ---- hardware ----
	LPUART_Type        *_base;
	lpuart_config_t     _config;
	uint32_t            _clk_freq;
	uint32_t            _instance;
	port_mux_t          _mux;
	reset_ip_name_t     _rst;
	IRQn_Type           _irqn;
	clock_attach_id_t   _clk_attach; ///< clock source selector
	clock_div_name_t    _clk_div;    ///< clock divider name
	int                 _tx_pin;
	int                 _rx_pin;

	// ---- software ring buffer (RX) ----
	volatile uint8_t    _rx_buf[ RX_RING_BUF_SIZE ];
	volatile uint16_t   _rx_head;   ///< written by ISR
	volatile uint16_t   _rx_tail;   ///< read  by getc()

	// ---- user callbacks ----
	func_ptr            _rx_callback;
	func_ptr            _tx_callback;
};

#endif // R01LIB_SERIAL_H

#include "r01lib.h"

// ---- 2 UART channels -------------------------------------------------------

Serial  pc  ( USBTX, USBRX, 115200 );   // PC side
Serial  uart( D1,    D0,    115200 );   // Target
//Serial  uart( MB_TX, MB_RX, 115200 );   // Target

DigitalOut led( GREEN );
DigitalOut xfr( RED   );

Ticker		t;

bool	xfr_flag	= false;

// ---- RX callbacks ----------------------------------------------------------

// PC → Target: byte received from PC, forward to uart
void on_pc_rx( void )
{
    int c = pc.getc();
    if ( c != -1 )
        uart.putc( c );
}

// Target → PC: byte received from Target, forward to pc
void on_uart_rx( void )
{
    int c = uart.getc();
    if ( c != -1 )
        pc.putc( c );
}

void on_timer( void )
{
	led = !led;
}

// ---- main ------------------------------------------------------------------

int main( void )
{
    pc.attach  ( on_pc_rx,   Serial::RxIrq );
    uart.attach( on_uart_rx, Serial::RxIrq );

    t.attach( on_timer, 0.5 );
}

#include "r01lib.h"

//Serial		uart( USBTX, USBRX, 115200);
//Serial		uart( D1, D0, 115200);
Serial		uart( MB_TX, MB_RX, 115200);
DigitalOut	led(GREEN);

int main(void)
{
#if 0
	CLOCK_EnableClock(kCLOCK_Lpuart1);
	CLOCK_SetLpuart1Clock(1U);  // IRC48M = 48MHz
	PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt3);  // PTE0 = LPUART1_TX
	PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt3);  // PTE1 = LPUART1_RX

	lpuart_config_t config;
	LPUART_GetDefaultConfig(&config);
	config.baudRate_Bps = 115200U;
	config.enableTx = true;
	config.enableRx = true;
	LPUART_Init(LPUART1, &config, 48000000UL);

	LPUART_WriteBlocking(LPUART1, (uint8_t*)"TEST\r\n", 6);
#endif


	uart.printf("Hello, world!\r\n");

    while (true)
    {
        if ( uart.readable() )
        {
            int	c	= uart.getc();
            uart.putc(c);   // echo back
            led = !led;
        }
    }
}

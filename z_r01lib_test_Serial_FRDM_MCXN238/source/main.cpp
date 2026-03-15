
#include "r01lib.h"

//Serial		uart1( USBTX, USBRX, 115200);
//Serial		uart( MB_TX, MB_RX, 115200);
//Serial		uart( D1, D0, 115200);
Serial		uart1( USBTX, USBRX, 115200);
Serial uart2( D19, D18, 115200 );  // P1_17(TX)/P1_16(RX) -> FC5
DigitalOut	led(GREEN);

int main(void)
{
    Serial uart1( USBTX, USBRX, 115200 );
    uart1.printf("TEST USBTX\r\n");

    // P1_17をGPIOとしてトグル確認
    PORT_SetPinMux(PORT1, 17U, kPORT_MuxAsGpio);
    gpio_pin_config_t cfg = { kGPIO_DigitalOutput, 0 };
    GPIO_PinInit(GPIO1, 17U, &cfg);
    GPIO_PinWrite(GPIO1, 17U, 1);
    wait(1.0);
    GPIO_PinWrite(GPIO1, 17U, 0);
    wait(1.0);

    // その直後にUARTとして使う
    Serial uart2( D19, D18, 115200 );
    uart2.printf("TEST D19\r\n");

    while (true) {
        led = !led;
        wait(0.5);
    }
}

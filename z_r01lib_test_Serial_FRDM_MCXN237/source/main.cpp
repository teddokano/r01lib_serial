#include "r01lib.h"

extern "C" {
#include "fsl_lpuart.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_lpflexcomm.h"
#include "fsl_reset.h"
#include "fsl_gpio.h"
}

DigitalOut led(GREEN);

void toggle(int n) {
    for (int i = 0; i < n; i++) {
        led = !led;
        wait(0.2);
    }
    wait(1.0);
}

int main(void)
{
	// FC4テスト（動作確認済み）
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);
	RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);
	RESET_ReleasePeripheralReset(kFC4_RST_SHIFT_RSTn);
	LP_FLEXCOMM_Init(4U, LP_FLEXCOMM_PERIPH_LPUART);
	PORT_SetPinMux(PORT1, 9U, kPORT_MuxAlt2);
	PORT_SetPinMux(PORT1, 8U, kPORT_MuxAlt2);
	lpuart_config_t config4;
	LPUART_GetDefaultConfig(&config4);
	config4.baudRate_Bps = 115200U;
	config4.enableTx = true;
	config4.enableRx = true;
	LPUART_Init(LPUART4, &config4, 12000000UL);
	LPUART_WriteBlocking(LPUART4, (uint8_t*)"TEST FC4\r\n", 10);

	// FC2テスト（FC4と全く同じ手順）
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1U);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
	RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
	RESET_ReleasePeripheralReset(kFC2_RST_SHIFT_RSTn);
	LP_FLEXCOMM_Init(2U, LP_FLEXCOMM_PERIPH_LPUART);
	PORT_SetPinMux(PORT4, 2U, kPORT_MuxAlt2);  // D1=TX
	PORT_SetPinMux(PORT4, 3U, kPORT_MuxAlt2);  // D0=RX
	lpuart_config_t config2;
	LPUART_GetDefaultConfig(&config2);
	config2.baudRate_Bps = 115200U;
	config2.enableTx = true;
	config2.enableRx = true;
	LPUART_Init(LPUART2, &config2, 12000000UL);
	LPUART_WriteBlocking(LPUART2, (uint8_t*)"TEST FC2\r\n", 10);

	volatile uint32_t fc2_freq = CLOCK_GetLPFlexCommClkFreq(2U);
	(void)fc2_freq;  // ← ここにブレークポイント、fc2_freqの値を確認

	volatile uint32_t *lpuart2_ctrl = (uint32_t*)0x40094018;  // CTRL register
	volatile uint32_t ctrl_val = *lpuart2_ctrl;
	(void)ctrl_val;  // ← ここにブレークポイント

	volatile uint32_t *lpuart2_stat = (uint32_t*)0x40094014;  // STAT register
	volatile uint32_t stat_val = *lpuart2_stat;
	(void)stat_val;  // ← ここにブレークポイント

    while (true) {
        led = !led;
        wait(1.0);
    }
}

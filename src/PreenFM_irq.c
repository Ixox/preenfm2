/*
 * Copyright 2013
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PreenFM.h"
#include "usb_hcd_int.h"
#include "usb_dcd_int.h"
#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

RingBuffer<uint8_t, 200> usartBufferIn  __attribute__ ((section(".ccmnoload")));
RingBuffer<uint8_t, 100> usartBufferOut  __attribute__ ((section(".ccmnoload")));
unsigned int preenTimer  __attribute__ ((section(".ccm"))) = 0;


/*
 * Interrupt handlers.
 */
#ifdef __cplusplus
extern "C" {
#endif


void blink(void) {
	while (1) {
		strobePin(2, 0x150000);
		strobePin(8, 0x60000);
	}
	return;
}

void NMI_Handler() {
	lcd.setRealTimeAction(true);
	lcd.setCursor(0,0);
	lcd.print("NMI_Handler");
	blink();
}

void printHex32bits(unsigned int dec) {
	char hexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	for (int pos = 7; pos >= 0; pos --) {
		int value = (dec >> (pos * 4)) & 0xF;
		lcd.print(hexChars[value]);
	}

}


void StackDebug(unsigned int * hardfault_args) {
	lcd.setRealTimeAction(true);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("HrdFlt  LR:");
	printHex32bits(hardfault_args[5]);
	lcd.setCursor(0,1);
	printHex32bits(hardfault_args[6]);
	lcd.print("   ");
	printHex32bits(hardfault_args[7]);
	lcd.setCursor(0,2);
	printHex32bits(hardfault_args[0]);
	lcd.print("   ");
	printHex32bits(hardfault_args[1]);
	lcd.setCursor(0,3);
	printHex32bits(hardfault_args[2]);
	lcd.print("   ");
	printHex32bits(hardfault_args[3]);

/*stacked_r0 = ((unsigned long) hardfault_args[0]);
stacked_r1 = ((unsigned long) hardfault_args[1]);
stacked_r2 = ((unsigned long) hardfault_args[2]);
stacked_r3 = ((unsigned long) hardfault_args[3]);

stacked_r12 = ((unsigned long) hardfault_args[4]);
stacked_lr = ((unsigned long) hardfault_args[5]);
stacked_pc = ((unsigned long) hardfault_args[6]);
stacked_psr = ((unsigned long) hardfault_args[7]);
*/

	blink();
}


void HardFault_Handler() {

	// hard fault handler wrapper in assembly
	// it extract the location of stack frame and pass it
	// to handler in C as pointer.
	int address;
	void *jmp = (void*)StackDebug;

	asm volatile (
			"TST LR, #4\n\t"
			"ITE EQ\n\t"
			"MRSEQ R0, MSP\n\t"
			"MRSNE R0, PSP\n\t"
			"BX %0\n\t"
			:
			: "r" (jmp)
			:
	);
}



void MemManage_Handler() {
	lcd.setRealTimeAction(true);
	lcd.setCursor(0,0);
	lcd.print("Mem Manage");
	blink();
}

void BusFault_Handler() {
	lcd.setRealTimeAction(true);
	lcd.setCursor(0,0);
	lcd.print("Bus Fault");
	blink();
}

void UsageFault_Handler() {
	lcd.setRealTimeAction(true);
	lcd.setCursor(0,0);
	lcd.print("Usage Fault");
	blink();
}


void USART3_IRQHandler(void) {
	uint8_t data;

	if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
		if (usartBufferOut.getCount() > 0) {
			USART_SendData(USART3, usartBufferOut.remove());
		} else {
			//disable Transmit Data Register empty interrupt
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
		}
	}


	//if Receive interrupt (rx not empty)
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		uint8_t newByte = USART_ReceiveData(USART3);
		usartBufferIn.insert(newByte);

		if (synthState.fullState.midiConfigValue[MIDICONFIG_THROUGH] == 1) {
	    	usartBufferOut.insert(newByte);
	    	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	    }
	}

}



void SysTick_Handler(void)
{

    int sample;

	switch (spiState) {
	case 0:
        // LATCH LOW
        GPIO_ResetBits(GPIOC, GPIO_Pin_12);
		// Update timer
		preenTimer += 1;
		// DAC 1 - MSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		GPIO_SetBits(GPIOB, GPIO_Pin_9);
        sample = (int)synth.rightSampleAtReadCursor();
        sample >>= 6;
        // LATCH HIGH
        GPIO_SetBits(GPIOC, GPIO_Pin_12);
		SPI_I2S_SendData(SPI1, 0x3000 | sample );
		spiState++;

		break;
	case 1:
		// DAC 2 - MSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_9);
		GPIO_SetBits(GPIOB, GPIO_Pin_4);
		sample = (int)synth.leftSampleAtReadCursor();
        sample >>= 6;
		SPI_I2S_SendData(SPI1, 0x3000 | sample);
        spiState++;
		break;
	case 2:
		// DAC 1 - LSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		GPIO_SetBits(GPIOB, GPIO_Pin_9);
		sample = (int)synth.rightSampleAtReadCursor() & 0x3f;
        SPI_I2S_SendData(SPI1, 0xb000 | sample);
		spiState++;
		break;
	case 3:
		// DAC 2 - LSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_9);
		GPIO_SetBits(GPIOB, GPIO_Pin_4);
		sample = (int)synth.leftSampleAtReadCursor() & 0x3f;
		SPI_I2S_SendData(SPI1, 0xb000 | sample);
        spiState = 0;
        synth.incReadCursor();
		break;
	}
}


#ifdef USE_USB_OTG_HS
void OTG_HS_IRQHandler(void) {
	USBH_OTG_ISR_Handler(&usbOTGHost);
}
#endif

#ifdef USE_USB_OTG_FS
void OTG_FS_IRQHandler(void) {
	USBD_OTG_ISR_Handler(&usbOTGDevice);
}
#endif


#ifdef __cplusplus
}
#endif

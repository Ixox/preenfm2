/*
 * PreenFMIrq.c
 *
 *  Created on: Sep 28, 2012
 *      Author: xhosxe
 */


#include "PreenFM.h"
#include "usb_hcd_int.h"
#include "usb_dcd_int.h"


RingBuffer<uint8_t, 200> usartBuffer;
unsigned int preenTimer = 0;


/*
 * Interrupt handlers.
 */
#ifdef __cplusplus
extern "C" {
#endif


void nmi_handler(void) {
	while (1) {
		strobePin(2, 0x150000);
		strobePin(8, 0x60000);
	}
	return;
}

void hardfault_handler(void) {
	while (1) {
		strobePin(2, 0x150000);
		strobePin(8, 0x60000);
	}
	return;
}

void USART3_IRQHandler(void) {
	usartBuffer.insert((char) USART3->DR);
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
        samples.sampleLeftMSB = 0x3000;
        samples.sampleLeftMSB |= sample >> 6;

        samples.sampleLeftLSB = 0xB000;
        samples.sampleLeftLSB |= sample & 0x3f;

        // LATCH HIGH
        GPIO_SetBits(GPIOC, GPIO_Pin_12);

		SPI_I2S_SendData(SPI1, samples.sampleLeftMSB );
		spiState = 1;

		break;
	case 1:
		// DAC 2 - MSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_9);
		GPIO_SetBits(GPIOB, GPIO_Pin_4);

		sample = (int)synth.leftSampleAtReadCursor();

        samples.sampleRightMSB = 0x3000;
        samples.sampleRightMSB |= sample >> 6;

        samples.sampleRightLSB = 0xB000;
        samples.sampleRightLSB |= sample & 0x3f;

        spiState = 2;

		SPI_I2S_SendData(SPI1, samples.sampleRightMSB );
		break;
	case 2:
		// DAC 1 - LSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_4);
		GPIO_SetBits(GPIOB, GPIO_Pin_9);

		spiState = 3;

        SPI_I2S_SendData(SPI1, samples.sampleLeftLSB);
		break;
	case 3:
		// DAC 2 - LSB
		GPIO_ResetBits(GPIOB, GPIO_Pin_9);
		GPIO_SetBits(GPIOB, GPIO_Pin_4);
        spiState = 0;
        synth.incReadCursor();

		SPI_I2S_SendData(SPI1, samples.sampleRightLSB);
        break;
	}
}


#ifdef USE_USB_OTG_HS
void OTG_HS_IRQHandler(void) {
	cpt2 ++;
	USBH_OTG_ISR_Handler(&usbOTGHost);
}
#endif

#ifdef USE_USB_OTG_FS
void OTG_FS_IRQHandler(void) {
	cpt1 ++;
	USBD_OTG_ISR_Handler(&usbOTGDevice);
}
#endif


#ifdef __cplusplus
}
#endif

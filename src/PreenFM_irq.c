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
extern Synth synth;
extern uint32_t dmaSampleBuffer[128];

RingBuffer<uint8_t, 200> usartBufferIn  __attribute__ ((section(".ccmnoload")));
RingBuffer<uint8_t, 100> usartBufferOut  __attribute__ ((section(".ccmnoload")));
unsigned int preenTimer  __attribute__ ((section(".ccm"))) = 0;


// DEBUG CVIN
#ifdef CVINDEBUG
unsigned int preenTimer2 = 0;
int cptTIM2 = 0;
int TIM2PerSeq = 0;
#endif

void fillSoundBuffer() {
    int cpt = 0;
    if (synth.getSampleCount() < 192) {
        while (synth.getSampleCount() < 128 && cpt++<20)
            synth.buildNewSampleBlockMcp4922();
    }
}


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

void assert_failed(char *file, unsigned int line) {
    strobePin(10, 0x150000);
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
    lcd.print("PFM2 v"PFM2_VERSION" "OVERCLOCK_STRING);
    lcd.setCursor(0,1);
    lcd.print("LR: 0x");
    printHex32bits(hardfault_args[5]);
    lcd.setCursor(0,2);
    lcd.print("PC: 0x");
    printHex32bits(hardfault_args[6]);
    lcd.setCursor(0,3);
    lcd.print("r0: 0x");
    printHex32bits(hardfault_args[0]);

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

    // Commented becuase it does not happen anymore / Was for debuging    
    // if (USART_GetITStatus(USART3, USART_IT_ORE_RX) != RESET) {
    //     usartError++;
    //     uint8_t newByte = USART_ReceiveData(USART3);
    //     usartBufferIn.insert(newByte);
    // } else 
    if ((USART3->SR & USART_FLAG_RXNE) != 0) {
        //if Receive interrupt (rx not empty)
        /* USART_ClearITPendingBit doc : 
         * @note   RXNE pending bit can be also cleared by a read to the USART_DR register 
         *          (USART_ReceiveData()).
         */
        uint8_t newByte = USART3->DR;
        usartBufferIn.insert(newByte);

        if (synthState.fullState.midiConfigValue[MIDICONFIG_THROUGH] == 1) {
            usartBufferOut.insert(newByte);
            USART3->CR1 |= USART_FLAG_TXE;
        }
    } 
    
    if ((USART3->SR & USART_FLAG_TXE) != 0) {
        if (usartBufferOut.getCount() > 0) {
            USART3->DR = usartBufferOut.remove();
        } else {
            //disable Transmit Data Register empty interrupt
            USART3->CR1 &= ~USART_FLAG_TXE;
        }
    } 
}



void SysTick_Handler(void)
{
    static int left  __attribute__ ((section(".ccmnoload")));
    static int right  __attribute__ ((section(".ccmnoload")));


    switch (spiState++) {
        case 0:
            // samples are int so no FP operation; this allows Lazy stacking feature
            // to avoid saving FPU registers
            // LATCH LOW
            GPIOC->BSRRH = GPIO_Pin_12;
            // Update timer
            preenTimer += 1;

            // Read samples...
            left = synth.leftSampleAtReadCursor();
            right = synth.rightSampleAtReadCursor();
            synth.incReadCursor();

            // DAC 1 - MSB
            // BSRRH RESET
            // BSRRL SET
            GPIOB->BSRRH = GPIO_Pin_4;
            GPIOB->BSRRL = GPIO_Pin_9;
            // LATCH HIGH
            GPIOC->BSRRL = GPIO_Pin_12;
            SPI1->DR = 0x3000 | (right >> 6);
            break;
        case 1:
            // DAC 2 - MSB
            GPIOB->BSRRH = GPIO_Pin_9;
            GPIOB->BSRRL = GPIO_Pin_4;
            SPI1->DR = 0x3000 | (left >> 6);
            break;
        case 2:
            // DAC 1 - LSB
            GPIOB->BSRRH = GPIO_Pin_4;
            GPIOB->BSRRL = GPIO_Pin_9;
            SPI1->DR = 0xb000 | (right & 0x3f);
            break;
        case 3:
            // DAC 2 - LSB
            GPIOB->BSRRH = GPIO_Pin_9;
            GPIOB->BSRRL = GPIO_Pin_4;
            SPI1->DR = 0xb000 | (left & 0x3f);
            spiState = 0;
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



void TIM2_IRQHandler() {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
#ifdef CVINDEBUG
        cptTIM2 ++;
        if (cptTIM2 >= 5000) {
            int t = preenTimer - preenTimer2;
            float s = PREENFM_FREQUENCY_INVERSED * t;
            TIM2PerSeq = ((float)cptTIM2 / s);
            preenTimer2 = preenTimer;
            cptTIM2 = 0;
        }
#endif

}


/*
 * CS4344 half complete or complete DMA transfer.
 * We must prepare the other Half
 */
void DMA1_Stream5_IRQHandler() {
    if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5)) {
        // Clear DMA Stream Half Transfer interrupt pending bit
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
        // 42000 / 128 = 1312 per seconds
        preenTimer++;
        // Fill part 1
        synth.buildNewSampleBlockCS4344(dmaSampleBuffer);
    } else if (DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5)) {
        // Clear DMA Stream Total Transfer complete interrupt pending bit
        DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        preenTimer++;
        // Fill part 2
        synth.buildNewSampleBlockCS4344(&dmaSampleBuffer[64]);
    }

}

#ifdef __cplusplus
}
#endif



#include "stm32f4xx_conf.h"
#include "RingBuffer.h"
#include "LiquidCrystal.h"
#include "usb_core.h"
#include "Synth.h"

#ifndef _PreenFM_H_
#define _PreenFM_H_

#define LEDPIN GPIO_Pin_6


struct sampleForSPI {
    uint16_t sampleLeftMSB ;
    uint16_t sampleLeftLSB;
    uint16_t sampleRightMSB;
    uint16_t sampleRightLSB;

};

extern struct sampleForSPI samples;
extern int spiState ;

extern LiquidCrystal lcd;
extern Synth synth;
extern RingBuffer<u8, 200> usartBuffer;
extern USB_OTG_CORE_HANDLE  usbOTGHost;
extern USB_OTG_CORE_HANDLE          usbOTGDevice;

extern uint32_t cpt1 ;
extern uint32_t cpt2 ;
extern int state;
extern bool usbReady;
extern char usbHostText[128];
extern unsigned int preenTimer;
extern uint8_t midiBuff[4];
extern uint8_t midiNote;

extern Synth synth;

void USART_Config();
void LED_Config();
void MCP4922_Config();
void SysTick_Config();
void RNG_Config();


void strobePin(u8 count, u32 rate);

#endif

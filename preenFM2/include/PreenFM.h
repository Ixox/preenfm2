/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <dot> hosxe (at) g m a i l <dot> com)
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

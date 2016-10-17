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
#include "MidiDecoder.h"

#ifndef _PreenFM_H_
#define _PreenFM_H_


#ifndef OVERCLOCK
#define OVERCLOCK_STRING
#else
#define OVERCLOCK_STRING "o"
#endif

#ifndef CVIN
#define CVIN_STRING
#else
#define CVIN_STRING "cv"
#endif


#define LEDPIN GPIO_Pin_6


extern int spiState ;

extern LiquidCrystal lcd;
extern Synth synth;
extern SynthState         synthState;
extern MidiDecoder        midiDecoder;
extern RingBuffer<uint8_t, 200> usartBufferIn;
extern RingBuffer<uint8_t, 100> usartBufferOut;

extern USB_OTG_CORE_HANDLE  usbOTGHost;
extern USB_OTG_CORE_HANDLE  usbOTGDevice;
extern USBD_Usr_cb_TypeDef midiStreamingUsrCallback;


extern uint32_t cpt1 ;
extern uint32_t cpt2 ;
extern int state;
extern bool usbReady;
extern char usbHostText[128];
extern unsigned int preenTimer;

extern Synth synth;

#ifdef CVIN
extern uint16_t ADCBuffer[];
#endif
#ifdef CVINDEBUG
extern int TIM2PerSeq;
#endif


void USART_Config();
void LED_Config();
void MCP4922_Config();
void SysTick_Config();
void RNG_Config();
void LCD_InitChars(LiquidCrystal *lcd);
void ADC_Config();



void strobePin(u8 count, u32 rate);

#endif

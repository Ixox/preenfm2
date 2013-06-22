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
#include "Encoders.h"
#include "usbh_core.h"
#include "usbKey_usr.h"
#include "usbh_msc_core.h"
#include "usbd_core.h"
#include "usbd_usr.h"
#include "usbd_preenFM_desc.h"
#include "usbd_midi_core.h"
#include "FMDisplay.h"
#include "Synth.h"
#include "RingBuffer.h"
#include "MidiDecoder.h"
#include "UsbKey.h"

#include "ff.h"

uint8_t midiBuff[4];
uint8_t midiNote = 0;
char keyName[32];

LiquidCrystal      lcd __attribute__ ((section(".ccmnoload")));
SynthState         synthState __attribute__ ((section(".ccmnoload")));
Synth              synth __attribute__ ((section(".ccmnoload")));
MidiDecoder        midiDecoder __attribute__ ((section(".ccmnoload")));
Encoders           encoders  __attribute__ ((section(".ccmnoload")));
FMDisplay          fmDisplay  __attribute__ ((section(".ccmnoload"))) ;
UsbKey             usbKey  __attribute__ ((section(".ccmnoload"))) ;

uint32_t cpt1 = 0;
uint32_t cpt2 = 0;

USB_OTG_CORE_HANDLE          usbOTGDevice;

// Init left/right
struct sampleForSPI samples __attribute__ ((section(".ccmnoload"))) ;
int spiState __attribute__ ((section(".ccm"))) = 0;


void fillSoundBuffer() {
    int cpt = 0;
    if (synth.getSampleCount() < 192) {
        while (synth.getSampleCount() < 128 && cpt++<20)
            synth.buildNewSampleBlock();
    }
}


void setup() {

    unsigned char midiIn[8] = {
            0b01100,
            0b10010,
            0b10010,
            0b01100,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };

    unsigned char midiOut[8] = {
            0b01100,
            0b11110,
            0b11110,
            0b01100,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };

    unsigned char minusPoint[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00011,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };

/*
    unsigned char modified[8] = {
            0b00101,
            0b00010,
            0b00101,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };
*/
    unsigned char stepCursor[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10001,
            0b01110,
            0b00100,
            0b00100,
    };

    unsigned char stepPos[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00100,
            0b00100,
            0b00000,
    };

    unsigned char firstSteps[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10000,
            0b10000,
    };


    unsigned char thirdStep[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10100,
            0b10100,
    };

    unsigned char note[8] = {
            0b00100,
            0b00110,
            0b00101,
            0b00101,
            0b00100,
            0b11100,
            0b11100,
            0b00000,
    };



    lcd.begin(20, 4);
    lcd.createChar(0, midiIn);
    lcd.createChar(1, midiOut);
    lcd.createChar(2, minusPoint);
    lcd.createChar(3, stepCursor);
    lcd.createChar(4, stepPos);
    lcd.createChar(5, firstSteps);
    lcd.createChar(6, thirdStep);
    lcd.createChar(7, note);


	LED_Config();
	USART_Config();
	MCP4922_Config();
	RNG_Config();


    lcd.setCursor(0,1);
    lcd.print("PreenFM mk2 v0.1");
	lcd.setCursor(5,2);
    lcd.print("By Xavier Hosxe");

    // XH : 1/2 of seconde to stabilise
    // Important Prevent USB from failing
    int us = 80000000 / 2;

    /* fudge for function call overhead  */
    //us--;
    asm volatile("   mov r0, %[us]          \n\t"
                 "1: subs r0, #1            \n\t"
                 "   bhi 1b                 \n\t"
                 :
                 : [us] "r" (us)
                 : "r0");


    // Init core FS as a midiStreaming device
#ifdef USE_USB_OTG_FS
    USBD_Init(&usbOTGDevice, USB_OTG_FS_CORE_ID, &preenFMDescriptor, &midiCallback, &midiStreamingUsrCallback);
#endif

    // ---------------------------------------
    // Dependencies Injection

    // to SynthStateAware Class
    // MidiDecoder, Synth (Env,Osc, Lfo, Matrix, Voice ), FMDisplay, PresetUtil...

    synth.setSynthState(&synthState);
    fmDisplay.setSynthState(&synthState);
    midiDecoder.setSynthState(&synthState);
    midiDecoder.setVisualInfo(&fmDisplay);
    PresetUtil::setSynthState(&synthState);
    midiDecoder.setSynth(&synth);
    midiDecoder.setStorage(&usbKey);

    // ---------------------------------------
    // Register listener

    // synthstate is updated by encoder change
    encoders.insertListener(&synthState);

    // fmDisplay and synth needs to be aware of synthState changes
    // synth must be first one, can modify param new value
    /// order of param listener is important... synth must be called first so it's inserted last.
    synthState.insertParamListener(&fmDisplay);
    synthState.insertParamListener(&midiDecoder);
    synthState.insertParamListener(&synth);
    synthState.insertMenuListener(&fmDisplay);
    // Synth can check and modify param new value
    synthState.insertParamChecker(&synth);

    synthState.setStorage(&usbKey);

    usbKey.init(synth.getTimbre(0)->getParamRaw(), synth.getTimbre(1)->getParamRaw(), synth.getTimbre(2)->getParamRaw(), synth.getTimbre(3)->getParamRaw());
    usbKey.loadConfig(synthState.fullState.midiConfigValue);
    if (usbKey.loadDefaultCombo()) {
    	synthState.propagateAfterNewComboLoad();
    }

    // launch the engine !!
    // Init DAC number
    samples.sampleLeftMSB = 0x3 << 12;
    samples.sampleLeftLSB = 0xB << 12;
    samples.sampleRightMSB = 0x3 << 12;
    samples.sampleRightLSB = 0xB << 12;

    fmDisplay.init(&lcd, &usbKey);

    int bootOption = synthState.fullState.midiConfigValue[MIDICONFIG_BOOT_START];

    if (bootOption == 0) {
        fmDisplay.displayPreset();
        fmDisplay.setRefreshStatus(12);
    } else {
        // Menu
        synthState.buttonPressed(BUTTON_MENUSELECT);
        // Load
        synthState.buttonPressed(BUTTON_MENUSELECT);
        if (bootOption == 6) {
            // Internal bank
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else {
            // User
            synthState.buttonPressed(BUTTON_MENUSELECT);
            // Bank
            for (int k = 0; k < bootOption - 1; k++) {
                synthState.encoderTurned(0, 1);
            }
            synthState.buttonPressed(BUTTON_MENUSELECT);
        }
    }


    SysTick_Config();
    synth.buildNewSampleBlock();
    synth.buildNewSampleBlock();
}

int mainCpt = 0;
unsigned short midiReceive = 0;
unsigned int ledMicros = 0;
unsigned int encoderMicros = 0;
unsigned int midiInMicros = 0;
unsigned int tempoMicros = 0;

bool ledOn = false;

void loop(void) {
    fillSoundBuffer();

    mainCpt++;
    unsigned int newMicros = preenTimer;

    /*
    if ((newMicros - ledMicros) > 10000) {
        if (ledOn) {
            GPIO_ResetBits(GPIOB, LEDPIN);
        } else {
            GPIO_SetBits(GPIOB, LEDPIN);
        }
        ledOn = !ledOn;
        ledMicros = newMicros;
	}
	*/

    while (midiDecoder.hasMidiToSend()) {
            fillSoundBuffer();
            midiDecoder.sendMidiOut();
    }

    while (usartBuffer.getCount() > 0) {
        fillSoundBuffer();
		midiDecoder.newByte(usartBuffer.remove());
	}

	// Comment following line for debug....
	lcd.setRealTimeAction(false);
    if ((newMicros - encoderMicros) > 80) {
        fillSoundBuffer();
        encoders.checkStatus(synthState.fullState.midiConfigValue[MIDICONFIG_ENCODER]);
        encoderMicros = newMicros;
    } else if (fmDisplay.needRefresh() && ((mainCpt & 0x3) == 0)) {
        fillSoundBuffer();
        fmDisplay.refreshAllScreenByStep();
    }
    lcd.setRealTimeAction(true);
    while (lcd.hasActions()) {
        if (usartBuffer.getCount() > 100) {
            while (usartBuffer.getCount() > 0) {
                fillSoundBuffer();
                midiDecoder.newByte(usartBuffer.remove());
            }
        }
        LCDAction action = lcd.nextAction();
        lcd.realTimeAction(&action, fillSoundBuffer);
    }

    if ((newMicros - tempoMicros) > 10000) {
         fillSoundBuffer();
         synthState.tempoClick();
         fmDisplay.tempoClick();
         tempoMicros = newMicros;
     }
}

int main(void) {
	setup();
	while (1) {
		loop();
	}
}

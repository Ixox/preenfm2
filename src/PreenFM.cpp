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
#include "usbd_midi_desc.h"
#include "usbd_midi_core.h"
#include "FMDisplay.h"
#include "Synth.h"
#include "RingBuffer.h"
#include "MidiDecoder.h"
#include "UsbKey.h"
#include "Hexter.h"

#include "ff.h"

#ifndef OVERCLOCK
#define OVERCLOCK_STRING
#else
#define OVERCLOCK_STRING "o"
#endif

SynthState         synthState __attribute__ ((section(".ccmnoload")));
Synth              synth __attribute__ ((section(".ccmnoload")));

// No need to put the following in the CCM memory
USB_OTG_CORE_HANDLE          usbOTGDevice;
LiquidCrystal      lcd ;
FMDisplay          fmDisplay ;
MidiDecoder        midiDecoder;
Encoders           encoders ;
UsbKey             usbKey ;
Hexter             hexter;


struct sampleForSPI samples  __attribute__ ((section(".ccmnoload")));
int spiState  __attribute__ ((section(".ccmnoload")));

void fillSoundBuffer() {
    int cpt = 0;
    if (synth.getSampleCount() < 192) {
        while (synth.getSampleCount() < 128 && cpt++<20)
            synth.buildNewSampleBlock();
    }
}

const char* line1 = "PreenFM2 v"PFM2_VERSION" "OVERCLOCK_STRING;
const char* line2 = "     By Xavier Hosxe";


void setup() {
    lcd.begin(20, 4);

    LCD_InitChars(&lcd);

    for (int r=0; r<20; r++) {
    	lcd.setCursor(r,0);
    	lcd.print((char)0);
    	lcd.setCursor(r,1);
    	lcd.print((char)0);
    	lcd.setCursor(r,2);
    	lcd.print((char)0);
    	lcd.setCursor(r,3);
    	lcd.print((char)0);
    }

    LED_Config();
	USART_Config();
	MCP4922_Config();
	RNG_Config();

    // ---------------------------------------
    // Dependencies Injection

    // to SynthStateAware Class
    // MidiDecoder, Synth (Env,Osc, Lfo, Matrix, Voice ), FMDisplay, PresetUtil...

    synth.setSynthState(&synthState);
    fmDisplay.setSynthState(&synthState);
    midiDecoder.setSynthState(&synthState);
    midiDecoder.setVisualInfo(&fmDisplay);
    PresetUtil::setSynthState(&synthState);
    PresetUtil::setStorage(&usbKey);
    PresetUtil::setMidiDecoder(&midiDecoder);
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
    synthState.setHexter(&hexter);

    usbKey.init(synth.getTimbre(0)->getParamRaw(), synth.getTimbre(1)->getParamRaw(), synth.getTimbre(2)->getParamRaw(), synth.getTimbre(3)->getParamRaw());
    // usbKey and hexter needs to know if arpeggiator must be loaded and saved
    usbKey.setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);
    hexter.setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);
    usbKey.loadConfig(synthState.fullState.midiConfigValue);

    // launch the engine !!
    // Init DAC number
    samples.sampleLeftMSB = 0x3 << 12;
    samples.sampleLeftLSB = 0xB << 12;
    samples.sampleRightMSB = 0x3 << 12;
    samples.sampleRightLSB = 0xB << 12;


    SysTick_Config();
    synth.buildNewSampleBlock();
    synth.buildNewSampleBlock();

    // shorten the release value for init sound...
    ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env1b.releaseTime = 1.1f;
    ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env4b.releaseTime = 0.8f;

    bool displayline1 = true;
    for (int r=0; r<20; r++) {
    	switch (r) {
    	case 0:
        	synth.noteOn(0, 40, 120);
        	break;
    	case 1:
    		synth.noteOff(0, 40);
    		break;
    	case 3:
        	synth.noteOn(0, 52, 120);
        	break;
    	case 4:
    		synth.noteOff(0,52);
    		break;
    	}

    	for (char s=1; s<6; s++) {
    	    fillSoundBuffer();
			lcd.setCursor(r,0);
			lcd.print(s);
		    fillSoundBuffer();
			lcd.setCursor(r,1);
			lcd.print(s);
		    fillSoundBuffer();
			lcd.setCursor(r,2);
			lcd.print(s);
		    fillSoundBuffer();
			lcd.setCursor(r,3);
			lcd.print(s);
		    for (int i=0; i<100; i++) {
		        fillSoundBuffer();
				PreenFM2_uDelay(50);
		    }
    	}

        fillSoundBuffer();


    	if (displayline1) {
    		if (line1[r] != 0) {
    			lcd.setCursor(r,1);
    			lcd.print(line1[r]);
    		} else {
    			displayline1 = false;
    		}
    	}

        fillSoundBuffer();
		lcd.setCursor(r,2);
		lcd.print(line2[r]);
	    fillSoundBuffer();
    }




    for (int i=0; i<4000; i++) {
        fillSoundBuffer();
		PreenFM2_uDelay(250);
    }



    // FS = Full speed : midi
    // HS = high speed : USB Key
    // Init core FS as a midiStreaming device
    if (synthState.fullState.midiConfigValue[MIDICONFIG_USB] != USBMIDI_OFF) {
    	USBD_Init(&usbOTGDevice, USB_OTG_FS_CORE_ID, &usbdMidiDescriptor, &midiCallback, &midiStreamingUsrCallback);
    }

    if (usbKey.loadDefaultCombo()) {
    	synthState.propagateAfterNewComboLoad();
    }

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
        if (bootOption == 1) {
        	// Bank
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else if (bootOption == 2) {
        	// Combo
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else if (bootOption == 3) {
        	// DX7
            synthState.encoderTurned(0, 1);
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        }
        // First preset...
        synthState.buttonPressed(BUTTON_MENUSELECT);
    }


}

unsigned int ledMicros = 0;
unsigned int encoderMicros = 0;
unsigned int tempoMicros = 0;

bool ledOn = false;

void loop(void) {
    fillSoundBuffer();

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


	// Comment following line for debug....
	lcd.setRealTimeAction(false);

	// newByte can display visual info
    while (usartBufferIn.getCount() > 0) {
        fillSoundBuffer();
		midiDecoder.newByte(usartBufferIn.remove());
	}

	if ((newMicros - encoderMicros) > 80) {
        fillSoundBuffer();
        encoders.checkStatus(synthState.fullState.midiConfigValue[MIDICONFIG_ENCODER]);
        encoderMicros = newMicros;
    } else if (fmDisplay.needRefresh()) {
        fillSoundBuffer();
        fmDisplay.refreshAllScreenByStep();
    }

    if ((newMicros - tempoMicros) > 10000) {
         fillSoundBuffer();
         synthState.tempoClick();
         fmDisplay.tempoClick();
         tempoMicros = newMicros;
     }

    lcd.setRealTimeAction(true);
    while (lcd.hasActions()) {
        if (usartBufferIn.getCount() > 20) {
            while (usartBufferIn.getCount() > 0) {
                fillSoundBuffer();
                midiDecoder.newByte(usartBufferIn.remove());
            }
        }
        LCDAction action = lcd.nextAction();
        lcd.realTimeAction(&action, fillSoundBuffer);
    }

}

int main(void) {
	setup();
	while (1) {
		loop();
	}
}

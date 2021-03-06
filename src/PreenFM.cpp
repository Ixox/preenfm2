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
#include "Storage.h"
#include "Hexter.h"
#include "CVIn.h"

#include "ff.h"




SynthState         synthState __attribute__ ((section(".ccmnoload")));
Synth              synth __attribute__ ((section(".ccmnoload")));

// No need to put the following in the CCM memory
USB_OTG_CORE_HANDLE          usbOTGDevice;
LiquidCrystal      lcd ;
FMDisplay          fmDisplay ;
MidiDecoder        midiDecoder;
Encoders           encoders ;
Storage            usbKey ;
Hexter             hexter;
#ifdef CVIN
CVIn               cvin;
#endif
int spiState  __attribute__ ((section(".ccmnoload")));
float PREENFM_FREQUENCY __attribute__ ((section(".ccmnoload")));;
float PREENFM_FREQUENCY_INVERSED __attribute__ ((section(".ccmnoload")));;
float PREENFM_FREQUENCY_INVERSED_LFO __attribute__ ((section(".ccmnoload")));;

// Must be in memory accessible by DMA
int32_t dmaSampleBuffer[128];

uint16_t pinTest;

void setup() {
    // All bits for preemption
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    // What PCB version;
    synthState.setPcbVersion(getPcbVersion());

    if (synthState.getPcbVersion() == PCB_R5) {
        // 192000000 / 1142 / 4  :
        PREENFM_FREQUENCY = 42031.52f;
        synth.setDacNumberOfBits(18);
        pinTest = GPIO_Pin_4;
    } else {
        // See configurator
        // 43 500 000 / 1024 = 
        PREENFM_FREQUENCY = 42480.47f;
        synth.setDacNumberOfBits(24);
        pinTest = GPIO_Pin_14;
    }
    LEDFront_Config();
    LEDTest_Config(pinTest);

    

    PREENFM_FREQUENCY_INVERSED = 1.0f / PREENFM_FREQUENCY;
    PREENFM_FREQUENCY_INVERSED_LFO = PREENFM_FREQUENCY_INVERSED * 32.0f;

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

	USART_Config();
	RNG_Config();

	// Set flush to zero mode...
	// FPU will treat denormal value as 0

	//	You can avoid some of these support code requirements by:
	//enabling flush-to-zero mode, by setting the FZ bit, FPSCR[24], to 1
	//enabling default NaN mode, by setting the DN bit, FPSCR[25], to 1.
	//Some of the other support code requirements only occur when the appropriate feature is enabled. You enable:
	//Inexact exceptions by setting the IXE bit, FPSCR[12], to 1
	//Overflow exceptions by setting the OFE bit, FPSCR[10], to 1
	//Invalid Operation exceptions by setting the IOE bit, FPSCR[8], to 1.
	// Fast mode
	FPU->FPDSCR |= FPU_FPDSCR_FZ_Msk;
	FPU->FPDSCR |= FPU_FPDSCR_DN_Msk;
	FPU->FPDSCR &= ~(1UL << 12);
	FPU->FPDSCR &= ~(1UL << 10);
	FPU->FPDSCR &= ~(1UL << 8);
    // ---------------------------------------
    // Dependencies Injection

    // to SynthStateAware Class
    // MidiDecoder, Synth (Env,Osc, Lfo, Matrix, Voice ), FMDisplay, PresetUtil...

    synth.setSynthState(&synthState);
    fmDisplay.setSynthState(&synthState);
    midiDecoder.setSynthState(&synthState);
    midiDecoder.setVisualInfo(&fmDisplay);
#ifdef CVIN
    // synth needs visualInfo for CV gate
    synth.setVisualInfo(&fmDisplay);
    synth.setCVIn(&cvin);
#endif
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

    // Synth must be second to ba called first (to update global tuning before it's displayed)
    synthState.insertMenuListener(&fmDisplay);
    synthState.insertMenuListener(&synth);
    // Synth can check and modify param new value
    synthState.insertParamChecker(&synth);

    synthState.setStorage(&usbKey);
    synthState.setHexter(&hexter);


    usbKey.getPatchBank()->setSysexSender(&midiDecoder);
    // usbKey and hexter needs to know if arpeggiator must be loaded and saved
    usbKey.getPatchBank()->setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);
    hexter.setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);


    usbKey.init(synth.getTimbre(0)->getParamRaw(), synth.getTimbre(1)->getParamRaw(), synth.getTimbre(2)->getParamRaw(), synth.getTimbre(3)->getParamRaw());
    uint32_t waitCpt = 0;
    if (!usbKey.isKeyReady()) {
    	lcd.setCursor(0, 1);
    	lcd.print("   Insert Usb Key   ");
    	lcd.setCursor(0, 2);
    	lcd.print("                    ");
    }
    // Animation while waiting for Usb Key
    while (!usbKey.isKeyReady()) {
    	lcd.setCursor(0, 2);
    	lcd.print("                    ");
        int start = (waitCpt % 30) - 10;
        start = start > 0 ? start : 0;
        int end = (waitCpt % 30);
        end = end > 20 ? 20 : end;
    	lcd.setCursor(start, 2);
        for (uint8_t t = start; t < end; t++) {
            lcd.print('-');
        }
        usbKey.init(synth.getTimbre(0)->getParamRaw(), synth.getTimbre(1)->getParamRaw(), synth.getTimbre(2)->getParamRaw(), synth.getTimbre(3)->getParamRaw());
        waitCpt++;
    }

    usbKey.getConfigurationFile()->loadConfig(synthState.fullState.midiConfigValue);

    // initialize global tuning
    synth.updateGlobalTuningFromConfig();
#ifdef CVIN
    // Init formula with value
    cvin.updateFormula(synthState.fullState.midiConfigValue[MIDICONFIG_CVIN_A2], synthState.fullState.midiConfigValue[MIDICONFIG_CVIN_A6]);
#endif

    usbKey.getConfigurationFile()->loadScalaConfig(&synthState.fullState.scalaScaleConfig);

    // Load scala scales if enabled
    if (synthState.fullState.scalaScaleConfig.scalaEnabled) {
    	usbKey.getScalaFile()->loadScalaScale(&synthState.fullState.scalaScaleConfig);
    }


    if (synthState.getPcbVersion() == PCB_R5) {
        spiState = 0;
        MCP4922_SysTick_Config();
        MCP4922_Config();
        MCP4922_screenBoot(synth);
        fmDisplay.init(&lcd, &usbKey, 60 * PREENFM_FREQUENCY * .0001f);
    } else {
        // Same method but special case
        lcd.setRealTimeAction(true);
        CS4344_Config(dmaSampleBuffer);
        CS4344_screenBoot();
        fmDisplay.init(&lcd, &usbKey, 60.0f * 4.0f);
    }

    // FS = Full speed : midi
    // HS = high speed : USB Key
    // Init core FS as a midiStreaming device
    if (synthState.fullState.midiConfigValue[MIDICONFIG_USB] != USBMIDI_OFF) {
    	USBD_Init(&usbOTGDevice, USB_OTG_FS_CORE_ID, &usbdMidiDescriptor, &midiCallback, &midiStreamingUsrCallback);
    }


    // Load default combo if any
    usbKey.getComboBank()->loadDefaultCombo();
    // Load User waveforms if any
    usbKey.getUserWaveform()->loadUserWaveforms();
    // In any case init tables
    synthState.propagateAfterNewComboLoad();


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


    // Init ADC
#ifdef CVIN
    ADC_Config(cvin.getADCBufferAdress());
#endif
}

unsigned int ledTimer = 0;
bool ledState = false;
unsigned int encoderTimer = 0;
unsigned int tempoTimer = 0;
unsigned int ADCTimer = 0;


#ifdef DEBUG_CPU_USAGE
unsigned int tDebug;
#endif

 
void strobeLed(uint32_t sync1, uint32_t sync2) {
    if (ledState) {
        if ((preenTimer - ledTimer) > sync1) {
            GPIO_ResetBits(GPIOC, pinTest);
            ledTimer = preenTimer;
            ledState = false;
        }
    } else {
        if ((preenTimer - ledTimer) > sync2) {
            GPIO_SetBits(GPIOC, pinTest);
            ledTimer = preenTimer;
            ledState = true;
        }
    }
}


void MCP4922_loop(void) {
    fillSoundBuffer();

    unsigned int newPreenTimer = preenTimer;

	// Comment following line for debug....
	lcd.setRealTimeAction(false);

	// newByte can display visual info
    while (usartBufferIn.getCount() > 0) {
        fillSoundBuffer();
		midiDecoder.newByte(usartBufferIn.remove());
	}

	if ((newPreenTimer - encoderTimer) > 80) {
        fillSoundBuffer();
        encoders.checkStatus(synthState.fullState.midiConfigValue[MIDICONFIG_ENCODER]);
        encoderTimer = newPreenTimer;
    } else if (fmDisplay.needRefresh()) {
        fillSoundBuffer();
        fmDisplay.refreshAllScreenByStep();
    }

    if ((newPreenTimer - tempoTimer) > 10000) {
         fillSoundBuffer();
         synthState.tempoClick();
         fmDisplay.tempoClick();


#ifdef CVIN
        if (synthState.fullState.currentMenuItem->menuState == MENU_CONFIG_SETTINGS &&
                (synthState.fullState.menuSelect == MIDICONFIG_CVIN_A2 || synthState.fullState.menuSelect == MIDICONFIG_CVIN_A6)) {
            if (cvin.getGate() > (synthState.fullState.midiConfigValue[MIDICONFIG_CV_GATE] * 9 + 62)) {
                lcd.setCursor(1, 3);
                lcd.print("(");
                lcd.print(cvin.getMidiNote());
                lcd.print(") ");
                lcd.setCursor(9, 3);
                lcd.print(": ");
                lcd.print(cvin.getMidiNote1024());
                lcd.print(" ");
            }
        }
#endif


         tempoTimer = newPreenTimer;
     }

    lcd.setRealTimeAction(true);
    while (lcd.hasActions()) {
        if (usartBufferIn.getCount() > 20) {
            // We don't want real time update of parameters
            lcd.setRealTimeAction(false);
            while (usartBufferIn.getCount() > 0) {
                fillSoundBuffer();
                midiDecoder.newByte(usartBufferIn.remove());
            }
            lcd.setRealTimeAction(true);
        }
        LCDAction action = lcd.nextAction();
        lcd.realTimeAction(&action, fillSoundBuffer);
    }

    strobeLed(4203, (42031 * 2- 4203));

#ifdef DEBUG_CPU_USAGE
    if ((preenTimer - tDebug) >= 500) {
        tDebug = preenTimer;
        lcd.setCursor(12, 1);
        lcd.print('>');
        lcd.printWithOneDecimal(synth.getCpuUsage());
        lcd.print('%');
        lcd.print('<');

        lcd.setCursor(14, 2);
        lcd.print('>');
        lcd.print((int)synth.getPlayingNotes());

        lcd.print('<');
    }
#endif


}


void CS4344_loop(void) {

	// Comment following line for debug....
	lcd.setRealTimeAction(false);


    // New midi data ?
    // Move to DMA1_Stream5_IRQHandler ? 
    // Would be better i think
    while (usartBufferIn.getCount() > 0) {
        midiDecoder.newByte(usartBufferIn.remove());
    }

    if ((preenTimer - encoderTimer) >= 2) {
        // Surface control ?
        encoders.checkStatus(synthState.fullState.midiConfigValue[MIDICONFIG_ENCODER]);
        encoderTimer = preenTimer;
    } 
    
    if (fmDisplay.needRefresh()) {
        // Display to refresh ?
        fmDisplay.refreshAllScreenByStep();
    }

    // 1312 per seconds
    if ((preenTimer - tempoTimer) > 328) {
        // display to update
        synthState.tempoClick();
        fmDisplay.tempoClick();

        tempoTimer = preenTimer;
    }

#ifdef DEBUG_CPU_USAGE
    if ((preenTimer - tDebug) >= 600) {
        tDebug = preenTimer;
        lcd.setCursor(12, 1);
        lcd.print('>');
        lcd.printWithOneDecimal(synth.getCpuUsage());
        lcd.print('%');
        lcd.print('<');

        lcd.setCursor(15, 2);
        lcd.print('>');
        lcd.print(synth.getPlayingNotes());

        lcd.print('<');
    }
#endif

    lcd.setRealTimeAction(true);
    while (lcd.hasActions()) {
        if (usartBufferIn.getCount() > 20) {
            // We don't want real time update of parameters
            lcd.setRealTimeAction(false);
            while (usartBufferIn.getCount() > 0) {
                midiDecoder.newByte(usartBufferIn.remove());
            }
            lcd.setRealTimeAction(true);
        }
        LCDAction action = lcd.nextAction();
        lcd.realTimeAction(&action, fillSoundBuffer);
    }

    strobeLed(131, (1312 * 2 - 131));
}

int main(void) {
    setup();
    if (synthState.getPcbVersion() == PCB_R5) {
        while (1) {
            MCP4922_loop();
        }
    } else {
        while (1) {
            CS4344_loop();
        }
    }
}

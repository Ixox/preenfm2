/*
 * Copyright 2011 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier.hosxe@gmail.com)
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

#include <stdlib.h>

#include "FMDisplay.h"
#include "Synth.h"
#include "RingBuffer.h"
#include "MidiDecoder.h"
#include "Encoders.h"
#include "Menu.h"


// Main timer define
#define TIME_NUMBER       2


SynthState		   synthState;
Synth              synth;
MidiDecoder        midiDecoder;
Encoders		   encoders;
FMDisplay          fmDisplay;

#ifdef PCB_R1
LiquidCrystal      lcd(2, 3, 28,29,30,31, 27, 26, 25, 22);
#define TIMER_OVERFLOW 2197
#define TIMER_IRQ_COMPARE 2048
#define TIMER_PRESCALE 1
#define AUDIO_PIN_LEFT    11
#define CHANNEL_PWM   TIMER_CH1
#define CHANNEL_LEFT_INTERUPT  TIMER_CH3
#endif

#ifdef PCB_R2
LiquidCrystal      lcd(2, 3, 4, 5, 6, 7, 31, 30, 29, 28);
#define TIMER_OVERFLOW 2197
#define TIMER_PRESCALE 1
#define TIMER_IRQ_COMPARE 2048
#define AUDIO_PIN_LEFT    11
#define CHANNEL_PWM   TIMER_CH1
#define CHANNEL_LEFT_INTERUPT  TIMER_CH3
#endif

#ifdef PCB_R3
//LiquidCrystal      lcd(31, 30, 29, 28, 2, 3, 4, 5, 6, 7);
LiquidCrystal      lcd(31, 30, 4,5,6,7);
#define TIMER_OVERFLOW 2197
#define TIMER_IRQ_COMPARE 2048
#define TIMER_PRESCALE 1
#define AUDIO_PIN_LEFT    11
#define AUDIO_PIN_RIGHT    10
#define CHANNEL_PWM   TIMER_CH1
#define CHANNEL_LEFT_INTERUPT  TIMER_CH3
timer_dev *dev = PIN_MAP[AUDIO_PIN_LEFT].timer_device;
__io unsigned int *ccr = &(dev->regs).gen->CCR1 + (PIN_MAP[AUDIO_PIN_LEFT].timer_channel - 1);
#endif

#ifdef PCB_R4
LiquidCrystal      lcd(8,9,10,11, 2, 3, 4, 5, 6, 7);
#define TIMER_OVERFLOW 274
#define TIMER_IRQ_COMPARE 0
#define TIMER_PRESCALE 8
#define CHANNEL_LEFT_INTERUPT  TIMER_CH1
#define CHANNEL_RIGHT_INTERUPT  TIMER_CH3
HardwareSPI spi(1);
#define SPI_CS_PIN 20
// gpio bit of GPIO 20 is 15 : see maple_mini.cpp
unsigned int CSPIN = 0bIT(15);

#endif





int mainCpt = 0;



#ifdef PCB_R4

inline void csHigh() {
    GPIOA->regs->BSRR = CSPIN;
}

inline void csLow() {
    GPIOA->regs->BRR = CSPIN;
}


struct toSend {
    unsigned short sampleLeft;
    unsigned short sampleRight;

};

// Init left/right
struct toSend samples = {  0x1 << 12,  0x9 << 12};

unsigned short toto = 0;
void IRQSendSampleRight() {
    csHigh();

    int sample = synth.sampleAtReadCursor()[1] + 32768;
    sample >>= 4;
    samples.sampleRight &=  0xf000 ;
    samples.sampleRight |= sample;
    // send right...
    csLow();
    SPI1->regs->DR = samples.sampleRight;

    synth.incReadCursor();
}
#endif



inline void fillSoundBuffer() {
    if (synth.getSampleCount() <= 128) {
        synth.buildNewSampleBlock();
    }
}

void setup()
{


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

    // ---------------------------------------
    // Register listener

    // synthstate is updated by encoder change
    encoders.insertListener(&synthState);

    // fmDisplay and synth needs to be aware of synthState changes
    synthState.insertParamListener(&fmDisplay);
    synthState.insertParamListener(&synth);
    synthState.insertParamListener(&midiDecoder);
    synthState.insertMenuListener(&fmDisplay);

    // Load config from EEPROM if has been saved...
    PresetUtil::loadConfigFromEEPROM();
    delay(500);

    fillSoundBuffer();

    synth.noteOn(48, 60);
    fillSoundBuffer();

    int notes[] = { 60, 64, 60, 67, 60, 72};
    for (int k=0; k<6; k++) {

        synth.noteOn(notes[k], 60);
        for (int cpt=0; cpt<1500; cpt++) {
            fillSoundBuffer();
            delayMicroseconds(35 - k*2);
        }
        synth.noteOff(notes[k]);
        for (int cpt=0; cpt<100; cpt++) {
            fillSoundBuffer();
            delayMicroseconds(35 - k*2);
        }
    }
    synth.noteOff(48);
    for (int cpt=0; cpt<7000; cpt++) {
        delayMicroseconds(30);
        fillSoundBuffer();
    }

    // Load default patch
    synthState.propagateBeforeNewParamsLoad();
    PresetUtil::loadDefaultPatchIfAny();
    synthState.propagateAfterNewParamsLoad();

    fmDisplay.init(&lcd);

    int bootOption = synthState.fullState.midiConfigValue[MIDICONFIG_BOOT_START];

    if (bootOption == 0) {
        fmDisplay.displayPreset();
        fmDisplay.setRefreshStatus(12);
    } else {
        // Menu
        synthState.buttonPressed(BUTTON_MENUSELECT);
        // Load
        synthState.buttonPressed(BUTTON_MENUSELECT);
        if (bootOption == 5) {
            // Internal bank
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else {
            // User
            synthState.buttonPressed(BUTTON_MENUSELECT);
            // 0bank
            for (int k = 0; k < bootOption - 1; k++) {
                synthState.encoderTurned(0, 1);
            }
            synthState.buttonPressed(BUTTON_MENUSELECT);
        }
    }
    srand(micros());
}

unsigned short midiReceive = 0;
unsigned short midiSent = 0;
unsigned int encoderMicros = 0;
unsigned int midiInMicros = 0;
unsigned int midiOutMicros = 0;
unsigned int synthStateMicros = 0;
unsigned int externGearMidiMicros = 0;
int ECCnumber = 0;



void loop() {
    unsigned int newMicros = micros();

    mainCpt++;

    fillSoundBuffer();

    if ((newMicros - midiOutMicros) > 240) {
        if (midiDecoder.hasMidiToSend()) {

            while (midiDecoder.hasMidiToSend()) {
                fillSoundBuffer();
                midiDecoder.sendMidiOut();
            }

            if (midiSent == 0) {
                fillSoundBuffer();
                fmDisplay.midiOut(true);
            }
            if (synthState.fullState.synthMode == SYNTH_MODE_MENU) {
                midiSent = 0;
            } else {
                midiSent = 2500;
            }
        }

        if (midiSent>0) {
            if (midiSent == 1) {
                fillSoundBuffer();
                fmDisplay.midiOut(false);
            }
            midiSent--;
        }

        midiOutMicros = newMicros;
    }

    if ((newMicros - externGearMidiMicros) > 2500) {
        fillSoundBuffer();
        midiDecoder.sendToExternalGear(ECCnumber);
        externGearMidiMicros = newMicros;
        ECCnumber++;
        ECCnumber &= 0x3;
    }

    // encoders and LCD modification that follows
    lcd.setRealTimeAction(false);
    if ((newMicros - encoderMicros) > 2000) {
        fillSoundBuffer();
        encoders.checkStatus();
        encoderMicros = newMicros;
    } else if (fmDisplay.needRefresh() && ((mainCpt & 0x3) == 0)) {
        fillSoundBuffer();
        fmDisplay.refreshAllScreenByStep();
    }
    lcd.setRealTimeAction(true);
    while (lcd.hasActions()) {
        fillSoundBuffer();
        lcd.nextAction();
    }


    if ((newMicros - synthStateMicros) > 200000) {
        fillSoundBuffer();
        synthState.tempoClick();
        synthStateMicros = newMicros;
    }
}


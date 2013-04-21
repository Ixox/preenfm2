/*
 * Copyright 2011
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

#include "MidiDecoder.h"
#include "usbKey_usr.h"


#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

#define INV127 .00787401574803149606f

#ifdef LCDDEBUG
int pos = 0;


void eraseNext(int pos) {
    int x = (pos % 2) * 10;
    int y = pos / 2;
    y = y % 4;
    lcd.setCursor(x,y);
    lcd.print("- --- --- ");
}


void debug(char *l, int i1, int i2) {
    int x = (pos % 2) * 10;
    int y = pos / 2;
    y = y % 4;
    lcd.setCursor(x,y);
    lcd.print("          ");
    lcd.setCursor(x,y);
    lcd.print(l);
    lcd.setCursor(x+2, y);
    lcd.print(i1);
    lcd.setCursor(x+6, y);
    lcd.print(i2);
    pos++;
    eraseNext(pos);
}
#endif

MidiDecoder::MidiDecoder() {
    currentEventState.eventState = MIDI_EVENT_WAITING;
    currentEventState.index = 0;
    this->isSequencerPlaying = false;
    this->midiClockCpt = 0;
    this->runningStatus = 0;
    for (int k=0; k<NUMBER_OF_ECC; k++) {
        previousECC[k] = 0;
    }
}

MidiDecoder::~MidiDecoder() {
}

void MidiDecoder::setSynth(Synth* synth) {
    this->synth = synth;
}

void MidiDecoder::setVisualInfo(VisualInfo *visualInfo) {
    this->visualInfo = visualInfo;
}

void MidiDecoder::newByte(unsigned char byte) {

    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_THROUGH] == 1) {
        // TODO XH
        //        Serial3.write((unsigned char) byte);
    }

    // Realtime first !
    if (byte >= 0xF8) {
        switch (byte) {
        case MIDI_CONTINUE:
            this->isSequencerPlaying = true;
            this->midiClockCpt = 0;
            this->synth->midiClockContinue(this->songPosition);
            if ((this->songPosition & 0x3) <= 1) {
                this->visualInfo->midiClock(true);
            }
            break;
        case MIDI_CLOCK:
            this->midiClockCpt++;
            if (this->midiClockCpt == 6) {
                if (this->isSequencerPlaying) {
                    this->songPosition++;
                }
                this->midiClockCpt = 0;
                this->synth->midiClockSongPositionStep(this->songPosition);

                if ((this->songPosition & 0x3) == 0) {
                    this->visualInfo->midiClock(true);
                }
                if ((this->songPosition & 0x3) == 0x2) {
                    this->visualInfo->midiClock(false);
                }
            }
            break;
        case MIDI_START:
            this->isSequencerPlaying = true;
            this->songPosition = 0;
            this->midiClockCpt = 0;
            this->visualInfo->midiClock(true);
            this->synth->midiClockStart();
            break;
        case MIDI_STOP:
            this->isSequencerPlaying = false;
            this->synth->midiClockStop();
            this->visualInfo->midiClock(false);
            break;
        }
    } else {
        if (currentEventState.eventState == MIDI_EVENT_WAITING && byte >= 0x80) {
            newMessageType(byte);
            this->runningStatus = byte;
        } else if (currentEventState.eventState == MIDI_EVENT_WAITING && byte < 0x80) {
            // midi source use running status...
            if (this->runningStatus>0) {
                newMessageType(this->runningStatus);
                newMessageData(byte);
            }
        } else if (currentEventState.eventState == MIDI_EVENT_IN_PROGRESS) {
            newMessageData(byte);
        } else if (currentEventState.eventState == MIDI_EVENT_SYSEX) {
            if (byte == MIDI_SYSEX_END) {
                currentEventState.eventState = MIDI_EVENT_WAITING;
                currentEventState.index = 0;
            } else if (currentEventState.index == 0) {

                currentEventState.index = 1;
                // Only do something for non commercial sysex
                // We assume it's for us !
                if (byte == 0x7d) {
                    // System exclusive
                    // Allow patch if real time allowed OR if currenly waiting for sysex
                    // Allow bank if currently waiting for sysex only
                    bool waitingForSysex = this->synthState->fullState.currentMenuItem->menuState == MENU_MIDI_SYSEX_GET;
                    bool realTimeSysexAllowed = this->synthState->fullState.midiConfigValue[MIDICONFIG_REALTIME_SYSEX] == 1;
                    int r = PresetUtil::readSysex(realTimeSysexAllowed || waitingForSysex, waitingForSysex);
                    currentEventState.eventState = MIDI_EVENT_WAITING;
                    currentEventState.index = 0;
                    if (r == 2) {
                        this->synthState->newBankReady();
                    }
                }
            } else {
                // We do nothing !
                // consume sysex message till the end !
            }
        }
    }
}

void MidiDecoder::newMessageData(unsigned char byte) {
    currentEvent.value[currentEventState.index++] = byte;
    if (currentEventState.index == currentEventState.numberOfBytes) {
        midiEventReceived(currentEvent);
        currentEventState.eventState = MIDI_EVENT_WAITING;
        currentEventState.index = 0;
    }
}


void MidiDecoder::newMessageType(unsigned char byte) {
    currentEvent.eventType = (EventType)(byte & 0xf0);
    currentEvent.channel = byte & 0x0f;

    switch (currentEvent.eventType) {
    case MIDI_NOTE_OFF:
    case MIDI_NOTE_ON:
    case MIDI_CONTROL_CHANGE:
    case MIDI_PITCH_BEND:
    case MIDI_POLY_AFTER_TOUCH:
        currentEventState.numberOfBytes = 2;
        currentEventState.eventState = MIDI_EVENT_IN_PROGRESS;
        break;
    case MIDI_AFTER_TOUCH:
    case MIDI_PROGRAM_CHANGE:
        currentEventState.numberOfBytes = 1;
        currentEventState.eventState = MIDI_EVENT_IN_PROGRESS;
        break;
    case MIDI_REAL_TIME_EVENT:
        // We must be sure it's 0xF0 and not 0xF1, 0xF8....
        this->runningStatus = 0;
        switch (byte)  {
        case MIDI_SYSEX:
            currentEventState.eventState = MIDI_EVENT_SYSEX;
            break;
        case MIDI_SONG_POSITION:
            currentEvent.eventType = MIDI_SONG_POSITION;
            // Channel hack to make it accepted
            currentEvent.channel = 	this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] -1;
            currentEventState.numberOfBytes = 2;
            currentEventState.eventState = MIDI_EVENT_IN_PROGRESS;
            break;
        }
        break;
        default :
#ifdef LCDDEBUG
            debug("W", currentEvent.eventType, 0);
#endif
            // Nothing to do...
            break;
    }
}

void MidiDecoder::midiEventReceived(MidiEvent midiEvent) {
    int timbreBitField = 0;
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1]-1) == midiEvent.channel ) {
        timbreBitField |= 0b0001;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2]-1) == midiEvent.channel ) {
        timbreBitField |= 0b0010;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3]-1) == midiEvent.channel ) {
        timbreBitField |= 0b0100;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4]-1) == midiEvent.channel ) {
        timbreBitField |= 0b1000;
    }

    if (timbreBitField == 0) {
        // No accurate channel
        return;
    }



    switch (midiEvent.eventType) {
    case MIDI_NOTE_OFF:
        for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
            if (((timbreBitField >> tk) & 0x1) == 0) continue;
            this->synth->noteOff(tk, midiEvent.value[0]);
        }
        break;
    case MIDI_NOTE_ON:
        if (midiEvent.value[1] == 0) {
            // Some keyboards send note-off this way
            for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
                if (((timbreBitField >> tk) & 0x1) == 0) continue;
                this->synth->noteOff(tk, midiEvent.value[0]);
            }
        } else {
            for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
                if (((timbreBitField >> tk) & 0x1) == 0) continue;
                this->synth->noteOn(tk, midiEvent.value[0], midiEvent.value[1]);
                this->synth->getTimbre(tk)->getMatrix()->setSource(MATRIX_SOURCE_KEY, INV127*(127.0f-midiEvent.value[0]));
                this->synth->getTimbre(tk)->getMatrix()->setSource(MATRIX_SOURCE_VELOCITY, INV127*midiEvent.value[1]);
                visualInfo->noteOn(tk, true);
            }
        }
        break;
    case MIDI_CONTROL_CHANGE:
        for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
            if (((timbreBitField >> tk) & 0x1) == 0) continue;
            controlChange(tk, midiEvent);
        }
        break;
    case MIDI_POLY_AFTER_TOUCH:
        // We don't do anything
        // this->synth->getMatrix()->setSource(MATRIX_SOURCE_AFTERTOUCH, midiEvent.value[1]);
        break;
    case MIDI_AFTER_TOUCH:
        for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
            if (((timbreBitField >> tk) & 0x1) == 0) continue;
            this->synth->getTimbre(tk)->getMatrix()->setSource(MATRIX_SOURCE_AFTERTOUCH, INV127*midiEvent.value[0]);
        }
        break;
    case MIDI_PITCH_BEND:
        for (int tk = 0; tk< NUMBER_OF_TIMBRES; tk++ ) {
            if (((timbreBitField >> tk) & 0x1) == 0) continue;
            this->synth->getTimbre(tk)->getMatrix()->setSource(MATRIX_SOURCE_PITCHBEND,
                    (int) ((((int) midiEvent.value[1] << 7) + (int) midiEvent.value[0]- 8192) >> 6));
        }
        break;
    case MIDI_PROGRAM_CHANGE:
        // Programm change
        this->synth->allSoundOff();
        storage->loadPatch(this->synthState->fullState.bankNumber, midiEvent.value[0], this->synthState->params);
        this->synthState->propagateAfterNewParamsLoad();
        this->synthState->resetDisplay();
        break;
    case MIDI_SONG_POSITION:
        this->songPosition = ((int) midiEvent.value[1] << 7) + midiEvent.value[0];
        this->synth->midiClockSetSongPosition(this->songPosition);
        break;
    }
}

void MidiDecoder::controlChange(int timbre, MidiEvent& midiEvent) {
    int receives = this->synthState->fullState.midiConfigValue[MIDICONFIG_RECEIVES] ;

    // the following one should always been received...
    switch (midiEvent.value[0]) {
    case CC_BANK_SELECT:
        // Not implemented... yet ?
        /*
        if (midiEvent.value[1] >= 1 and midiEvent.value[1] <= 3) {
            this->synthState->fullState.bankNumber = midiEvent.value[1];
        }
        */
        break;
    case CC_MODWHEEL:
        this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_MODWHEEL, midiEvent.value[1]);
        break;
    case CC_ALL_NOTES_OFF:
        this->synth->allNoteOff();
        break;
    case CC_ALL_SOUND_OFF:
        this->synth->allSoundOff();
        break;
    }

    if (receives == 1 || receives ==3) {
        switch (midiEvent.value[0]) {
        case CC_VOICE:
            if (midiEvent.value[1] >= 1 and midiEvent.value[1] <= 4) {
                this->synth->setNewValueFromMidi(timbre, ROW_ENGINE, ENCODER_ENGINE_VOICE, midiEvent.value[1]);
            }
            break;
        case CC_GLIDE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENGINE, ENCODER_ENGINE_GLIDE,
                    midiEvent.value[1]);
            break;
        case CC_IM1:
            this->synth->setNewValueFromMidi(timbre, ROW_MODULATION, ENCODER_ENGINE_IM1,
                    midiEvent.value[1]);
            break;
        case CC_IM2:
            this->synth->setNewValueFromMidi(timbre, ROW_MODULATION, ENCODER_ENGINE_IM2,
                    midiEvent.value[1]);
            break;
        case CC_IM3:
            this->synth->setNewValueFromMidi(timbre, ROW_MODULATION, ENCODER_ENGINE_IM3,
                    midiEvent.value[1]);
            break;
        case CC_IM4:
            this->synth->setNewValueFromMidi(timbre, ROW_MODULATION, ENCODER_ENGINE_IM4,
                    midiEvent.value[1]);
            break;
        case CC_MIX1:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_MIX1,
                    midiEvent.value[1]);
            break;
        case CC_MIX2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_MIX2,
                    midiEvent.value[1]);
            break;
        case CC_MIX3:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_MIX3,
                    midiEvent.value[1]);
            break;
        case CC_PAN1:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_PAN1,
                    midiEvent.value[1]);
            break;
        case CC_PAN2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_PAN2,
                    midiEvent.value[1]);
            break;
        case CC_PAN3:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_PAN3,
                    midiEvent.value[1]);
            break;
        case CC_OSC1_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC1, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC2_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC2, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC3_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC3, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC4_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC4, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC5_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC5, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC6_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC6, ENCODER_OSC_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_OSC1_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC1, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_OSC2_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC2, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_OSC3_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC3, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_OSC4_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC4, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_OSC5_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC5, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_OSC6_DETUNE:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC6, ENCODER_OSC_FTUNE,
                    midiEvent.value[1]);
            break;
        case CC_ENV1_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV1, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV1_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV1, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_ENV2_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV2, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV2_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV2, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_ENV3_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV3, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV3_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV3, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_ENV4_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV4, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV4_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV4, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_ENV5_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV5, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV5_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV5, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_ENV6_ATTACK:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV6, ENCODER_ENV_A,
                    midiEvent.value[1]);
            break;
        case CC_ENV6_RELEASE:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV6, ENCODER_ENV_R,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW1_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX1, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW2_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX2, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW3_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX3, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW4_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX4, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW5_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX5, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_MATRIXROW6_MUL:
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX6, ENCODER_MATRIX_MUL,
                    midiEvent.value[1]);
            break;
        case CC_LFO1_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC1, ENCODER_LFO_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_LFO2_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC2, ENCODER_LFO_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_LFO3_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC3, ENCODER_LFO_FREQ,
                    midiEvent.value[1]);
            break;
        case CC_STEPSEQ5_BPM:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOENV1, ENCODER_STEPSEQ_BPM,
                    midiEvent.value[1]);
            break;
        case CC_STEOSEQ6_BPM:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOENV2, ENCODER_STEPSEQ_BPM,
                    midiEvent.value[1]);
            break;
        case CC_STEPSEQ5_GATE:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOSEQ1, ENCODER_STEPSEQ_GATE,
                    midiEvent.value[1]);
            break;
        case CC_STEPSEQ6_GATE:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOSEQ2, ENCODER_STEPSEQ_GATE,
                    midiEvent.value[1]);
            break;
        case CC_MATRIX_SOURCE_CC1:
            this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_CC1, midiEvent.value[1]);
            this->synth->setNewValueFromMidi(timbre, ROW_PERFORMANCE, ENCODER_PERFORMANCE_CC1, midiEvent.value[1]);
            break;
        case CC_MATRIX_SOURCE_CC2:
            this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_CC2, midiEvent.value[1]);
            this->synth->setNewValueFromMidi(timbre, ROW_PERFORMANCE, ENCODER_PERFORMANCE_CC2, midiEvent.value[1]);
            break;
        case CC_MATRIX_SOURCE_CC3:
            this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_CC3, midiEvent.value[1]);
            this->synth->setNewValueFromMidi(timbre, ROW_PERFORMANCE, ENCODER_PERFORMANCE_CC3, midiEvent.value[1]);
            break;
        case CC_MATRIX_SOURCE_CC4:
            this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_CC4, midiEvent.value[1]);
            this->synth->setNewValueFromMidi(timbre, ROW_PERFORMANCE, ENCODER_PERFORMANCE_CC4, midiEvent.value[1]);
            break;
        }
    }

    // Do we accept NRPN in the menu
    if (receives == 2 || receives ==3) {
        switch (midiEvent.value[0]) {
        case 99:
            this->currentNrpn[timbre].paramMSB = midiEvent.value[1];
            break;
        case 98:
            this->currentNrpn[timbre].paramLSB = midiEvent.value[1];
            break;
        case 6:
            this->currentNrpn[timbre].valueMSB = midiEvent.value[1];
            break;
        case 38:
            this->currentNrpn[timbre].valueLSB = midiEvent.value[1];
            this->currentNrpn[timbre].readyToSend = true;
            break;
        case 96:
            // nrpn increment
            if (this->currentNrpn[timbre].valueLSB == 127) {
                this->currentNrpn[timbre].valueLSB = 0;
                this->currentNrpn[timbre].valueMSB ++;
            } else {
                this->currentNrpn[timbre].valueLSB ++;
            }
            this->currentNrpn[timbre].readyToSend = true;
            break;
        case 97:
            // nrpn decremenet
            if (this->currentNrpn[timbre].valueLSB == 0) {
                this->currentNrpn[timbre].valueLSB = 127;
                this->currentNrpn[timbre].valueMSB --;
            } else {
                this->currentNrpn[timbre].valueLSB --;
            }
            this->currentNrpn[timbre].readyToSend = true;
            break;
        default:
            break;
        }

        if (this->currentNrpn[timbre].readyToSend) {
            decodeNrpn(timbre);
            this->currentNrpn[timbre].readyToSend = false;
        }
    }
}

void MidiDecoder::decodeNrpn(int timbre) {
    if (this->currentNrpn[timbre].paramMSB < 2) {
        unsigned int index = (this->currentNrpn[timbre].paramMSB << 7) + this->currentNrpn[timbre].paramLSB;
        unsigned int value = (this->currentNrpn[timbre].valueMSB << 7) + this->currentNrpn[timbre].valueLSB;
        unsigned int row = index / NUMBER_OF_ENCODERS;
        unsigned int encoder = index % NUMBER_OF_ENCODERS;


        if (row < NUMBER_OF_ROWS) {
            // Performance row do note receive/send nrpn.
            if (row >= ROW_PERFORMANCE)
                row ++;
            this->synth->setNewValueFromMidi(timbre, row, encoder, value);
        } else if (index >= 228 && index < 240) {
            this->synthState->params->presetName[index - 228] = (char) value;
            this->synthState->propagateNewPresetName((value == 0));
        }
    } else if (this->currentNrpn[timbre].paramMSB < 4)  {
        unsigned int whichStepSeq = this->currentNrpn[timbre].paramMSB -2;
        unsigned int step = this->currentNrpn[timbre].paramLSB;
        unsigned int value = this->currentNrpn[timbre].valueLSB;

        this->synthState->setNewStepValue(timbre, whichStepSeq, step, value);
    } else if (this->currentNrpn[timbre].paramMSB == 127 && this->currentNrpn[timbre].paramLSB == 127)  {
        PresetUtil::sendCurrentPatchAsNrpns(timbre);
    }
}

void MidiDecoder::newParamValueFromExternal(int timbre, SynthParamType type,
        int currentrow, int encoder, ParameterDisplay* param, float oldValue,
        float newValue) {
    // Do nothing here...
}

void MidiDecoder::newParamValue(int timbre, SynthParamType type, int currentrow,
    int encoder, ParameterDisplay* param, float oldValue, float newValue) {

    int sendCCOrNRPN = this->synthState->fullState.midiConfigValue[MIDICONFIG_SENDS] ;
    int channel = this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1 + timbre] -1;

    struct MidiEvent cc;
    cc.eventType = MIDI_CONTROL_CHANGE;
    // Si channel = ALL envoie sur 1
    if (channel == -1) {
        channel = 0;
    }
    cc.channel = channel;

    // Do we send NRPN ?
    if (sendCCOrNRPN == 2) {
        // Special case for Step sequencer
        if (currentrow >= ROW_LFOSEQ1 && encoder >=2) {
            if (encoder == 2) {
                return;
            }
            if (encoder == 3) {
                // We modify a step value, we want to send that
                int currentStepSeq = currentrow - ROW_LFOSEQ1;
                // NRPN is 4 control change
                cc.value[0] = 99;
                cc.value[1] = currentStepSeq + 2;
                midiToSend.insert(cc);
                cc.value[0] = 98;
                cc.value[1] = this->synthState->stepSelect[currentStepSeq];
                midiToSend.insert(cc);
                cc.value[0] = 6;
                cc.value[1] = 0;
                midiToSend.insert(cc);
                cc.value[0] = 38;
                cc.value[1] = (unsigned char) newValue;
                midiToSend.insert(cc);
            }
        } else {
            // performance do not send NRPN
            if (currentrow > ROW_PERFORMANCE) {
                currentrow --;
            } else if (currentrow == ROW_PERFORMANCE) {
                return;
            }
            // MSB / LSB
            int paramNumber =  currentrow * NUMBER_OF_ENCODERS + encoder;
            // Value to send must be positive
            int valueToSend = newValue - param->minValue;
            // NRPN is 4 control change
            cc.value[0] = 99;
            cc.value[1] = (unsigned char)(paramNumber >> 7);
            midiToSend.insert(cc);
            cc.value[0] = 98;
            cc.value[1] = (unsigned char)(paramNumber & 127);
            midiToSend.insert(cc);
            cc.value[0] = 6;
            cc.value[1] = (unsigned char) (valueToSend >> 7);
            midiToSend.insert(cc);
            cc.value[0] = 38;
            cc.value[1] = (unsigned char) (valueToSend & 127);
            midiToSend.insert(cc);
        }
    }

    // Do we send control change
    if (sendCCOrNRPN == 1) {
        cc.value[0] = 0;

        switch (currentrow) {
        case ROW_ENGINE:
            if (encoder == ENCODER_ENGINE_VOICE) {
                cc.value[0] = CC_VOICE;
                cc.value[1] = newValue;
            } else if (encoder == ENCODER_ENGINE_GLIDE) {
                cc.value[0] = CC_GLIDE;
                cc.value[1] = newValue;
            }
            break;
        case ROW_MODULATION:
            // TO DO LATER
            // We only send even value so that we're not stick we receving the host midi loopback
            /*
            if ((newValue & 0x1) >0)
                break;
            cc.value[0] = CC_IM1 + encoder;
            cc.value[1] = newValue >> 1;
            break;
            */
        case ROW_OSC_MIX1:
            cc.value[0] = CC_MIX1 + encoder;
            cc.value[1] = newValue;
            break;
        case ROW_OSC_MIX2:
            cc.value[0] = CC_MIX3 + encoder;
            cc.value[1] = newValue;
            break;
        case ROW_OSC_FIRST ... ROW_OSC_LAST:
        if ((((int)newValue) & 0x1) >0)
            break;
        if (encoder == ENCODER_OSC_FREQ) {
            cc.value[0] = CC_OSC1_FREQ + (currentrow - ROW_OSC_FIRST);
            cc.value[1] = newValue * .5f + .1f;
        } else if (encoder == ENCODER_OSC_FTUNE) {
            cc.value[0] = CC_OSC1_DETUNE + (currentrow - ROW_OSC_FIRST);
            cc.value[1] = newValue * .5f + .1f;
        }
        break;
        case ROW_ENV_FIRST ... ROW_ENV_LAST:
        if ((((int)newValue) & 0x1) >0)
            break;
        if (encoder == ENCODER_ENV_A) {
            cc.value[0] = CC_ENV1_ATTACK + (currentrow - ROW_ENV_FIRST);
            cc.value[1] = newValue * .5f + .1f;
        } else if (encoder == ENCODER_ENV_R) {
            cc.value[0] = CC_ENV1_RELEASE + (currentrow - ROW_ENV_FIRST);
            cc.value[1] = newValue * .5f + .1f;
        }
        break;
        case ROW_MATRIX_FIRST ... ROW_MATRIX6:
        if ((((int)newValue) & 0x1) >0)
            break;
        if (encoder == ENCODER_MATRIX_MUL) {
            cc.value[0] = CC_MATRIXROW1_MUL + currentrow - ROW_MATRIX_FIRST;
            cc.value[1] = newValue * .5f + .1f;
        }
        break;
        case ROW_LFO_FIRST ... ROW_LFOOSC3:
        if ((((int)newValue) & 0x1) >0)
            break;
        if (encoder == ENCODER_LFO_FREQ) {
            cc.value[0] = CC_LFO1_FREQ + (currentrow - ROW_LFO_FIRST);
            cc.value[1] = newValue * .5f + .1f;
        }
        break;
        case ROW_LFOSEQ1 ... ROW_LFOSEQ2:
        if (encoder == ENCODER_STEPSEQ_BPM) {
            if ((((int)newValue) & 0x1) >0)
                break;
            cc.value[0] = CC_STEPSEQ5_BPM + (currentrow - ROW_LFOSEQ1);
            cc.value[1] = newValue * .5f + .1f;
        }
        else if (encoder == ENCODER_STEPSEQ_GATE) {
            cc.value[0] = CC_STEPSEQ5_GATE + (currentrow - ROW_LFOSEQ1);
            cc.value[1] = newValue;
        }
        break;
        }

        if (cc.value[0] != 0) {
            midiToSend.insert(cc);
        }
    }
}

void MidiDecoder::sendMidiOut() {
    // TODO
    /*
    // new event only if midiSendState = 0;
    if (midiSendState == 0) {
        toSend = midiToSend.remove();
    }

    switch (toSend.eventType) {
    case MIDI_NOTE_OFF:
    case MIDI_NOTE_ON:
    case MIDI_CONTROL_CHANGE:
    case MIDI_PITCH_BEND:
        switch (midiSendState) {
        case 0:
            Serial3.write((unsigned char) (toSend.eventType + toSend.channel));
            midiSendState = 1;
            break;
        case 1:
            Serial3.write((unsigned char) toSend.value[0]);
            midiSendState = 2;
            break;
        case 2:
            Serial3.write((unsigned char) toSend.value[1]);
            midiSendState = 0;
            break;
        }
    break;
    case MIDI_AFTER_TOUCH:
    case MIDI_PROGRAM_CHANGE:
        switch (midiSendState) {
        case 0:
            Serial3.write((unsigned char) (toSend.eventType + toSend.channel));
            midiSendState = 1;
            break;
        case 1:
            Serial3.write((unsigned char) toSend.value[0]);
            midiSendState = 0;
        break;
        }
    }
    */
}


void MidiDecoder::sendToExternalGear(int enumber) {
    // only on timbre 0
    int currentValue = this->synth->getTimbre(0)->getMatrix()->getDestination((DestinationEnum)(EXTERNAL_CC1 + enumber)) ;
    currentValue >>= 4;

    if (currentValue<0) {
        currentValue =0;
    }
    if (currentValue>127) {
        currentValue = 127;
    }
    if (currentValue != previousECC[enumber]) {
        previousECC[enumber] = currentValue;

        struct MidiEvent cc;
        cc.eventType = MIDI_CONTROL_CHANGE;
        cc.channel = this->synthState->fullState.midiConfigValue[MIDICONFIG_ECHANNEL];
        cc.value[0] = this->synthState->fullState.midiConfigValue[MIDICONFIG_ECC1 + enumber] ;
        cc.value[1] = currentValue;
        midiToSend.insert(cc);
    }
}

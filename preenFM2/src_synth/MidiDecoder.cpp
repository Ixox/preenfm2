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

#include "stm32f4xx_usart.h"
#include "MidiDecoder.h"
#include "usbKey_usr.h"
#include "usb_dcd.h"


extern USB_OTG_CORE_HANDLE          usbOTGDevice;

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
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)byte;
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
                // Do something for non commercial sysex We assume it's for us !
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
                        this->synthState->newSysexBankReady();
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
    int timbreIndex = 0;
    int timbres[4];
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 0;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 1;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 2;
    }
    if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 3;
    }

    if (timbreIndex == 0) {
        // No accurate channel
        return;
    }



    switch (midiEvent.eventType) {
    case MIDI_NOTE_OFF:
        for (int tk = 0; tk< timbreIndex; tk++ ) {
            this->synth->noteOff(timbres[tk], midiEvent.value[0]);
        }
        break;
    case MIDI_NOTE_ON:
        if (midiEvent.value[1] == 0) {
            // Some keyboards send note-off this way
            for (int tk = 0; tk< timbreIndex; tk++ ) {
                this->synth->noteOff(timbres[tk], midiEvent.value[0]);
            }
        } else {
            for (int tk = 0; tk< timbreIndex; tk++ ) {
                this->synth->noteOn(timbres[tk], midiEvent.value[0], midiEvent.value[1]);
                this->synth->getTimbre(timbres[tk])->getMatrix()->setSource(MATRIX_SOURCE_KEY, INV127*midiEvent.value[0]);
                this->synth->getTimbre(timbres[tk])->getMatrix()->setSource(MATRIX_SOURCE_VELOCITY, INV127*midiEvent.value[1]);
                visualInfo->noteOn(timbres[tk], true);
            }
        }
        break;
    case MIDI_CONTROL_CHANGE:
        for (int tk = 0; tk< timbreIndex; tk++ ) {
            controlChange(timbres[tk], midiEvent);
        }
        break;
    case MIDI_POLY_AFTER_TOUCH:
        // We don't do anything
        // this->synth->getMatrix()->setSource(MATRIX_SOURCE_AFTERTOUCH, midiEvent.value[1]);
        break;
    case MIDI_AFTER_TOUCH:
        for (int tk = 0; tk< timbreIndex; tk++ ) {
            this->synth->getTimbre(timbres[tk])->getMatrix()->setSource(MATRIX_SOURCE_AFTERTOUCH, INV127*midiEvent.value[0]);
        }
        break;
    case MIDI_PITCH_BEND:
        for (int tk = 0; tk< timbreIndex; tk++ ) {
            int pb = ((int) midiEvent.value[1] << 7) + (int) midiEvent.value[0] - 8192;
            this->synth->getTimbre(timbres[tk])->getMatrix()->setSource(MATRIX_SOURCE_PITCHBEND, (float)pb * .00012207031250000000f  );
        }
        break;
    case MIDI_PROGRAM_CHANGE:
        // Programm change
        this->synth->allSoundOff();
        storage->loadPreenFMPatch(this->synthState->fullState.preenFMBank, midiEvent.value[0], this->synthState->params);
        this->synthState->propagateAfterNewParamsLoad();
        this->synthState->resetDisplay();
        break;
    case MIDI_SONG_POSITION:
        this->songPosition = ((int) midiEvent.value[1] << 7) + midiEvent.value[0];
        this->synth->midiClockSetSongPosition(this->songPosition);
        break;
    }
}

int cccpt = 0;
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
        this->synth->getTimbre(timbre)->getMatrix()->setSource(MATRIX_SOURCE_MODWHEEL, INV127 * midiEvent.value[1]);
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
        case CC_IM1:
        case CC_IM2:
        case CC_IM3:
        case CC_IM4:
			this->synth->setNewValueFromMidi(timbre, ROW_MODULATION1, ENCODER_ENGINE_IM1  + midiEvent.value[0] - CC_IM1,
					(float)midiEvent.value[1] * .1f);
            break;
        case CC_IM5:
        case CC_IM6:
			this->synth->setNewValueFromMidi(timbre, ROW_MODULATION2, ENCODER_ENGINE_IM5  + midiEvent.value[0] - CC_IM5,
					(float)midiEvent.value[1] * .1f);
            break;
        case CC_MIX1:
        case CC_MIX2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_MIX1 + (midiEvent.value[0] - CC_MIX1),
                    (float)midiEvent.value[1] * .01f);
            break;
        case CC_MIX3:
        case CC_MIX4:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_MIX3 + (midiEvent.value[0] - CC_MIX3),
                    (float)midiEvent.value[1] * .01f);
            break;
        case CC_PAN1:
        case CC_PAN2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_PAN1 + (midiEvent.value[0] - CC_PAN1),
                    (float)midiEvent.value[1] * .02f - 1.0f);
            break;
        case CC_PAN3:
        case CC_PAN4:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_PAN3 + (midiEvent.value[0] - CC_PAN3),
                    (float)midiEvent.value[1] * .02f - 1.0f);
            break;
        case CC_OSC1_FREQ:
        case CC_OSC2_FREQ:
        case CC_OSC3_FREQ:
        case CC_OSC4_FREQ:
        case CC_OSC5_FREQ:
        case CC_OSC6_FREQ:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC1 + midiEvent.value[0] - CC_OSC1_FREQ , ENCODER_OSC_FREQ,
                    (float) midiEvent.value[1] * .0833333333333333f);
            break;
        case CC_MATRIXROW1_MUL:
        case CC_MATRIXROW2_MUL:
        case CC_MATRIXROW3_MUL:
        case CC_MATRIXROW4_MUL:
            // cc.value[1] = newValue * 5.0f + 50.1f;
            this->synth->setNewValueFromMidi(timbre, ROW_MATRIX1 + midiEvent.value[0] - CC_MATRIXROW1_MUL, ENCODER_MATRIX_MUL,
                    (float) midiEvent.value[1] * .2f - 10.0f);
            break;
        case CC_LFO1_FREQ:
        case CC_LFO2_FREQ:
        case CC_LFO3_FREQ:
            // cc.value[1] = newValue * 5.0f + .1f;
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC1 + midiEvent.value[0] - CC_LFO1_FREQ, ENCODER_LFO_FREQ,
                    (float) midiEvent.value[1] *.2f);
            break;
        case CC_LFO_ENV2_SILENCE:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOENV2, ENCODER_LFO_ENV2_SILENCE,
                    (float) midiEvent.value[1] * .125f);
            break;
        case CC_STEPSEQ5_GATE:
        case CC_STEPSEQ6_GATE:
            // cc.value[1] = newValue * 100.0f + .1f;
            this->synth->setNewValueFromMidi(timbre, ROW_LFOSEQ1 + midiEvent.value[0] - CC_STEPSEQ5_GATE, ENCODER_STEPSEQ_GATE,
                    (float)midiEvent.value[1] * .01f);
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
        float value = (this->currentNrpn[timbre].valueMSB << 7) + this->currentNrpn[timbre].valueLSB;
        unsigned int row = index / NUMBER_OF_ENCODERS;
        unsigned int encoder = index % NUMBER_OF_ENCODERS;
        struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);


        if (row < NUMBER_OF_ROWS) {
            if (param->displayType == DISPLAY_TYPE_FLOAT || param->displayType == DISPLAY_TYPE_FLOAT_OSC_FREQUENCY || param->displayType == DISPLAY_TYPE_FLOAT_LFO_FREQUENCY) {
            	value = value * .01f + param->minValue;
            }
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
            int valueToSend;

            if (param->displayType == DISPLAY_TYPE_FLOAT || param->displayType == DISPLAY_TYPE_FLOAT_OSC_FREQUENCY || param->displayType == DISPLAY_TYPE_FLOAT_LFO_FREQUENCY) {
                valueToSend = (newValue - param->minValue) * 100.0f + .1f ;
            } else {
                valueToSend = newValue + .1f ;
            }
            // MSB / LSB
            int paramNumber =  currentrow * NUMBER_OF_ENCODERS + encoder;
            // Value to send must be positive

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
        case ROW_MODULATION1:

			cc.value[1] = newValue * 10;
			cc.value[0] = CC_IM1 + encoder;

            break;
        case ROW_MODULATION2:
        	cc.value[1] = newValue * 10;
			cc.value[0] = CC_IM5 + encoder;

			break;
        case ROW_OSC_MIX1:
            cc.value[0] = CC_MIX1 + encoder;
            if ((encoder & 0x1) == 0) {
                // mix
                cc.value[1] = newValue * 100.0f + .1f;
            } else {
                cc.value[1] = (newValue + 1.0f ) * 50.0f + .1f;
            }
            break;
        case ROW_OSC_MIX2:
            cc.value[0] = CC_MIX3 + encoder;
            if ((encoder & 0x1) == 0) {
                // mix
                cc.value[1] = newValue * 100.0f + .1f;
            } else {
                cc.value[1] = (newValue + 1.0f ) * 50.0f + .1f;
            }
            break;
        case ROW_OSC_FIRST ... ROW_OSC_LAST:
        if (encoder == ENCODER_OSC_FREQ) {
            cc.value[0] = CC_OSC1_FREQ + (currentrow - ROW_OSC_FIRST);
            cc.value[1] = newValue * 12.0f + .1f;
        }
        break;
        case ROW_MATRIX_FIRST ... ROW_MATRIX4:
        if (encoder == ENCODER_MATRIX_MUL) {
            cc.value[0] = CC_MATRIXROW1_MUL + currentrow - ROW_MATRIX_FIRST;
            cc.value[1] = newValue * 5.0f + 50.1f;
        }
        break;
        case ROW_LFO_FIRST ... ROW_LFOOSC3:
        if (encoder == ENCODER_LFO_FREQ) {
            cc.value[0] = CC_LFO1_FREQ + (currentrow - ROW_LFO_FIRST);
            cc.value[1] = newValue * 5.0f + .1f;
        }
        break;
        case ROW_LFOENV2:
        	if (encoder == ENCODER_LFO_ENV2_SILENCE) {
                cc.value[0] = CC_LFO_ENV2_SILENCE;
                cc.value[1] = newValue * 8.0f + .1f;
        	}
        	break;
        case ROW_LFOSEQ1 ... ROW_LFOSEQ2:
        if (encoder == ENCODER_STEPSEQ_GATE) {
            cc.value[0] = CC_STEPSEQ5_GATE + (currentrow - ROW_LFOSEQ1);
            cc.value[1] = newValue * 100.0f + .1f;
        }
        break;
        }

        if (cc.value[0] != 0) {
        	// CC limited to 127
        	if (cc.value[1] > 127) {
    			cc.value[1] = 127;
            }

        	if (lastSentCC.value[1] != cc.value[1] || lastSentCC.value[0] != cc.value[0] || lastSentCC.channel != cc.channel) {
        		midiToSend.insert(cc);
        		lastSentCC = cc;
        	}
        }
    }
}


void MidiDecoder::sendMidiOut() {

    MidiEvent toSend = midiToSend.remove();

    switch (toSend.eventType) {
    case MIDI_NOTE_OFF:
    case MIDI_NOTE_ON:
    case MIDI_CONTROL_CHANGE:
    case MIDI_PITCH_BEND:

        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)toSend.eventType + toSend.channel;

        // Between the USART3 to save a couple of cycle
        if (usbOTGDevice.dev.device_status == USB_OTG_CONFIGURED) {
            // usbBuf[0] = [number of cable on 4 bits] [event type on 4 bites]
            usbBuf[0] = 0x00  | (toSend.eventType >> 4);
            usbBuf[1] = toSend.eventType + toSend.channel;
            usbBuf[2] = toSend.value[0];
            usbBuf[3] = toSend.value[1];

            DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuf, 4);
        }

        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)toSend.value[0];

        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)toSend.value[1];

        break;
    case MIDI_AFTER_TOUCH:
    case MIDI_PROGRAM_CHANGE:
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)toSend.eventType + toSend.channel;

        // Between the USART3 to save a couple of cycle
        if (usbOTGDevice.dev.device_status == USB_OTG_CONFIGURED) {
            // usbBuf[0] = [number of cable on 4 bits] [event type on 4 bites]
            usbBuf[0] = 0x00  | (toSend.eventType >> 4);
            usbBuf[1] = toSend.eventType + toSend.channel;
            usbBuf[2] = toSend.value[0];
            usbBuf[3] = 0;

            DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuf, 4);
        }


        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
        USART3->DR = (uint16_t)toSend.value[0];
        break;
    }
}


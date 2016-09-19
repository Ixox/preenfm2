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
#include "PreenFM.h"
#include "MidiDecoder.h"
#include "usbKey_usr.h"
#include "usb_dcd.h"


extern USB_OTG_CORE_HANDLE          usbOTGDevice;


#define INV127 .00787401574803149606f

// Let's have sysexBuffer in regular RAM.
#define SYSEX_BUFFER_SIZE 1024
uint8_t sysexBuffer[SYSEX_BUFFER_SIZE];


#include "LiquidCrystal.h"
extern LiquidCrystal lcd;


#ifdef LCDDEBUG

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

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
    this->isSequencerPlaying = true;
    this->midiClockCpt = 0;
    this->runningStatus = 0;
    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
    	omniOn[t] = false;
        bankNumber[t] = 0;
        bankNumberLSB[t] = 0;
    }

    usbBufRead = usbBuf;
    usbBufWrite = usbBuf;
    sysexIndex = 0;

    for (int k=0; k<64; k++) {
    	usbBuf[k] = 0;
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
    // Realtime first !
    if (byte >= 0xF8) {
        switch (byte) {
        case MIDI_CLOCK:
            this->midiClockCpt++;
            this->synth->midiTick();
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
        	if (!this->isSequencerPlaying) {
				this->isSequencerPlaying = true;
				this->songPosition = 0;
				this->midiClockCpt = 0;
				this->synth->midiClockStart();
        	}
            break;
        case MIDI_CONTINUE:
        	if (!this->isSequencerPlaying) {
				this->isSequencerPlaying = true;
				this->midiClockCpt = 0;
				this->synth->midiClockContinue(this->songPosition);
        	}
            break;
        case MIDI_STOP:
        	if (this->isSequencerPlaying) {
				this->isSequencerPlaying = false;
				this->synth->midiClockStop();
				this->visualInfo->midiClock(false);
        	}
            break;
        }
    } else {
    	switch (currentEventState.eventState) {
    	case MIDI_EVENT_WAITING:
            if (byte >= 0x80) {
            	// Running status is cleared later in newMEssageType() if byte >= 0xF0
                this->runningStatus = byte;
                newMessageType(byte);
            } else {
                // midi source use running status...
                if (this->runningStatus > 0) {
                    newMessageType(this->runningStatus);
                    newMessageData(byte);
                }
            }
            break;
    	case MIDI_EVENT_IN_PROGRESS:
            newMessageData(byte);
            break;
    	case MIDI_EVENT_SYSEX:
			if (currentEventState.index < SYSEX_BUFFER_SIZE) {
				sysexBuffer[currentEventState.index++] = byte;
			}
			if (byte == MIDI_SYSEX_END) {
				if (currentEventState.index < SYSEX_BUFFER_SIZE) {
					// End of sysex =>Analyse Sysex
					this->synthState->analyseSysexBuffer(sysexBuffer);
				}
				currentEventState.eventState = MIDI_EVENT_WAITING;
				currentEventState.index = 0;
			}
			break;
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
    case MIDI_SYSTEM_COMMON:
        this->runningStatus = 0;
        switch (byte)  {
        case MIDI_SYSEX:
            currentEventState.eventState = MIDI_EVENT_SYSEX;
			currentEventState.index = 0;
            break;
        case MIDI_SONG_POSITION:
            currentEvent.eventType = MIDI_SONG_POSITION;
            // Channel hack to make it accepted
            if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] == 0) {
            	currentEvent.channel = 0;
            } else {
            	currentEvent.channel = 	this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] -1;
            }
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
    if (omniOn[0]
    		|| this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 0;
    }
    if (omniOn[1]
            || this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 1;
    }
    if (omniOn[2]
    		|| this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3] == 0
            || (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3]-1) == midiEvent.channel ) {
        timbres[timbreIndex++] = 2;
    }
    if (omniOn[3]
       		|| this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4] == 0
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
            this->synth->getTimbre(timbres[tk])->setMatrixSource(MATRIX_SOURCE_AFTERTOUCH, INV127*midiEvent.value[0]);
        }
        break;
    case MIDI_PITCH_BEND:
        for (int tk = 0; tk< timbreIndex; tk++ ) {
            int pb = ((int) midiEvent.value[1] << 7) + (int) midiEvent.value[0] - 8192;
            this->synth->getTimbre(timbres[tk])->setMatrixSource(MATRIX_SOURCE_PITCHBEND, (float)pb * .00012207031250000000f  );
        }
        break;
    case MIDI_PROGRAM_CHANGE:
    	if (this->synthState->fullState.midiConfigValue[MIDICONFIG_PROGRAM_CHANGE]) {
			for (int tk = 0; tk< timbreIndex; tk++ ) {
				this->synth->loadPreenFMPatchFromMidi(timbres[tk], bankNumber[timbres[tk]], bankNumberLSB[timbres[tk]], midiEvent.value[0]);
			}
    	}
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

    // the following one should always been treated...
    switch (midiEvent.value[0]) {
    case CC_BANK_SELECT:
    	bankNumber[timbre] = midiEvent.value[1];
    	break;
    case CC_BANK_SELECT_LSB:
    	bankNumberLSB[timbre] = midiEvent.value[1];
        break;
    case CC_MODWHEEL:
        this->synth->getTimbre(timbre)->setMatrixSource(MATRIX_SOURCE_MODWHEEL, INV127 * midiEvent.value[1]);
        break;
    case CC_BREATH:
        this->synth->getTimbre(timbre)->setMatrixSource(MATRIX_SOURCE_BREATH, INV127 * midiEvent.value[1]);
        break;
    case CC_ALL_NOTES_OFF:
        this->synth->allNoteOff(timbre);
        break;
    case CC_ALL_SOUND_OFF:
        this->synth->allSoundOff(timbre);
        break;
    case CC_HOLD_PEDAL:
    	this->synth->setHoldPedal(timbre, midiEvent.value[1]);
    	break;
    case CC_OMNI_OFF:
    	// Omni on && omni OFF only accepted on original bas channel
        if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1 + timbre] == midiEvent.channel ) {
            this->synth->allNoteOff(timbre);
            omniOn[timbre] = false;
        }
    	break;
    case CC_OMNI_ON:
    	// Omni on && omni OFF only accepted on original bas channel
        if (this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1 + timbre] == midiEvent.channel ) {
            this->synth->allNoteOff(timbre);
            omniOn[timbre] = true;
        }
    	break;
    case CC_RESET:
        this->synth->allNoteOff(timbre);
        this->runningStatus = 0;
        this->songPosition = 0;
        break;

    }

    // Receive CC enabled?
    if (receives & 0x01) {
        switch (midiEvent.value[0]) {
        case CC_ALGO:
            this->synth->setNewValueFromMidi(timbre, ROW_ENGINE, ENCODER_ENGINE_ALGO,
                    (float)midiEvent.value[1]);
            break;
        case CC_IM1:
        case CC_IM2:
            this->synth->setNewValueFromMidi(timbre, ROW_MODULATION1, ENCODER_ENGINE_IM1  + (midiEvent.value[0] - CC_IM1) * 2,
                    (float)midiEvent.value[1] * .1f);
            break;
        case CC_IM3:
        case CC_IM4:
			this->synth->setNewValueFromMidi(timbre, ROW_MODULATION2, ENCODER_ENGINE_IM3  + (midiEvent.value[0] - CC_IM3) *2,
					(float)midiEvent.value[1] * .1f);
            break;
        case CC_IM5:
			this->synth->setNewValueFromMidi(timbre, ROW_MODULATION3, ENCODER_ENGINE_IM5  + (midiEvent.value[0] - CC_IM5),
					(float)midiEvent.value[1] * .1f);
            break;
        case CC_MIX1:
        case CC_MIX2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_MIX1 + (midiEvent.value[0] - CC_MIX1),
                    (float)midiEvent.value[1] * .00787401574803149606f);
            break;
        case CC_MIX3:
        case CC_MIX4:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_MIX3 + (midiEvent.value[0] - CC_MIX3),
                    (float)midiEvent.value[1] * .00787401574803149606f);
            break;
        case CC_PAN1:
        case CC_PAN2:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX1, ENCODER_ENGINE_PAN1 + (midiEvent.value[0] - CC_PAN1),
                    (float)midiEvent.value[1] * .00787401574803149606f * 2.0f - 1.0f);
            break;
        case CC_PAN3:
        case CC_PAN4:
            this->synth->setNewValueFromMidi(timbre, ROW_OSC_MIX2, ENCODER_ENGINE_PAN3 + (midiEvent.value[0] - CC_PAN3),
                    (float)midiEvent.value[1] * .00787401574803149606f * 2.0f - 1.0f);
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
        case CC_MATRIX_SOURCE_CC1:
        case CC_MATRIX_SOURCE_CC2:
        case CC_MATRIX_SOURCE_CC3:
        case CC_MATRIX_SOURCE_CC4:
            // cc.value[1] = newValue * 5.0f + 50.1f;
            this->synth->setNewValueFromMidi(timbre, ROW_PERFORMANCE1,  midiEvent.value[0] - CC_MATRIX_SOURCE_CC1,
                                (float)(midiEvent.value[1] -64) * INV127 * 2.0 );
                                    break;

            break;
        case CC_FILTER_TYPE:
            this->synth->setNewValueFromMidi(timbre, ROW_EFFECT, ENCODER_EFFECT_TYPE,
                    (float)midiEvent.value[1]);
            break;
        case CC_FILTER_PARAM1:
        case CC_FILTER_PARAM2:
        case CC_FILTER_GAIN:
            this->synth->setNewValueFromMidi(timbre, ROW_EFFECT, midiEvent.value[0] - CC_FILTER_PARAM1 + 1,
                    (float)midiEvent.value[1] * .01f);
            break;
        case CC_ENV_ATK_OP1:
        case CC_ENV_ATK_OP2:
        case CC_ENV_ATK_OP3:
        case CC_ENV_ATK_OP4:
        case CC_ENV_ATK_OP5:
        case CC_ENV_ATK_OP6:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV1a + (midiEvent.value[0] - CC_ENV_ATK_OP1)* 2, ENCODER_ENV_A,
                    (float)midiEvent.value[1] * .01562500000000000000f);
            break;
        case CC_ENV_ATK_ALL:
            for (int e = 0; e < 6; e++) {
                this->synth->setNewValueFromMidi(timbre, ROW_ENV1a + e * 2, ENCODER_ENV_A,
                        (float)midiEvent.value[1] * .01562500000000000000f);
            }
            break;
        case CC_ENV_REL_OP1:
        case CC_ENV_REL_OP2:
        case CC_ENV_REL_OP3:
        case CC_ENV_REL_OP4:
        case CC_ENV_REL_OP5:
        case CC_ENV_REL_OP6:
            this->synth->setNewValueFromMidi(timbre, ROW_ENV1b + (midiEvent.value[0] - CC_ENV_REL_OP1)* 2, ENCODER_ENV_R,
                    (float)midiEvent.value[1] * .03125000000000000000f);
            break;
        case CC_ENV_REL_ALL:
            for (int e = 0; e < 6; e++) {
                this->synth->setNewValueFromMidi(timbre, ROW_ENV1b + e * 2, ENCODER_ENV_R,
                        (float)midiEvent.value[1] * .03125000000000000000f);
            }
            break;
        case CC_LFO1_PHASE:
        case CC_LFO2_PHASE:
        case CC_LFO3_PHASE:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOPHASES, ENCODER_LFO_PHASE1 + midiEvent.value[0] - CC_LFO1_PHASE,
                    (float)midiEvent.value[1] * .01f);
            break;
        case CC_LFO1_BIAS:
        case CC_LFO2_BIAS:
        case CC_LFO3_BIAS:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC1 + midiEvent.value[0] - CC_LFO1_BIAS, ENCODER_LFO_BIAS,
                    (float)midiEvent.value[1] * .01f);
            break;
        case CC_LFO1_SHAPE:
        case CC_LFO2_SHAPE:
        case CC_LFO3_SHAPE:
            this->synth->setNewValueFromMidi(timbre, ROW_LFOOSC1 + midiEvent.value[0] - CC_LFO1_SHAPE, ENCODER_LFO_SHAPE,
                    (float)midiEvent.value[1]);
            break;
        }
    }

    // Do we accept NRPN in the menu
    if (receives & 0x02) {
        switch (midiEvent.value[0]) {
        case 99:
            this->currentNrpn[timbre].paramMSB = midiEvent.value[1];
            break;
        case 98:  {
            this->currentNrpn[timbre].paramLSB = midiEvent.value[1];
            int index = (this->currentNrpn[timbre].paramMSB << 7) + this->currentNrpn[timbre].paramLSB;
            this->currentNrpn[timbre].row = getParamRowFromNrpnRow(index / NUMBER_OF_ENCODERS);
            this->currentNrpn[timbre].encoder = index % NUMBER_OF_ENCODERS;
            this->currentNrpn[timbre].nrpnType = getNrpnType(&this->currentNrpn[timbre]);
            // init param to null
            this->currentNrpn[timbre].param = NULL;
            }
            break;
        case 6:
            this->currentNrpn[timbre].valueMSB = midiEvent.value[1];
            if (!requires14bits(&this->currentNrpn[timbre])) {
                executeNrpn(timbre, midiEvent.value[1]);
            } 
            break;
        case 38:
            this->currentNrpn[timbre].valueLSB = midiEvent.value[1];
            executeNrpn(timbre,  (float)((this->currentNrpn[timbre].valueMSB << 7) + this->currentNrpn[timbre].valueLSB));
            break;
        case 96:
            // nrpn increment
            if (requires14bits(&this->currentNrpn[timbre])) {
                if (this->currentNrpn[timbre].valueLSB == 127) {
                    this->currentNrpn[timbre].valueLSB = 0;
                    this->currentNrpn[timbre].valueMSB ++;
                } else {
                    this->currentNrpn[timbre].valueLSB ++;
                }
                executeNrpn(timbre,  (float)((this->currentNrpn[timbre].valueMSB << 7) + this->currentNrpn[timbre].valueLSB));
            } else {
                this->currentNrpn[timbre].valueMSB ++;
                executeNrpn(timbre,  (float)this->currentNrpn[timbre].valueMSB);                
            }
            break;
        case 97:
            // nrpn decremenet
            if (requires14bits(&this->currentNrpn[timbre])) {
                if (this->currentNrpn[timbre].valueLSB == 0) {
                    this->currentNrpn[timbre].valueLSB = 127;
                    this->currentNrpn[timbre].valueMSB --;
                } else {
                    this->currentNrpn[timbre].valueLSB --;
                }
                executeNrpn(timbre,  (float)((this->currentNrpn[timbre].valueMSB << 7) + this->currentNrpn[timbre].valueLSB));
            } else {
                this->currentNrpn[timbre].valueMSB --;
                executeNrpn(timbre,  (float)this->currentNrpn[timbre].valueMSB);                                
            }
            break;
        default:
            break;
        }

    }
}


bool MidiDecoder::requires14bits(struct Nrpn * nrpn) {
    if (!this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN]
        || nrpn->nrpnType == NRPN_VALUE_FLOAT 
        || (nrpn->nrpnType == NRPN_VALUE_INT && nrpn->param->numberOfValues > 127)) {
        return true;
    }
    return false;
}

NrpnType MidiDecoder::getNrpnType(struct Nrpn * nrpn) {
    if (nrpn->paramMSB < 2) {

        if (nrpn->row < NUMBER_OF_ROWS) {
            nrpn->param = &(allParameterRows.row[nrpn->row]->params[nrpn->encoder]);
            if (nrpn->param->displayType < DISPLAY_TYPE_INT) {
                // Float parameter
               return NRPN_VALUE_FLOAT;
            }

            return NRPN_VALUE_INT;
        } else {
            unsigned int index = (nrpn->paramMSB << 7) + nrpn->paramLSB;
            if (index >= 228 && index < 240) {
                return NRPN_LETTER;
            }
        }
    } else if (nrpn->paramMSB < 4)  {
            return NRPN_STEPSEQ;
    } else if (nrpn->paramMSB == 127 && nrpn->paramLSB == 127)  {
        return NRPN_FLUSHALL;
    }
    return NRPN_BADCOMMAND;
}

/*
NrpnType MidiDecoder::getNrpnType(int timbre) {
    if (this->currentNrpn[timbre].paramMSB < 2) {

        if (this->currentNrpn[timbre].row < NUMBER_OF_ROWS) {
            this->currentNrpn[timbre].param = &(allParameterRows.row[this->currentNrpn[timbre].row]->params[this->currentNrpn[timbre].encoder]);
            if (this->currentNrpn[timbre].param->displayType < DISPLAY_TYPE_INT) {
                // Float parameter
               return NRPN_VALUE_FLOAT;
            }

            return NRPN_VALUE_INT;
        } else {
            unsigned int index = (this->currentNrpn[timbre].paramMSB << 7) + this->currentNrpn[timbre].paramLSB;
            if (index >= 228 && index < 240) {
                return NRPN_LETTER;
            }
        }
    } else if (this->currentNrpn[timbre].paramMSB < 4)  {
            return NRPN_STEPSEQ;
    } else if (this->currentNrpn[timbre].paramMSB == 127 && this->currentNrpn[timbre].paramLSB == 127)  {
        return NRPN_FLUSHALL;
    }
    return NRPN_BADCOMMAND;
}
*/

void MidiDecoder::executeNrpn(int timbre, float nrpnValue) {
    switch (this->currentNrpn[timbre].nrpnType) {
        case NRPN_VALUE_INT: 
            this->synth->setNewValueFromMidi(timbre, this->currentNrpn[timbre].row, this->currentNrpn[timbre].encoder, nrpnValue);
            break;
        case NRPN_VALUE_FLOAT: { 
            unsigned int row = this->currentNrpn[timbre].row;
            unsigned int encoder = this->currentNrpn[timbre].encoder;
            struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
           	nrpnValue = nrpnValue * .01f + param->minValue;
            this->synth->setNewValueFromMidi(timbre, row,  encoder, nrpnValue);
        }
        break;
        case NRPN_LETTER: {
            unsigned int index = (this->currentNrpn[timbre].paramMSB << 7) + this->currentNrpn[timbre].paramLSB;
            this->synth->setNewSymbolInPresetName(timbre, index - 228, nrpnValue);
            if (index == 239) {
                this->synthState->propagateNewPresetName();
            }
        }
        break;
        case NRPN_STEPSEQ: { 
            unsigned int whichStepSeq = this->currentNrpn[timbre].paramMSB -2;
            unsigned int step = this->currentNrpn[timbre].paramLSB;

            this->synthState->setNewStepValue(timbre, whichStepSeq, step, nrpnValue);
        }
        break;
       case NRPN_FLUSHALL:
            sendCurrentPatchAsNrpns(timbre);
        
       break;
    }
/*
    if (this->currentNrpn[timbre].paramMSB < 2) {
        unsigned int row = this->currentNrpn[timbre].row;
        unsigned int encoder = this->currentNrpn[timbre].encoder;


        if (row < NUMBER_OF_ROWS) {
            struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
            if (param->displayType < DISPLAY_TYPE_INT) {
                // Float parameter
            	value = value * .01f + param->minValue;
            }

            this->synth->setNewValueFromMidi(timbre, row, encoder, value);
        } else if (index >= 228 && index < 240) {
            unsigned int index = (this->currentNrpn[timbre].paramMSB << 7) + this->currentNrpn[timbre].paramLSB;
            this->synth->setNewSymbolInPresetName(timbre, index - 228, value);
            if (index == 239) {
                this->synthState->propagateNewPresetName();
            }
        }
    } else if (this->currentNrpn[timbre].paramMSB < 4)  {
        unsigned int whichStepSeq = this->currentNrpn[timbre].paramMSB -2;
        unsigned int step = this->currentNrpn[timbre].paramLSB;

        this->synthState->setNewStepValue(timbre, whichStepSeq, step, nrpnValue);
    } else if (this->currentNrpn[timbre].paramMSB == 127 && this->currentNrpn[timbre].paramLSB == 127)  {
        sendCurrentPatchAsNrpns(timbre);
    }
    */
}


void MidiDecoder::sendCurrentPatchAsNrpns(int timbre) {
    struct MidiEvent cc;

    OneSynthParams * paramsToSend = this->synth->getTimbre(timbre)->getParamRaw();

    cc.eventType = MIDI_CONTROL_CHANGE;
    // Si channel = ALL envoie sur 1
    int channel = this->synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1 + timbre] -1;
    if (channel == -1) {
        channel = 0;
    }
    cc.channel = channel;


    // Send the title
    for (unsigned int k=0; k<12; k++) {
        int valueToSend = paramsToSend->presetName[k];
        cc.value[0] = 99;
        cc.value[1] = 1;
        sendMidiCCOut(&cc, false);
        cc.value[0] = 98;
        cc.value[1] = 100+k;
        sendMidiCCOut(&cc, false);
        cc.value[0] = 6;
        if (!this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN]) {
            cc.value[1] = 0;
            sendMidiCCOut(&cc, false);
            cc.value[0] = 38;
        }
        cc.value[1] = (unsigned char) (valueToSend & 127);
        sendMidiCCOut(&cc, false);

        flushMidiOut();
        // Wait for midi to be flushed
        while (usartBufferOut.getCount()>0) {}
    }

    // flush all parameters
    for (int currentrow = 0; currentrow < NUMBER_OF_ROWS; currentrow++) {
        for (int encoder = 0; encoder < NUMBER_OF_ENCODERS; encoder++) {
            struct ParameterDisplay* param = &allParameterRows.row[currentrow]->params[encoder];
            float floatValue = ((float*)paramsToSend)[currentrow * NUMBER_OF_ENCODERS + encoder];

            // MSB / LSB
            int paramNumber =  getNrpnRowFromParamRow(currentrow) * NUMBER_OF_ENCODERS + encoder;
            // Value to send must be positive

            // NRPN is 4 control change
            cc.value[0] = 99;
            cc.value[1] = (unsigned char)(paramNumber >> 7);
            sendMidiCCOut(&cc, false);
            cc.value[0] = 98;
            cc.value[1] = (unsigned char)(paramNumber & 127);
            sendMidiCCOut(&cc, false);

            int valueToSend;
            bool requires14B = true;
            if (param->displayType < DISPLAY_TYPE_INT) {
                valueToSend = (floatValue - param->minValue) * 100.0f + .1f ;
            } else {
                valueToSend = floatValue + .1f ;
                if (this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN] && param->numberOfValues <= 127.0) {
                    requires14B = false;
                }
            }

            if (requires14B) {
                cc.value[0] = 6;
                cc.value[1] = (unsigned char) (valueToSend >> 7);
                sendMidiCCOut(&cc, false);
                cc.value[0] = 38;
                cc.value[1] = (unsigned char) (valueToSend & 127);
                sendMidiCCOut(&cc, false);
            } else {
                cc.value[0] = 6;
                cc.value[1] = (unsigned char) (valueToSend & 127);
                sendMidiCCOut(&cc, false);
            }

            flushMidiOut();
            // Wait for midi to be flushed
            while (usartBufferOut.getCount()>0) {}
        }
    }

    // Step Seq
    for (int whichStepSeq = 0; whichStepSeq < 2; whichStepSeq++) {
         for (int step = 0; step<16; step++) {
             cc.value[0] = 99;
             cc.value[1] = whichStepSeq + 2;
             sendMidiCCOut(&cc, false);
             cc.value[0] = 98;
             cc.value[1] = step;
             sendMidiCCOut(&cc, false);
             cc.value[0] = 6;
             if (!this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN]) {
                cc.value[1] = 0;
                sendMidiCCOut(&cc, false);
                cc.value[0] = 38;
             }
             StepSequencerSteps *seqSteps = &((StepSequencerSteps * )(&paramsToSend->lfoSteps1))[whichStepSeq];
             cc.value[1] = seqSteps->steps[step];
             sendMidiCCOut(&cc, false);

             flushMidiOut();
             // Wait for midi to be flushed
             while (usartBufferOut.getCount()>0) {}
         }
     }

}


void MidiDecoder::newParamValueFromExternal(int timbre, int currentrow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {
    // Nothing to do
}


void MidiDecoder::newParamValue(int timbre, int currentrow,
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
        if ((currentrow == ROW_LFOSEQ1 || currentrow == ROW_LFOSEQ2) && encoder >=2) {
            if (encoder == 2) {
                return;
            }
            if (encoder == 3) {
                // We modify a step value, we want to send that
                int currentStepSeq = currentrow - ROW_LFOSEQ1;
                // NRPN is 4 control change
                cc.value[0] = 99;
                cc.value[1] = currentStepSeq + 2;
                sendMidiCCOut(&cc, false);
                cc.value[0] = 98;
                cc.value[1] = this->synthState->stepSelect[currentStepSeq];
                sendMidiCCOut(&cc, false);
                cc.value[0] = 6;
                if (!this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN]) {
                    cc.value[1] = 0;
                    sendMidiCCOut(&cc, false);
                    cc.value[0] = 38;
                }                
                cc.value[1] = (unsigned char) newValue;
                sendMidiCCOut(&cc, false);

                flushMidiOut();
            }
        } else {
            int valueToSend;
            bool requires14B = true;
            if (param->displayType < DISPLAY_TYPE_INT) {
                valueToSend = (newValue - param->minValue) * 100.0f + .1f ;
            } else {
                valueToSend = newValue + .1f ;
                if (this->synthState->fullState.midiConfigValue[MIDICONFIG_STRICT_NRPN] && param->numberOfValues <= 127.0) {
                    requires14B = false;
                }
            }
            // MSB / LSB
            int paramNumber =  getNrpnRowFromParamRow(currentrow) * NUMBER_OF_ENCODERS + encoder;
            // Value to send must be positive

            // NRPN is 4 control change
            cc.value[0] = 99;
            cc.value[1] = (unsigned char)(paramNumber >> 7);
            sendMidiCCOut(&cc, false);
            cc.value[0] = 98;
            cc.value[1] = (unsigned char)(paramNumber & 127);
            sendMidiCCOut(&cc, false);
            if (requires14B) {
                cc.value[0] = 6;
                cc.value[1] = (unsigned char) (valueToSend >> 7);
                sendMidiCCOut(&cc, false);
                cc.value[0] = 38;
                cc.value[1] = (unsigned char) (valueToSend & 127);
                sendMidiCCOut(&cc, false);
            } else {
                cc.value[0] = 6;
                cc.value[1] = (unsigned char) (valueToSend & 127);
                sendMidiCCOut(&cc, false);                
            }

            flushMidiOut();
        }
    }

    // Do we send control change
    if (sendCCOrNRPN == 1) {
        cc.value[0] = 0;

        switch (currentrow) {
        case ROW_ENGINE:
            if (encoder == ENCODER_ENGINE_ALGO) {
                cc.value[1] = newValue;
                cc.value[0] = CC_ALGO;
            }
            break;
        case ROW_MODULATION1:
            if ((encoder & 0x1) == 0) {
                cc.value[1] = newValue * 10 + .001f;
                cc.value[0] = CC_IM1 + (encoder >> 1);
            }
            break;
        case ROW_MODULATION2:
            if ((encoder & 0x1) == 0) {
                cc.value[1] = newValue * 10 + .001f;
                cc.value[0] = CC_IM3 + encoder;
            }
			break;
        case ROW_MODULATION3:
            if ((encoder & 0x1) == 0) {
                cc.value[1] = newValue * 10 + .001f;
                cc.value[0] = CC_IM5 + encoder;
            }
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
        switch (encoder) {
        case ENCODER_LFO_SHAPE:
            cc.value[0] = CC_LFO1_SHAPE + (currentrow - ROW_LFO_FIRST);
            cc.value[1] = newValue + .1f;
            break;
        case ENCODER_LFO_FREQ:
            cc.value[0] = CC_LFO1_FREQ + (currentrow - ROW_LFO_FIRST);
            cc.value[1] = newValue * 5.0f + .1f;
            break;
        case ENCODER_LFO_BIAS:
            cc.value[0] = CC_LFO1_BIAS + (currentrow - ROW_LFO_FIRST);
            cc.value[1] = newValue * 100.0f + .1f;
            break;
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
        case ROW_EFFECT:
            if (encoder == ENCODER_EFFECT_TYPE) {
                cc.value[0] = CC_FILTER_TYPE;
                cc.value[1] = newValue + .1f;
            } else {
                cc.value[0] = CC_FILTER_PARAM1 + encoder - 1;
                cc.value[1] = newValue * 100.0f + .1f;
            }
            break;
        case ROW_ENV1a:
        case ROW_ENV2a:
        case ROW_ENV3a:
        case ROW_ENV4a:
        case ROW_ENV5a:
        case ROW_ENV6a:
            if (encoder == ENCODER_ENV_A) {
                cc.value[0] = CC_ENV_ATK_OP1 + ((currentrow - ROW_ENV1a) >> 1);
                cc.value[1] = newValue * 64.0f + .1f;
            }
            break;
        case ROW_ENV1b:
        case ROW_ENV2b:
        case ROW_ENV3b:
        case ROW_ENV4b:
        case ROW_ENV5b:
        case ROW_ENV6b:
            if (encoder == ENCODER_ENV_R) {
                cc.value[0] = CC_ENV_REL_OP1 + ((currentrow - ROW_ENV1b) >> 1);
                cc.value[1] = newValue * 32.0f + .1f;
            }
            break;
        case ROW_LFOPHASES:
            cc.value[0] = CC_LFO1_PHASE + encoder;
            cc.value[1] = newValue * 100.0f + .1f;
            break;

        }

        if (cc.value[0] != 0) {
        	// CC limited to 127
        	if (cc.value[1] > 127) {
    			cc.value[1] = 127;
            }

        	if (lastSentCC.value[1] != cc.value[1] || lastSentCC.value[0] != cc.value[0] || lastSentCC.channel != cc.channel) {
        		sendMidiCCOut(&cc, true);
        		lastSentCC = cc;
        	}
        }
    }
}


/** Only send control change */
void MidiDecoder::sendMidiCCOut(struct MidiEvent *toSend, bool flush) {

   // We don't send midi to both USB and USART at the same time...
	if (this->synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT) {
		// usbBuf[0] = [number of cable on 4 bits] [event type on 4 bites]
		*usbBufWrite++ = 0x00  | (toSend->eventType >> 4);
		*usbBufWrite++ = toSend->eventType + toSend->channel;
		*usbBufWrite++ = toSend->value[0];
		*usbBufWrite++  = toSend->value[1];

	}

	usartBufferOut.insert(toSend->eventType + toSend->channel);
	usartBufferOut.insert(toSend->value[0]);
	usartBufferOut.insert(toSend->value[1]);

	if (flush) {
		flushMidiOut();
	}
}
void MidiDecoder::sendSysexByte(uint8_t byte) {

	if (this->synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT) {
		switch (usbBufWrite - usbBufRead) {
		case 0:
			*usbBufWrite++ = 0x00  | 0x4;
			*usbBufWrite++ = byte;
			break;
		case 1:
			//
			break;
		case 2:
			*usbBufWrite++ = byte;
			break;
		case 3:
			*usbBufWrite++ = byte;
			// Don't send now as we don't know yet if it's the last packet...
			break;
		case 4:
			DCD_EP_Flush (&usbOTGDevice,0x81);
	        DCD_EP_Tx(&usbOTGDevice, 0x81, usbBufRead, 64);

	        usbBufRead += 64;
			if (usbBufRead >= usbBuf+128) {
				usbBufRead = usbBuf;
			}
			usbBufWrite = usbBufRead;

			*usbBufWrite++ = 0x00  | 0x4;
			*usbBufWrite++ = byte;
			break;
		}
	}

	usartBufferOut.insert(byte);
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	// Wait for midi to be flushed
	while (usartBufferOut.getCount()>0) {}

}


void MidiDecoder::sendSysexFinished() {
	bool usbMidi = false;

	if (synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT) {
		switch (usbBufWrite - usbBufRead) {
		case 0:
			// impossible : see sendSysexByte
			break;
		case 1:
			// 0x5 SysEx ends with 1 byte
			usbBufRead[0] = 0x00  | 0x5;
			// Zero the rest
			usbBufRead[2] = 0;
			usbBufRead[3] = 0;
			break;
		case 2:
			// 0x6 SysEx ends with 2 bytes
			usbBufRead[0] = 0x00  | 0x6;
			// Zero the rest
			usbBufRead[3] = 0;
			break;
		case 3:
			// 0x7 SysEx ends with 3 bytes
			usbBufRead[0] = 0x00  | 0x7;
			break;
		}
	}
	DCD_EP_Flush (&usbOTGDevice,0x81);
	DCD_EP_Tx(&usbOTGDevice, 0x81, usbBufRead, 64);
    usbBufRead += 64;
	if (usbBufRead >= usbBuf+128) {
		usbBufRead = usbBuf;
	}
	usbBufWrite = usbBufRead;
}


void MidiDecoder::flushMidiOut() {

	if (this->synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT) {
		if (usbOTGDevice.dev.device_status != USB_OTG_SUSPENDED) {
			DCD_EP_Flush (&usbOTGDevice,0x81);
			DCD_EP_Tx(&usbOTGDevice, 0x81, usbBufRead, 64);
		}

		usbBufRead += 64;
		if (usbBufRead >= usbBuf+128) {
			usbBufRead = usbBuf;
		}
		usbBufRead[0] = 0;
		usbBufRead[4] = 0;
		usbBufRead[8] = 0;
		usbBufRead[12] = 0;
		usbBufWrite = usbBufRead;
	}

	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}

int MidiDecoder::getNrpnRowFromParamRow(int paramRow) {
    switch (paramRow) {
    case ROW_LFOPHASES:
        // Move after ROW_LFOSEQ2
        return paramRow + 4;
    case ROW_LFOENV1:
    case ROW_LFOENV2:
    case ROW_LFOSEQ1:
    case ROW_LFOSEQ2:
        // Move before ROW_LFOPHASE
        return paramRow - 1;
    default:
        return paramRow;
    }
}

int MidiDecoder::getParamRowFromNrpnRow(int nrpmRow) {
    // Move back row
    switch (nrpmRow) {
    case ROW_LFOPHASES + 4:
        return ROW_LFOPHASES;
    case ROW_LFOENV1 - 1:
    case ROW_LFOENV2 - 1:
    case ROW_LFOSEQ1 - 1:
    case ROW_LFOSEQ2 - 1:
        return nrpmRow + 1;
    default:
        return nrpmRow;
    }
    return nrpmRow;
}




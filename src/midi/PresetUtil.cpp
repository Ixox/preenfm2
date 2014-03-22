/*
 * Copyright 2013 Xavier Hosxe
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
#include "PresetUtil.h"
#include "Storage.h"
#include "SynthState.h"
#include "LiquidCrystal.h"
#include "ff.h"
#include "usb_core.h"
#include "usbh_core.h"
#include "stm32f4xx_usart.h"
#include "usb_dcd.h"
#include "usbKey_usr.h"

extern RingBuffer<uint8_t, 200> usartBufferIn;


extern LiquidCrystal lcd;


SynthState * PresetUtil::synthState;
Storage * PresetUtil::storage;
MidiDecoder * PresetUtil::midiDecoder;

uint8_t PresetUtil::sysexBuffer[3];
int PresetUtil::sysexIndex = 0;

uint8_t PresetUtil::sysexTmpMem[8 + PATCH_SIZE_PFM2 * 128 + 1]; // +1 to store if it's a PFM1 '1' or PFM2 '2'


struct OneSynthParams oneSynthParamsTmp;

#define SYSEX_PFM1_BYTE_PATCH 1
#define SYSEX_PFM1_BYTE_BANK 2
#define SYSEX_PFM2_BYTE_PATCH 3
#define SYSEX_PFM2_BYTE_BANK 4

enum {
	SYSEX_UNKNOWN = 0,
    SYSEX_PREENFM2_PATCH,
    SYSEX_PREENFM2_BANK,
    SYSEX_DX7_BANK,
    SYSEX_PREENFM1_PATCH,
    SYSEX_PREENFM1_BANK,
};


PresetUtil::PresetUtil() {
}

PresetUtil::~PresetUtil() {
}



void PresetUtil::setSynthState(SynthState* synthState) {
    PresetUtil::synthState = synthState;
}

void PresetUtil::setStorage(Storage* storage) {
    PresetUtil::storage = storage;
}

void PresetUtil::setMidiDecoder(MidiDecoder* midiDecoder) {
    PresetUtil::midiDecoder = midiDecoder;
}




void PresetUtil::sendCurrentPatchToSysex() {
    uint8_t newPatch[] = { 0xf0, 0x7d, SYSEX_PFM2_BYTE_PATCH };
    for (int k = 0; k <= 2; k++) {
        midiDecoder->sendSysexByte(newPatch[k]);
    }

    PresetUtil::convertSynthStateToCharArray(PresetUtil::synthState->params, sysexTmpMem);
    PresetUtil::sendParamsToSysex(sysexTmpMem);

    midiDecoder->sendSysexByte(0xf7);
    midiDecoder->sendSysexFinished();
}


void PresetUtil::sendBankToSysex(int bankNumber) {
    unsigned char newPatch[] = { 0xf0, 0x7d, SYSEX_PFM2_BYTE_BANK };
    for (int k = 0; k <= 2; k++) {
    	midiDecoder->sendSysexByte(newPatch[k]);
    }
    const struct BankFile* bank = storage->getPreenFMBank(bankNumber);

    // send bank Name
    char bankNameToSend[8];
    for (int k=0; k<8; k++) {
    	bankNameToSend[k] = '\0';
    }
    const char* bankName = bank->name;
    for (int k=0; k<8 && bankName[k] != '\0' && bankName[k] != '.'; k++) {
		char c1 = bankName[k];
		if (c1 >= 'a' && c1<='z') {
			c1 = 'A' + c1 - 'a';
		}
		bankNameToSend[k] = c1;
    }
    for (int k=0; k<8; k++) {
    	midiDecoder->sendSysexByte(bankNameToSend[k]);
    }

    for (int preset = 0; preset < 128; preset++) {
        lcd.setCursor(3,2);
        lcd.print(preset);
        lcd.print(" / 128");
        storage->loadPreenFMPatch(bank, preset, &oneSynthParamsTmp);
        PresetUtil::convertSynthStateToCharArray(&oneSynthParamsTmp, sysexTmpMem);
        PresetUtil::sendParamsToSysex(sysexTmpMem);
    }

    midiDecoder->sendSysexByte((unsigned char) 0xf7);
    midiDecoder->sendSysexFinished();
}




void PresetUtil::sendNrpn(struct MidiEvent cc) {
	/*
    sendSysexByte((unsigned char) (cc.eventType + cc.channel));
    sendSysexByte((unsigned char) cc.value[0]);
    sendSysexByte((unsigned char) cc.value[1]);
    */
}

void PresetUtil::sendCurrentPatchAsNrpns(int timbre) {
/* TODO : when working on the VST...
	int channel = PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] -1;
    struct MidiEvent cc;
    cc.eventType = MIDI_CONTROL_CHANGE;
    // Si channel = ALL envoie sur 1
    if (channel == -1) {
        channel = 0;
    }
    cc.channel = channel;


    // Send the title
    for (unsigned int k=0; k<12; k++) {
        int valueToSend = PresetUtil::synthState->params->presetName[k];
        cc.value[0] = 99;
        cc.value[1] = 1;
        sendNrpn(cc);
        cc.value[0] = 98;
        cc.value[1] = 100+k;
        sendNrpn(cc);
        cc.value[0] = 6;
        cc.value[1] = (unsigned char) (valueToSend >> 7);
        sendNrpn(cc);
        cc.value[0] = 38;
        cc.value[1] = (unsigned char) (valueToSend & 127);
        sendNrpn(cc);
    }

    // MSB / LSB
    for (int currentrow = 0; currentrow < NUMBER_OF_ROWS; currentrow++) {

        for (int encoder = 0; encoder < NUMBER_OF_ENCODERS; encoder++) {

            struct ParameterDisplay param = allParameterRows.row[currentrow]->params[encoder];
            int newValue = ((float*)&PresetUtil::synthState->params)[currentrow*NUMBER_OF_ENCODERS+encoder];
            int valueToSend = newValue - param.minValue;
            int paramNumber = currentrow * NUMBER_OF_ENCODERS+ encoder;
            // NRPN is 4 control change
            cc.value[0] = 99;
            cc.value[1] = (unsigned char)(paramNumber >> 7);
            sendNrpn(cc);
            cc.value[0] = 98;
            cc.value[1] = (unsigned char)(paramNumber & 127);
            sendNrpn(cc);
            cc.value[0] = 6;
            cc.value[1] = (unsigned char) (valueToSend >> 7);
            sendNrpn(cc);
            cc.value[0] = 38;
            cc.value[1] = (unsigned char) (valueToSend & 127);
            sendNrpn(cc);
        }
    }

    for (int whichStepSeq = 0; whichStepSeq < 2; whichStepSeq++) {
        for (int step = 0; step<16; step++) {
            cc.value[0] = 99;
            cc.value[1] = whichStepSeq + 2;
            sendNrpn(cc);
            cc.value[0] = 98;
            cc.value[1] = step;
            sendNrpn(cc);
            cc.value[0] = 6;
            cc.value[1] = 0;
            sendNrpn(cc);
            cc.value[0] = 38;
            StepSequencerSteps * seqSteps = &((StepSequencerSteps * )(&PresetUtil::synthState->params->lfoSteps1))[whichStepSeq];
            cc.value[1] = seqSteps->steps[step];
            sendNrpn(cc);
        }
    }
*/
}


void PresetUtil::sendParamsToSysex(unsigned char* params) {
    int checksum = 0;

    for (int k = 0; k < PATCH_SIZE_PFM2; k++) {
        unsigned char byte = params[k];
        checksum += byte;
        midiDecoder->sendSysexByte(byte);
    }

    midiDecoder->sendSysexByte((unsigned char) (checksum % 128));
}

int PresetUtil::readSysexPatch(unsigned char* params) {
    int checkSum = 0;
    unsigned int index = 0;
    int value = 0;

    for (int k = 0; k < PATCH_SIZE_PFM2; k++) {

        int byte = PresetUtil::getNextMidiByte();
        if (byte < 0) {
            return -k - 1000;
        }

        params[k] = byte;
		checkSum += byte;
		value = 0;
    }

    int sentChecksum = PresetUtil::getNextMidiByte();


    if (sentChecksum <0) {
        return -198;
    } else {
        checkSum = checkSum % 128;

        if (checkSum != sentChecksum) {
        	lcd.setCursor(0,0);
            return -200 - sentChecksum;
        }
    }
    return sentChecksum;
}

//void PresetUtil::resetConfigAndSaveToEEPROM() {
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_USB] = 2;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] = 1;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2] = 2;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3] = 3;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4] = 4;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_THROUGH] = 0;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_RECEIVES] = 3;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_SENDS] = 1;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_REALTIME_SYSEX] = 0;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_TEST_NOTE] = 60;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_TEST_VELOCITY] = 120;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_ENCODER] = 0;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_OP_OPTION] = 0;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_LED_CLOCK] = 1;
//    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET] = 1;
//}



int PresetUtil::getNextMidiByte() {
    int timeout = 0;
    while (usartBufferIn.getCount() == 0) {
        if (timeout++ >= 50000000) {
            return -1;
        }
    }
	return (int)usartBufferIn.remove();
}

/*
 * 0xf0 is already read.
 * 0x7d : non commercial
 * 01 = patch, 02 = bank
 *
 * return value : -1 : error
 *                1 : patch read
 *                2 : bank read (and store to temporary eeprom)
 */

int PresetUtil::readSysex(bool patchAllowed, bool bankAllowed) {
	lcd.setRealTimeAction(true);
	int sysexType = SYSEX_UNKNOWN;
	bool bError = false;
    bool bSysexRead = false;
    int index = 0;

    while (true) {
        int byte = PresetUtil::getNextMidiByte();
        if (byte<0) {
            bError = true;
            break;
        }

        if (byte == 0xF7) {
            // Should be found after patch or full bank is read....
            break;
        }

        if (index  == 0) {
            // Batch or bank
            index++;
            if (byte == SYSEX_PFM2_BYTE_PATCH) {
            	sysexType = SYSEX_PREENFM2_PATCH;
            } else if (byte == SYSEX_PFM2_BYTE_BANK) {
            	sysexType = SYSEX_PREENFM2_BANK;
            } else if (byte == SYSEX_PFM1_BYTE_PATCH) {
                	sysexType = SYSEX_PREENFM1_PATCH;
            } else if (byte == SYSEX_PFM1_BYTE_BANK) {
                	sysexType = SYSEX_PREENFM1_BANK;
            } else {
                bError = true;
                break;
            }
            if (((sysexType == SYSEX_PREENFM2_PATCH || sysexType == SYSEX_PREENFM1_PATCH) && !patchAllowed)
            		|| ((sysexType == SYSEX_PREENFM2_BANK || sysexType == SYSEX_PREENFM1_BANK) && !bankAllowed)) {
                // Will wait untill F7 is received (end of sysex)
                bSysexRead = true;
                bError = true;
            }
        }

        if (index >0 && !bSysexRead) {
            bSysexRead = true;
            if (sysexType == SYSEX_PREENFM2_PATCH) {
                int errorCode = PresetUtil::readSysexPatch(sysexTmpMem);

                if (errorCode < 0) {
                    index = -errorCode;
                    bError = true;
                } else {
                    PresetUtil::synthState->propagateBeforeNewParamsLoad(0);
                    PresetUtil::convertCharArrayToSynthState(sysexTmpMem, PresetUtil::synthState->params);
                    PresetUtil::synthState->propagateAfterNewParamsLoad(0);
                    PresetUtil::synthState->resetDisplay();
                }
            } else if (sysexType == SYSEX_PREENFM2_BANK) {
                // Bank !!
                int errorCode = 0;
                if ((errorCode = PresetUtil::readSysexBank()) <0) {
                    index = -errorCode;
                    bError = true;
                }
            } else if (sysexType == SYSEX_PREENFM1_PATCH) {
                int errorCode = PresetUtil::readSysexPatchPFM1(sysexTmpMem);

                if (errorCode < 0) {
                    index = -errorCode;
                    bError = true;
                } else {
                    PresetUtil::synthState->propagateBeforeNewParamsLoad(0);
                    PresetUtil::convertPFM1CharArrayToSynthState(sysexTmpMem, PresetUtil::synthState->params, true);
                    PresetUtil::synthState->propagateAfterNewParamsLoad(0);
                    PresetUtil::synthState->resetDisplay();
                }
            } else if (sysexType == SYSEX_PREENFM1_BANK) {
                // Bank !!
                int errorCode = 0;
                if ((errorCode = PresetUtil::readSysexBankPFM1()) <0) {
                    index = -errorCode;
                    bError = true;
                }
            }

        }
    }
	lcd.setRealTimeAction(false);

    return bError ? -index : ((sysexType == SYSEX_PREENFM2_PATCH || sysexType == SYSEX_PREENFM1_PATCH) ? 1 : 2);
}


void PresetUtil::copySynthParams(char* source, char* dest) {
    for (int k=0; k<sizeof(struct OneSynthParams); k++) {
        dest[k] = source[k];
    }
}


int PresetUtil::readSysexBank() {
    int errorCode = 0;

    // receive bank name
    for (int k=0; k<8; k++) {
    	sysexTmpMem[k] = getNextMidiByte();
    }

    lcd.setCursor(1,3);
    lcd.print("Bank:");

    for (int preset = 0; preset<128 && errorCode>=0; preset++) {
        lcd.setCursor(7,3);
        lcd.print(preset);

        if ((errorCode = PresetUtil::readSysexPatch(sysexTmpMem + 8 + PATCH_SIZE_PFM2 * preset)) <0) {
            lcd.setCursor(11,3);
            lcd.print("##");
            lcd.print(errorCode);
            errorCode = -500 - preset;
            break;
        }
    }

    sysexTmpMem[8 + PATCH_SIZE_PFM2 * 128 ] = '2';

    return errorCode;
}






// ABSTRACTION OF MEMORY/SYSEX MANAGEMENT FOR FUTUR COMPATIBILITY


bool PresetUtil::isSpecialSyexValue(int row, int encoder) {
    if (row == ROW_MODULATION1 && row == ROW_MODULATION2) {
    	return true;
    }
    if (row >= ROW_LFOOSC1 && row <= ROW_LFOOSC3) {
    	if (encoder == ENCODER_LFO_FREQ) {
    		return true;
    	}
    }
    return false;
}

uint16_t PresetUtil::getShortFromParamFloat(int row, int encoder, float value) {
    // performance do not send NRPN
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    uint16_t rValue;
    uint16_t valueToSend = 0;

    if (isSpecialSyexValue(row, encoder)) {
        valueToSend = value * 100.0f + .1f ;
    } else {
        valueToSend = ((float)(param->numberOfValues - 1.0f) * (value - param->minValue) / (param->maxValue - param->minValue) + .1f);
    }

    return valueToSend;
}

float PresetUtil::getParamFloatFromShort(int row, int encoder, short value) {
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    if (isSpecialSyexValue(row, encoder)) {
		return value * .01f;
	} else {
		if (param->displayType == DISPLAY_TYPE_STRINGS) {
			return param->minValue + ((float)value) / (param->numberOfValues - 1.0f) * (param->maxValue - param->minValue) + .1f;
		} else {
			return param->minValue + ((float)value) / (param->numberOfValues - 1.0f) * (param->maxValue - param->minValue);
		}
	}
}


void PresetUtil::convertSynthStateToCharArray(OneSynthParams* params, uint8_t* chars) {
    int indexParam;
    int indexSysex = 0;
    for (int row=0; row < NUMBER_OF_ROWS; row++) {
        for (int encoder = 0; encoder < NUMBER_OF_ENCODERS; encoder++) {
            indexParam = row * NUMBER_OF_ENCODERS + encoder;
            float value = ((float*)params)[indexParam];
            uint16_t toSend = PresetUtil::getShortFromParamFloat(row, encoder, value);
            //  MSB and LSB
            chars[indexSysex++] = toSend >> 7;
            chars[indexSysex++] = toSend & 0x7F;
        }
    }
    char *params2 = params->lfoSteps1.steps;
    // Step seq + title
    for (int k=0; k< 16*2 + 13; k++) {
        uint8_t toSend =
        chars[indexSysex++] = params2[k] & 0x7F;
    }
}

void PresetUtil::convertCharArrayToSynthState(const unsigned char* chars, OneSynthParams* params) {
    int indexSysex = 0;
    for (int row=0; row < NUMBER_OF_ROWS; row++) {
        for (int encoder = 0; encoder < NUMBER_OF_ENCODERS; encoder++) {
        	short shortValue = chars[indexSysex++] << 7;
        	shortValue |= chars[indexSysex++];

        	int indexParam = row * NUMBER_OF_ENCODERS + encoder;
            ((float*)params)[indexParam] =  getParamFloatFromShort(row, encoder, shortValue);
        }
    }
    // Step seq + title
    char *params2 = params->lfoSteps1.steps;
    for (int k=0; k< 16*2 + 13; k++) {
        params2[k] = chars[indexSysex++];
    }
}




// ======================================
// from PFM1 code



void PresetUtil::copy4Charto4Float(unsigned char* source, float * dest) {
	for (int k=0; k<4; k++) {
		dest[k] = source[k];
	}
}


// When source is STESEQ and destination matrix is FREQ the ratio is not the same
float PresetUtil::getValueForFreq(struct PFM1MatrixRowParams* pfm1mtx) {
	float ratio1 = 1.0f;
	float ratio2 = 16.0f;

	if (pfm1mtx->source >= PFM1_MATRIX_SOURCE_LFO1 && pfm1mtx->source <= PFM1_MATRIX_SOURCE_LFO3) {
		ratio1 = 0.5f;
	}


	if (pfm1mtx->destination >= PFM1_INDEX_MODULATION1 && pfm1mtx->destination <= PFM1_INDEX_MODULATION4) {
		ratio2 = 1.0f;
	} else if ((pfm1mtx->destination >= PFM1_OSC1_FREQ && pfm1mtx->destination <= PFM1_OSC6_FREQ)
		|| (pfm1mtx->destination == PFM1_ALL_OSC_FREQ)) {
		ratio2 = 6.4f;
	}


	float ret = pfm1mtx->mul / ratio2 * ratio1;

	if (ret>10.0f) {
		ret = 10.0f;
	}

	return ret;
}


void PresetUtil::convertPFM1CharArrayToSynthState(const unsigned char* chars, OneSynthParams* params, bool convert) {
	PFM1SynthParams pfm1params;

	// In 2.0...
	// performance mode added... CC1..4 must not be saved
	// Copy first part A
	unsigned int firstPartASize = 3*4;
	for (unsigned int k=0; k<firstPartASize; k++) {
		((char*) &pfm1params)[k] = chars[k];
	}
	// performance
	for (unsigned int k=0; k<4; k++) {
		((char*) &pfm1params.engine4)[k] = 0;
	}
	// Copy first part B
	unsigned  int firstPartBSize = 20*4;
	for (unsigned int k=0; k<firstPartBSize; k++) {
		((char*) &pfm1params.osc1)[k] = chars[firstPartASize + k];
	}
	int firstPartSize = firstPartASize + firstPartBSize;
	// LFO 1->4
	unsigned int secondPartSize = 4*4;
	for (unsigned int k=0; k<secondPartSize; k++) {
		((char*) &pfm1params.lfo1)[k] = chars[firstPartSize + k];
	}


	// 120 - 128 : step seqs
	for (unsigned int k=0; k<8; k++) {
		((char*) &pfm1params.lfo5)[k] = chars[120 + k];
	}

	// 128 : 16 step of seq 1 on 8 bits
	// 4 left bits + 4 right bits
	for (unsigned int k=0; k<8; k++) {
		pfm1params.steps5.steps[k*2]   = chars[128+k] >> 4;
		pfm1params.steps5.steps[k*2+1] = chars[128+k] & 0xf;
	}
	// 136 : 16 step of seq 2 on 8 bits
	for (unsigned int k=0; k<8; k++) {
		pfm1params.steps6.steps[k*2]   = chars[136+k] >> 4;
		pfm1params.steps6.steps[k*2+1] = chars[136+k] & 0xf;
	}
	// 144 : Matrix 9->12
	for (unsigned int k=0; k<16; k++) {
		((char*) &pfm1params.matrixRowState9)[k] = chars[144 + k];
	}


	// Title
	for (unsigned int k=0; k<12; k++) {
		pfm1params.presetName[k] = chars[27*4+k];
	}
	pfm1params.presetName[12] = '\0';

	/* ================================================== */
	/// init PFM2 params from this

	for (int k=0; k<NUMBER_OF_ROWS * NUMBER_OF_ENCODERS; k++) {
		((float*)params)[k] = 0.0f;
	}
	// Engine
    copy4Charto4Float((unsigned char*)&pfm1params.engine1, (float*)&params->engine1);

    params->engineIm1.modulationIndex1 = pfm1params.engine2.modulationIndex1 / 8.0f;
    params->engineIm1.modulationIndex2 = pfm1params.engine2.modulationIndex2 / 8.0f;
    params->engineIm2.modulationIndex3 = pfm1params.engine2.modulationIndex3 / 8.0f;
    params->engineIm2.modulationIndex4 = pfm1params.engine2.modulationIndex4 / 8.0f;


    params->engineMix1.mixOsc1 = pfm1params.engine3.mixOsc1 / 128.0f;
    params->engineMix1.mixOsc2 = pfm1params.engine3.mixOsc2 / 128.0f;
    params->engineMix2.mixOsc3 = pfm1params.engine3.mixOsc3 / 128.0f;
    params->engineMix2.mixOsc4 = pfm1params.engine3.mixOsc4 / 128.0f;


    // Oscillator
    for (int o=0; o<NUMBER_OF_OPERATORS; o++) {
    	struct OscillatorParams* osc = &((struct OscillatorParams*)&params->osc1)[o];
    	struct PFM1OscillatorParams* pfm1osc = &((struct PFM1OscillatorParams*)&pfm1params.osc1)[o];

    	switch (pfm1osc->shape) {
    	case PFM1_OSC_SHAPE_SIN:
    		osc->shape = OSC_SHAPE_SIN;
    		break;
    	case PFM1_OSC_SHAPE_SIN2:
    		osc->shape = OSC_SHAPE_SIN_SQUARE;
    		break;
    	case PFM1_OSC_SHAPE_SIN3:
    		osc->shape = OSC_SHAPE_SIN_ZERO;
    		break;
    	case PFM1_OSC_SHAPE_SIN4:
    		osc->shape = OSC_SHAPE_SIN_POS;
    		break;
    	case PFM1_OSC_SHAPE_RAND:
    		osc->shape = OSC_SHAPE_RAND;
    		break;
    	case PFM1_OSC_SHAPE_SQUARE:
    		osc->shape = OSC_SHAPE_SQUARE;
    		break;
    	case PFM1_OSC_SHAPE_SAW:
    		osc->shape = OSC_SHAPE_SAW;
    		break;
    	}
    	osc->frequencyMul = pfm1osc->frequencyMul / 16.0f;
    	osc->detune = pfm1osc->detune / 100.0f;
    	osc->frequencyType = pfm1osc->frequencyType;
    }

    // Oscillator
    for (int e=0; e<NUMBER_OF_OPERATORS; e++) {
    	struct EnvelopeParamsA* envA = &((struct EnvelopeParamsA*)&params->env1a)[e*2];
    	struct EnvelopeParamsB* envB = &((struct EnvelopeParamsB*)&params->env1b)[e*2];
    	struct PFM1EnvelopeParams* pfm1env = &((struct PFM1EnvelopeParams*)&pfm1params.env1)[e];

    	envA->attackLevel = 1.0f;
    	envA->attackTime = pfm1env->attack * pfm1env->attack / 32768.0f;
    	envA->decayLevel = 1.0f;
    	envA->decayTime = 0.0f;

    	envB->sustainLevel = pfm1env->sustain / 255.0f;
    	envB->sustainTime = pfm1env->decay * pfm1env->decay / 32768.0f;

    	envB->releaseLevel = 0.0f;
    	envB->releaseTime = pfm1env->release * pfm1env->release / 32768.0f;
    }


    // Oscillator
    for (int m=0; m<MATRIX_SIZE; m++) {
    	struct MatrixRowParams* mtx = &((struct MatrixRowParams*)&params->matrixRowState1)[m];
    	struct PFM1MatrixRowParams* pfm1mtx = &((struct PFM1MatrixRowParams*)&pfm1params.matrixRowState1)[m];

    	mtx->mul = getValueForFreq(pfm1mtx);


    	switch (pfm1mtx->source) {
    	case PFM1_MATRIX_SOURCE_LFO1:
    		mtx->source = MATRIX_SOURCE_LFO1;
    		break;
    	case PFM1_MATRIX_SOURCE_LFO2:
    		mtx->source = MATRIX_SOURCE_LFO2;
    		break;
    	case PFM1_MATRIX_SOURCE_LFO3:
    		mtx->source = MATRIX_SOURCE_LFO3;
    		break;
    	case PFM1_MATRIX_SOURCE_LFO4:
    		mtx->source = MATRIX_SOURCE_LFOENV1;
    		break;
    	case PFM1_MATRIX_SOURCE_PITCHBEND:
    		mtx->source = MATRIX_SOURCE_PITCHBEND;
    		break;
    	case PFM1_MATRIX_SOURCE_AFTERTOUCH:
    		mtx->source = MATRIX_SOURCE_AFTERTOUCH;
    		break;
    	case PFM1_MATRIX_SOURCE_MODWHEEL:
    		mtx->source = MATRIX_SOURCE_MODWHEEL;
    		break;
    	case PFM1_MATRIX_SOURCE_VELOCITY:
    		mtx->source = MATRIX_SOURCE_VELOCITY;
    		break;
    	case PFM1_MATRIX_SOURCE_LFO5:
    		mtx->source = MATRIX_SOURCE_LFOSEQ1;
    		break;
    	case PFM1_MATRIX_SOURCE_LFO6:
    		mtx->source = MATRIX_SOURCE_LFOSEQ2;
    		break;
    	case PFM1_MATRIX_SOURCE_KEY:
    		mtx->source = MATRIX_SOURCE_KEY;
    		break;
    	default:
    		mtx->source = MATRIX_SOURCE_NONE;
    		break;
    	}

    	switch (pfm1mtx->destination) {
    	case PFM1_OSC1_FREQ:
    		mtx->destination = OSC1_FREQ;
    		break;
    	case PFM1_OSC2_FREQ:
    		mtx->destination = OSC2_FREQ;
    		break;
    	case PFM1_OSC3_FREQ:
    		mtx->destination = OSC3_FREQ;
    		break;
    	case PFM1_OSC4_FREQ:
    		mtx->destination = OSC4_FREQ;
    		break;
    	case PFM1_OSC5_FREQ:
    		mtx->destination = OSC5_FREQ;
    		break;
    	case PFM1_OSC6_FREQ:
    		mtx->destination = OSC6_FREQ;
    		break;
    	case PFM1_ALL_OSC_FREQ:
    		mtx->destination = ALL_OSC_FREQ;
    		break;
    	case PFM1_INDEX_MODULATION1:
    		mtx->destination = INDEX_MODULATION1;
    		break;
    	case PFM1_INDEX_MODULATION2:
    		mtx->destination = INDEX_MODULATION2;
    		break;
    	case PFM1_INDEX_MODULATION3:
    		mtx->destination = INDEX_MODULATION3;
    		break;
    	case PFM1_INDEX_MODULATION4:
    		mtx->destination = INDEX_MODULATION4;
    		break;
    	case PFM1_MIX_OSC1:
    		mtx->destination = MIX_OSC1;
    		break;
    	case PFM1_MIX_OSC2:
    		mtx->destination = MIX_OSC2;
    		break;
    	case PFM1_MIX_OSC3:
    		mtx->destination = MIX_OSC3;
    		break;
    	case PFM1_MIX_OSC4:
    		mtx->destination = MIX_OSC4;
    		break;
    	case PFM1_LFO1_FREQ:
    		mtx->destination = LFO1_FREQ;
    		break;
    	case PFM1_LFO2_FREQ:
    		mtx->destination = LFO2_FREQ;
    		break;
    	case PFM1_LFO3_FREQ:
    		mtx->destination = LFO3_FREQ;
    		break;
    	case PFM1_MTX1_MUL:
    		mtx->destination = MTX1_MUL;
    		break;
    	case PFM1_MTX2_MUL:
    		mtx->destination = MTX2_MUL;
    		break;
    	case PFM1_MTX3_MUL:
    		mtx->destination = MTX3_MUL;
    		break;
    	case PFM1_MTX4_MUL:
    		mtx->destination = MTX4_MUL;
    		break;
    	case PFM1_LFO5_GATE:
    		mtx->destination = LFOSEQ1_GATE;
    		break;
    	case PFM1_LFO6_GATE:
    		mtx->destination = LFOSEQ2_GATE;
    		break;
    	case PFM1_MAIN_GATE:
    		mtx->destination = MAIN_GATE;
    		break;
    	case PFM1_ENV1_ATTACK:
    		mtx->destination = ENV1_ATTACK;
    		break;
    	case PFM1_ENV2_ATTACK:
    		mtx->destination = ENV2_ATTACK;
    		break;
    	case PFM1_ENV3_ATTACK:
    		mtx->destination = ENV3_ATTACK;
    		break;
    	case PFM1_ENV4_ATTACK:
    		mtx->destination = ENV4_ATTACK;
    		break;
    	case PFM1_ENV5_ATTACK:
    		mtx->destination = ENV5_ATTACK;
    		break;
    	case PFM1_ENV6_ATTACK:
    		mtx->destination = ENV6_ATTACK;
    		break;
    	case PFM1_ALL_ENV_ATTACK:
    		mtx->destination = ALL_ENV_ATTACK;
    		break;
    	default:
    		mtx->destination = DESTINATION_NONE;
    		break;
    	}
    }


    for (int l=0; l<3; l++) {
    	struct LfoParams* lfo = &((struct LfoParams*)&params->lfoOsc1)[l];
    	struct PFM1LfoParams* pfm1lfo = &((struct PFM1LfoParams*)&pfm1params.lfo1)[l];
    	switch (pfm1lfo->shape) {
    	case PFM1_LFO_SAW:
    		lfo->shape = LFO_SAW;
    		break;
    	case PFM1_LFO_RAMP:
    		lfo->shape = LFO_RAMP;
    		break;
    	case PFM1_LFO_SQUARE:
    		lfo->shape = LFO_SQUARE;
    		break;
    	case PFM1_LFO_RANDOM:
    		lfo->shape = LFO_RANDOM;
    		break;
    	case PFM1_LFO_SIN:
    		lfo->shape = LFO_SIN;
    		break;
    	}
    	if (pfm1lfo->freq >= 247) {
    		lfo->freq = (pfm1lfo->freq - 6.0f) / 10.0f;
    	} else {
    		// * 2.0f because preenFM1 is buggy...
    		lfo->freq = pfm1lfo->freq / 16.0f * 2.0f;
    	}

    	lfo->bias = pfm1lfo->bias / 127.0f;
    	lfo->keybRamp = pfm1lfo->keybRamp / 64.0f;
    }

    params->lfoEnv1.attack = pfm1params.lfo4.attack * pfm1params.lfo4.attack / 32768.0f;
    params->lfoEnv1.decay = pfm1params.lfo4.decay * pfm1params.lfo4.decay / 32768.0f;
    params->lfoEnv1.sustain = pfm1params.lfo4.sustain / 255.0f;
    params->lfoEnv1.release = pfm1params.lfo4.release * pfm1params.lfo4.release / 32768.0f;

    for (int s=0; s<2; s++) {
    	struct StepSequencerParams* stepSeq = &((struct StepSequencerParams*)&params->lfoSeq1)[s];
    	struct PFM1StepSequencerParams* pfm1StepSeq = &((struct PFM1StepSequencerParams*)&pfm1params.lfo5)[s];

    	stepSeq->gate = pfm1StepSeq->gate / 32.0f;
    	stepSeq->bpm = pfm1StepSeq->bpm;

    	struct StepSequencerSteps* steps = &((struct StepSequencerSteps*)&params->lfoSteps1)[s];
    	struct PFM1StepSequencerSteps* pfm1Steps = &((struct PFM1StepSequencerSteps*)&pfm1params.steps5)[s];

    	for (int step=0; step<16; step++) {
    		steps->steps[step] = pfm1Steps->steps[step];
    	}
    }

    for (int n=0; n<13; n++) {
    	params->presetName[n] = pfm1params.presetName[n];
    }

}




int PresetUtil::readSysexPatchPFM1(unsigned char* params) {
    int checkSum = 0;
    unsigned int index = 0;
    int value = 0;

    while (index < PFM1_PATCH_SIZE) {

        int byte = PresetUtil::getNextMidiByte();
        if (byte < 0) {
            return -index - 1000;
        }
        value += byte;


        if (byte < 127) {
            params[index] = value;
            index++;
            checkSum += value;
            value = 0;
        }
    }

    int sentChecksum = PresetUtil::getNextMidiByte();

    if (sentChecksum <0) {
        return -198;
    } else {
        checkSum = checkSum % 128;

        if (checkSum != sentChecksum) {
            lcd.setCursor(11,3);
            lcd.print("*");
            lcd.print(checkSum);
            lcd.print("/");
            lcd.print(sentChecksum);
            while(1);
            return -200 - sentChecksum;
        }
    }
    return sentChecksum;
}

int PresetUtil::readSysexBankPFM1() {
    int errorCode = 0;

    lcd.setCursor(1,3);
    lcd.print("PFM1:");

    for (int preset = 0; preset<128 && errorCode>=0; preset++) {

    	lcd.setCursor(7,3);
        lcd.print(preset);

        if ((errorCode = PresetUtil::readSysexPatchPFM1(sysexTmpMem + PFM1_PATCH_SIZE * preset)) < 0) {
            lcd.setCursor(11,3);
            lcd.print("##");
            lcd.print(errorCode);
            errorCode = -500 - preset;
            break;
        }
    }
    // PREENFM1
    sysexTmpMem[8 + PATCH_SIZE_PFM2 * 128 ] = '1';

    return errorCode;
}


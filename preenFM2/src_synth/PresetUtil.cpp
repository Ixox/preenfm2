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

extern RingBuffer<uint8_t, 200> usartBuffer;


#define NULL 0
extern LiquidCrystal lcd;
extern const struct MidiConfig midiConfig[];


SynthState * PresetUtil::synthState;
Storage * PresetUtil::storage;


uint8_t sysexTmpMem[PATCH_SIZE_PFM2 * 128];
struct OneSynthParams oneSynthParamsTmp;

uint8_t usbBuffer[4];
uint8_t sysexBuffer[3];
int sysexIndex = 0;

#define SYSEX_BYTE_PATCH 3
#define SYSEX_BYTE_BANK 4

enum {
	SYSEX_UNKNOWN = 0,
    SYSEX_PREENFM_PATCH,
    SYSEX_PREENFM_BANK,
    SYSEX_DX7_BANK
};
OneSynthParams synthParamsEmpty  =  {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO9, 7, 8, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SIN, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SIN, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SIN_POS, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0, 1, 0, 1 },
                { 0, 1, 2.56 , 0},

                { .5, 1, .1, .8 },
                { 0, .8, 4, 0},

                { .1, 1, .5, .6 },
                { 0, .6, 4, 0},

                { .1, 1, .1, .56 },
                { .1, 1, .1, .56 },

                { 0, 1, .6, 0},
                { 0, 0, 0.66, 0},

                { .1, 1, .5, .6},
                { 0, .5, 0.3, 0},
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX4_MUL, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION1, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFO2, 0, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFO3, 0, PAN_OSC1, 0} ,
                { MATRIX_SOURCE_NONE, 0, DESTINATION_NONE, 0} ,
                // LFOs
                { LFO_SIN, 4.5, 0, 0} ,
                { LFO_SIN, 4.8, 0, 1.0} ,
                { LFO_SIN, 6.0, 0, 4.0} ,
                { .2, 0, 1.0, 1.0} ,
                { 1, .2, .2, 1.0} ,
                { 110, .5,  0, 0}  ,
                { 140, .6, 0, 0},
                {{ 0,7,15,15,15,0,15,15,  0,15,15,15,0,15,15,15}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "Preen mk2"
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


#ifdef DEBUG

const char* engineEnums [] = { "ALGO1", "ALGO2", "ALGO3", "ALGO4", "ALGO5", "ALGO6", "ALGO7", "ALGO8", "ALGO9" };
const char* oscTypeEnums [] = { "OSC_FT_KEYBOARD ", "OSC_FT_FIXE" };
const char* mixOscShapeEnums [] = { "OSC_SHAPE_SIN", "OSC_SHAPE_SIN2", "OSC_SHAPE_SIN3", "OSC_SHAPE_SIN4", "OSC_SHAPE_RAND", "OSC_SHAPE_SQUARE", "OSC_SHAPE_SAW", "OSC_SHAPE_OFF" };
const char* lfoShapeEnums [] = { "LFO_SAW", "LFO_RAMP", "LFO_SQUARE", "LFO_RANDOM", "LFO_TYPE_MAX" };
const char* matrixSourceEnums [] = { "MATRIX_SOURCE_NONE", "MATRIX_SOURCE_LFO1", "MATRIX_SOURCE_LFO2", "MATRIX_SOURCE_LFO3", "MATRIX_SOURCE_LFO4", "MATRIX_SOURCE_PITCHBEND", "MATRIX_SOURCE_AFTERTOUCH", "MATRIX_SOURCE_MODWHEEL", "MATRIX_SOURCE_VELOCITY", "MATRIX_SOURCE_CC1", "MATRIX_SOURCE_CC2", "MATRIX_SOURCE_CC3", "MATRIX_SOURCE_CC4", "MATRIX_SOURCE_LFO5", "MATRIX_SOURCE_LFO6","MATRIX_SOURCE_MAX" };
const char* matrixDestEnums [] = { "DESTINATION_NONE", "OSC1_FREQ", "OSC2_FREQ", "OSC3_FREQ", "OSC4_FREQ", "OSC5_FREQ", "OSC6_FREQ", "INDEX_MODULATION1", "INDEX_MODULATION2", "INDEX_MODULATION3", "INDEX_MODULATION4", "MIX_OSC1", "MIX_OSC2", "MIX_OSC3", "MIX_OSC4", "LFO1_FREQ", "LFO2_FREQ", "LFO3_FREQ", "LFO4_FREQ", "MTX1_MUL", "MTX2_MUL", "MTX3_MUL", "MTX4_MUL", "MTX5_MUL", "MTX6_MUL", "MTX7_MUL", "MTX8_MUL", "MTX9_MUL", "MTX10_MUL", "MTX11_MUL", "MTX12_MUL", "ALL_OSC_FREQ", "LFO5_GATE", "LFO6_GATE","DESTINATION_MAX" };


void PresetUtil::dumpLine(const char *enums1[], int a, const char *enums2[], int b, const char *enums3[], int c, const char *enums4[], int d) {

    SerialUSB.print("{ ");
    if (enums1 == NULL) {
        SerialUSB.print(a);
    } else {
        SerialUSB.print(enums1[a]);
    }
    SerialUSB.print(", ");
    if (enums2 == NULL) {
        SerialUSB.print(b);
    } else {
        SerialUSB.print(enums2[b]);
    }
    SerialUSB.print(", ");
    if (enums3 == NULL) {
        SerialUSB.print(c);
    } else {
        SerialUSB.print(enums3[c]);
    }
    SerialUSB.print(", ");
    if (enums4 == NULL) {
        SerialUSB.print(d);
    } else {
        SerialUSB.print(enums4[d]);
    }
    SerialUSB.print("} ");
    SerialUSB.println(", ");
}

void PresetUtil::dumpPatch() {
    SerialUSB.print("// patch name : '");
    SerialUSB.print(PresetUtil::synthState->params.presetName);
    SerialUSB.println("'");
    SerialUSB.println("// Engine ");
    dumpLine(engineEnums,
            PresetUtil::synthState->params.engine1.algo,
            NULL,
            PresetUtil::synthState->params.engine1.velocity,
            NULL,
            PresetUtil::synthState->params.engine1.numberOfVoice,
            NULL,
            PresetUtil::synthState->params.engine1.glide);
    dumpLine(NULL,
            PresetUtil::synthState->params.engine2.modulationIndex1,
            NULL,
            PresetUtil::synthState->params.engine2.modulationIndex2,
            NULL,
            PresetUtil::synthState->params.engine2.modulationIndex3,
            NULL,
            PresetUtil::synthState->params.engine2.modulationIndex4);
    dumpLine(NULL,
            PresetUtil::synthState->params.engine3.mixOsc1,
            NULL,
            PresetUtil::synthState->params.engine3.mixOsc2,
            NULL,
            PresetUtil::synthState->params.engine3.mixOsc3,
            NULL,
            PresetUtil::synthState->params.engine3.mixOsc4);
    SerialUSB.println("// Oscillator");
    OscillatorParams * o =
            (OscillatorParams *) (&(PresetUtil::synthState->params.osc1));
    for (int k = 0; k < 6; k++) {
        dumpLine(mixOscShapeEnums,
                o[k].shape,
                oscTypeEnums,
                o[k].frequencyType,
                NULL,
                o[k].frequencyMul,
                NULL,
                o[k].detune);
    }
    SerialUSB.println("// Enveloppe");
    EnvelopeParams * e =
            (EnvelopeParams*) (&(PresetUtil::synthState->params.env1));
    for (int k = 0; k < 6; k++) {
        dumpLine(
                NULL,
                e[k].attack,
                NULL,
                e[k].decay,
                NULL,
                e[k].sustain,
                NULL,
                e[k].release);
    }
    SerialUSB.println("// Modulation matrix");
    MatrixRowParams	* m = (MatrixRowParams*) (&(PresetUtil::synthState->params.matrixRowState1));
    for (int k = 0; k < 12; k++) {
        dumpLine(
                matrixSourceEnums,
                m[k].source,
                NULL,
                m[k].mul,
                matrixDestEnums,
                m[k].destination,
                NULL,
                0);
    }
    SerialUSB.println("// LFOs");
    LfoParams* l = (LfoParams*) (&(PresetUtil::synthState->params.lfoOsc1));
    for (int k = 0; k < 3; k++) {
        dumpLine(
                lfoShapeEnums,
                l[k].shape,
                NULL,
                l[k].freq,
                NULL,
                l[k].bias,
                NULL,
                l[k].keybRamp);
    }
    EnvelopeParams* le = (EnvelopeParams*) (&(PresetUtil::synthState->params.lfo4));
    dumpLine(
            NULL,
            le[0].attack,
            NULL,
            le[0].decay,
            NULL,
            le[0].sustain,
            NULL,
            le[0].release
    );
    StepSequencerParams* ls = (StepSequencerParams*) (&(PresetUtil::synthState->params.lfoSeq1));
    for (int k = 0; k < 2; k++) {
        dumpLine(
                NULL,
                ls[k].bpm,
                NULL,
                ls[k].gate,
                NULL,
                0,
                NULL,
                0
        );
    }
    StepSequencerSteps* step = (StepSequencerSteps*) (&(PresetUtil::synthState->params.steps5));
    for (int k = 0; k < 2; k++) {
        SerialUSB.print("{ { ");
        for (int j = 0; j < 16; j++) {
            SerialUSB.print((int)step[k].steps[j]);
            if (j != 15) {
                SerialUSB.print(", ");
            }
        }
        SerialUSB.print("} } ");
        SerialUSB.println(", ");
    }

    SerialUSB.print("\"");
    SerialUSB.print(PresetUtil::synthState->params.presetName);
    SerialUSB.println("\"");
}

#endif







void PresetUtil::sendSysexByte(uint8_t byte) {
	if (sysexIndex > 2) {
	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[0];

	    if (synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT
	    		&& usbOTGDevice.dev.device_status == USB_OTG_CONFIGURED) {
	        // usbBuf[0] = [number of cable on 4 bits] [event type on 4 bites]
	    	// 0x4 SysEx starts or continues
	        usbBuffer[0] = 0x00  | 0x4;
	        usbBuffer[1] = sysexBuffer[0];
	        usbBuffer[2] = sysexBuffer[1];
	        usbBuffer[3] = sysexBuffer[2];

	        DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuffer, 4);
	    }

	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[1];
	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[2];

	    sysexIndex = 0;
	}
	sysexBuffer[sysexIndex] = byte;
	sysexIndex ++;
}

void PresetUtil::sendSysexFinished() {
	bool usbMidi = false;

	if (synthState->fullState.midiConfigValue[MIDICONFIG_USB] == USBMIDI_IN_AND_OUT
    		&& usbOTGDevice.dev.device_status == USB_OTG_CONFIGURED) {
    	usbMidi = true;
    }

	if (sysexIndex == 1) {

	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[0];

	    if (usbMidi) {
	    	// 0x5 SysEx ends with 1 byte
	        usbBuffer[0] = 0x00  | 0x5;
	        usbBuffer[1] = sysexBuffer[0];
	        usbBuffer[2] = 0;
	        usbBuffer[3] = 0;

	        DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuffer, 4);
	    }

	} else if (sysexIndex == 2) {

		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[0];

	    if (usbMidi) {
	    	// 0x6 SysEx ends with 2 bytes
	        usbBuffer[0] = 0x00  | 0x6;
	        usbBuffer[1] = sysexBuffer[0];
	        usbBuffer[2] = sysexBuffer[1];
	        usbBuffer[3] = 0;

	        DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuffer, 4);
	    }
	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[1];

	} else if (sysexIndex == 3) {

		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[0];

	    if (usbMidi) {
	    	// 0x7 SysEx ends with 3 bytes
	        usbBuffer[0] = 0x00  | 0x7;
	        usbBuffer[1] = sysexBuffer[0];
	        usbBuffer[2] = sysexBuffer[1];
	        usbBuffer[3] = sysexBuffer[2];

	        DCD_EP_Tx(&usbOTGDevice, 0x81, usbBuffer, 4);
	    }
	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[1];

	    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {}
	    USART3->DR = (uint16_t)sysexBuffer[2];
	}
}

void PresetUtil::sendCurrentPatchToSysex() {
    uint8_t newPatch[] = { 0xf0, 0x7d, SYSEX_BYTE_PATCH };
    for (int k = 0; k <= 2; k++) {
        sendSysexByte(newPatch[k]);
    }

    PresetUtil::convertSynthStateToCharArray(PresetUtil::synthState->params, sysexTmpMem);
    PresetUtil::sendParamsToSysex(sysexTmpMem);

    sendSysexByte(0xf7);
    sendSysexFinished();
}


void PresetUtil::sendBankToSysex(int bankNumber) {
    unsigned char newPatch[] = { 0xf0, 0x7d, SYSEX_BYTE_BANK };
    for (int k = 0; k <= 2; k++) {
        sendSysexByte(newPatch[k]);
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
    	sendSysexByte(bankNameToSend[k]);
    }

    for (int preset = 0; preset < 128; preset++) {
        lcd.setCursor(3,2);
        lcd.print(preset);
        lcd.print(" / 128");
        storage->loadPreenFMPatch(bank, preset, &oneSynthParamsTmp);
        PresetUtil::convertSynthStateToCharArray(&oneSynthParamsTmp, sysexTmpMem);
        PresetUtil::sendParamsToSysex(sysexTmpMem);
    }

    sendSysexByte((unsigned char) 0xf7);
    sendSysexFinished();
}




void PresetUtil::sendNrpn(struct MidiEvent cc) {
    sendSysexByte((unsigned char) (cc.eventType + cc.channel));
    sendSysexByte((unsigned char) cc.value[0]);
    sendSysexByte((unsigned char) cc.value[1]);
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
        sendSysexByte(byte);
    }

    sendSysexByte((unsigned char) (checksum % 128));
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

void PresetUtil::resetConfigAndSaveToEEPROM() {
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_USB] = 0;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL1] = 1;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL2] = 2;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL3] = 3;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_CHANNEL4] = 4;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_THROUGH] = 0;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_RECEIVES] = 3;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_SENDS] = 1;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_REALTIME_SYSEX] = 0;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_TEST_NOTE] = 60;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_TEST_VELOCITY] = 120;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_ENCODER] = 0;
    PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_OP_OPTION] = 0;
}



int PresetUtil::getNextMidiByte() {
    int timeout = 0;
    while (usartBuffer.getCount() == 0) {
        if (timeout++ >= 50000000) {
            return -1;
        }
    }
    unsigned char byte = usartBuffer.remove();

    /* TODO : to put back...
    if (PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_THROUGH] == 1) {
        sendSysexByte((unsigned char) byte);
    }
    */

    return (int)byte;

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
            if (byte == SYSEX_BYTE_PATCH) {
            	sysexType = SYSEX_PREENFM_PATCH;
            } else if (byte == SYSEX_BYTE_BANK) {
            	sysexType = SYSEX_PREENFM_BANK;
            } else {
                bError = true;
                break;
            }
            if (((sysexType == SYSEX_PREENFM_PATCH) && !patchAllowed) || ((sysexType == SYSEX_PREENFM_BANK) && !bankAllowed)) {
                // Will wait untill F7 is received (end of sysex)
                bSysexRead = true;
                bError = true;
            }
        }

        if (index >0 && !bSysexRead) {
            bSysexRead = true;
            if (sysexType == SYSEX_PREENFM_PATCH) {
                int errorCode = PresetUtil::readSysexPatch(sysexTmpMem);
                lcd.setCursor(2,3);
                lcd.print("Patch Err ");
                lcd.print(errorCode);

                if (errorCode < 0) {
                    index = -errorCode;
                    bError = true;
                } else {
                    PresetUtil::synthState->propagateBeforeNewParamsLoad();
                    PresetUtil::convertCharArrayToSynthState(sysexTmpMem, PresetUtil::synthState->params);
                    PresetUtil::synthState->propagateAfterNewParamsLoad();
                    PresetUtil::synthState->resetDisplay();
                }
            } else {
                // Bank !!
                int errorCode = 0;
                if ((errorCode = PresetUtil::readSysexBank()) <0) {
                    index = -errorCode;
                    bError = true;
                }
            }
        }
    }

    return bError ? -index : ((sysexType == SYSEX_PREENFM_PATCH) ? 1 : 2);
}


void PresetUtil::copySynthParams(char* source, char* dest) {
    for (int k=0; k<sizeof(struct OneSynthParams); k++) {
        dest[k] = source[k];
    }
}


int PresetUtil::readSysexBank() {
    int errorCode = 0;

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

    return errorCode;
}






// ABSTRACTION OF MEMORY/SYSEX MANAGEMENT FOR FUTUR COMPATIBILITY

uint16_t PresetUtil::getShortFromParamFloat(int row, int encoder, float value) {
    // performance do not send NRPN
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    uint16_t rValue;
    uint16_t valueToSend = 0;

    if (row != ROW_MODULATION1 && row != ROW_MODULATION2) {
        valueToSend = (param->numberOfValues - 1.0f) * (value - param->minValue) / (param->maxValue - param->minValue) + .1f ;
    } else {
        valueToSend = value * 100.0f + .1f ;
    }

    return valueToSend;
}

float PresetUtil::getParamFloatFromShort(int row, int encoder, short value) {
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    if (row != ROW_MODULATION1 && row != ROW_MODULATION2) {
		return param->minValue + value / (param->numberOfValues - 1.0f) * (param->maxValue - param->minValue);
	} else {
		return value * .01f;
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


void PresetUtil::copyBank(int source, int dest) {
	if (source == 4) {
		lcd.setCursor(1,3);
		lcd.print("Save:");
		for (int preset=0; preset<128; preset++) {
			lcd.setCursor(7,3);
			lcd.print(preset);
			PresetUtil::convertCharArrayToSynthState(sysexTmpMem + PATCH_SIZE_PFM2 * preset, &oneSynthParamsTmp);
			storage->savePatch(dest, preset, &oneSynthParamsTmp);
		}
	}  else {
		// CAN ONLY COPY FROM sysexTmpMem for the moment
	}
}




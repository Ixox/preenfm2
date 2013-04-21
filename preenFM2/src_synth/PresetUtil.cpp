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

#include "PresetUtil.h"
#include "SynthState.h"
#include "LiquidCrystal.h"
#include "ff.h"
#include "usb_core.h"
#include "usbh_core.h"

#define NULL 0
extern LiquidCrystal lcd;
extern const struct MidiConfig midiConfig[];


char PresetUtil::readName[13];
SynthState * PresetUtil::synthState;

int PresetUtil::midiBufferWriteIndex;
int PresetUtil::midiBufferReadIndex;
unsigned char PresetUtil::midiBuffer[1024];

OneSynthParams synthParamsEmpty  =  {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO9, 7, 8, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 0, 0, 0, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SIN, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SIN, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SIN4, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0,0,1,2.56} ,
                { .5,.5,1,4} ,
                { .1,.5,.6,4} ,
                { .1,.1,.56,2.66} ,
                { 0,.6,0,0.6} ,
                { .1,.5,.6,.3} ,
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX5_MUL, 0} ,
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
    midiBufferWriteIndex = 0;
    midiBufferReadIndex = 0;
    for (int k=0; k<13; k==0) {
        readName[k] = 0;
    }
}

PresetUtil::~PresetUtil() {
}



void PresetUtil::setSynthState(SynthState* synthState) {
    // init
    PresetUtil::synthState = synthState;
    // enable the i2c bus
    // i2c_master_enable(I2C1, I2C_FAST_MODE & I2C_BUS_RESET);
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





void PresetUtil::resetConfigAndSaveToEEPROM() {
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

//    saveConfigToEEPROM();
}


void PresetUtil::createEmptyBanks() {
    for (int bankNumber = 0; bankNumber < 4; bankNumber++) {
        lcd.setCursor(3,2);
        lcd.print("Format ");
        lcd.print((char)('A'+bankNumber));
        for (int preset = 0; preset < 128; preset++) {

            lcd.setCursor(12,2);
            lcd.print(preset);
            lcd.print("  ");

            // PresetUtil::savePatchToEEPROM(&synthParamsEmpty, bankNumber, preset);
        }
    }
}

void PresetUtil::sendBankToSysex(int bankNumber) {
    /*
    unsigned char paramChars[PATCH_SIZE];

    unsigned char newPatch[] = { 0xf0, 0x7d, 0x02 };
    for (int k = 0; k <= 2; k++) {
        Serial3.print(newPatch[k]);
    }

    for (int preset = 0; preset < 128; preset++) {
        lcd.setCursor(3,2);
        lcd.print(preset);
        lcd.print(" / 128");
        PresetUtil::readCharsFromEEPROM(bankNumber, preset, paramChars);
        PresetUtil::sendParamsToSysex(paramChars);
    }

    Serial3.print((unsigned char) 0xf7);
*/
}


void PresetUtil::checkReadEEPROM() {
}


void PresetUtil::sendCurrentPatchToSysex() {
    /*
    unsigned char paramChars[PATCH_SIZE];
    unsigned char newPatch[] = { 0xf0, 0x7d, 0x01 };
    for (int k = 0; k <= 2; k++) {
        Serial3.print(newPatch[k]);
    }

    PresetUtil::convertSynthStateToCharArray(&PresetUtil::synthState->params, paramChars);
    PresetUtil::sendParamsToSysex(paramChars);

    Serial3.print((unsigned char) 0xf7);
    */
}


void PresetUtil::sendNrpn(struct MidiEvent cc) {
    /*
    Serial3.print((unsigned char) (cc.eventType + cc.channel));
    Serial3.print((unsigned char) cc.value[0]);
    Serial3.print((unsigned char) cc.value[1]);
    */
}

void PresetUtil::sendCurrentPatchAsNrpns(int timbre) {

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
        if (currentrow == ROW_PERFORMANCE) {
            continue;
        }

        for (int encoder = 0; encoder < NUMBER_OF_ENCODERS; encoder++) {

            struct ParameterDisplay param = allParameterRows.row[currentrow]->params[encoder];
            int newValue = ((float*)&PresetUtil::synthState->params)[currentrow*NUMBER_OF_ENCODERS+encoder];
            int valueToSend = newValue - param.minValue;
            int paramNumber;
            if (currentrow > ROW_PERFORMANCE) {
                paramNumber = (currentrow-1) * NUMBER_OF_ENCODERS+ encoder;
            } else {
                paramNumber = currentrow * NUMBER_OF_ENCODERS+ encoder;
            }
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
}


void PresetUtil::sendParamsToSysex(unsigned char* params) {
    /*
    int checksum = 0;

    for (int k = 0; k < PATCH_SIZE; k++) {
        unsigned char byte = params[k];
        checksum += byte;
        while (byte >= 127) {
            Serial3.print((unsigned char) 127);
            byte -= 127;
        }
        Serial3.print(byte);
    }

    Serial3.print((unsigned char) (checksum % 128));
    */
}

int PresetUtil::readSysexPatch(unsigned char* params) {
    int checkSum = 0;
    unsigned int index = 0;
    int value = 0;

    while (index < PATCH_SIZE) {

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
            return -200 - sentChecksum;
        }
    }
    return sentChecksum;
}


int PresetUtil::fillBufferWithNextMidiByte() {
    /*
    if (midiBufferWriteIndex == 1024) {
        midiBufferWriteIndex = 0;
    }
    while (Serial3.available()) {
        midiBuffer[midiBufferWriteIndex++] = Serial3.read();
    }
    */
}

int PresetUtil::getNextMidiByte() {
    /*
    int timeout = 0;
    if (midiBufferReadIndex != midiBufferWriteIndex) {
        if (midiBufferReadIndex == 1024) {
            midiBufferReadIndex = 0;
        }
        return midiBuffer[midiBufferReadIndex++];
    }
    while (!Serial3.available()) {
        if (timeout++ >= 1000000) {
            return -1;
        }
    }
    unsigned char byte = Serial3.read();
    if (PresetUtil::synthState->fullState.midiConfigValue[MIDICONFIG_THROUGH] == 1) {
        Serial3.print((unsigned char) byte);
    }

    return (int)byte;
    */
    return 0;
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
    unsigned char paramChars[PATCH_SIZE];

    bool isPatch = true;
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
            if (byte == 1) {
                isPatch = true;
            } else if (byte == 2) {
                isPatch = false;
            } else {
                bError = true;
                break;
            }
            if ((isPatch && !patchAllowed) || (!isPatch && !bankAllowed)) {
                // Will wait untill F7 is received (end of sysex)
                bSysexRead = true;
                bError = true;
            }
        }

        if (index >0 && !bSysexRead) {
            bSysexRead = true;
            if (isPatch) {
                int errorCode = 0;
                if ((errorCode = PresetUtil::readSysexPatch(paramChars)) <0) {
                    index = -errorCode;
                    bError = true;
                } else {
                    PresetUtil::synthState->propagateBeforeNewParamsLoad();
                    PresetUtil::convertCharArrayToSynthState(paramChars, PresetUtil::synthState->params);
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

    return bError ? -index : (isPatch ? 1 : 2);
}


void PresetUtil::copySynthParams(char* source, char* dest) {
    for (int k=0; k<sizeof(struct OneSynthParams); k++) {
        dest[k] = source[k];
    }
}


int PresetUtil::readSysexBank() {
    unsigned char paramChars[PATCH_SIZE];
    int errorCode = 0;

    lcd.setCursor(1,3);
    lcd.print("Bank:");
    midiBufferWriteIndex = 0;
    midiBufferReadIndex = 0;

    for (int preset = 0; preset<128 && errorCode>=0; preset++) {
        lcd.setCursor(7,3);
        lcd.print(preset);

        if ((errorCode = PresetUtil::readSysexPatch(paramChars)) <0) {
            lcd.setCursor(11,3);
            lcd.print("##");
            lcd.print(errorCode);
            errorCode = -500 - preset;
            break;
        }

        if (midiBufferWriteIndex == midiBufferReadIndex) {
            midiBufferWriteIndex = 0;
            midiBufferReadIndex = 0;
        }
        // PresetUtil::saveCharParamsToEEPROM(paramChars, 4, preset, true);
    }

    return errorCode;
}






// ABSTRACTION OF MEMORY/SYSEX MANAGEMENT FOR FUTUR COMPATIBILITY

void PresetUtil::convertSynthStateToCharArray(OneSynthParams* params, unsigned char* chars) {
    // Clean
    for (unsigned int k=0; k<PATCH_SIZE; k++) {
        chars[k] = 0;
    }

    // 2.00 compatibility performance mode which is not saved
    unsigned int firstPartASize = 3*4;
    for (unsigned int k=0; k<firstPartASize; k++) {
        chars[k] = ((char*) params)[k];
    }
    unsigned  int firstPartBSize = 20*4;
    for (unsigned int k=0; k<firstPartBSize; k++) {
        chars[firstPartASize + k] = ((char*) &params->osc1)[k];
    }
    int firstPartSize = firstPartASize + firstPartBSize;
    // LFO 1->4
    unsigned int secondPartSize = 4*4;
    for (unsigned int k=0; k<secondPartSize; k++) {
        chars[firstPartSize + k] = ((char*) &params->lfoOsc1)[k];
    }

    // Then the title
    for (unsigned int k=0; k<12; k++) {
        chars[27*4 + k] = params->presetName[k];
    }

    // 120 - 128 : step seqs
    for (unsigned int k=0; k<8; k++) {
        chars[120 + k] = ((char*) &params->lfoSeq1)[k];
    }

    // 128 : 16 step of seq 1 on 8 bits
    // 4 left bits + 4 right bits
    for (unsigned int k=0; k<8; k++) {
        chars[128+k] = (params->lfoSteps1.steps[k*2]<<4) + params->lfoSteps1.steps[k*2+1] ;
    }
    // 136 : 16 step of seq 2 on 8 bits
    for (unsigned int k=0; k<8; k++) {
        chars[136+k] = (params->lfoSteps2.steps[k*2]<<4) + params->lfoSteps2.steps[k*2+1] ;
    }

    // 144 : Matrix 9->12
    for (unsigned int k=0; k<16; k++) {
        chars[144 + k] = ((char*) &params->matrixRowState9)[k];
    }

}

void PresetUtil::convertCharArrayToSynthState(unsigned char* chars, OneSynthParams* params) {
    // In 2.0...
    // performance mode added... CC1..4 must not be saved

    // Copy first part A
    unsigned int firstPartASize = 3*4;
    for (unsigned int k=0; k<firstPartASize; k++) {
        ((char*) params)[k] = chars[k];
    }
    // performance
    for (unsigned int k=0; k<4; k++) {
        ((char*) &params->engine4)[k] = 0;
    }
    // Copy first part B
    unsigned  int firstPartBSize = 20*4;
    for (unsigned int k=0; k<firstPartBSize; k++) {
        ((char*) &params->osc1)[k] = chars[firstPartASize + k];
    }
    int firstPartSize = firstPartASize + firstPartBSize;
    // LFO 1->4
    unsigned int secondPartSize = 4*4;
    for (unsigned int k=0; k<secondPartSize; k++) {
        ((char*) &params->lfoOsc1)[k] = chars[firstPartSize + k];
    }

    // Title
    for (unsigned int k=0; k<12; k++) {
        params->presetName[k] = chars[27*4+k];
    }
    params->presetName[12] = '\0';


    // 120 - 128 : step seqs
    for (unsigned int k=0; k<8; k++) {
        ((char*) &params->lfoSeq1)[k] = chars[120 + k];
    }

    // 128 : 16 step of seq 1 on 8 bits
    // 4 left bits + 4 right bits
    for (unsigned int k=0; k<8; k++) {
        params->lfoSteps1.steps[k*2]   = chars[128+k] >> 4;
        params->lfoSteps1.steps[k*2+1] = chars[128+k] & 0xf;
    }
    // 136 : 16 step of seq 2 on 8 bits
    for (unsigned int k=0; k<8; k++) {
        params->lfoSteps2.steps[k*2]   = chars[136+k] >> 4;
        params->lfoSteps2.steps[k*2+1] = chars[136+k] & 0xf;
    }
    // 144 : Matrix 9->12
    for (unsigned int k=0; k<16; k++) {
        ((char*) &params->matrixRowState9)[k] = chars[144 + k];
    }
}




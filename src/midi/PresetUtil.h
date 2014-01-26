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

#ifndef PRESETUTIL_H_
#define PRESETUTIL_H_


#include <stdint.h>


class SynthState;
class Storage;
class MidiDecoder;
struct OneSynthParams;
struct ParameterDisplay;



struct PFM1Engine1Params {
    unsigned char algo;
    unsigned char velocity;
    unsigned char numberOfVoice;
    unsigned char glide;
};

struct PFM1Engine2Params {
    unsigned char modulationIndex1;
    unsigned char modulationIndex2;
    unsigned char modulationIndex3;
    unsigned char modulationIndex4;
};

struct PFM1Engine3Params {
    unsigned char mixOsc1;
    unsigned char mixOsc2;
    unsigned char mixOsc3;
    unsigned char mixOsc4;
};

struct PFM1PerformanceParams {
    unsigned char cc1;
    unsigned char cc2;
    unsigned char cc3;
    unsigned char cc4;
};

struct PFM1OscillatorParams {
        unsigned char shape; // OSC_SHAPE_*
        unsigned char frequencyType; // OSC_FT_*
        unsigned char frequencyMul;
        char  detune;
};

struct PFM1EnvelopeParams {
        unsigned char attack;
        unsigned char decay;
        unsigned char sustain;
        unsigned char release;
};

struct PFM1LfoParams {
        unsigned char shape; // LFO_SHAPE_*
        unsigned char freq;  // lfoFreq[]*
    char bias;
        unsigned char keybRamp;
};

struct PFM1MatrixRowParams {
        unsigned char source;
        char mul;
        unsigned char destination;
        char not_used;
};

struct PFM1StepSequencerParams {
        unsigned char bpm;
        unsigned char gate;
        unsigned char unused1;
        unsigned char unused2;
};

struct PFM1StepSequencerSteps {
        char steps[16];
};

struct PFM1SynthParams {
	struct PFM1Engine1Params engine1;
	struct PFM1Engine2Params engine2;
	struct PFM1Engine3Params engine3;
	struct PFM1PerformanceParams engine4;
	struct PFM1OscillatorParams osc1;
	struct PFM1OscillatorParams osc2;
	struct PFM1OscillatorParams osc3;
	struct PFM1OscillatorParams osc4;
	struct PFM1OscillatorParams osc5;
	struct PFM1OscillatorParams osc6;
	struct PFM1EnvelopeParams env1;
	struct PFM1EnvelopeParams env2;
	struct PFM1EnvelopeParams env3;
	struct PFM1EnvelopeParams env4;
	struct PFM1EnvelopeParams env5;
	struct PFM1EnvelopeParams env6;
	struct PFM1MatrixRowParams matrixRowState1;
	struct PFM1MatrixRowParams matrixRowState2;
	struct PFM1MatrixRowParams matrixRowState3;
	struct PFM1MatrixRowParams matrixRowState4;
	struct PFM1MatrixRowParams matrixRowState5;
	struct PFM1MatrixRowParams matrixRowState6;
	struct PFM1MatrixRowParams matrixRowState7;
	struct PFM1MatrixRowParams matrixRowState8;
	struct PFM1MatrixRowParams matrixRowState9;
	struct PFM1MatrixRowParams matrixRowState10;
	struct PFM1MatrixRowParams matrixRowState11;
	struct PFM1MatrixRowParams matrixRowState12;
	struct PFM1LfoParams lfo1;
	struct PFM1LfoParams lfo2;
	struct PFM1LfoParams lfo3;
	struct PFM1EnvelopeParams lfo4;
	struct PFM1StepSequencerParams lfo5;
	struct PFM1StepSequencerParams lfo6;
	struct PFM1StepSequencerSteps steps5;
	struct PFM1StepSequencerSteps steps6;
	char presetName[13];
};

enum PFM1LfoType {
	PFM1_LFO_SAW= 0,
	PFM1_LFO_RAMP,
	PFM1_LFO_SQUARE,
	PFM1_LFO_RANDOM,
	PFM1_LFO_SIN,
	PFM1_LFO_TYPE_MAX
};

enum PFM1OscShape {
	PFM1_OSC_SHAPE_SIN = 0,
	PFM1_OSC_SHAPE_SIN2,
	PFM1_OSC_SHAPE_SIN3,
	PFM1_OSC_SHAPE_SIN4,
	PFM1_OSC_SHAPE_RAND,
	PFM1_OSC_SHAPE_SQUARE,
	PFM1_OSC_SHAPE_SAW,
	PFM1_OSC_SHAPE_OFF,
	PFM1_OSC_SHAPE_LAST
};

enum PFM1SourceEnum {
	PFM1_MATRIX_SOURCE_NONE = 0,
	PFM1_MATRIX_SOURCE_LFO1,
	PFM1_MATRIX_SOURCE_LFO2,
	PFM1_MATRIX_SOURCE_LFO3,
	PFM1_MATRIX_SOURCE_LFO4,
	PFM1_MATRIX_SOURCE_PITCHBEND,
	PFM1_MATRIX_SOURCE_AFTERTOUCH,
	PFM1_MATRIX_SOURCE_MODWHEEL,
	PFM1_MATRIX_SOURCE_VELOCITY,
	PFM1_MATRIX_SOURCE_CC1,
	PFM1_MATRIX_SOURCE_CC2,
	PFM1_MATRIX_SOURCE_CC3,
	PFM1_MATRIX_SOURCE_CC4,
	PFM1_MATRIX_SOURCE_LFO5,
	PFM1_MATRIX_SOURCE_LFO6,
	PFM1_MATRIX_SOURCE_KEY,
	PFM1_MATRIX_SOURCE_MAX
};


enum PFM1DestinationEnum {
	PFM1_DESTINATION_NONE = 0,
	PFM1_OSC1_FREQ,
	PFM1_OSC2_FREQ,
	PFM1_OSC3_FREQ,
	PFM1_OSC4_FREQ,
	PFM1_OSC5_FREQ,
	PFM1_OSC6_FREQ,
	PFM1_INDEX_MODULATION1,
	PFM1_INDEX_MODULATION2,
	PFM1_INDEX_MODULATION3,
	PFM1_INDEX_MODULATION4,
	PFM1_MIX_OSC1,
	PFM1_MIX_OSC2,
	PFM1_MIX_OSC3,
	PFM1_MIX_OSC4,
	PFM1_LFO1_FREQ,
	PFM1_LFO2_FREQ,
	PFM1_LFO3_FREQ,
	PFM1_LFO4_FREQ,
	PFM1_MTX1_MUL,
	PFM1_MTX2_MUL,
	PFM1_MTX3_MUL,
	PFM1_MTX4_MUL,
	PFM1_MTX5_MUL,
	PFM1_MTX6_MUL,
	PFM1_MTX7_MUL,
	PFM1_MTX8_MUL,
	PFM1_MTX9_MUL,
	PFM1_MTX10_MUL,
	PFM1_MTX11_MUL,
	PFM1_MTX12_MUL,
	PFM1_ALL_OSC_FREQ,
	PFM1_LFO5_GATE,
	PFM1_LFO6_GATE,
	PFM1_MAIN_GATE,
	PFM1_ENV1_ATTACK,
	PFM1_ENV2_ATTACK,
	PFM1_ENV3_ATTACK,
	PFM1_ENV4_ATTACK,
	PFM1_ENV5_ATTACK,
	PFM1_ENV6_ATTACK,
	PFM1_ALL_ENV_ATTACK,
	PFM1_EXTERNAL_CC1,
	PFM1_EXTERNAL_CC2,
	PFM1_EXTERNAL_CC3,
	PFM1_EXTERNAL_CC4,
	PFM1_DESTINATION_MAX
};

// is included by SynthState so cannot be SynthStateAware...
// have to implement its own get/set synthState

class PresetUtil  {
public:
    PresetUtil();
    ~PresetUtil();

    static void setSynthState(SynthState* synthState);
    static void setStorage(Storage* storage);
    static void setMidiDecoder(MidiDecoder *midiDecoder);
    static void dumpPatch();
    static void dumpLine(const char *enums1[], int a, const char *enums2[], int b, const char *enums3[], int c, const char *enums4[], int d) ;

    static unsigned short getShortFromParamFloat(int row, int encoder, float value);
    static float getParamFloatFromShort(int row, int encoder, short value);

    static void saveConfigToEEPROM();
    static void loadConfigFromEEPROM();

    static void sendBankToSysex(int bankNumber);
    static void sendCurrentPatchToSysex();
    static void sendParamsToSysex(unsigned char* params);
    static int  readSysex(bool patchAllowed, bool bankAllowed);
    static int  readSysexPatch(unsigned char* params);
    static int  readSysexBank();

    static int  readSysexPatchPFM1(unsigned char* params);
    static int  readSysexBankPFM1();

    static int  getNextMidiByte();
    static void copySynthParams(char* source, char* dest);


    static void convertSynthStateToCharArray(OneSynthParams* params, unsigned char* chars);
    static void convertCharArrayToSynthState(const unsigned char* chars, OneSynthParams* params);

    static void convertPFM1CharArrayToSynthState(const unsigned char* chars, OneSynthParams* params, bool convert);

    static void sendNrpn(struct MidiEvent cc);
    static void sendCurrentPatchAsNrpns(int timbre);

    static uint8_t sysexTmpMem[];

private:
    static bool isSpecialSyexValue(int row, int encoder);
    static void copy4Charto4Float(unsigned char* source, float*dest);
    static float getValueForFreq(struct PFM1MatrixRowParams* pfm1mtx);

    static SynthState * synthState;
    static Storage * storage;
    static MidiDecoder * midiDecoder;

    static uint8_t sysexBuffer[];
    static int sysexIndex;

};

#endif /* PRESETUTIL_H_ */

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

#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define BLOCK_SIZE 32

#define PREENFM_FREQUENCY 40000.0f
#define PREENFM_FREQUENCY_INVERSED 1.0f/PREENFM_FREQUENCY
#define PREENFM_FREQUENCY_INVERSED_LFO PREENFM_FREQUENCY_INVERSED*32.0f
#define PREENFM_FREQUENCY_DIV_2 PREENFM_FREQUENCY/2.0f
#define NUMBER_OF_WAVETABLES 8

struct AlgoInformation {
    unsigned char osc;
    unsigned char im;
    unsigned char mix;
};

extern const struct OneSynthParams presets[];
extern const char* allChars;
extern struct AlgoInformation algoInformation[];

struct Engine1Params {
    float algo;
    float velocity;
    float numberOfVoice;
    float glide;
};

struct Engine2Params {
    float modulationIndex1;
    float modulationIndex2;
    float modulationIndex3;
    float modulationIndex4;
};

struct Engine3Params {
    float mixOsc1;
    float panOsc1;
    float mixOsc2;
    float panOsc2;
};

struct Engine4Params {
    float mixOsc3;
    float panOsc3;
    float mixOsc4;
    float panOsc4;
};

struct EnvelopeParams {
    float attack;
    float decay;
    float sustain;
    float release;
};

struct Envelope2Params {
    float silence;
    float attack;
    float decay;
    float loop;
};

struct OscillatorParams {
    float shape; // OSC_SHAPE_*
    float frequencyType; // OSC_FT_*
    float frequencyMul;
    float  detune;
};

struct MatrixRowParams {
    float source;
    float mul;
    float destination;
    float not_used;
};

struct LfoParams {
    float shape; // LFO_SHAPE_*
    float freq;  // lfoFreq[]*
    float bias;
    float keybRamp;
};


struct StepSequencerParams {
    float bpm;
    float gate;
    float unused1;
    float unused2;
};

struct StepSequencerSteps {
    char steps[16];
};


struct OneSynthParams {
    struct Engine1Params engine1;
    struct Engine2Params engine2;
    struct Engine3Params engine3;
    struct Engine4Params engine4;
    struct OscillatorParams osc1;
    struct OscillatorParams osc2;
    struct OscillatorParams osc3;
    struct OscillatorParams osc4;
    struct OscillatorParams osc5;
    struct OscillatorParams osc6;
    struct EnvelopeParams env1;
    struct EnvelopeParams env2;
    struct EnvelopeParams env3;
    struct EnvelopeParams env4;
    struct EnvelopeParams env5;
    struct EnvelopeParams env6;
    struct MatrixRowParams matrixRowState1;
    struct MatrixRowParams matrixRowState2;
    struct MatrixRowParams matrixRowState3;
    struct MatrixRowParams matrixRowState4;
    struct MatrixRowParams matrixRowState5;
    struct MatrixRowParams matrixRowState6;
    struct MatrixRowParams matrixRowState7;
    struct MatrixRowParams matrixRowState8;
    struct MatrixRowParams matrixRowState9;
    struct MatrixRowParams matrixRowState10;
    struct MatrixRowParams matrixRowState11;
    struct MatrixRowParams matrixRowState12;
    struct LfoParams lfoOsc1;
    struct LfoParams lfoOsc2;
    struct LfoParams lfoOsc3;
    struct EnvelopeParams lfoEnv1;
    struct Envelope2Params lfoEnv2;
    struct StepSequencerParams lfoSeq1;
    struct StepSequencerParams lfoSeq2;
    struct StepSequencerSteps lfoSteps1;
    struct StepSequencerSteps lfoSteps2;
    char presetName[13];
};

enum SourceEnum {
    MATRIX_SOURCE_NONE = 0,
    MATRIX_SOURCE_LFO1,
    MATRIX_SOURCE_LFO2,
    MATRIX_SOURCE_LFO3,
    MATRIX_SOURCE_LFOENV1,
    MATRIX_SOURCE_LFOENV2,
    MATRIX_SOURCE_LFOSEQ1,
    MATRIX_SOURCE_LFOSEQ2,
    MATRIX_SOURCE_PITCHBEND,
    MATRIX_SOURCE_AFTERTOUCH,
    MATRIX_SOURCE_MODWHEEL,
    MATRIX_SOURCE_VELOCITY,
    MATRIX_SOURCE_KEY,
    MATRIX_SOURCE_CC1,
    MATRIX_SOURCE_CC2,
    MATRIX_SOURCE_CC3,
    MATRIX_SOURCE_CC4,
    MATRIX_SOURCE_MAX
};


enum DestinationEnum {
    DESTINATION_NONE = 0,
    MAIN_GATE,
    INDEX_MODULATION1,
    INDEX_MODULATION2,
    INDEX_MODULATION3,
    INDEX_MODULATION4,
    OSC1_FREQ,
    OSC2_FREQ,
    OSC3_FREQ,
    OSC4_FREQ,
    OSC5_FREQ,
    OSC6_FREQ,
    ALL_OSC_FREQ,
    LFO1_FREQ,
    LFO2_FREQ,
    LFO3_FREQ,
    LFOENV2_SILENCE,
    LFOSEQ1_GATE,
    LFOSEQ2_GATE,
    MIX_OSC1,
    PAN_OSC1,
    MIX_OSC2,
    PAN_OSC2,
    MIX_OSC3,
    PAN_OSC3,
    ENV1_ATTACK,
    ENV2_ATTACK,
    ENV3_ATTACK,
    ENV4_ATTACK,
    ENV5_ATTACK,
    ENV6_ATTACK,
    ALL_ENV_ATTACK,
    MTX1_MUL,
    MTX2_MUL,
    MTX3_MUL,
    MTX4_MUL,
    MTX5_MUL,
    MTX6_MUL,
    MTX7_MUL,
    MTX8_MUL,
    MTX9_MUL,
    MTX10_MUL,
    MTX11_MUL,
    MTX12_MUL,
    EXTERNAL_CC1,
    EXTERNAL_CC2,
    EXTERNAL_CC3,
    EXTERNAL_CC4,
    DESTINATION_MAX
};




#endif /* COMMON_H_ */

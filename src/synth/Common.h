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

// #define DEBUG 1

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)
#define ARRAY_SIZE(x)  ( sizeof(x) / sizeof((x)[0]) )

#define BLOCK_SIZE 32

#define NUMBER_OF_ENCODERS 4
#define NUMBER_OF_BUTTONS 8



#define MAX_NUMBER_OF_VOICES 14
#define MAX_NUMBER_OF_OPERATORS 48

#define NUMBER_OF_TIMBRES 4
#define NUMBER_OF_OPERATORS 6


#ifndef OVERCLOCK
// 168000000 / 1105 / 4
#define PREENFM_FREQUENCY 38009.04f
#else
// 192000000 / 1116 / 4  :
// #define PREENFM_FREQUENCY 43010.75f

// Polyphony !!!
// 192000000 / 1142 / 4  :
#define PREENFM_FREQUENCY 42031.52f

#endif

#define PREENFM_FREQUENCY_INVERSED 1.0f/PREENFM_FREQUENCY
#define PREENFM_FREQUENCY_INVERSED_LFO PREENFM_FREQUENCY_INVERSED*32.0f

#define NUMBER_OF_WAVETABLES 14


typedef enum {
    FILE_OK = 0,
    FILE_READ_ONLY,
    FILE_EMPTY
} FileType;


struct WaveTable {
    float* table;
    int max;
    float useFreq;
    float floatToAdd;
    float precomputedValue;
};

struct AlgoInformation {
    unsigned char osc;
    unsigned char im;
    unsigned char mix;
};

enum {
    ROW_ENGINE_FIRST = 0,
    ROW_ENGINE = ROW_ENGINE_FIRST,
    ROW_MODULATION1 ,
    ROW_MODULATION2 ,
    ROW_MODULATION3 ,
    ROW_OSC_MIX1,
    ROW_OSC_MIX2,
    ROW_OSC_MIX3,
    ROW_ARPEGGIATOR1,
    ROW_ARPEGGIATOR2,
    ROW_ARPEGGIATOR3,
    ROW_EFFECT,
    ROW_ENGINE_LAST = ROW_EFFECT,
    ROW_OSC_FIRST = ROW_ENGINE_LAST+1,
    ROW_OSC1 = ROW_OSC_FIRST,
    ROW_OSC2 ,
    ROW_OSC3 ,
    ROW_OSC4 ,
    ROW_OSC5 ,
    ROW_OSC6 ,
    ROW_OSC_LAST = ROW_OSC6,
    ROW_ENV_FIRST = ROW_OSC_LAST+1,
    ROW_ENV1a = ROW_ENV_FIRST,
    ROW_ENV1b,
    ROW_ENV2a ,
    ROW_ENV2b ,
    ROW_ENV3a ,
    ROW_ENV3b ,
    ROW_ENV4a ,
    ROW_ENV4b ,
    ROW_ENV5a ,
    ROW_ENV5b ,
    ROW_ENV6a ,
    ROW_ENV6b ,
    ROW_ENV_LAST = ROW_ENV6b,
    ROW_MATRIX_FIRST = ROW_ENV_LAST+1,
    ROW_MATRIX1 = ROW_MATRIX_FIRST,
    ROW_MATRIX2 ,
    ROW_MATRIX3 ,
    ROW_MATRIX4 ,
    ROW_MATRIX5 ,
    ROW_MATRIX6 ,
    ROW_MATRIX7 ,
    ROW_MATRIX8 ,
    ROW_MATRIX9 ,
    ROW_MATRIX10 ,
    ROW_MATRIX11 ,
    ROW_MATRIX12 ,
    ROW_MATRIX_LAST = ROW_MATRIX12,
	ROW_PERFORMANCE1 = ROW_MATRIX_LAST + 1,
    ROW_LFO_FIRST = ROW_PERFORMANCE1+1,
    ROW_LFOOSC1 = ROW_LFO_FIRST,
    ROW_LFOOSC2 ,
    ROW_LFOOSC3 ,
    ROW_LFOPHASES,
    ROW_LFOENV1 ,
    ROW_LFOENV2 ,
    ROW_LFOSEQ1 ,
    ROW_LFOSEQ2 ,
    ROW_MIDINOTE1CURVE,
    ROW_MIDINOTE2CURVE,
    ROW_LFO_LAST = ROW_MIDINOTE2CURVE
};



enum {
	LFO_SEQ_MIDICLOCK_DIV_4 = 241,
	LFO_SEQ_MIDICLOCK_DIV_2,
	LFO_SEQ_MIDICLOCK,
	LFO_SEQ_MIDICLOCK_TIME_2,
	LFO_SEQ_MIDICLOCK_TIME_4,
};

#define LFO_FREQ_MAX  99.9f
#define LFO_FREQ_MAX_TIMES_10 (int)(LFO_FREQ_MAX*10.0f)

enum LfoMidiClockMc {
    LFO_BEFORE_MIDI_CLOCK = LFO_FREQ_MAX_TIMES_10,
	LFO_MIDICLOCK_MC_DIV_16, // 1000
	LFO_MIDICLOCK_MC_DIV_8,
	LFO_MIDICLOCK_MC_DIV_4,
	LFO_MIDICLOCK_MC_DIV_2,
	LFO_MIDICLOCK_MC,
	LFO_MIDICLOCK_MC_TIME_2,
	LFO_MIDICLOCK_MC_TIME_3,
	LFO_MIDICLOCK_MC_TIME_4,
	LFO_MIDICLOCK_MC_TIME_8
};

enum {
  ARPEGGIATOR_PRESET_PATTERN_COUNT = 22,
  ARPEGGIATOR_USER_PATTERN_COUNT = 4,
  ARPEGGIATOR_PATTERN_COUNT = ARPEGGIATOR_PRESET_PATTERN_COUNT + ARPEGGIATOR_USER_PATTERN_COUNT
};

#define NUMBER_OF_ROWS ROW_LFO_LAST+1

#define PATCH_SIZE_PFM2 ((NUMBER_OF_ROWS)*4*2 + 16*2 + 13)
#define PFM1_PATCH_SIZE (128+64)


extern const struct OneSynthParams preenMainPreset;
extern const struct OneSynthParams defaultPreset;
extern const char* allChars;

extern struct AlgoInformation algoInformation[];

#define OPERATOR_CARRIER 1
#define OPERATOR_MODULATOR 2
extern int algoOpInformation[][NUMBER_OF_OPERATORS];

extern uint8_t sysexTmpMem[];
extern struct OneSynthParams oneSynthParamsTmp;

struct Engine1Params {
    float algo;
    float velocity;
    float numberOfVoice;
    float glide;
};

//{ "Acti", "Clk ", "BPM ", "Dire" },
//{ "Ptrn", "Divi", "Dura", "Latc" },

struct EngineArp1 {
    float clock;
    float BPM;
    float direction;
    float octave;
};

struct EngineArp2 {
    float pattern;
    float division;
    float duration;
    float latche;
};

/* low 16 bits = pattern mask, high reserved for now */
typedef uint32_t arp_pattern_t;
#define ARP_PATTERN_GETMASK(x) (uint16_t)( (x) & 0xffff)
#define ARP_PATTERN_SETMASK(p,m) do { (p) = ((p) & ~0xffff) | ((m) & 0xffff); } while (0)
#define ARP_PATTERN(x) ((uint32_t)(x) & 0xffff)
#define ARP_MASK_GETNIBBLE(m,i) (((m) >> (i*4)) & 0xf)
#define ARP_MASK_SETNIBBLE(m,i,v) do {  mask &= ~(0xf << (i*4)); mask |= (v << (i*4)); } while(0)

struct EngineArpUserPatterns {
  arp_pattern_t patterns[ 4 ];
};

struct EngineIm1 {
    float modulationIndex1;
    float modulationIndexVelo1;
    float modulationIndex2;
    float modulationIndexVelo2;
};

struct EngineIm2 {
    float modulationIndex3;
    float modulationIndexVelo3;
    float modulationIndex4;
    float modulationIndexVelo4;
};

struct EngineIm3 {
    float modulationIndex5;
    float modulationIndexVelo5;
    float modulationIndex6;
    float modulationIndexVelo6;
};

struct FlashEngineIm1 {
    float modulationIndex1;
    float modulationIndex2;
    float modulationIndex3;
    float modulationIndex4;
};

struct FlashEngineIm2 {
    float modulationIndex5;
    float modulationIndex6;
    float notUsed1;
    float notUsed2;
};

struct FlashEngineVeloIm1 {
    float modulationIndexVelo1;
    float modulationIndexVelo2;
    float modulationIndexVelo3;
    float modulationIndexVelo4;
};

struct FlashEngineVeloIm2 {
    float modulationIndexVelo5;
    float modulationIndexVelo6;
    float notUsed1;
    float notUsed2;
};


struct EngineMix1 {
    float mixOsc1;
    float panOsc1;
    float mixOsc2;
    float panOsc2;
};

struct EngineMix2 {
    float mixOsc3;
    float panOsc3;
    float mixOsc4;
    float panOsc4;
};

struct EngineMix3 {
    float mixOsc5;
    float panOsc5;
    float mixOsc6;
    float panOsc6;
};

struct EnvelopeParamsA {
    float attackTime;
    float attackLevel;
    float decayTime;
    float decayLevel;
};

struct EnvelopeParamsB {
    float sustainTime;
    float sustainLevel;
    float releaseTime;
    float releaseLevel;
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

struct EffectRowParams {
    float type;
    float param1;
    float param2;
    float param3;
};

struct PerformanceRowParams {
	float perf1;
	float perf2;
	float perf3;
	float perf4;
};

struct LfoPhaseRowParams {
    float phaseLfo1;
    float phaseLfo2;
    float phaseLfo3;
    float unused1;
};

struct MidiNoteCurveRowParams {
    float curveBefore;
    float breakNote;
    float curveAfter;
    float unused1;
};


struct OneSynthParams {
    struct Engine1Params engine1;
    struct EngineIm1 engineIm1;
    struct EngineIm2 engineIm2;
    struct EngineIm3 engineIm3;
    struct EngineMix1 engineMix1;
    struct EngineMix2 engineMix2;
    struct EngineMix3 engineMix3;
    struct EngineArp1 engineArp1;
    struct EngineArp2 engineArp2;
    struct EngineArpUserPatterns engineArpUserPatterns;
    struct EffectRowParams effect;
    struct OscillatorParams osc1;
    struct OscillatorParams osc2;
    struct OscillatorParams osc3;
    struct OscillatorParams osc4;
    struct OscillatorParams osc5;
    struct OscillatorParams osc6;
    struct EnvelopeParamsA env1a;
    struct EnvelopeParamsB env1b;
    struct EnvelopeParamsA env2a;
    struct EnvelopeParamsB env2b;
    struct EnvelopeParamsA env3a;
    struct EnvelopeParamsB env3b;
    struct EnvelopeParamsA env4a;
    struct EnvelopeParamsB env4b;
    struct EnvelopeParamsA env5a;
    struct EnvelopeParamsB env5b;
    struct EnvelopeParamsA env6a;
    struct EnvelopeParamsB env6b;
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
    struct PerformanceRowParams performance1;
    struct LfoParams lfoOsc1;
    struct LfoParams lfoOsc2;
    struct LfoParams lfoOsc3;
    struct LfoPhaseRowParams lfoPhases;
    struct EnvelopeParams lfoEnv1;
    struct Envelope2Params lfoEnv2;
    struct StepSequencerParams lfoSeq1;
    struct StepSequencerParams lfoSeq2;
    struct MidiNoteCurveRowParams midiNote1Curve;
    struct MidiNoteCurveRowParams midiNote2Curve;
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
    MATRIX_SOURCE_MODWHEEL,
    MATRIX_SOURCE_PITCHBEND,
    MATRIX_SOURCE_AFTERTOUCH,
    MATRIX_SOURCE_VELOCITY,
    MATRIX_SOURCE_NOTE1,
    MATRIX_SOURCE_CC1,
    MATRIX_SOURCE_CC2,
    MATRIX_SOURCE_CC3,
    MATRIX_SOURCE_CC4,
    MATRIX_SOURCE_NOTE2,
    MATRIX_SOURCE_BREATH,
#ifdef CVIN
    MATRIX_SOURCE_CVIN1,
    MATRIX_SOURCE_CVIN2,
    MATRIX_SOURCE_CVIN3,
    MATRIX_SOURCE_CVIN4,
#endif
    MATRIX_SOURCE_MPESLIDE,
    MATRIX_SOURCE_MAX
};


enum DestinationEnum {
    DESTINATION_NONE = 0,
    MAIN_GATE,
    INDEX_MODULATION1,
    INDEX_MODULATION2,
    INDEX_MODULATION3,
    INDEX_MODULATION4,
    INDEX_ALL_MODULATION,
    MIX_OSC1,
    PAN_OSC1,
    MIX_OSC2,
    PAN_OSC2,
    MIX_OSC3,
    PAN_OSC3,
    MIX_OSC4,
    PAN_OSC4,
    ALL_MIX,
    ALL_PAN,
    OSC1_FREQ,
    OSC2_FREQ,
    OSC3_FREQ,
    OSC4_FREQ,
    OSC5_FREQ,
    OSC6_FREQ,
    ALL_OSC_FREQ,
    ENV1_ATTACK,
    ENV2_ATTACK,
    ENV3_ATTACK,
    ENV4_ATTACK,
    ENV5_ATTACK,
    ENV6_ATTACK,
    ALL_ENV_ATTACK,
    ALL_ENV_RELEASE,
    MTX1_MUL,
    MTX2_MUL,
    MTX3_MUL,
    MTX4_MUL,
    LFO1_FREQ,
    LFO2_FREQ,
    LFO3_FREQ,
    LFOENV2_SILENCE,
    LFOSEQ1_GATE,
    LFOSEQ2_GATE,
    FILTER_FREQUENCY,
    ALL_OSC_FREQ_HARM,
    ALL_ENV_DECAY,
    ALL_ENV_ATTACK_MODULATOR,
    ALL_ENV_DECAY_MODULATOR,
    ALL_ENV_RELEASE_MODULATOR,
    DESTINATION_MAX
};

int strcmp(const char *s1, const char *s2);

// In separate structure to be easily sent to config saver/loader
struct ScalaScaleConfig {
    const struct PFM2File* scalaFile;
    float scalaFreq;
    bool scalaEnabled;
    bool keyboardMapping;
};


#endif /* COMMON_H_ */

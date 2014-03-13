/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <.> hosxe < a t > gmail.com)
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



#ifndef TIMBRE_H_
#define TIMBRE_H_

#include "Common.h"

#include "Osc.h"
#include "Env.h"
#include "Lfo.h"
#include "LfoOsc.h"
#include "LfoEnv.h"
#include "LfoEnv2.h"
#include "LfoStepSeq.h"
#include "Matrix.h"
#include "core_cmInstr.h"
#include "note_stack.h"
#include "event_scheduler.h"


extern float panTable[];
class Voice;

enum {  CLOCK_OFF,
		CLOCK_INTERNAL,
		CLOCK_EXTERNAL
};


class Timbre {
    friend class Synth;
    friend class Voice;
public:
    Timbre();
    virtual ~Timbre();
    void init(int timbreNumber);
    void setVoiceNumber(int v, int n);
    void initVoicePointer(int n, Voice* voice);
    void prepareForNextBlock();
    void cleanNextBlock();
    void fxAfterBlock(float ratioTimbres);
    void afterNewParamsLoad();
    void setNewValue(int index, struct ParameterDisplay* param, float newValue);
    void setNewEffecParam(int encoder);
    // Arpegiator
    void arpeggiatorNoteOn(char note, char velocity);
    void arpeggiatorNoteOff(char note);
    void StartArpeggio();
    void StepArpeggio();
    void Start();
    void arpeggiatorSetHoldPedal(uint8_t value);
    void setLatchMode(uint8_t value);
    void setDirection(uint8_t value);
    void setNewBPMValue(float bpm);
    void setArpeggiatorClock(float bpm);
    void resetArpeggiator();

    void noteOn(char note, char velocity);
    void noteOff(char note);

    void preenNoteOn(char note, char velocity);
    void preenNoteOff(char note);
    void numberOfVoicesChanged() {
    	if (params.engine1.numberOfVoice > 0) {
    		numberOfVoiceInverse = 1.0f / params.engine1.numberOfVoice;
    	} else {
    		numberOfVoiceInverse = 1.0f;
    	}
    }
    float* getPerformanceValuesAddress() {
    	return &matrix.sources[MATRIX_SOURCE_KEY];
    }

    void lfoNoteOn() {
    	lfoOsc[0].noteOn();
    	lfoOsc[1].noteOn();
    	lfoOsc[2].noteOn();
    	lfoEnv[0].noteOn();
    	lfoEnv2[0].noteOn();
    	lfoStepSeq[0].noteOn();
    	lfoStepSeq[1].noteOn();
    }

    void lfoNoteOff() {
    	lfoOsc[0].noteOff();
    	lfoOsc[1].noteOff();
    	lfoOsc[2].noteOff();
    	lfoEnv[0].noteOff();
    	lfoEnv2[0].noteOff();
    	lfoStepSeq[0].noteOff();
    	lfoStepSeq[1].noteOff();
    }

    void lfoValueChange(int currentRow, int encoder, float newValue);
    // timbres[timbre].lfo[currentRow - ROW_LFOOSC1]->valueChanged(encoder);

/*
    void calculateFrequencyWithMatrix(struct OscState* oscState[NUMBER_OF_OPERATORS]) {
        for (int k=0; k<algoInformation[(int)params.engine1.algo].osc; k++) {
            osc1.calculateFrequencyWithMatrix(oscState[0]);
        }
    }
*/


    void updateAllModulationIndexes() {
    	int numberOfIMs = algoInformation[(int)(params.engine1.algo)].im;
    	modulationIndex1 = params.engineIm1.modulationIndex1 + matrix.getDestination(INDEX_MODULATION1) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex1 < 0.0f)) {
    		modulationIndex1 = 0.0f;
    	}
        modulationIndex2 = params.engineIm1.modulationIndex2 + matrix.getDestination(INDEX_MODULATION2) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex2 < 0.0f)) {
    		modulationIndex2 = 0.0f;
    	}

        modulationIndex3 = params.engineIm2.modulationIndex3 + matrix.getDestination(INDEX_MODULATION3) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex3 < 0.0f)) {
    		modulationIndex3 = 0.0f;
    	}

    	if (likely(numberOfIMs < 3)) {
        	return;
        }

    	modulationIndex4 = params.engineIm2.modulationIndex4 + matrix.getDestination(INDEX_MODULATION4) + matrix.getDestination(INDEX_ALL_MODULATION);
    	if (unlikely(modulationIndex4 < 0.0f)) {
    		modulationIndex4 = 0.0f;
    	}

        modulationIndex5 = params.engineIm3.modulationIndex5 + matrix.getDestination(INDEX_ALL_MODULATION);;
    	if (unlikely(modulationIndex5 < 0.0f)) {
    		modulationIndex5 = 0.0f;
    	}

    }

    void updateAllMixOscsAndPans() {
    	int numberOfMixes = algoInformation[(int)(params.engine1.algo)].mix;
    	int mix;
    	float inv65535 = .0000152587890625; // 1/ 65535

    	mix1 = params.engineMix1.mixOsc1 + matrix.getDestination(MIX_OSC1) + matrix.getDestination(ALL_MIX);
    	// Optimization to check mix1 is between 0 and 1
    	mix1 = __USAT((int)(mix1 * 65536) , 16) * inv65535;
        float pan1 = params.engineMix1.panOsc1 + matrix.getDestination(PAN_OSC1) + matrix.getDestination(ALL_PAN) + 1.0f;
        // pan1 is between -1 and 1 : Scale from 0.0 to 256
        int pan = __USAT((int)(pan1 * 128), 8);
		pan1Left = panTable[256 - pan];
		pan1Right = panTable[pan];


        mix2 = params.engineMix1.mixOsc2 + matrix.getDestination(MIX_OSC2) + matrix.getDestination(ALL_MIX);
        mix2 = __USAT((int)(mix2 * 65535) , 16) * inv65535;

        float pan2 = params.engineMix1.panOsc2 + matrix.getDestination(PAN_OSC2) + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan2 * 128), 8);
		pan2Left = panTable[256 - pan];
		pan2Right = panTable[pan];

        // A bit lighter for algo with 1 or 2 mix...
        if (numberOfMixes <=2) {
        	return;
        }

        mix3 = params.engineMix2.mixOsc3 + matrix.getDestination(MIX_OSC3) + matrix.getDestination(ALL_MIX);
        mix3 = __USAT((int)(mix3 * 65535) , 16) * inv65535;

        float pan3 = params.engineMix2.panOsc3 + matrix.getDestination(PAN_OSC3) + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan3 * 128), 8);
		pan3Left = panTable[256 - pan];
		pan3Right = panTable[pan];

        // No matrix for mix4 and pan4
        mix4 = params.engineMix2.mixOsc4 + matrix.getDestination(ALL_MIX);
        mix4 = __USAT((int)(mix4 * 65535) , 16) * inv65535;
        float pan4 = params.engineMix2.panOsc4 + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan4 * 128), 8);
		pan4Left = panTable[256 - pan];
		pan4Right = panTable[pan];


        // A bit lighter for algo with 5 or 6 mix...
        if (likely(numberOfMixes <=4)) {
        	return;
        }

        // No more matrix....

        mix5 = params.engineMix3.mixOsc5  + matrix.getDestination(ALL_MIX);
        mix5 = __USAT((int)(mix5 * 65535) , 16) * inv65535;
        float pan5 = params.engineMix3.panOsc5 + matrix.getDestination(ALL_PAN)  + 1.0f;
        pan = __USAT((int)(pan5 * 128), 8);
		pan5Left = panTable[256 - pan];
		pan5Right = panTable[pan];

        mix6 = params.engineMix3.mixOsc6 + matrix.getDestination(ALL_MIX);
        mix6 = __USAT((int)(mix6 * 65535) , 16) * inv65535;
        float pan6 = params.engineMix3.panOsc6 + matrix.getDestination(ALL_PAN) + 1.0f;
        pan = __USAT((int)(pan6 * 128), 8);
		pan6Left = panTable[256 - pan];
		pan6Right = panTable[pan];

    }

    void setHoldPedal(int value);

    Matrix* getMatrix() {
        return &matrix;
    }

    void midiClockContinue(int songPosition) {
    	lfoOsc[0].midiClock(songPosition, false);
    	lfoOsc[1].midiClock(songPosition, false);
    	lfoOsc[2].midiClock(songPosition, false);
    	lfoEnv[0].midiClock(songPosition, false);
    	lfoEnv2[0].midiClock(songPosition, false);
    	lfoStepSeq[0].midiClock(songPosition, false);
    	lfoStepSeq[1].midiClock(songPosition, false);


        this->recomputeNext = ((songPosition&0x1)==0);
        OnMidiContinue();
    }


    void midiClockStart() {
    	lfoOsc[0].midiContinue();
    	lfoOsc[1].midiContinue();
    	lfoOsc[2].midiContinue();
    	lfoEnv[0].midiContinue();
    	lfoEnv2[0].midiContinue();
    	lfoStepSeq[0].midiContinue();
    	lfoStepSeq[1].midiContinue();
        this->recomputeNext = true;
        OnMidiStart();
    }


    void midiClockStop() {
    	OnMidiStop();
    }

    void midiClockSongPositionStep(int songPosition) {
    	lfoOsc[0].midiClock(songPosition, this->recomputeNext);
    	lfoOsc[1].midiClock(songPosition, this->recomputeNext);
    	lfoOsc[2].midiClock(songPosition, this->recomputeNext);
    	lfoEnv[0].midiClock(songPosition, this->recomputeNext);
    	lfoEnv2[0].midiClock(songPosition, this->recomputeNext);
    	lfoStepSeq[0].midiClock(songPosition, this->recomputeNext);
    	lfoStepSeq[1].midiClock(songPosition, this->recomputeNext);

        if ((songPosition & 0x1)==0) {
            this->recomputeNext = true;
        }
    }
    struct OneSynthParams * getParamRaw() {
        return &params;
    }

    float* getSampleBlock() {
        return sampleBlock;
    }

    // optimization
    float modulationIndex1, modulationIndex2, modulationIndex3, modulationIndex4, modulationIndex5;
    float mix1, mix2, mix3, mix4, mix5, mix6;
    float pan1Left, pan2Left, pan3Left, pan4Left, pan5Left, pan6Left  ;
    float pan1Right, pan2Right, pan3Right, pan4Right, pan5Right, pan6Right ;


    // Needed for debuging
    char voiceNumber[MAX_NUMBER_OF_VOICES];
private:

    // MiniPal Arpegiator
    void SendLater(uint8_t note, uint8_t velocity, uint8_t when, uint8_t tag);
    void SendScheduledNotes();
    void FlushQueue();
    void Tick();
    void OnMidiContinue();
    void OnMidiStart();
    void OnMidiStop();
    void OnMidiClock();


    int timbreNumber;
    struct OneSynthParams params;
    Matrix matrix;
    float sampleBlock[BLOCK_SIZE * 2];
    float *sbMax;
    float numberOfVoiceInverse;
    float mixerGain;
    Voice *voices[MAX_NUMBER_OF_VOICES];
    bool holdPedal;

    LfoOsc lfoOsc[NUMBER_OF_LFO_OSC];
    LfoEnv lfoEnv[NUMBER_OF_LFO_ENV];
    LfoEnv2 lfoEnv2[NUMBER_OF_LFO_ENV2];
    LfoStepSeq lfoStepSeq[NUMBER_OF_LFO_STEP];

    // 6 oscillators Max
    Osc osc1;
    Osc osc2;
    Osc osc3;
    Osc osc4;
    Osc osc5;
    Osc osc6;

    // And their 6 envelopes
    Env env1;
    Env env2;
    Env env3;
    Env env4;
    Env env5;
    Env env6;


    // Must recompute LFO steps ?
    bool recomputeNext;
    float currentGate;
    // Arpeggiator

    // TO REFACTOR
    float ticksPerSecond;
    const float calledPerSecond = PREENFM_FREQUENCY / 32.0f;
    float ticksEveryNCalls;
    int ticksEveyNCallsInteger;



    float arpegiatorStep;
    NoteStack note_stack;
    EventScheduler event_scheduler;
//
//
//    uint8_t clk_mode_;
//    uint8_t groove_template_;
//    uint8_t groove_amount_;
//    uint8_t channel_;
//    uint8_t pattern_;
//


    uint8_t running_;
    uint8_t latch_;
    uint8_t tick_;
    uint8_t idle_ticks_;
    uint16_t bitmask_;
    int8_t current_direction_;
    int8_t current_octave_;
    int8_t current_step_;
    uint8_t ignore_note_off_messages_;
    uint8_t recording_;
    // Low pass filter
	float fxParam1, fxParam2, fxParam3;
    float v0L, v1L;
    float v0R, v1R;

};

#endif /* TIMBRE_H_ */

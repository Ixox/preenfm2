/*
 * Copyright 2011 Xavier Hosxe
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


#define NUMBER_OF_OPERATORS 6



class Timbre {
    friend class Synth;
    friend class Voice;
public:
    Timbre();
    virtual ~Timbre();
    void init();

    void prepareForNextBlock();
    void fxAfterBlock();
    void afterNewParamsLoad();
    void setNewValue(int index, struct ParameterDisplay* param, float newValue);

    void noteOn() {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->noteOn();
        }
    }

    void calculateFrequencyWithMatrix(struct OscState* oscState[NUMBER_OF_OPERATORS]) {
        for (int k=0; k<algoInformation[(int)params.engine1.algo].osc; k++) {
            osc[k]->calculateFrequencyWithMatrix(oscState[k]);
        }
    }

    void updateAllModulationIndexes() {
        modulationIndex1 = params.engine2.modulationIndex1 + matrix.getDestination(INDEX_MODULATION1);
        modulationIndex1 = (modulationIndex1 < 0.0f) ? 0.0f : modulationIndex1;
        modulationIndex2 = params.engine2.modulationIndex2 + matrix.getDestination(INDEX_MODULATION2);
        modulationIndex2 = (modulationIndex2 < 0.0f) ? 0.0f : modulationIndex2;
        modulationIndex3 = params.engine2.modulationIndex3 + matrix.getDestination(INDEX_MODULATION3);
        modulationIndex3 = (modulationIndex3 < 0.0f) ? 0.0f : modulationIndex3;
        modulationIndex4 = params.engine2.modulationIndex4 + matrix.getDestination(INDEX_MODULATION4);
        modulationIndex4 = (modulationIndex4 < 0.0f) ? 0.0f : modulationIndex4;
    }

    void updateAllMixOscsAndPans() {
        mix1 = params.engine3.mixOsc1 + matrix.getDestination(MIX_OSC1);
        mix1 = (mix1 < 0) ? 0 : (mix1 >1.0f ? 1.0f : mix1);
        mix2 = params.engine3.mixOsc2 + matrix.getDestination(MIX_OSC2);
        mix2 = (mix2 < 0) ? 0 : (mix2 >1.0f ? 1.0f : mix2);
        mix3 = params.engine4.mixOsc3 + matrix.getDestination(MIX_OSC3);
        mix3 = (mix3 < 0) ? 0 : (mix3 >1.0f ? 1.0f : mix3);

        float pan1 = params.engine3.panOsc1 + matrix.getDestination(PAN_OSC1) + 1;
        if (pan1 < 1) {
            pan1Left = pan1 * .80f +.2f;
            pan1Right = 1;
        } else {
            pan1Right = 1.8f - pan1 * .8f;
            pan1Left = 1;
        }

        float pan2 = params.engine3.panOsc2 + matrix.getDestination(PAN_OSC2) + 1;
        if (pan2 < 1) {
            pan2Left = pan2 * .80f +.2f;
            pan2Right = 1;
        } else {
            pan2Right = 1.8f - pan2 * .8f;
            pan2Left = 1;
        }

        float pan3 = params.engine4.panOsc3 + matrix.getDestination(PAN_OSC3) + 1;
        if (pan3 < 1) {
            pan3Left = pan3 * .80f +.2f;
            pan3Right = 1;
        } else {
            pan3Right = 1.8f - pan3 * .8f;
            pan3Left = 1;
        }

    }

    Matrix* getMatrix() {
        return &matrix;
    }

    void midiClockContinue(int songPosition) {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiClock(songPosition, false);
        }
        this->recomputeNext = ((songPosition&0x1)==0);
    }


    void midiClockStart() {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiContinue();
        }
        this->recomputeNext = true;
    }

    void midiClockStop() {
    }

    void midiClockSongPositionStep(int songPosition) {
        for (int k=0; k<NUMBER_OF_LFO; k++) {
            lfo[k]->midiClock(songPosition, this->recomputeNext);
        }
        if ((songPosition & 0x1)==0) {
            this->recomputeNext = true;
        }
    }
    uint8_t* getParamRaw() {
        return (uint8_t*)&params;
    }

    float* getSampleBlock() {
        return sampleBlock;
    }

    // optimization
    float modulationIndex1, modulationIndex2, modulationIndex3, modulationIndex4;
    float mix1, mix2, mix3;
    float pan1Left, pan2Left, pan3Left;
    float pan1Right, pan2Right, pan3Right;

private:
    struct OneSynthParams params;
    Matrix matrix;
    float sampleBlock[BLOCK_SIZE * 2];
    float *sbMax;

    LfoOsc lfoOsc[NUMBER_OF_LFO_OSC];
    LfoEnv lfoEnv[NUMBER_OF_LFO_ENV];
    LfoEnv2 lfoEnv2[NUMBER_OF_LFO_ENV2];
    LfoStepSeq lfoStepSeq[NUMBER_OF_LFO_STEP];
    Lfo* lfo[NUMBER_OF_LFO];

    // 6 oscillators Max
    Osc osc1;
    Osc osc2;
    Osc osc3;
    Osc osc4;
    Osc osc5;
    Osc osc6;
    Osc *osc[NUMBER_OF_OPERATORS];
    // And their 6 envelopes
    Env env1;
    Env env2;
    Env env3;
    Env env4;
    Env env5;
    Env env6;
    Env *env[NUMBER_OF_OPERATORS];

    // Must recompute LFO steps ?
    bool recomputeNext;

    float currentGate;

};

#endif /* TIMBRE_H_ */

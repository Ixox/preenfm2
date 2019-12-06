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

#pragma once

#include "SynthStateAware.h"
#include "Matrix.h"

extern float sinTable[];



struct OscState {
    float index;
    float frequency;
    float phase;
    float mainFrequencyPlusMatrix;
    float mainFrequency;
    float fromFrequency;
    float nextFrequency;
};


extern struct WaveTable waveTables[];
extern float exp2_harm[];


class Osc : public SynthStateAware
{
public:
    Osc() {};
    virtual ~Osc() {};

    void init(SynthState* sState, struct OscillatorParams *oscParams, DestinationEnum df);

    void newNote(struct OscState* oscState, int note);
    void glideToNote(struct OscState* oscState, int note);
    void glideStep(struct OscState* oscState, float phase);

    inline void calculateFrequencyWithMatrix(struct OscState *oscState, Matrix* matrix, float expHarm) {
        oscState->mainFrequencyPlusMatrix = oscState->mainFrequency;
        oscState->mainFrequencyPlusMatrix *= expHarm;
        oscState->mainFrequencyPlusMatrix +=  (oscState->mainFrequency  * (matrix->getDestination(destFreq) + matrix->getDestination(ALL_OSC_FREQ)) * .1f);
    }

    inline float getNextSample(struct OscState *oscState)  {
        struct WaveTable* waveTable = &waveTables[(int) oscillator->shape];

        oscState->index +=  oscState->frequency * waveTable->precomputedValue + waveTable->floatToAdd;

        // convert to int;
        int indexInteger = oscState->index;
        // keep decimal part;
        oscState->index -= indexInteger;
        // Put it back inside the table
        indexInteger &= waveTable->max;
        // Readjust the floating pont inside the table
        oscState->index += indexInteger;

        return waveTable->table[indexInteger];
    }

    inline float getNextSampleHQ(struct OscState *oscState)  {
        struct WaveTable* waveTable = &waveTables[(int) oscillator->shape];

        oscState->index +=  oscState->frequency * waveTable->precomputedValue + waveTable->floatToAdd;

        // convert to int;
        int indexInteger = oscState->index;
        // keep decimal part;
        oscState->index -= indexInteger;
        float fp = oscState->index;
        // Put it back inside the table
        indexInteger &= waveTable->max;
        // Readjust the floating pont inside the table
        oscState->index += indexInteger;

        return waveTable->table[indexInteger]* (1-fp) + waveTable->table[indexInteger+1] * fp;
    }

    inline float getNextSampleMP(struct OscState *oscState)  {
        struct WaveTable* waveTable = &waveTables[(int) oscillator->shape];

        oscState->index +=  oscState->frequency * waveTable->precomputedValue + waveTable->floatToAdd;

        int phase = oscState->phase * waveTable->max * .014;

        // convert to int;
        int indexInteger = oscState->index;
        // keep decimal part;
        oscState->index -= indexInteger;
        // Put it back inside the table
        indexInteger &= waveTable->max;
        // Readjust the floating pont inside the table
        oscState->index += indexInteger;

        phase += indexInteger;
        phase &=  waveTable->max;

        return waveTable->table[phase];
    }

   	float* getNextBlock(struct OscState *oscState)  {
        int shape = (int) oscillator->shape;
   		int max = waveTables[shape].max;
   		float *wave = waveTables[shape].table;
   		float freq = oscState->frequency * waveTables[shape].precomputedValue + waveTables[shape].floatToAdd;
   		float fIndex = oscState->index;
   		int iIndex;
   		float* oscValuesToFill = oscValues[oscValuesCpt];
    	oscValuesCpt++;
    	oscValuesCpt &= 0x3;

   		for (int k=0; k<32; ) {
            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex];

   		}
    	oscState->index = fIndex;
    	return oscValuesToFill;
    };

   	float* getNextBlockHQ(struct OscState *oscState)  {
        int shape = (int) oscillator->shape;
   		int max = waveTables[shape].max;
   		float *wave = waveTables[shape].table;
   		float freq = oscState->frequency * waveTables[shape].precomputedValue + waveTables[shape].floatToAdd;
   		float fIndex = oscState->index;
   		int iIndex;
   		float fp;
   		float* oscValuesToFill = oscValues[oscValuesCpt];
    	oscValuesCpt++;
    	oscValuesCpt &= 0x3;
   		for (int k=0; k<32; ) {
            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            fp = fIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex] * (1-fp) + wave[iIndex]* fp;

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            fp = fIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex] * (1-fp) + wave[iIndex]* fp;

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            fp = fIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex] * (1-fp) + wave[iIndex]* fp;

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            fp = fIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValuesToFill[k++] = wave[iIndex] * (1-fp) + wave[iIndex]* fp;
   		}
    	oscState->index = fIndex;
    	return oscValuesToFill;
    };

private:
    DestinationEnum destFreq;
    Matrix* matrix;
    static float* oscValues[4];
    static int oscValuesCpt;
    OscillatorParams* oscillator;
};

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

#pragma once

#include "SynthStateAware.h"
#include "Matrix.h"

extern float sinTable[];



struct OscState {
    float index;
    float frequency;
    float mainFrequencyPlusMatrix;
    float mainFrequency;
    float fromFrequency;
    float nextFrequency;
};

struct WaveTable {
	float* table;
	int max;
	float useFreq;
	float floatToAdd;
	float precomputedValue;
};

extern struct WaveTable waveTables[];


class Osc : public SynthStateAware
{
public:
    Osc() {};
    virtual ~Osc() {};

    void init(Matrix* matrix, struct OscillatorParams *oscParams, DestinationEnum df);

    void newNote(struct OscState* oscState, int note);
    void glideToNote(struct OscState* oscState, int note);
    void glideStep(struct OscState* oscState, float phase);

    inline void calculateFrequencyWithMatrix(struct OscState *oscState) {
		oscState->mainFrequencyPlusMatrix = oscState->mainFrequency + ((oscState->mainFrequency   * (this->matrix->getDestination(destFreq) + this->matrix->getDestination(ALL_OSC_FREQ)))) * .1f;
    }

    float getNextSample(struct OscState *oscState)  {
        struct WaveTable* waveTable = &waveTables[(int) oscillator->shape];

        oscState->index +=  oscState->frequency * waveTable->precomputedValue + waveTable->floatToAdd;

        // convert to int;
        int indexInteger = oscState->index;
        // keep decimal part;
        oscState->index -= indexInteger;
        // float fp = oscState->index;

        indexInteger &= waveTable->max;
        oscState->index += indexInteger;

        // return waveTable->table[indexInteger] * (1-fp) + waveTable->table[indexInteger+1] * fp;
        return waveTable->table[indexInteger];
    }


   	float* getNextBlock(struct OscState *oscState)  {
        int shape = (int) oscillator->shape;
   		int max = waveTables[shape].max;
   		float *wave = waveTables[shape].table;
   		float freq = oscState->frequency * waveTables[shape].precomputedValue + waveTables[shape].floatToAdd;
   		float fIndex = oscState->index;
   		int iIndex;

   		for (int k=0; k<32; ) {
            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValues[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValues[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValues[k++] = wave[iIndex];

            fIndex +=  freq;
            iIndex = fIndex;
            fIndex -= iIndex;
            iIndex &=  max;
            fIndex += iIndex;
            oscValues[k++] = wave[iIndex];

   		}
    	oscState->index = fIndex;
    	return oscValues;
    };


private:
    DestinationEnum destFreq;
    Matrix* matrix;
    float oscValues[32];
    OscillatorParams* oscillator;
};



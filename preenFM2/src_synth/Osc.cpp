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

#include "Osc.h"

float silence[1]  __attribute__ ((section(".ccmnoload")));
float realSquare[2] = { 1, -1};
float noise[32] __attribute__ ((section(".ccmnoload")));

#include "../waveforms/waves.c"



struct WaveTable waveTables[NUMBER_OF_WAVETABLES] __attribute__ ((section(".ccm"))) = {
//		OSC_SHAPE_SIN = 0,
		{
			sinTable,
			0x7ff,
			1,0
		},
//		OSC_SHAPE_SIN2,
		{
			sinSquareTable,
            0x3ff,
			1,0
		},
//		OSC_SHAPE_SIN3,
		{
			sinOrZeroTable,
            0x3ff,
			1,0
		},
//		OSC_SHAPE_SIN4,
		{
			sinPosTable,
            0x3ff,
			1,0
		},
//		OSC_SHAPE_RAND,
		{
			noise,
            0x1f,
			0,1 //
		},
//		OSC_SHAPE_SQUARE,
		{
			squareTable,
            0x7ff,
			1,0
		},
//		OSC_SHAPE_SAW,
		{
			sawTable,
            0x7ff,
			1,0
		},
		//	OSC_SHAPE_OFF,
		{
			silence,
			0x7ff, // any value works
			0,0
		},
		//	OSC_SHAPE_SUARE 2,
		{
			realSquare,
            0x7ff,
			1,0
		}
};


void Osc::init(Matrix* matrix, struct OscillatorParams *oscParams, DestinationEnum df) {
    this->destFreq = df;

    this->matrix = matrix;

    oscillator = oscParams;

    if (waveTables[0].precomputedValue <= 0) {
        for (int k=0; k<NUMBER_OF_WAVETABLES; k++) {
            waveTables[k].precomputedValue = (waveTables[k].max + 1) * waveTables[k].useFreq * PREENFM_FREQUENCY_INVERSED;
        }
    }
}

void Osc::newNote(struct OscState* oscState, int note) {
    oscState->index = 1; // << number;
    switch ((int)oscillator->frequencyType) {
    case OSC_FT_KEYBOARD:
        oscState->mainFrequency = frequency[note] * oscillator->frequencyMul * (1.0f + oscillator->detune);
        break;
    case OSC_FT_FIXE:
        oscState->mainFrequency = oscillator->frequencyMul* 1000.0f * (1.0f + oscillator->detune);
        break;
    }
    oscState->frequency = oscState->mainFrequency;
}


void Osc::glideToNote(struct OscState* oscState, int note) {
    switch ((int)oscillator->frequencyType) {
    case OSC_FT_KEYBOARD:
        oscState->nextFrequency = frequency[note] * oscillator->frequencyMul * (1.0f + oscillator->detune);
        break;
    case OSC_FT_FIXE:
        oscState->mainFrequency = oscillator->frequencyMul* 1000.0f * (1.0f + oscillator->detune);
        break;
    }
    oscState->fromFrequency = oscState->mainFrequency;
}


void Osc::glideStep(struct OscState* oscState, float phase) {
    oscState->mainFrequency = oscState->fromFrequency * (1 - phase) + oscState->nextFrequency * phase;
}

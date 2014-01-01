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


#include "Timbre.h"

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

const float panTable[] = {
		0.0000, 0.0010, 0.0028, 0.0052, 0.0080, 0.0112, 0.0147, 0.0185, 0.0226, 0.0270,
		0.0316, 0.0365, 0.0416, 0.0469, 0.0524, 0.0581, 0.0640, 0.0701, 0.0764, 0.0828,
		0.0894, 0.0962, 0.1032, 0.1103, 0.1176, 0.1250, 0.1326, 0.1403, 0.1482, 0.1562,
		0.1643, 0.1726, 0.1810, 0.1896, 0.1983, 0.2071, 0.2160, 0.2251, 0.2342, 0.2436,
		0.2530, 0.2625, 0.2722, 0.2820, 0.2919, 0.3019, 0.3120, 0.3222, 0.3326, 0.3430,
		0.3536, 0.3642, 0.3750, 0.3858, 0.3968, 0.4079, 0.4191, 0.4303, 0.4417, 0.4532,
		0.4648, 0.4764, 0.4882, 0.5000, 0.5120, 0.5240, 0.5362, 0.5484, 0.5607, 0.5732,
		0.5857, 0.5983, 0.6109, 0.6237, 0.6366, 0.6495, 0.6626, 0.6757, 0.6889, 0.7022,
		0.7155, 0.7290, 0.7425, 0.7562, 0.7699, 0.7837, 0.7975, 0.8115, 0.8255, 0.8396,
		0.8538, 0.8681, 0.8824, 0.8969, 0.9114, 0.9259, 0.9406, 0.9553, 0.9702, 0.9850,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000} ;

Timbre::Timbre() {
    osc[0] = &osc1;
    osc[1] = &osc2;
    osc[2] = &osc3;
    osc[3] = &osc4;
    osc[4] = &osc5;
    osc[5] = &osc6;

    env[0] = &env1;
    env[1] = &env2;
    env[2] = &env3;
    env[3] = &env4;
    env[4] = &env5;
    env[5] = &env6;

    lfo[0] = &lfoOsc[0];
    lfo[1] = &lfoOsc[1];
    lfo[2] = &lfoOsc[2];
    lfo[3] = &lfoEnv[0];
    lfo[4] = &lfoEnv2[0];
    lfo[5] = &lfoStepSeq[0];
    lfo[6] = &lfoStepSeq[1];


    this->recomputeNext = true;
    this->currentGate = 0;
    this->sbMax = &this->sampleBlock[64];
}

Timbre::~Timbre() {
}

void Timbre::init() {
    struct EnvelopeParamsA* envParamsA[] = { &params.env1a, &params.env2a, &params.env3a, &params.env4a, &params.env5a, &params.env6a};
    struct EnvelopeParamsB* envParamsB[] = { &params.env1b, &params.env2b, &params.env3b, &params.env4b, &params.env5b, &params.env6b};
    struct OscillatorParams* oscParams[] = { &params.osc1, &params.osc2, &params.osc3, &params.osc4, &params.osc5, &params.osc6};
    struct LfoParams* lfoParams[] = { &params.lfoOsc1, &params.lfoOsc2, &params.lfoOsc3};
    struct StepSequencerParams* stepseqparams[] = { &params.lfoSeq1, &params.lfoSeq2};
    struct StepSequencerSteps* stepseqs[] = { &params.lfoSteps1, &params.lfoSteps2};

    matrix.init(&params.matrixRowState1);

    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        env[k]->init(&matrix, envParamsA[k],  envParamsB[k], (DestinationEnum)(ENV1_ATTACK + k));
        osc[k]->init(&matrix, oscParams[k], (DestinationEnum)(OSC1_FREQ + k));
    }
    // OSC
    for (int k = 0; k < NUMBER_OF_LFO_OSC; k++) {
        lfoOsc[k].init(lfoParams[k], &this->matrix, (SourceEnum)(MATRIX_SOURCE_LFO1 + k), (DestinationEnum)(LFO1_FREQ + k));
    }

    // ENV
    lfoEnv[0].init(&params.lfoEnv1 , &this->matrix, MATRIX_SOURCE_LFOENV1, (DestinationEnum)0);
    lfoEnv2[0].init(&params.lfoEnv2 , &this->matrix, MATRIX_SOURCE_LFOENV2, (DestinationEnum)LFOENV2_SILENCE);

    // Step sequencer
    for (int k = 0; k< NUMBER_OF_LFO_STEP; k++) {
        lfoStepSeq[k].init(stepseqparams[k], stepseqs[k], &matrix, (SourceEnum)(MATRIX_SOURCE_LFOSEQ1+k), (DestinationEnum)(LFOSEQ1_GATE+k));
    }
}

void Timbre::prepareForNextBlock() {
    this->lfo[0]->nextValueInMatrix();
    this->lfo[1]->nextValueInMatrix();
    this->lfo[2]->nextValueInMatrix();
    this->lfo[3]->nextValueInMatrix();
    this->lfo[4]->nextValueInMatrix();
    this->lfo[5]->nextValueInMatrix();
    this->lfo[6]->nextValueInMatrix();

    this->matrix.computeAllFutureDestintationAndSwitch();

    updateAllModulationIndexes();
    updateAllMixOscsAndPans();

    float *sp = this->sampleBlock;
    while (sp < this->sbMax) {
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
        *sp++ = 0;
    }
}

#define GATE_INC 0.02f

void Timbre::fxAfterBlock() {
    // Gate algo !!
    float gate = this->matrix.getDestination(MAIN_GATE);
    if (gate <= 0 && currentGate <= 0) {
        return;
    }
    gate *=.72547132656922730694f; // 0 < gate < 1.0
    if (gate > 1.0f) {
        gate = 1.0f;
    }
    float incGate = (gate - currentGate) * .03125f; // ( *.03125f = / 32)
    // limit the speed.
    if (incGate > 0.002f) {
        incGate = 0.002f;
    } else if (incGate < -0.002f) {
        incGate = -0.002f;
    }

    float *sp = this->sampleBlock;
    float coef;
    while (sp < this->sbMax) {
        currentGate += incGate;
        coef = 1.0f - currentGate;
        *sp = *sp * coef;
        sp++;
        *sp = *sp * coef;
        sp++;
    }
    //    currentGate = gate;
}


void Timbre::afterNewParamsLoad() {
    this->matrix.resetSources();
    this->matrix.resetAllDestination();
    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        for (int j=0; j<NUMBER_OF_ENCODERS; j++) {
        	this->env[k]->reloadADSR(j);
        	this->env[k]->reloadADSR(j+4);
        }
    }
    for (int k=0; k<NUMBER_OF_LFO; k++) {
        for (int j=0; j<NUMBER_OF_ENCODERS; j++) {
        	this->lfo[k]->valueChanged(j);
        }
    }
}

void Timbre::setNewValue(int index, struct ParameterDisplay* param, float newValue) {
    if (newValue > param->maxValue) {
        newValue= param->maxValue;
    } else if (newValue < param->minValue) {
        newValue= param->minValue;
    }
    ((float*)&params)[index] = newValue;
}

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

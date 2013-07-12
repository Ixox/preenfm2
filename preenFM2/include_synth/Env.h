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

#include "Common.h"
#include "Matrix.h"

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

enum EnvState {
    ENV_STATE_ON_A = 0,
    ENV_STATE_ON_D,
    ENV_STATE_ON_S,
    ENV_STATE_ON_R,
    ENV_STATE_DEAD,
    ENV_STATE_ON_QUICK_R,
    ENV_NUMBER_OF_STATES
};


struct EnvData {
    // State of the env
    int envState;
    // Current sample
    float currentValue;
    // previous state value and next state value
    float previousStateValue, nextStateValue;
    // currentPhase
    float currentPhase;
};


class Env
{
public:
    Env() {
        stateTarget[ENV_STATE_ON_A] = 1.0f;
        stateTarget[ENV_STATE_ON_R] = 0.0f;
        stateTarget[ENV_STATE_ON_QUICK_R] = 0.0f;
        stateTarget[ENV_STATE_DEAD] = 0.0f;

        stateInc[ENV_STATE_ON_S] = 0.0f;
        stateInc[ENV_STATE_DEAD] = 0.0f;
        // quick release : 2 steps = 64 samples...
        stateInc[ENV_STATE_ON_QUICK_R] = 0.5f;

        nextState[ENV_STATE_ON_A] = ENV_STATE_ON_D;
        nextState[ENV_STATE_ON_D] = ENV_STATE_ON_S;
        nextState[ENV_STATE_ON_S] = ENV_STATE_ON_S;
        nextState[ENV_STATE_ON_R] = ENV_STATE_DEAD;
        nextState[ENV_STATE_ON_QUICK_R] = ENV_STATE_DEAD;
    }

    virtual ~Env(void) {
    }

    void init(Matrix* matrix, struct EnvelopeParams *envParams, DestinationEnum da);

    void reloadADSR(int encoder) {
        switch (encoder) {
        case 0:
            // Not sure it's necessary... recalculated in noteOn....
            // stateInc[ENV_STATE_ON_A] = incTabAtt[(int)(envParams->attack * 100)];
            break;
        case 1:
            stateInc[ENV_STATE_ON_D] = incTabRel[(int)(envParams->decay * 50)];
            break;
        case 2:
            stateTarget[ENV_STATE_ON_D] =  envParams->sustain;
            stateTarget[ENV_STATE_ON_S] =  envParams->sustain;
            break;
        case 3:
            // Not sure it's necessary... reaclculated in noteOn....
            stateInc[ENV_STATE_ON_R] = incTabRel[(int)(envParams->release * 50)];
            break;
        }
    }

    void newState(struct EnvData* env) {

        if (env->envState == ENV_STATE_DEAD) {
            env->currentValue = 0;
        }
        env->previousStateValue = env->currentValue;
        env->nextStateValue = stateTarget[env->envState];
        env->currentPhase = 0;
    }

    inline float getNextAmp(struct EnvData* env) {
        env->currentPhase += stateInc[env->envState];

        if (env->currentPhase  >= 1.0f) {
            env->currentValue = env->nextStateValue;
            // Next state
            env->envState = nextState[env->envState];
            newState(env);
        } else if (stateInc[env->envState] > 0) {
            env->currentValue = env->previousStateValue * (1- env->currentPhase) + env->nextStateValue * env->currentPhase;
        }
        return env->currentValue;
      }

    void noteOn(struct EnvData* env) {
        float attack = envParams->attack + this->matrix->getDestination(destAttack) + this->matrix->getDestination(ALL_ENV_ATTACK);
        if (attack > 2.0f) {
            attack = 2.0f;
        } else if (attack < 0.0f) {
            attack = 0.0f;
        }
        stateInc[ENV_STATE_ON_A] = incTabAtt[(int)(attack * 100)];

        float release = envParams->release; // + this->matrix->getDestination(ALL_ENV_RELEASE);

        stateInc[ENV_STATE_ON_R] = incTabRel[(int)(release * 50)];

        env->currentValue = 0;
        env->envState = ENV_STATE_ON_A;
        newState(env);
    }

    void noteOffQuick(struct EnvData* env) {
        env->envState = ENV_STATE_ON_QUICK_R;
        newState(env);
    }

    void noteOff(struct EnvData* env) {
        env->envState = ENV_STATE_ON_R;
        newState(env);
    }


    bool isDead(struct EnvData* env)  __attribute__((always_inline))  {
        return env->envState == ENV_STATE_DEAD;
    }

private:
    // target values of ADSR
    float stateTarget[ENV_NUMBER_OF_STATES];
    // float
    float stateInc[ENV_NUMBER_OF_STATES];

    int nextState[ENV_NUMBER_OF_STATES];

    EnvelopeParams* envParams;
    Matrix* matrix;
    DestinationEnum destAttack;

    static int initTab;
    static float incTabAtt[201];
    static float incTabRel[201];
};


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
    ENV_STATE_ON_A,
    ENV_STATE_ON_D,
    ENV_STATE_ON_S,
    ENV_STATE_ON_REAL_S,
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

struct table {
	float* table;
	int size;
};

extern float envLinear[];

extern float envExponential[];

class Env
{
public:
    Env() {
        stateTarget[ENV_STATE_ON_QUICK_R] = 0.0f;
        stateTarget[ENV_STATE_DEAD] = 0.0f;

        stateInc[ENV_STATE_DEAD] = 0.0f;
        stateInc[ENV_STATE_ON_REAL_S] = 0.0f;
        // quick release : 4 steps = 128 samples...
        stateInc[ENV_STATE_ON_QUICK_R] = 0.25f;

        nextState[ENV_STATE_ON_A] = ENV_STATE_ON_D;
        nextState[ENV_STATE_ON_D] = ENV_STATE_ON_S;
        nextState[ENV_STATE_ON_S] = ENV_STATE_ON_REAL_S;
        nextState[ENV_STATE_ON_REAL_S] = ENV_STATE_ON_REAL_S;
        nextState[ENV_STATE_ON_R] = ENV_STATE_DEAD;
        nextState[ENV_STATE_ON_QUICK_R] = ENV_STATE_DEAD;

        tables[ENV_STATE_ON_A].table = envLinear;
        tables[ENV_STATE_ON_A].size = 1;
        tables[ENV_STATE_ON_D].table = envExponential;
        tables[ENV_STATE_ON_D].size = 63;
        tables[ENV_STATE_ON_S].table = envExponential;
        tables[ENV_STATE_ON_S].size = 63;
        tables[ENV_STATE_ON_R].table = envExponential;
        tables[ENV_STATE_ON_R].size = 63;
        tables[ENV_STATE_ON_QUICK_R].table = envLinear;
        tables[ENV_STATE_ON_QUICK_R].size = 63;
    }

    virtual ~Env(void) {
    }

    void init(Matrix* matrix, struct EnvelopeParamsA *envParamsA, struct EnvelopeParamsB *envParamsB, DestinationEnum da);

    void reloadADSR(int encoder) {
    	// 0 Attack time
    	// 1 Attack rate
    	// 2 Decay time
    	// 3 Decay rate
    	// 4 Sustain time
    	// 5 Sustain rate
    	// 6 Release time
    	// 7 Release rate
        switch (encoder) {
        case 0:
        case 1:
        	stateTarget[ENV_STATE_ON_A] =  envParamsA->attackLevel;
        	stateInc[ENV_STATE_ON_A] = incTabAtt[(int)(envParamsA->attackTime * 50)];
            break;
        case 2:
        case 3:
        	stateTarget[ENV_STATE_ON_D] =  envParamsA->decayLevel;
        	stateInc[ENV_STATE_ON_D] = incTabRel[(int)(envParamsA->decayTime * 25)];
            break;
        case 4:
        case 5:
        	stateTarget[ENV_STATE_ON_S] =  envParamsB->sustainLevel;
        	stateInc[ENV_STATE_ON_S] = incTabRel[(int)(envParamsB->sustainTime * 25)];
        	break;
        case 6:
        case 7:
        	stateTarget[ENV_STATE_ON_R] =  envParamsB->releaseLevel;
            // Not sure it's necessary... recalculated in noteOn....
            stateInc[ENV_STATE_ON_R] = incTabRel[(int)(envParamsB->releaseTime * 25)];
            break;
        }
    }

    void newState(struct EnvData* env) {

        if (env->envState == ENV_STATE_DEAD) {
            // env->currentValue = 0;
        }
        env->previousStateValue = env->currentValue;
        env->nextStateValue = stateTarget[env->envState];
        env->currentPhase = 0;
    }

    inline float getNextAmp(struct EnvData* env) {
		env->currentPhase += stateInc[env->envState];

		if (env->currentPhase  >= 1.0f) {
			// Next state
			env->currentValue = env->nextStateValue;
			newState(env);
		} else if (stateInc[env->envState] > 0) {
			env->currentValue = env->previousStateValue * (1- env->currentPhase) + env->nextStateValue * env->currentPhase;
		}
        return env->currentValue;
      }


    inline float getNextAmpExp(struct EnvData* env) {
		env->currentPhase += stateInc[env->envState];

		if (env->currentPhase  >= 1.0f) {
			// Next state
			env->currentValue = env->nextStateValue;
			env->envState = nextState[env->envState];
			newState(env);
		} else if (stateInc[env->envState] > 0) {
			float fIndex = env->currentPhase * tables[env->envState].size;
			int index = (int)fIndex;

			float floatingPart = fIndex - index;
			float from = tables[env->envState].table[index];
			float to = tables[env->envState].table[index + 1];
			float tmpValue = from * (1.0f - floatingPart) + to *  floatingPart;
			// When table value is 0 => next value
			// When table value is 1 => previous value
			// see table
			env->currentValue = tmpValue * (env->nextStateValue - env->previousStateValue) + env->previousStateValue;
		}
        return env->currentValue;
      }



    void noteOn(struct EnvData* env) {

    	float attack = envParamsA->attackTime + this->matrix->getDestination(destAttack) + this->matrix->getDestination(ALL_ENV_ATTACK);
    	if (attack < 0.0f) {
            attack = 0.0f;
        }
        stateInc[ENV_STATE_ON_A] = incTabAtt[(int)(attack * 50)];


        env->currentValue = 0;
        env->envState = ENV_STATE_ON_A;
        newState(env);
    }

    void noteOffQuick(struct EnvData* env) {
        env->envState = ENV_STATE_ON_QUICK_R;
        newState(env);

        int duration = 8 * env->currentValue ;

        if (duration == 0) {
        	stateInc[ENV_STATE_ON_QUICK_R] = 1.0f;
        } else {
        	stateInc[ENV_STATE_ON_QUICK_R] = 1.0f / (float)duration;
        }

    }

    void noteOff(struct EnvData* env) {
        float release = envParamsB->releaseTime + this->matrix->getDestination(ALL_ENV_RELEASE);
    	if (release < 0.0f) {
    		release = 0.0f;
        }
    	// The lower we are the shorter the release is...
    	if (env->currentValue > stateTarget[ENV_STATE_ON_R]) {
    		release *= (env->currentValue -  stateTarget[ENV_STATE_ON_R]);
    	} else {
    		release *= (stateTarget[ENV_STATE_ON_R] - env->currentValue);
    	}
        stateInc[ENV_STATE_ON_R] = incTabRel[(int)(release * 25)];

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
    struct table tables[ENV_NUMBER_OF_STATES];

    EnvelopeParamsA* envParamsA;
    EnvelopeParamsB* envParamsB;
    Matrix* matrix;
    DestinationEnum destAttack;

    static int initTab;
    static float incTabAtt[201];
    static float incTabRel[201];
};


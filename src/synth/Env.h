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
    ENV_STATE_ON_REAL_S,
    ENV_STATE_ON_R,
    ENV_STATE_DEAD,
    ENV_STATE_ON_QUICK_R,
    ENV_NUMBER_OF_STATES
};


struct EnvData {
    // State of the env
    uint8_t envState;
    // Current sample
    float currentValue;
    // previous state value and next state value
    float previousStateValue, nextStateValue;
    // currentPhase
    float currentPhase;
    // Inc attack and Inc Release is per voice since firmware 2.0
    float stateIncAttack;
    float stateIncRelease;
};

struct table {
	float* table;
	uint8_t size;
};

extern float envLinear[];

extern float envExponential[];

class Env
{
public:
    Env() {
        stateTarget[ENV_STATE_ON_QUICK_R] = 0.0f;
        stateTarget[ENV_STATE_DEAD] = 0.0f;

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
        tables[ENV_STATE_ON_QUICK_R].size = 1;
    }

    virtual ~Env(void) {
    }

    void init(struct EnvelopeParamsA *envParamsA, struct EnvelopeParamsB *envParamsB, uint8_t envNumber);

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
            // Not necessary... recalculated in noteOn....
        	// stateInc[ENV_STATE_ON_A] = incTab[(int)(envParamsA->attackTime * 100.0f)];
            break;
        case 2:
        case 3:
        	stateTarget[ENV_STATE_ON_D] =  envParamsA->decayLevel;
        	stateInc[ENV_STATE_ON_D] = incTab[(int)(envParamsA->decayTime * 100.0f)];
            break;
        case 4:
        case 5:
        	stateTarget[ENV_STATE_ON_S] =  envParamsB->sustainLevel;
        	stateInc[ENV_STATE_ON_S] = incTab[(int)(envParamsB->sustainTime * 100.0f)];
        	break;
        case 6:
        case 7:
        	stateTarget[ENV_STATE_ON_R] =  envParamsB->releaseLevel;
            // Not necessary... recalculated in noteOff....
        	// stateInc[ENV_STATE_ON_R] = incTab[(int)(envParamsB->releaseTime * 100.0f)];
            break;
        }
    }

    void newState(struct EnvData* env) {

        env->previousStateValue = env->currentValue;
        env->nextStateValue = stateTarget[env->envState];
        env->currentPhase = 0;
    }


    inline float getNextAmpExp(struct EnvData* env) {
        float incPhase;

        switch(env->envState) {
        case ENV_STATE_ON_A:
            incPhase = env->stateIncAttack;
            break;
        case ENV_STATE_ON_R:
            incPhase = env->stateIncRelease;
            break;
        default:
            incPhase = stateInc[env->envState];
            break;
        }
		env->currentPhase += incPhase;

		if (unlikely(env->currentPhase  >= 1.0f)) {
			env->currentValue = env->nextStateValue;
			env->envState = nextState[env->envState];
			newState(env);

	        return env->currentValue;
		}

		if (likely(incPhase > 0)) {
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
			return env->currentValue;
		}

		return env->currentValue;
      }

    void noteOnAfterMatrixCompute(struct EnvData* env, Matrix* matrix) {
        //
        env->currentValue = 0;
        env->envState = ENV_STATE_ON_A;
        newState(env);

        float attack = envParamsA->attackTime + matrix->getDestination(ALL_ENV_ATTACK) + matrix->getDestination((enum DestinationEnum)(ENV1_ATTACK + envNumber));
        if (attack < 0.0f) {
            attack = 0.0f;
        }
        //stateInc[ENV_STATE_ON_A] = incTab[(int)(attack * 100.0f)];
        env->stateIncAttack = incTab[(int)(attack * 100.0f)];
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

    void noteOff(struct EnvData* env, Matrix* matrix) {
        float release = envParamsB->releaseTime + matrix->getDestination(ALL_ENV_RELEASE);
    	if (release < 0.0f) {
    		release = 0.0f;
        }
    	// The lower we are the shorter the release is...
    	if (env->currentValue > stateTarget[ENV_STATE_ON_R]) {
    		release *= (1.0f + (env->currentValue -  stateTarget[ENV_STATE_ON_R])) / 2.0f;
    	} else {
    		release *= (1.0f + (stateTarget[ENV_STATE_ON_R] - env->currentValue)) / 2.0f;
    	}
        env->stateIncRelease = incTab[(int)(release * 100.0f)];
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
    uint8_t nextState[ENV_NUMBER_OF_STATES];
    struct table tables[ENV_NUMBER_OF_STATES];

    EnvelopeParamsA* envParamsA;
    EnvelopeParamsB* envParamsB;
    uint8_t envNumber;


    static int initTab;
    static float incTab[1601];
};


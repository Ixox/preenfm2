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

/*
 * Eveloppe with 3 states
 * S = silence
 * A = attack
 * R = release
 * After release go to S or finished
 */

#ifndef LFOENV2_H_
#define LFOENV2_H_

#include "Lfo.h"
#include "Env.h"

enum Env2State {
    ENV2_STATE_ON_S = 0,
    ENV2_STATE_ON_A,
    ENV2_STATE_ON_D,
    ENV2_STATE_DEAD,
    ENV2_NUMBER_OF_STATES
};

class LfoEnv2: public Lfo {
public:
    LfoEnv2();

	void init(struct Envelope2Params * envParams, Matrix* matrix, SourceEnum source, DestinationEnum dest);

	void valueChanged(int encoder) {
        switch (encoder) {
        case 0:
            stateInc[ENV2_STATE_ON_S] = 0.0008f / (envParams->silence + 0.0001);
            break;
        case 1:
            stateInc[ENV2_STATE_ON_A] = 0.0008f / (envParams->attack + 0.0001);
            break;
        case 2:
            stateInc[ENV2_STATE_ON_D] = 0.0008f / (envParams->decay + 0.0001);
            break;
        }
	}

    void newState() {
        if (env.envState == ENV2_STATE_DEAD) {
            env.currentValue = 0;
            if (envParams->loop > 1) {
                env.envState = ENV2_STATE_ON_A;
            } else if (envParams->loop > 0) {
                env.envState = ENV2_STATE_ON_S;
            }
        }
        env.previousStateValue = env.currentValue;
        env.nextStateValue = stateTarget[env.envState];
        env.currentPhase = 0;
        if (env.envState == ENV2_STATE_ON_S) {
            if (this->matrix->getDestination(this->destination) != 0) {
                float silencePlusMatrix = envParams->silence + this->matrix->getDestination(this->destination);
                if (silencePlusMatrix <= 0) {
                    stateInc[ENV2_STATE_ON_S] = 1.0;
                } else if (silencePlusMatrix > 8) {
                    stateInc[ENV2_STATE_ON_S] = 0.0001f ;
                } else {
                    stateInc[ENV2_STATE_ON_S] = 0.0008f / silencePlusMatrix;
                }
            }
        }
    }



    void nextValueInMatrix() {
        env.currentPhase += stateInc[env.envState];

        if (env.currentPhase  >= 1.0f) {
            env.currentValue = env.nextStateValue;
            env.envState++;
            newState();
        } else if (stateInc[env.envState] > 0) {
            env.currentValue = env.previousStateValue * (1- env.currentPhase) + env.nextStateValue * env.currentPhase;
        }

		matrix->setSource(source, env.currentValue);
	}

	void noteOn() {
        // index is decremented in the first call...
        env.currentValue = 0;
        env.envState = ENV2_STATE_ON_S;
        newState();
	}

	void noteOff() {
	}


private:
    // target values of ADSR
    float stateTarget[ENV2_NUMBER_OF_STATES];
    // float
    float stateInc[ENV2_NUMBER_OF_STATES];

    Envelope2Params* envParams;
	EnvData env;
};

#endif /* LfoEnv2_H_ */

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

#ifndef LFOENV_H_
#define LFOENV_H_

#include "Lfo.h"
#include "Env.h"


class LfoEnv: public Lfo {
public:
    LfoEnv();

	void init(struct EnvelopeParams * envParams, Matrix* matrix, SourceEnum source, DestinationEnum dest);

	void valueChanged(int encoder) {
        switch (encoder) {
        case 0:
            stateInc[ENV_STATE_ON_A] = 0.0008f / (envParams->attack + 0.0001);
            break;
        case 1:
            stateInc[ENV_STATE_ON_D] = 0.0008f / (envParams->decay + 0.0001);
            break;
        case 2:
            stateTarget[ENV_STATE_ON_D] =  envParams->sustain;
            stateTarget[ENV_STATE_ON_S] =  envParams->sustain;
            break;
        case 3:
            stateInc[ENV_STATE_ON_R] = 0.0008f / (envParams->release + 0.0001);
            break;
        }	}

    void newState() {

        if (env.envState == ENV_STATE_DEAD) {
            env.currentValue = 0;
        }
        env.previousStateValue = env.currentValue;
        env.nextStateValue = stateTarget[env.envState];
        env.currentPhase = 0;
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
        env.envState = ENV_STATE_ON_A;
        newState();
	}

	void noteOff() {
		env.envState = ENV_STATE_ON_R;
		newState();
	}


private:
    // target values of ADSR
    float stateTarget[ENV_NUMBER_OF_STATES];
    // float
    float stateInc[ENV_NUMBER_OF_STATES];

    EnvelopeParams* envParams;
	EnvData env;
};

#endif /* LFOENV_H_ */

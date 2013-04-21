/*
 * Copyright 2012 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <dot> hosxe (at) gmail <dot> com)
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

#include "LfoEnv.h"

LfoEnv::LfoEnv() {
    stateTarget[ENV_STATE_ON_A] = 1.0f;
    stateTarget[ENV_STATE_ON_R] = 0.0f;
    stateTarget[ENV_STATE_DEAD] = 0.0f;
    stateInc[ENV_STATE_ON_S] = 0.0f;
    stateInc[ENV_STATE_DEAD] = 0.0f;
}


void LfoEnv::init(struct EnvelopeParams * envParams, Matrix *matrix, SourceEnum source, DestinationEnum dest) {
	Lfo::init(matrix, source, dest);
	this->envParams = envParams;
	this->valueChanged(0);
    this->valueChanged(1);
    this->valueChanged(2);
    this->valueChanged(3);
}

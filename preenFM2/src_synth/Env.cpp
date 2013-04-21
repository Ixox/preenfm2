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

#include "Env.h"

// Standard 128kb memory
float Env::incTabAtt[201];
float Env::incTabRel[201];
int Env::initTab = 0;

void Env::init(Matrix* matrix, struct EnvelopeParams *envParams, DestinationEnum da) {
    this->destAttack = da;
    this->matrix = matrix;
	this->envParams = envParams;

    reloadADSR(0);
    reloadADSR(1);
    reloadADSR(2);
    reloadADSR(3);
    if (initTab == 0) {
        initTab = 1;
        for (float k=0; k<201; k+=1) {
            incTabAtt[(int)k] = BLOCK_SIZE / PREENFM_FREQUENCY / (k / 100.0f + .0005f);
            incTabRel[(int)k] = BLOCK_SIZE / PREENFM_FREQUENCY / (k / 50.0f + .0005f);
        }
    }
}



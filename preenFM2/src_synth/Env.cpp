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

#include "Env.h"

// Standard 128kb memory
float Env::incTabAtt[201];
float Env::incTabRel[201];
int Env::initTab = 0;


float envLinear[] = { 0.0f, 1.0f};

float envExponential[] = {
		0.000000,0.062148,0.120174,0.174684,0.225892,0.273997,0.319188,0.361640,
		0.401521,0.438985,0.474180,0.507242,0.538301,0.567478,0.594887,0.620636,
		0.644825,0.667548,0.688895,0.708948,0.727786,0.745483,0.762108,0.777725,
		0.792397,0.806179,0.819126,0.831289,0.842715,0.853449,0.863532,0.873005,
		0.881903,0.890263,0.898115,0.905493,0.912423,0.918933,0.925049,0.930794,
		0.936192,0.941262,0.946025,0.950499,0.954703,0.958651,0.962361,0.965846,
		0.969119,0.972195,0.975083,0.977797,0.980347,0.982742,0.984992,0.987105,
		0.989091,0.990956,0.992708,0.994354,0.995901,0.997353,0.998718,1.000000
};


void Env::init(Matrix* matrix, struct EnvelopeParamsA *envParamsA, struct EnvelopeParamsB *envParamsB, DestinationEnum da) {
    this->destAttack = da;
    this->matrix = matrix;
	this->envParamsA = envParamsA;
	this->envParamsB = envParamsB;

	// Init All ADSR
	for (int k=0; k<4; k++) {
		reloadADSR(k);
		reloadADSR(k + 4);
	}
    if (initTab == 0) {
        initTab = 1;
        for (float k=0; k<201; k+=1) {
            incTabAtt[(int)k] = BLOCK_SIZE / PREENFM_FREQUENCY / (k / 50.0f + .0005f);
            incTabRel[(int)k] = BLOCK_SIZE / PREENFM_FREQUENCY / (k / 25.0f + .0005f);
        }
    }
}



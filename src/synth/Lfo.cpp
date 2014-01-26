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

#include "Lfo.h"


// Standard 128kb memory
float Lfo::invTab[2048];
int Lfo::initTab = 0;

Lfo::Lfo() {
}

void Lfo::init(Matrix *matrix, SourceEnum source, DestinationEnum dest) {
    this->destination = dest;
	this->matrix = matrix;
	this->source = source;
	this->index = 0;

    if (initTab == 0) {
        initTab = 1;
        for (float k=1; k<2048; k += 1.0f) {
            invTab[(int)(k + .005f)] = 1 / k;
        }
        invTab[0] = 10000;
    }
}

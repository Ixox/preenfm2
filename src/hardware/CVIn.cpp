/*
 * Copyright 2018 Xavier Hosxe
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

#include "CVIn.h"


extern float *frequencyToUse;


CVIn::CVIn() {
    a = .1f;
    b = 8.0f;
    gate = 0;
    midiNote = 64;
    midiNote1024 = 512;
    cvin1 = 0.0f;
    cvin2 = 0.0f;

}

void CVIn::updateValues() {
    float divBy2048 = .001953125f * .25f;


    int cvin = ADCBuffer[0] + ADCBuffer[4] + ADCBuffer[8] + ADCBuffer[12];
    this->gate = cvin >> 2;
    this->cvin1 = ((float)cvin) * divBy2048 - 1.0f;

    cvin = ADCBuffer[1] + ADCBuffer[5] + ADCBuffer[9] + ADCBuffer[13];
    this->midiNote1024 = cvin >> 2;
    this->midiNote = ((float)this->midiNote1024) * a;
    this->midiNote += b;
    this->cvin2 = ((float)cvin) * divBy2048 - 1.0f;

    cvin = ADCBuffer[2] + ADCBuffer[6] + ADCBuffer[10] + ADCBuffer[14];
    this->cvin3 = ((float)cvin) * divBy2048 - 1.0f;

    cvin = ADCBuffer[3] + ADCBuffer[7] + ADCBuffer[11] + ADCBuffer[15];
    this->cvin4 = ((float)cvin) * divBy2048 - 1.0f;


    float notef = this->midiNote;
    int notei = (int) notef;
    notef -= notei;
    if (notei >= 127) {
        notei = 126;
        notef = 1.0f;
    }
    this->frequency = frequencyToUse[notei] * (1.0f - notef) + frequencyToUse[notei + 1 ] * notef;
}

void CVIn::updateFormula(short newA2, short newA6) {
    this->a2 = newA2;
    this->a6 = newA6;
    // A2 is midi note 33
    // A6 is midi note 81
    this->a = 48.0f / (float)(this->a6 - this->a2);
    this->b = 81.0f - this->a * (float)this->a6;
}


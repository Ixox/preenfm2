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

#include "Common.h"
#include "SynthMenuListener.h"

#ifdef CVIN

#ifndef CVIN_H_
#define CVIN_H_



class CVIn {
public:
    CVIn();
    void updateValues();
    void updateFormula(short a2, short a6);

    const float getMidiNote() const { return midiNote; }
    const uint16_t getMidiNote1024() const { return midiNote1024; }
    const uint16_t getGate() const { return gate; }
    const float getCvin1() const { return cvin1; }
    const float getCvin2() const { return cvin2; }
    const float getCvin3() const { return cvin3; }
    const float getCvin4() const { return cvin4; }
    const float getFrequency() const { return frequency; }
    uint32_t getADCBufferAdress() { return (uint32_t)&ADCBuffer[0]; }

private:
    uint16_t ADCBuffer[16];
    float frequency;
    float midiNote;
    uint16_t midiNote1024;
    uint16_t gate;
    float cvin1, cvin2, cvin3, cvin4;
    float a, b;
    short a2, a6;
};

#endif /* CVIN_H_ */


#endif /* CVIN */

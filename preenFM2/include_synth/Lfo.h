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

#ifndef LFO_H_
#define LFO_H_

#include "SynthStateAware.h"
#include "Matrix.h"



#define NUMBER_OF_LFO_OSC 3
#define NUMBER_OF_LFO_ENV 1
#define NUMBER_OF_LFO_ENV2 1
#define NUMBER_OF_LFO_STEP 2

#define NUMBER_OF_LFO NUMBER_OF_LFO_OSC+NUMBER_OF_LFO_ENV+NUMBER_OF_LFO_ENV2+NUMBER_OF_LFO_STEP

class Lfo {
public:
	Lfo();
	virtual ~Lfo() {}
	virtual void init(Matrix* matrix, SourceEnum source, DestinationEnum dest);
	virtual void valueChanged(int encoder) = 0;
	virtual void nextValueInMatrix() = 0 ;
	virtual void noteOn() = 0;
	virtual void noteOff() = 0;
	virtual void midiClock(int songPosition, bool computeStep) {}
	void midiContinue() {
		ticks = 0;
	}


protected:
	Matrix *matrix;
    DestinationEnum destination;
	SourceEnum source;
	int index;
	// Midi Clock sync
	int ticks;
};

#endif /* LFO_H_ */

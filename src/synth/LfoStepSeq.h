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

#ifndef LFOSTEPSEQ_H_
#define LFOSTEPSEQ_H_

#include "Lfo.h"



class LfoStepSeq: public Lfo {
public:
	void init(struct StepSequencerParams* stepSeqParam, struct StepSequencerSteps* stepSeqSteps, Matrix* matrix, SourceEnum source, DestinationEnum dest);
	void valueChanged(int encoder);
	void nextValueInMatrix();
	void noteOn();
	void noteOff();
	void midiClock(int songPosition, bool computeStep);


private:
	StepSequencerParams* seqParams;
	StepSequencerSteps* seqSteps;

    float phase;
    float phaseStep;
    int gateValue;
    int target;
    int currentValue;
    bool gated;
    DestinationEnum matrixGateDestination;
    DestinationEnum startSource;
};

#endif /* LFOENV_H_ */
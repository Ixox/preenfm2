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

#include "Voice.h"

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

float Voice::glidePhaseInc[10];

Voice::Voice(void)
{
	this->envState[0] = &envState1;
	this->envState[1] = &envState2;
	this->envState[2] = &envState3;
	this->envState[3] = &envState4;
	this->envState[4] = &envState5;
	this->envState[5] = &envState6;

	this->oscState[0] = &oscState1;
	this->oscState[1] = &oscState2;
	this->oscState[2] = &oscState3;
	this->oscState[3] = &oscState4;
	this->oscState[4] = &oscState5;
	this->oscState[5] = &oscState6;


	if (glidePhaseInc[0] != .2f) {
		float tmp[] = {
				5.0f,
				9.0f,
				15.0f,
				22.0f,
				35.0f,
				50.0f,
				90.0f,
				140.0f,
				200.0f,
				500.0f
		};
		for (int k = 0 ; k <10 ; k++) {
			glidePhaseInc[k] = 1.0f/tmp[k];
		}
	}
}


Voice::~Voice(void)
{
}

void Voice::init(Timbre* timbre0, Timbre* timbre1, Timbre* timbre2, Timbre* timbre3) {
	this->voiceTimbre = 0;
	this->timbres[0] = timbre0;
	this->timbres[1] = timbre1;
	this->timbres[2] = timbre2;
	this->timbres[3] = timbre3;
	this->currentTimbre = this->timbres[0];
	this->playing = false;
	this->newNotePending = false;
	this->note = 0;
	this->holdedByPedal = false;
}


void Voice::glideToNote(short newNote) {
	// Must glide...
	this->gliding = true;
	this->glidePhase = 0.0f;
	this->nextGlidingNote = newNote;
	if (this->holdedByPedal) {
		glideFirstNoteOff();
	}

	for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
		currentTimbre->osc[k]->glideToNote(oscState[k], newNote);
	}
}

void Voice::noteOnWithoutPop(short newNote, short velocity, unsigned int index) {
	// Update index : so that few chance to be chosen again during the quick dying
	this->index = index;
	if (!this->released && (int)currentTimbre->params.engine1.numberOfVoice == 1 && currentTimbre->params.engine1.glide > 0) {
		glideToNote(newNote);
		this->holdedByPedal = false;
	} else {
		// update note now so that the noteOff is triggered by the new note
		this->note = newNote;
		// Quick dead !
		this->newNotePending = true;
		this->nextVelocity = velocity;
		this->nextPendingNote = newNote;
		// Not release anymore... not available for new notes...
		this->released = false;
		for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
			this->currentTimbre->env[k]->noteOffQuick(envState[k]);
		}
	}
}

void Voice::glide() {
	if (!this->gliding) {
		return;
	}
	this->glidePhase += glidePhaseInc[(int)(currentTimbre->params.engine1.glide - .95f)];
	if (glidePhase < 1.0f) {

		for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
			currentTimbre->osc[k]->glideStep(oscState[k], this->glidePhase);
		}
	} else {
		// last with phase set to 1 to have exact frequencry
		for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
			currentTimbre->osc[k]->glideStep(oscState[k], 1.0f);
		}
		this->gliding = false;
	}
}

void Voice::noteOn(short timbre, short newNote, short velo, unsigned int index) {
	this->voiceTimbre = timbre;
	this->currentTimbre = timbres[timbre];

	this->released = false;
	this->playing = true;
	this->note = newNote;
	this->nextPendingNote = 0;
	this->newNotePending = false;
	this->holdedByPedal = false;
	this->index = index;
	this->velocity = (float)velo * .0078125f; // divide by 127


	for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
		currentTimbre->osc[k]->newNote(oscState[k], newNote);
		currentTimbre->env[k]->noteOn(envState[k]);
	}

	currentTimbre->noteOn();
}

void Voice::endNoteOrBeginNextOne() {
    if (this->newNotePending) {
        noteOn(voiceTimbre, nextPendingNote, nextVelocity, index);
        this->newNotePending = false;
    } else {
        this->playing = false;
    	this->released = false;
    }
    this->env1ValueMem = 0;
    this->env2ValueMem = 0;
    this->env3ValueMem = 0;
    this->env4ValueMem = 0;
    this->env5ValueMem = 0;
    this->env6ValueMem = 0;
}


void Voice::glideFirstNoteOff() {
	// while gliding the first note was released
	this->note = this->nextGlidingNote;
	this->nextGlidingNote = 0;
}

void Voice::noteOff() {
	this->note = 0;
	this->released = true;
	this->nextPendingNote = 0;
	this->gliding = false;
	this->holdedByPedal = false;

	for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
		currentTimbre->env[k]->noteOff(envState[k]);
	}
	for (int k=0; k<NUMBER_OF_LFO; k++) {
		currentTimbre->lfo[k]->noteOff();
	}
}

void Voice::killNow() {
	this->note = 0;
	this->playing = false;
	this->nextPendingNote = 0;
	this->nextGlidingNote = 0;
	this->gliding = false;
	this->released = false;
    this->env1ValueMem = 0;
    this->env2ValueMem = 0;
    this->env3ValueMem = 0;
    this->env4ValueMem = 0;
    this->env5ValueMem = 0;
    this->env6ValueMem = 0;
}


void Voice::nextBlock() {
	float env1Value;
	float env2Value;
	float env3Value;
	float env4Value;
	float env5Value;
	float env6Value;
	float envNextValue;

	float env1Inc;
	float env2Inc;
	float env3Inc;
	float env4Inc;
	float env5Inc;
	float env6Inc;

	if (unlikely(!playing)) {
		return;
	}

	float *sample = currentTimbre->getSampleBlock();
	float inv32 = .03125f;
	currentTimbre->calculateFrequencyWithMatrix(oscState);
	int numberOfOscillators = algoInformation[(int)currentTimbre->params.engine1.algo].osc;

	env1Value = this->env1ValueMem;
	envNextValue = currentTimbre->env1.getNextAmpExp(&envState1);
	env1Inc = (envNextValue - env1Value) * inv32;  // divide by 32
	this->env1ValueMem = envNextValue;

	env2Value = this->env2ValueMem;
	envNextValue = currentTimbre->env2.getNextAmpExp(&envState2);
	env2Inc = (envNextValue - env2Value) * inv32;
	this->env2ValueMem = envNextValue;

	env3Value = this->env3ValueMem;
	envNextValue = currentTimbre->env3.getNextAmpExp(&envState3);
	env3Inc = (envNextValue - env3Value) * inv32;
	this->env3ValueMem = envNextValue;

	env4Value = this->env4ValueMem;
	envNextValue = currentTimbre->env4.getNextAmpExp(&envState4);
	env4Inc = (envNextValue - env4Value) * inv32;
	this->env4ValueMem = envNextValue;
	if (numberOfOscillators > 4 ) {
		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;
		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

	}


	switch ((int)currentTimbre->params.engine1.algo) {

	case ALGO1:
		/*
				  IM3
				 <----
			 .---.  .---.
			 | 2 |  | 3 |
			 '---'  '---'
			   \IM1   /IM2
				 .---.
				 | 1 |
				 '---'
		 */

	{
		oscState3.frequency =  oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		for (int k = 0; k < BLOCK_SIZE; k++) {
			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex3 + oscState2.mainFrequencyPlusMatrix;
			float freq2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;

			oscState1.frequency =  oscState1.mainFrequencyPlusMatrix + currentTimbre->modulationIndex1 * freq2 + currentTimbre->modulationIndex2 * freq3;
			float currentSample = currentTimbre->osc1.getNextSample(&oscState1)  * this->velocity * env1Value * currentTimbre->mix1;

			*sample++  += currentSample * currentTimbre->pan1Left;
			*sample++  += currentSample * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
		}

		if (currentTimbre->env1.isDead(&envState1)) {
			endNoteOrBeginNextOne();
		}

		break;
	}

	case ALGO2:
		/*
				 .---.
				 | 3 |
				 '---'
				   |
			   .------.
			   |IM1   |IM2
			 .---.  .---.
			 | 1 |  | 2 |
			 '---'  '---'
			   |Mix1  |Mix2
		 */
	{
		oscState3.frequency =  oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		float div2TimesVelocity = this->velocity * .5f;

		for (int k =0; k< BLOCK_SIZE; k++) {
			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2)* env2Value * currentTimbre->mix2 * div2TimesVelocity ;

			oscState1.frequency =  freq3 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2)) {
			endNoteOrBeginNextOne();
		}
	}
	break;

	case ALGO3:
		/*

					   IM4
					  ---->
		  .---.  .---.     .---.
		  | 2 |  | 3 |     | 4 |
		  '---'  '---'     '---'
			 \IM1  |IM2    /IM3
				 .---.
				 | 1 |
				 '---'
		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);


		for (int k =0; k< BLOCK_SIZE; k++) {
			float freq2 = osc2Values[k] * env2Value * oscState2.frequency;

			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState4.frequency =  freq3 * currentTimbre->modulationIndex4 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;

			oscState1.frequency =  freq2 * currentTimbre->modulationIndex1 + freq3 * currentTimbre->modulationIndex2 + freq4 * currentTimbre->modulationIndex3 + oscState1.mainFrequencyPlusMatrix;
			float currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * this->velocity;

			*sample++  += currentSample * currentTimbre->pan2Left;
			*sample++  += currentSample * currentTimbre->pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;

		}

		if (currentTimbre->env1.isDead(&envState1)) {
			endNoteOrBeginNextOne();
		}


		break;
	}
	case ALGO4:
		/*           IM4
			  .---. <----   .---.
			  | 3 |         | 4 |
			  '---'         '---'
			   |IM1 \IM3     |IM2
			 .---.          .---.
			 | 1 |          | 2 |
			 '---'          '---'
			   |Mix1          |Mix2

		 */
	{
		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div2TimesVelocity = this->velocity * .5f;

		for (int k = 0; k< BLOCK_SIZE; k++) {
			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex4 + oscState3.mainFrequencyPlusMatrix;
			float freq3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;

			oscState2.frequency =  freq4 * currentTimbre->modulationIndex2 +  freq3 * currentTimbre->modulationIndex3 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) *  env2Value *  currentTimbre->mix2 * div2TimesVelocity;

			oscState1.frequency =  freq3 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2)) {
			endNoteOrBeginNextOne();
		}
		break;
	}
	case ALGO5:
		/*
				 .---.
				 | 4 |
				 '---'  \
				   |IM3  |
				 .---.   |
				 | 3 |   | IM4
				 '---'   |
				   |IM2  |
				 .---.  /
				 | 2 |
				 '---'
				   |IM1
				 .---.
				 | 1 |
				 '---'

		 */
	{
		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex3  + oscState3.mainFrequencyPlusMatrix;
			float freq3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + freq4 * currentTimbre->modulationIndex4 + oscState2.mainFrequencyPlusMatrix;
			float freq2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;

			oscState1.frequency =  freq2 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * this->velocity * currentTimbre->mix1;

			*sample++  += car1 * currentTimbre->pan1Left;
			*sample++  += car1 * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}
		if (currentTimbre->env1.isDead(&envState1)) {
			endNoteOrBeginNextOne();
		}
		break;
	}
	case ALGO6:
		/*
				.---.
				| 4 |
				'---'
			 /IM1 |IM2 \IM3
		 .---.  .---.  .---.
		 | 1 |  | 2 |  | 3 |
		 '---'  '---'  '---'
		   |Mix1  |Mix2  | Mix3

		 */
	{
		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div3TimesVelocity =   .33f * this->velocity;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex3 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * div3TimesVelocity;

			oscState2.frequency =  freq4*currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * currentTimbre->mix2 * div3TimesVelocity;

			oscState1.frequency =  freq4*currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car3 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car3 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2)&& currentTimbre->env3.isDead(&envState3)) {
			endNoteOrBeginNextOne();
		}


		break;
	}

	case ALGO7:
		/*
						IM4
					   ---->
			 .---.  .---.  .---.
			 | 2 |  | 4 |  | 6 |
			 '---'  '---'  '---'
			   |IM1   |IM2   |IM3
			 .---.  .---.  .---.
			 | 1 |  | 3 |  | 5 |
			 '---'  '---'  '---'
			   |Mix1  |Mix2  |Mix3

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div3TimesVelocity =   .33f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState6.frequency = freq4 * currentTimbre->modulationIndex4 +  oscState6.mainFrequencyPlusMatrix;
			float freq6 = currentTimbre->osc6.getNextSample(&oscState6) * env6Value * oscState6.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix3 *  div3TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car3 * currentTimbre->pan2Left + car5 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car3 * currentTimbre->pan2Right + car5 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALGO8:
		/*
		  .---.  .---.  .---.       .---.
		  | 2 |  | 3 |  | 4 |       | 6 |
		  '---'  '---'  '---'       '---'
			 \IM1  |IM2  /IM3         | IM4
				 .---.              .---.
				 | 1 |              | 5 |
				 '---'              '---'
				   |Mix1              | Mix2

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;


			oscState1.frequency =  freq2*currentTimbre->modulationIndex1 + freq3*currentTimbre->modulationIndex2 + freq4*currentTimbre->modulationIndex3 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car1  * currentTimbre->pan2Left + car5  * currentTimbre->pan2Left;
			*sample++ += car1  * currentTimbre->pan2Right + car5  * currentTimbre->pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;

		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}

		break;
	}


	case ALGO9:
		/*
								.---.
								| 6 |
								'---'
								  |IM4
		 .---.      .---.       .---.
		 | 2 |      | 3 |       | 5 |
		 '---'      '---'       '---'
			 \IM1    /IM2         | IM3
			   .---.            .---.
			   | 1 |            | 4 |
			   '---'            '---'
				 |Mix1            | Mix2

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*currentTimbre->modulationIndex1 + freq3*currentTimbre->modulationIndex2 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex4 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency = freq5 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car4  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car4  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4)) {
			endNoteOrBeginNextOne();
		}

		break;
	}

	// =========== BELOW THIS LINE DX7 ALGORITHMS !!! =================


	case ALG10:
		/* DX7 Algo 1 & 2
								.---.
								| 6 |
								'---'
								  |IM4
								.---.
								| 5 |
								'---'
								  |IM3
			   .---.            .---.
			   | 2 |            | 4 |
			   '---'            '---'
				 |IM1             | IM2
			   .---.            .---.
			   | 1 |            | 3 |
			   '---'            '---'
				 |Mix1            | Mix2

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {


			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex4 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * currentTimbre->modulationIndex3 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value;
			freq4 *=  oscState4.frequency;

			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div2TimesVelocity;



			*sample++ += car3  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car3  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3)) {
			endNoteOrBeginNextOne();
		}

		break;

	}
	case ALG11:
		/*
		 * DX7 algo 3 & 4
					  .---.            .---.
					  | 3 |            | 6 |
					  '---'            '---'
						|IM2             |IM4
					  .---.            .---.
					  | 2 |            | 5 |
					  '---'            '---'
						|IM1            | IM3
					  .---.            .---.
					  | 1 |            | 4 |
					  '---'            '---'
						|Mix1            | Mix2

		 */
	{
		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float freq2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex4 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency = freq5 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car4  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car4  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG12:
		/*
		 * DX7 algo 5 & 6

			 .---.  .---.  .---.
			 | 2 |  | 4 |  | 6 |
			 '---'  '---'  '---'
			   |IM1   |IM2   |IM3
			 .---.  .---.  .---.
			 | 1 |  | 3 |  | 5 |
			 '---'  '---'  '---'
			   |Mix1  |Mix2  |Mix3

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix3 *  div3TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car3 * currentTimbre->pan2Left + car5 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car3 * currentTimbre->pan2Right + car5 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG13:
		/* DX7 Algo 7, 8, 9

						  .---.
						  | 6 |
						  '---'
							|IM4
		 .---.     .---.  .---.
		 | 2 |     | 4 |  | 5 |
		 '---'     '---'  '---'
		   |IM1      |IM2 /IM3
		 .---.     .---.
		 | 1 |     | 3 |
		 '---'     '---'
		   |Mix1     |Mix2
		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity = this->velocity * .5f;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex4 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 + freq5 * currentTimbre->modulationIndex3 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car1  * currentTimbre->pan1Left + car3  * currentTimbre->pan2Left;
			*sample++ += car1  * currentTimbre->pan1Right + car3  * currentTimbre->pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG14:
		/* DX Algo 10 & 11
					  .---.
					  | 3 |
					  '---'
						|IM2
					  .---.            .---.   .---.
					  | 2 |            | 5 |   | 6 |
					  '---'            '---'   '---'
						|IM1            | IM3 / IM4
					  .---.            .---.
					  | 1 |            | 4 |
					  '---'            '---'
						|Mix1            | Mix2

		 */
	{
		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float freq2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2  *currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			oscState4.frequency = freq5 * currentTimbre->modulationIndex3 + freq6 * currentTimbre->modulationIndex4 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car4  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car4  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG15:
		/*
		 * DX Algo 12 & 13


					  .---.     .---.   .---.   .---.
					  | 2 |     | 4 |   | 5 |   | 6 |
					  '---'     '---'   '---'   '---'
						|IM1        \IM2  | IM3 / IM4
					  .---.             .---.
					  | 1 |             | 3 |
					  '---'             '---'
						|Mix1             | Mix2

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;
			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;
			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;


			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 + freq5 * currentTimbre->modulationIndex3 + freq6 * currentTimbre->modulationIndex4 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div2TimesVelocity;

			*sample++ += car3  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car3  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG16:
		/* DX7 Algo 14 and 15
								.---.  .---.
								| 5 |  | 6 |
								'---'  '---'
								  |IM3/IM4
			   .---.            .---.
			   | 2 |            | 4 |
			   '---'            '---'
				 |IM1             | IM2
			   .---.            .---.
			   | 1 |            | 3 |
			   '---'            '---'
				 |Mix1            | Mix2

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {


			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * currentTimbre->modulationIndex3 + freq6 * currentTimbre->modulationIndex4 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value;
			freq4 *=  oscState4.frequency;

			oscState3.frequency = freq4 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div2TimesVelocity;


			*sample++ += car3  * currentTimbre->pan2Left + car1  * currentTimbre->pan1Left;
			*sample++ += car3  * currentTimbre->pan2Right + car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG17:
		/*
		 * DX ALGO 16 & 17
						   .---.     .---.
						   | 4 |     | 6 |
						   '---'     '---'
							 |IM3      |IM5
				 .---.     .---.     .---.
				 | 2 |     | 3 |     | 5 |
				 '---'     '---'     '---'
					 \IM1    |IM2   / IM4
						  .---.
						  | 1 |
						  '---'
							|Mix1

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex3 + oscState3.mainFrequencyPlusMatrix;
			float freq3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value;
			freq3 *=  oscState3.frequency;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex5 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 + freq3 * currentTimbre->modulationIndex2 + freq5 * currentTimbre->modulationIndex4 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1;

			*sample++ += car1  * currentTimbre->pan1Left;
			*sample++ += car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1)) {
			endNoteOrBeginNextOne();
		}

		break;
	}
	case ALG18:
		/*
		 * DX ALGO 18
									 .---.
									 | 6 |
									 '---'
									   |IM5
									 .---.
									 | 5 |
									 '---'
									   |IM4
				 .---.     .---.     .---.
				 | 2 |     | 3 |     | 4 |
				 '---'     '---'     '---'
					 \IM1    |IM2   / IM3
						  .---.
						  | 1 |
						  '---'
							|Mix1
							*/
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState5.frequency =  freq6 * currentTimbre->modulationIndex5 + oscState5.mainFrequencyPlusMatrix;
			float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * currentTimbre->modulationIndex4 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value;
			freq4 *=  oscState4.frequency;


			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 + freq3 * currentTimbre->modulationIndex2 + freq4 * currentTimbre->modulationIndex3 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1;

			*sample++ += car1  * currentTimbre->pan1Left;
			*sample++ += car1  * currentTimbre->pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1)) {
			endNoteOrBeginNextOne();
		}

		break;
	}

	case ALG19:
		/*
		 * DX7 algo 19

			 .---.
			 | 3 |
			 '---'
			   |IM2
			 .---.        .---.
			 | 2 |        | 6 |
			 '---'        '---'
			   |IM1    IM3/   \IM4
			 .---.  .---.     .---.
			 | 1 |  | 4 |     | 5 |
			 '---'  '---'     '---'
			   |Mix1  |Mix2     |Mix3

		 */
	{
		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float freq2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value;
			freq2 *=  oscState2.frequency;


			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			oscState4.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix3 *  div3TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car4 * currentTimbre->pan2Left + car5 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car4 * currentTimbre->pan2Right + car5 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG20:
		/*
		 * DX7 algo 20, 26 & 27

				.---.     .---.  .---.
				| 3 |     | 5 |  | 6 |
				'---'     '---'  '---'
			   /IM1 \IM2     \IM3 /IM4
			 .---.  .---.    .---.
			 | 1 |  | 2 |    | 4 |
			 '---'  '---'    '---'
			   |Mix1  |Mix2    |Mix3
		 */
	{

		oscState3.frequency =  oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;


			oscState1.frequency = freq3 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2)* env2Value * currentTimbre->mix2 * div3TimesVelocity ;

			oscState4.frequency = freq5 * currentTimbre->modulationIndex3 + freq6 * currentTimbre->modulationIndex4 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * div3TimesVelocity;


			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car4 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car4 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG21:
		/* DX ALGO 21 & 23
				  .---.         .---.
				  | 3 |         | 6 |
				  '---'         '---'
				 /IM1  \IM2   /IM3  \IM4
			  .---.  .---.  .---.  .---.
			  | 1 |  | 2 |  | 4 |  | 5 |
			  '---'  '---'  '---'  '---'
				|Mix1  |Mix2  |Mix3  |Mix4

		 */
	{
		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div4TimesVelocity =   .25f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			oscState1.frequency = freq3 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div4TimesVelocity;

			oscState2.frequency = freq3 * currentTimbre->modulationIndex2 +  oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * currentTimbre->mix2 * div4TimesVelocity;

			oscState4.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix4 * div4TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car4 * currentTimbre->pan3Left + car5 * currentTimbre->pan4Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car4 * currentTimbre->pan3Right + car5 * currentTimbre->pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG22:
		/* DX7 algo 22
			  .---.         .---.
			  | 2 |         | 6 |
			  '---'         '---'
				|IM1      /IM2 |IM3 \IM4
			  .---.  .---.  .---.  .---.
			  | 1 |  | 3 |  | 4 |  | 5 |
			  '---'  '---'  '---'  '---'
				|Mix1  |Mix2  |Mix3  |Mix4

		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div4TimesVelocity =   .25f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div4TimesVelocity;

			oscState3.frequency = freq6 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * div4TimesVelocity;

			oscState4.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix4 * div4TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car3 * currentTimbre->pan2Left + car4 * currentTimbre->pan3Left + car5 * currentTimbre->pan4Left;
			*sample++  += car1 * currentTimbre->pan1Right + car3 * currentTimbre->pan2Right + car4 * currentTimbre->pan3Right + car5 * currentTimbre->pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG23:
		/* DX ALGO 24, 25 & 31

								   .---.
								   | 6 |
								   '---'
								 /IM1|IM2\IM3
			  .---.  .---.  .---.  .---.   .---.
			  | 1 |  | 2 |  | 3 |  | 4 |   | 5 |
			  '---'  '---'  '---'  '---'   '---'
				|Mix1  |Mix2  |Mix3  |Mix4   |Mix5

		 */
	{
		oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
		float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

		oscState2.frequency =  oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div5TimesVelocity =   .2f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float car1 = osc1Values[k] * env1Value * currentTimbre->mix1 * div5TimesVelocity;

			float car2 = osc2Values[k] * env2Value * currentTimbre->mix2 * div5TimesVelocity;

			oscState3.frequency = freq6 * currentTimbre->modulationIndex1 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * div5TimesVelocity;

			oscState4.frequency = freq6 * currentTimbre->modulationIndex2 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix4 * div5TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix5 * div5TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car3 * currentTimbre->pan3Left  + car4 * currentTimbre->pan4Left + car5 * currentTimbre->pan5Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car3 * currentTimbre->pan3Right + car4 * currentTimbre->pan4Right + car5 * currentTimbre->pan5Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG24:
		/*
		 * DX ALGO 28

					.---.
					| 5 |
					'---'
					  |IM3
			 .---.  .---.
			 | 2 |  | 4 |
			 '---'  '---'
			   |IM1   |IM2
			 .---.  .---.  .---.
			 | 1 |  | 3 |  | 6 |
			 '---'  '---'  '---'
			   |Mix1  |Mix2  |Mix3


		 */
	{
		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * currentTimbre->modulationIndex3 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value;
			freq4 *=  oscState4.frequency;

			oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * div3TimesVelocity;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex2 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * div3TimesVelocity;

			float car6 = osc6Values[k] * env6Value * currentTimbre->mix6 * div3TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car3 * currentTimbre->pan2Left + car6 * currentTimbre->pan3Left;
			*sample++  += car1 * currentTimbre->pan1Right + car3 * currentTimbre->pan2Right + car6 * currentTimbre->pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env6.isDead(&envState6)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG25:
		/*
		 * DX ALGO 29
		 *

							.---.  .---.
							| 4 |  | 6 |
							'---'  '---'
							  |IM1   |IM2
			  .---.  .---.  .---.  .---.
			  | 1 |  | 2 |  | 3 |  | 5 |
			  '---'  '---'  '---'  '---'
				|Mix1  |Mix2  |Mix3  |Mix4

		 */
	{
		oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
		float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div4TimesVelocity =   .25f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *= oscState4.frequency;

			float car1 = osc1Values[k] * env1Value * currentTimbre->mix1 * div4TimesVelocity;

			float car2 = osc2Values[k] * env2Value * currentTimbre->mix2 * div4TimesVelocity;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex1 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * currentTimbre->modulationIndex2 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix4 * div4TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car3 * currentTimbre->pan3Left + car5 * currentTimbre->pan4Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car3 * currentTimbre->pan3Right + car5 * currentTimbre->pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5)) {
			endNoteOrBeginNextOne();
		}
	}
	break;
	case ALG26:
		/*
		 * DX Algo 30
							.---.
							| 5 |
							'---'
							  |IM2
							.---.
							| 4 |
							'---'
							  |IM1
			  .---.  .---.  .---.  .---.
			  | 1 |  | 2 |  | 3 |  | 6 |
			  '---'  '---'  '---'  '---'
				|Mix1  |Mix2  |Mix3  |Mix4


		 */
	{
		oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
		float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div4TimesVelocity =   .25f * this->velocity;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq5 = osc5Values[k] * env5Value;
			freq5 *= oscState5.frequency;

			float car1 = osc1Values[k] * env1Value * currentTimbre->mix1 * div4TimesVelocity;

			float car2 = osc2Values[k] * env2Value * currentTimbre->mix2 * div4TimesVelocity;

			oscState4.frequency =  freq5 * currentTimbre->modulationIndex2 + oscState4.mainFrequencyPlusMatrix;
			float freq4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * currentTimbre->modulationIndex1 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * div4TimesVelocity;

			float car6 = osc6Values[k] * env6Value * currentTimbre->mix6 * div4TimesVelocity;

			*sample++  += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car3 * currentTimbre->pan3Left + car6 * currentTimbre->pan4Left;
			*sample++  += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car3 * currentTimbre->pan3Right + car6 * currentTimbre->pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env6.isDead(&envState6)) {
			endNoteOrBeginNextOne();
		}
	}
	break;

	case ALG27:
		/*
		 * DX ALGO 32
		 *
			  .---.  .---.  .---.  .---.   .---.   .---.
			  | 1 |  | 2 |  | 3 |  | 4 |   | 5 |   | 6 |
			  '---'  '---'  '---'  '---'   '---'   '---'
				|Mix1  |Mix2  |Mix3  |Mix4   |Mix5   |Mix6

		 */
		 {
			oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
			float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

			oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
			float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

			oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
			float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

			oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
			float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

			oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
			float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

			oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
			float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

			float div6TimesVelocity =   .16f * this->velocity;

			for (int k =0; k< BLOCK_SIZE; k++) {


				float car1 = osc1Values[k] * env1Value * currentTimbre->mix1 * div6TimesVelocity;
				float car2 = osc2Values[k] * env2Value * currentTimbre->mix2 * div6TimesVelocity;
				float car3 = osc3Values[k] * env3Value * currentTimbre->mix3 * div6TimesVelocity;
				float car4 = osc4Values[k] * env4Value * currentTimbre->mix4 * div6TimesVelocity;
				float car5 = osc5Values[k] * env5Value * currentTimbre->mix5 * div6TimesVelocity;
				float car6 = osc6Values[k] * env6Value * currentTimbre->mix6 * div6TimesVelocity;

//					oscState6.phase =  this->feedback * currentTimbre->modulationIndex1;
//					float car6 = currentTimbre->osc6.getNextSample(&oscState) * env6Value;
//					this->feedback = car6 ;
//					car6 = car6 * currentTimbre->mix6 * div6TimesVelocity;

				*sample++ += car1 * currentTimbre->pan1Left + car2 * currentTimbre->pan2Left + car3 * currentTimbre->pan3Left + car4 * currentTimbre->pan4Left + car5 * currentTimbre->pan5Left + car6 * currentTimbre->pan6Left;
				*sample++ += car1 * currentTimbre->pan1Right + car2 * currentTimbre->pan2Right + car3 * currentTimbre->pan3Right + car4 * currentTimbre->pan4Right + car5 * currentTimbre->pan5Right + car6 * currentTimbre->pan6Right;

				env1Value += env1Inc;
				env2Value += env2Inc;
				env3Value += env3Inc;
				env4Value += env4Inc;
				env5Value += env5Inc;
				env6Value += env6Inc;
			}
			if (currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3)  && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5) && currentTimbre->env6.isDead(&envState6)) {
				endNoteOrBeginNextOne();
			}
		 }
		 break;

	} // End switch
}




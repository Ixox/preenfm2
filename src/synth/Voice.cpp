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
#include "Timbre.h"


float Voice::glidePhaseInc[10];


Voice::Voice(void)
{

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
	freqAi = freqAo = 0.0f;
	freqBi = freqBo = 0.0f;
	freqHarm = 1.0f;
	targetFreqHarm = 0.0f;
}


Voice::~Voice(void)
{
}

void Voice::init() {
	this->currentTimbre = 0;
	this->playing = false;
	this->newNotePending = false;
	this->note = 0;
	this->holdedByPedal = false;
    this->newNotePlayed = false;
}


void Voice::glideToNote(short newNote) {
	// Must glide...
	this->gliding = true;
	this->glidePhase = 0.0f;
	this->nextGlidingNote = newNote;
	if (this->holdedByPedal) {
		glideFirstNoteOff();
	}

	currentTimbre->osc1.glideToNote(&oscState1, newNote);
	currentTimbre->osc2.glideToNote(&oscState2, newNote);
	currentTimbre->osc3.glideToNote(&oscState3, newNote);
	currentTimbre->osc4.glideToNote(&oscState4, newNote);
	currentTimbre->osc5.glideToNote(&oscState5, newNote);
	currentTimbre->osc6.glideToNote(&oscState6, newNote);
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

		this->currentTimbre->env1.noteOffQuick(&envState1);
		this->currentTimbre->env2.noteOffQuick(&envState2);
		this->currentTimbre->env3.noteOffQuick(&envState3);
		this->currentTimbre->env4.noteOffQuick(&envState4);
		this->currentTimbre->env5.noteOffQuick(&envState5);
		this->currentTimbre->env6.noteOffQuick(&envState6);
		// We don't want noteOnAfterMatrixCompute to restart the env or => Hanging note !!!
	    this->newNotePlayed = false;
	}
}

void Voice::glide() {
	this->glidePhase += glidePhaseInc[(int)(currentTimbre->params.engine1.glide - .95f)];
	if (glidePhase < 1.0f) {

		currentTimbre->osc1.glideStep(&oscState1, this->glidePhase);
		currentTimbre->osc2.glideStep(&oscState2, this->glidePhase);
		currentTimbre->osc3.glideStep(&oscState3, this->glidePhase);
		currentTimbre->osc4.glideStep(&oscState4, this->glidePhase);
		currentTimbre->osc5.glideStep(&oscState5, this->glidePhase);
		currentTimbre->osc6.glideStep(&oscState6, this->glidePhase);

	} else {
		// last with phase set to 1 to have exact frequencry
		currentTimbre->osc1.glideStep(&oscState1, 1);
		currentTimbre->osc2.glideStep(&oscState2, 1);
		currentTimbre->osc3.glideStep(&oscState3, 1);
		currentTimbre->osc4.glideStep(&oscState4, 1);
		currentTimbre->osc5.glideStep(&oscState5, 1);
		currentTimbre->osc6.glideStep(&oscState6, 1);
		this->gliding = false;
	}
}

void Voice::noteOn(short newNote, short velocity, unsigned int index) {

	this->released = false;
	this->playing = true;
	this->note = newNote;
	this->nextPendingNote = 0;
	this->newNotePending = false;
	this->holdedByPedal = false;
	this->index = index;

	this->velIm1 = currentTimbre->params.engineIm1.modulationIndexVelo1 * (float)velocity * .0078125f;
	this->velIm2 = currentTimbre->params.engineIm1.modulationIndexVelo2 * (float)velocity * .0078125f;
	this->velIm3 = currentTimbre->params.engineIm2.modulationIndexVelo3 * (float)velocity * .0078125f;
	this->velIm4 = currentTimbre->params.engineIm2.modulationIndexVelo4 * (float)velocity * .0078125f;
	this->velIm5 = currentTimbre->params.engineIm3.modulationIndexVelo5 * (float)velocity * .0078125f;

	int zeroVelo = (16 - currentTimbre->params.engine1.velocity) * 8;
	int newVelocity = zeroVelo + ((velocity * (128 - zeroVelo)) >> 7);
	this->velocity = newVelocity * .0078125f; // divide by 127

	currentTimbre->osc1.newNote(&oscState1, newNote);
	currentTimbre->osc2.newNote(&oscState2, newNote);
	currentTimbre->osc3.newNote(&oscState3, newNote);
	currentTimbre->osc4.newNote(&oscState4, newNote);
	currentTimbre->osc5.newNote(&oscState5, newNote);
	currentTimbre->osc6.newNote(&oscState6, newNote);

	/* Firmware v2.0  Env must be initialized after matrix computation so in
	currentTimbre->env1.noteOn(&envState1, &matrix, freqHarm);
	currentTimbre->env2.noteOn(&envState2, &matrix, freqHarm);
	currentTimbre->env3.noteOn(&envState3, &matrix, freqHarm);
	currentTimbre->env4.noteOn(&envState4, &matrix, freqHarm);
	currentTimbre->env5.noteOn(&envState5, &matrix, freqHarm);
	currentTimbre->env6.noteOn(&envState6, &matrix, freqHarm);
	*/
	// Tell nextBlock() to init Env...
    this->newNotePlayed = true;

	lfoNoteOn();
}

void Voice::endNoteOrBeginNextOne() {
    if (this->newNotePending) {
		if (nextPendingNote >=0) {
        	noteOn(nextPendingNote, nextVelocity, index);
		} else {
		    // Note off have already been received....
			this->playing = false;
			this->nextPendingNote = 0;
	        this->newNotePending = false;
		}
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
	this->freqAi = 0.0f;
	this->freqAo = 0.0f;

}


void Voice::glideFirstNoteOff() {
	// while gliding the first note was released
	this->note = this->nextGlidingNote;
	this->nextGlidingNote = 0;
}


void Voice::noteOff() {

	if (unlikely(!this->playing)) {
		return;
	}
	if (likely(!this->newNotePending)) {
        if (this->newNotePlayed) {
            // Note hasn't played
            killNow();
            return;
        }
		this->released = true;
		this->gliding = false;
		this->holdedByPedal = false;

		currentTimbre->env1.noteOff(&envState1, &matrix);
		currentTimbre->env2.noteOff(&envState2, &matrix);
		currentTimbre->env3.noteOff(&envState3, &matrix);
		currentTimbre->env4.noteOff(&envState4, &matrix);
		currentTimbre->env5.noteOff(&envState5, &matrix);
		currentTimbre->env6.noteOff(&envState6, &matrix);

		lfoNoteOff();
	} else {
		// We receive a note off before the note actually started
		this->nextPendingNote = -1;
	}
}

void Voice::killNow() {
	this->newNotePlayed = false;
	this->playing = false;
	this->newNotePending = false;
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


int cptDebug32 = 0;

void Voice::nextBlock() {
    updateAllMixOscsAndPans();
    // After matrix
    if (unlikely(this->newNotePlayed)) {
        this->newNotePlayed = false;

        currentTimbre->env1.noteOnAfterMatrixCompute(&envState1, &matrix);
        currentTimbre->env2.noteOnAfterMatrixCompute(&envState2, &matrix);
        currentTimbre->env3.noteOnAfterMatrixCompute(&envState3, &matrix);
        currentTimbre->env4.noteOnAfterMatrixCompute(&envState4, &matrix);
        currentTimbre->env5.noteOnAfterMatrixCompute(&envState5, &matrix);
        currentTimbre->env6.noteOnAfterMatrixCompute(&envState6, &matrix);
    }

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


	float *sample = currentTimbre->getSampleBlock();
	float inv32 = .03125f;



    if (matrix.getDestination(ALL_OSC_FREQ_HARM) != targetFreqHarm)  {
        targetFreqHarm = matrix.getDestination(ALL_OSC_FREQ_HARM);
        float findex = 512 + targetFreqHarm * 20;
        int index = findex;
        float fp = findex - index;
        freqHarm = (exp2_harm[index]* (1.0f-fp) + exp2_harm[index + 1] * fp );
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);

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


		oscState3.frequency =  oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

	    float f2x;
	    float f2xm1 = freqAi;
	    float freq2 = freqAo;

		for (int k = 0; k < BLOCK_SIZE; k++) {
			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState2.frequency =  freq3 * voiceIm3 + oscState2.mainFrequencyPlusMatrix;
			f2x = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;
			freq2 = f2x - f2xm1 + 0.99525f * freq2;
			f2xm1 = f2x;

			oscState1.frequency =  oscState1.mainFrequencyPlusMatrix + voiceIm1 * freq2 + voiceIm2 * freq3;
			float currentSample = currentTimbre->osc1.getNextSample(&oscState1)  * this->velocity * env1Value * mix1;

			*sample++  += currentSample * pan1Left;
			*sample++  += currentSample * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
		}

		freqAi = f2xm1;
		freqAo = freq2;

		if (unlikely(currentTimbre->env1.isDead(&envState1))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);

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


		oscState3.frequency =  oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		float div2TimesVelocity = this->velocity * .5f;

		for (int k =0; k< BLOCK_SIZE; k++) {
			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState2.frequency =  freq3 * voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2)* env2Value * mix2 * div2TimesVelocity ;

			oscState1.frequency =  freq3 * voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);

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


		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {
			float freq2 = osc2Values[k] * env2Value * oscState2.frequency;

			float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

			oscState4.frequency =  freq3 * voiceIm4 + oscState4.mainFrequencyPlusMatrix;

			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState1.frequency =  freq2 * voiceIm1 + freq3 * voiceIm2 + freq4 * voiceIm3 + oscState1.mainFrequencyPlusMatrix;
			float currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 *  this->velocity;

			*sample++  += currentSample * pan1Left;
			*sample++  += currentSample * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;

		}

		freqAi = f4xm1;
		freqAo = freq4;

		if (unlikely(currentTimbre->env1.isDead(&envState1))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);

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


		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div2TimesVelocity = this->velocity * .5f;

		float f3x;
	    float f3xm1 = freqAi;
	    float freq3 = freqAo;

		for (int k = 0; k< BLOCK_SIZE; k++) {
			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * voiceIm4 + oscState3.mainFrequencyPlusMatrix;
			f3x = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;
			freq3 = f3x - f3xm1 + 0.99525f * freq3;
			f3xm1 = f3x;

			oscState2.frequency =  freq4 * voiceIm2 +  freq3 * voiceIm3 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) *  env2Value *  mix2 * div2TimesVelocity;

			oscState1.frequency =  freq3 * voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}

		freqAi = f3xm1;
		freqAo = freq3;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);

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


		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

	    float f2x;
	    float f2xm1 = freqAi;
	    float freq2 = freqAo;
		float f3x;
		float f3xm1 = freqBi;
		float freq3 = freqBo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * voiceIm3  + oscState3.mainFrequencyPlusMatrix;
			f3x = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;
			freq3 = f3x - f3xm1 + 0.99535f * freq3;
			f3xm1 = f3x;

			oscState2.frequency =  freq3 * voiceIm2 + freq4 * voiceIm4 + oscState2.mainFrequencyPlusMatrix;
			f2x = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;
			freq2 = f2x - f2xm1 + 0.99525f * freq2;
			f2xm1 = f2x;

			oscState1.frequency =  freq2 * voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * this->velocity * mix1;

			*sample++  += car1 * pan1Left;
			*sample++  += car1 * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}
		freqAi = f2xm1;
		freqAo = freq2;
		freqBi = f3xm1;
		freqBo = freq3;
		if (unlikely(currentTimbre->env1.isDead(&envState1))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);

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


		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div3TimesVelocity =   .33f * this->velocity;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

			oscState3.frequency =  freq4 * voiceIm3 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix3 * div3TimesVelocity;

			oscState2.frequency =  freq4*voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * mix2 * div3TimesVelocity;

			oscState1.frequency =  freq4*voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left + car3 * pan3Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car3 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2)&& currentTimbre->env3.isDead(&envState3))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);


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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		float div3TimesVelocity =   .33f * this->velocity;

		// F2 will be for OP6
	    float f6x;
	    float f6xm1 = freqAi;
	    float freq6 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState6.frequency = freq4 * voiceIm4 +  oscState6.mainFrequencyPlusMatrix;
			f6x = currentTimbre->osc6.getNextSample(&oscState6) * env6Value * oscState6.frequency;
			freq6 = f6x - f6xm1 + 0.99525f * freq6;
			f6xm1 = f6x;

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			oscState3.frequency = freq4 * voiceIm2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * voiceIm3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix3 *  div3TimesVelocity;

			*sample++  += car1 * pan1Left + car3 * pan2Left + car5 * pan3Left;
			*sample++  += car1 * pan1Right + car3 * pan2Right + car5 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f6xm1;
		freqAo = freq6;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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


			oscState1.frequency =  freq2*voiceIm1 + freq3*voiceIm2 + freq4*voiceIm3 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			oscState5.frequency = freq6 * voiceIm4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix2 * div2TimesVelocity;

			*sample++ += car1  * pan2Left + car5  * pan2Left;
			*sample++ += car1  * pan2Right + car5  * pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;

		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		// F2 will be for OP5
	    float f5x;
	    float f5xm1 = freqAi;
	    float freq5 = freqAo;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*voiceIm1 + freq3*voiceIm2 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			oscState5.frequency =  freq6 * voiceIm4 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState4.frequency = freq5 * voiceIm3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix2 * div2TimesVelocity;

			*sample++ += car4  * pan2Left + car1  * pan1Left;
			*sample++ += car4  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
	    freqAi = f5xm1;
	    freqAo = freq5;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;


	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;

	    float f5x;
	    float f5xm1 = freqBi;
	    float freq5 = freqBo;

	    for (int k =0; k< BLOCK_SIZE; k++) {


			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState5.frequency =  freq6 * voiceIm4 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value *  oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState4.frequency =  freq5 * voiceIm3 + oscState4.mainFrequencyPlusMatrix;
			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState3.frequency = freq4 * voiceIm2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div2TimesVelocity;



			*sample++ += car3  * pan2Left + car1  * pan1Left;
			*sample++ += car3  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

	    freqBi = f5xm1;
	    freqBo = freq5;

	    freqAi = f4xm1;
	    freqAo = freq4;

	    if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

		float f2x;
	    float f2xm1 = freqAi;
	    float freq2 = freqAo;

		float f5x;
	    float f5xm1 = freqBi;
	    float freq5 = freqBo;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			oscState2.frequency =  freq3 * voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			f2x = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;
			freq2 = f2x - f2xm1 + 0.99525f * freq2;
			f2xm1 = f2x;

			oscState1.frequency =  freq2 * voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState5.frequency =  freq6 * voiceIm4 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState4.frequency = freq5 * voiceIm3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix2 * div2TimesVelocity;

			*sample++ += car4  * pan2Left + car1  * pan1Left;
			*sample++ += car4  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

	    freqAi = f2xm1;
	    freqAo = freq2;

	    freqBi = f5xm1;
		freqBo = freq5;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			oscState3.frequency = freq4 * voiceIm2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * voiceIm3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix3 *  div3TimesVelocity;

			*sample++  += car1 * pan1Left + car3 * pan2Left + car5 * pan3Left;
			*sample++  += car1 * pan1Right + car3 * pan2Right + car5 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity = this->velocity * .5f;

		float f5x;
	    float f5xm1 = freqAi;
	    float freq5 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			oscState5.frequency =  freq6 * voiceIm4 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState3.frequency = freq4 * voiceIm2 + freq5 * voiceIm3 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div2TimesVelocity;

			*sample++ += car1  * pan1Left + car3  * pan2Left;
			*sample++ += car1  * pan1Right + car3  * pan2Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f5xm1;
		freqAo = freq5;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

	    float f2x;
	    float f2xm1 = freqAi;
	    float freq2 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState2.frequency =  freq3 * voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			f2x = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;
			freq2 = f2x - f2xm1 + 0.99525f * freq2;
			f2xm1 = f2x;

			oscState1.frequency =  freq2  *voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			oscState4.frequency = freq5 * voiceIm3 + freq6 * voiceIm4 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix2 * div2TimesVelocity;

			*sample++ += car4  * pan2Left + car1  * pan1Left;
			*sample++ += car4  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

	    freqAi = f2xm1;
	    freqAo = freq2;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			oscState1.frequency =  freq2*voiceIm1 + oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;
			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;
			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;


			oscState3.frequency = freq4 * voiceIm2 + freq5 * voiceIm3 + freq6 * voiceIm4 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div2TimesVelocity;

			*sample++ += car3  * pan2Left + car1  * pan1Left;
			*sample++ += car3  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div2TimesVelocity =   .5f * this->velocity;

	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {


			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState1.frequency =  freq2*voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div2TimesVelocity;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * voiceIm3 + freq6 * voiceIm4 + oscState4.mainFrequencyPlusMatrix;
			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState3.frequency = freq4 * voiceIm2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div2TimesVelocity;


			*sample++ += car3  * pan2Left + car1  * pan1Left;
			*sample++ += car3  * pan2Right + car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3))) {
			endNoteOrBeginNextOne();
		}

		freqAi = f4xm1;
		freqAo = freq4;

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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;
		float voiceIm5 = modulationIndex5;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
		float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

	    float f3x;
	    float f3xm1 = freqAi;
	    float freq3 = freqAo;

	    float f5x;
	    float f5xm1 = freqBi;
	    float freq5 = freqBo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq4 = osc4Values[k] * env4Value;
			freq4 *=  oscState4.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState3.frequency =  freq4 * voiceIm3 + oscState3.mainFrequencyPlusMatrix;
			f3x = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;
			freq3 = f3x - f3xm1 + 0.99525f * freq3;
			f3xm1 = f3x;

			oscState5.frequency =  freq6 * voiceIm5 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState1.frequency = freq2 * voiceIm1 + freq3 * voiceIm2 + freq5 * voiceIm4 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1  * velocity;

			*sample++ += car1  * pan1Left;
			*sample++ += car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f3xm1;
		freqAo = freq3;

		freqBi = f5xm1;
		freqBo = freq5;

		if (unlikely(currentTimbre->env1.isDead(&envState1))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;
		float voiceIm5 = modulationIndex5;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;

	    float f5x;
	    float f5xm1 = freqBi;
	    float freq5 = freqBo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			oscState5.frequency =  freq6 * voiceIm5 + oscState5.mainFrequencyPlusMatrix;
			f5x = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * oscState5.frequency;
			freq5 = f5x - f5xm1 + 0.99525f * freq5;
			f5xm1 = f5x;

			oscState4.frequency =  freq5 * voiceIm4 + oscState4.mainFrequencyPlusMatrix;
			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState1.frequency = freq2 * voiceIm1 + freq3 * voiceIm2 + freq4 * voiceIm3 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * velocity;

			*sample++ += car1  * pan1Left;
			*sample++ += car1  * pan1Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f4xm1;
		freqAo = freq4;

		freqBi = f5xm1;
		freqBo = freq5;

		if (unlikely(currentTimbre->env1.isDead(&envState1))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
		float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

	    float f2x;
	    float f2xm1 = freqAi;
	    float freq2 = freqAo;

		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq3 = osc3Values[k] * env3Value;
			freq3 *=  oscState3.frequency;

			float freq6 = osc6Values[k] * env6Value;
			freq6 *=  oscState6.frequency;

			oscState2.frequency =  freq3 * voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			f2x = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * oscState2.frequency;
			freq2 = f2x - f2xm1 + 0.99525f * freq2;
			f2xm1 = f2x;

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			oscState4.frequency = freq6 * voiceIm3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix2 * div3TimesVelocity;

			oscState5.frequency = freq6 * voiceIm4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix3 *  div3TimesVelocity;

			*sample++  += car1 * pan1Left + car4 * pan2Left + car5 * pan3Left;
			*sample++  += car1 * pan1Right + car4 * pan2Right + car5 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f2xm1;
		freqAo = freq2;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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


			oscState1.frequency = freq3 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			oscState2.frequency =  freq3 * voiceIm2 + oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2)* env2Value * mix2 * div3TimesVelocity ;

			oscState4.frequency = freq5 * voiceIm3 + freq6 * voiceIm4 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix3 * div3TimesVelocity;


			*sample++  += car1 * pan1Left + car2 * pan2Left + car4 * pan3Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car4 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			oscState1.frequency = freq3 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div4TimesVelocity;

			oscState2.frequency = freq3 * voiceIm2 +  oscState2.mainFrequencyPlusMatrix;
			float car2 = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * mix2 * div4TimesVelocity;

			oscState4.frequency = freq6 * voiceIm3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * voiceIm4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix4 * div4TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left + car4 * pan3Left + car5 * pan4Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car4 * pan3Right + car5 * pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;
		float voiceIm4 = modulationIndex4;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1 = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div4TimesVelocity;

			oscState3.frequency = freq6 * voiceIm2 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div4TimesVelocity;

			oscState4.frequency = freq6 * voiceIm3 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * voiceIm4 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix4 * div4TimesVelocity;

			*sample++  += car1 * pan1Left + car3 * pan2Left + car4 * pan3Left + car5 * pan4Left;
			*sample++  += car1 * pan1Right + car3 * pan2Right + car4 * pan3Right + car5 * pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			float car1 = osc1Values[k] * env1Value * mix1 * div5TimesVelocity;

			float car2 = osc2Values[k] * env2Value * mix2 * div5TimesVelocity;

			oscState3.frequency = freq6 * voiceIm1 +  oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix3 * div5TimesVelocity;

			oscState4.frequency = freq6 * voiceIm2 +  oscState4.mainFrequencyPlusMatrix;
			float car4 = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * mix4 * div5TimesVelocity;

			oscState5.frequency = freq6 * voiceIm3 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix5 * div5TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left + car3 * pan3Left  + car4 * pan4Left + car5 * pan5Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car3 * pan3Right + car4 * pan4Right + car5 * pan5Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;
		float voiceIm3 = modulationIndex3;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div3TimesVelocity =   .33f * this->velocity;

	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq2 = osc2Values[k] * env2Value;
			freq2 *=  oscState2.frequency;

			float freq5 = osc5Values[k] * env5Value;
			freq5 *=  oscState5.frequency;

			oscState4.frequency =  freq5 * voiceIm3 + oscState4.mainFrequencyPlusMatrix;
			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState1.frequency = freq2 * voiceIm1 +  oscState1.mainFrequencyPlusMatrix;
			float car1= currentTimbre->osc1.getNextSample(&oscState1) * env1Value * mix1 * div3TimesVelocity;

			oscState3.frequency =  freq4 * voiceIm2 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix2 * div3TimesVelocity;

			float car6 = osc6Values[k] * env6Value * mix3 * div3TimesVelocity;

			*sample++  += car1 * pan1Left + car3 * pan2Left + car6 * pan3Left;
			*sample++  += car1 * pan1Right + car3 * pan2Right + car6 * pan3Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f4xm1;
		freqAo = freq4;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env6.isDead(&envState6))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

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

			float car1 = osc1Values[k] * env1Value * mix1 * div4TimesVelocity;

			float car2 = osc2Values[k] * env2Value * mix2 * div4TimesVelocity;

			oscState3.frequency =  freq4 * voiceIm1 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix3 * div4TimesVelocity;

			oscState5.frequency = freq6 * voiceIm2 +  oscState5.mainFrequencyPlusMatrix;
			float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix4 * div4TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left + car3 * pan3Left + car5 * pan4Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car3 * pan3Right + car5 * pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}
		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env5.isDead(&envState5))) {
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
		float voiceIm1 = modulationIndex1;
		float voiceIm2 = modulationIndex2;

		currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
		currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
		currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
		currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
		currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
		currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

		env5Value = this->env5ValueMem;
		envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
		env5Inc = (envNextValue - env5Value) * inv32;
		this->env5ValueMem = envNextValue;

		env6Value = this->env6ValueMem;
		envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
		env6Inc = (envNextValue - env6Value) * inv32;
		this->env6ValueMem = envNextValue;

		oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
		float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

		oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
		float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

		oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
		float* osc5Values = currentTimbre->osc5.getNextBlock(&oscState5);

		oscState6.frequency = oscState6.mainFrequencyPlusMatrix;
		float* osc6Values = currentTimbre->osc6.getNextBlock(&oscState6);

		float div4TimesVelocity =   .25f * this->velocity;

	    float f4x;
	    float f4xm1 = freqAi;
	    float freq4 = freqAo;


		for (int k =0; k< BLOCK_SIZE; k++) {

			float freq5 = osc5Values[k] * env5Value;
			freq5 *= oscState5.frequency;

			float car1 = osc1Values[k] * env1Value * mix1 * div4TimesVelocity;

			float car2 = osc2Values[k] * env2Value * mix2 * div4TimesVelocity;

			oscState4.frequency =  freq5 * voiceIm2 + oscState4.mainFrequencyPlusMatrix;
			f4x = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * oscState4.frequency;
			freq4 = f4x - f4xm1 + 0.99525f * freq4;
			f4xm1 = f4x;

			oscState3.frequency =  freq4 * voiceIm1 + oscState3.mainFrequencyPlusMatrix;
			float car3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * mix3 * div4TimesVelocity;

			float car6 = osc6Values[k] * env6Value * mix4 * div4TimesVelocity;

			*sample++  += car1 * pan1Left + car2 * pan2Left + car3 * pan3Left + car6 * pan4Left;
			*sample++  += car1 * pan1Right + car2 * pan2Right + car3 * pan3Right + car6 * pan4Right;

			env1Value += env1Inc;
			env2Value += env2Inc;
			env3Value += env3Inc;
			env4Value += env4Inc;
			env5Value += env5Inc;
			env6Value += env6Inc;
		}

		freqAi = f4xm1;
		freqAo = freq4;

		if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3) && currentTimbre->env6.isDead(&envState6))) {
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
			 currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
			 currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
			 currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
			 currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
			 currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
			 currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

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

			env5Value = this->env5ValueMem;
			envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
			env5Inc = (envNextValue - env5Value) * inv32;
			this->env5ValueMem = envNextValue;

			env6Value = this->env6ValueMem;
			envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
			env6Inc = (envNextValue - env6Value) * inv32;
			this->env6ValueMem = envNextValue;

			oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
			float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

			oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
			float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

			oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
			float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

			oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
			float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

			oscState5.frequency = oscState5.mainFrequencyPlusMatrix;
			oscState6.frequency = oscState6.mainFrequencyPlusMatrix;

			float div6TimesVelocity =   .16f * this->velocity;

			for (int k =0; k< BLOCK_SIZE; k++) {

				float car1 = osc1Values[k] * env1Value * mix1;
				float car2 = osc2Values[k] * env2Value * mix2;
				float car3 = osc3Values[k] * env3Value * mix3;
				float car4 = osc4Values[k] * env4Value * mix4;

				float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix5;
				float car6 = currentTimbre->osc6.getNextSample(&oscState6) * env6Value * mix6;

				*sample++ += (car1 * pan1Left + car2 * pan2Left + car3 * pan3Left + car4 * pan4Left + car5 * pan5Left + car6 * pan6Left) * div6TimesVelocity;
				*sample++ += (car1 * pan1Right + car2 * pan2Right + car3 * pan3Right + car4 * pan4Right + car5 * pan5Right + car6 * pan6Right) * div6TimesVelocity;

				env1Value += env1Inc;
				env2Value += env2Inc;
				env3Value += env3Inc;
				env4Value += env4Inc;
				env5Value += env5Inc;
				env6Value += env6Inc;
			}
			if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3)  && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5) && currentTimbre->env6.isDead(&envState6))) {
				endNoteOrBeginNextOne();
			}
		 }
		 break;
	case ALG28:
		/*
		 * DX ALGO 31

		    				               .---.
							               | 6 |
                						   '---'
							                 |IM1
			  .---.  .---.  .---.  .---.   .---.
			  | 1 |  | 2 |  | 3 |  | 4 |   | 5 |
			  '---'  '---'  '---'  '---'   '---'
				|Mix1  |Mix2  |Mix3  |Mix4   |Mix5

		 */
		 {
			currentTimbre->osc1.calculateFrequencyWithMatrix(&oscState1, &matrix, freqHarm);
			currentTimbre->osc2.calculateFrequencyWithMatrix(&oscState2, &matrix, freqHarm);
			currentTimbre->osc3.calculateFrequencyWithMatrix(&oscState3, &matrix, freqHarm);
			currentTimbre->osc4.calculateFrequencyWithMatrix(&oscState4, &matrix, freqHarm);
			currentTimbre->osc5.calculateFrequencyWithMatrix(&oscState5, &matrix, freqHarm);
			currentTimbre->osc6.calculateFrequencyWithMatrix(&oscState6, &matrix, freqHarm);

			float voiceIm1 = modulationIndex1;

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

			env5Value = this->env5ValueMem;
			envNextValue = currentTimbre->env5.getNextAmpExp(&envState5);
			env5Inc = (envNextValue - env5Value) * inv32;
			this->env5ValueMem = envNextValue;

			env6Value = this->env6ValueMem;
			envNextValue = currentTimbre->env6.getNextAmpExp(&envState6);
			env6Inc = (envNextValue - env6Value) * inv32;
			this->env6ValueMem = envNextValue;

			oscState1.frequency = oscState1.mainFrequencyPlusMatrix;
			float* osc1Values = currentTimbre->osc1.getNextBlock(&oscState1);

			oscState2.frequency = oscState2.mainFrequencyPlusMatrix;
			float* osc2Values = currentTimbre->osc2.getNextBlock(&oscState2);

			oscState3.frequency = oscState3.mainFrequencyPlusMatrix;
			float* osc3Values = currentTimbre->osc3.getNextBlock(&oscState3);

			oscState4.frequency = oscState4.mainFrequencyPlusMatrix;
			float* osc4Values = currentTimbre->osc4.getNextBlock(&oscState4);

			oscState6.frequency = oscState6.mainFrequencyPlusMatrix;

			float div5TimesVelocity =   .2f * this->velocity;

			for (int k =0; k< BLOCK_SIZE; k++) {


				float car1 = osc1Values[k] * env1Value * mix1;
				float car2 = osc2Values[k] * env2Value * mix2;
				float car3 = osc3Values[k] * env3Value * mix3;
				float car4 = osc4Values[k] * env4Value * mix4;

				float freq6 = currentTimbre->osc6.getNextSample(&oscState6) * env6Value * oscState6.frequency;

				oscState5.frequency = freq6 * voiceIm1 + oscState5.mainFrequencyPlusMatrix;
				float car5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * mix5;

				*sample++ += (car1 * pan1Left + car2 * pan2Left + car3 * pan3Left + car4 * pan4Left + car5 * pan5Left)  * div5TimesVelocity;
				*sample++ += (car1 * pan1Right + car2 * pan2Right + car3 * pan3Right + car4 * pan4Right + car5 * pan5Right)  * div5TimesVelocity;

				env1Value += env1Inc;
				env2Value += env2Inc;
				env3Value += env3Inc;
				env4Value += env4Inc;
				env5Value += env5Inc;
				env6Value += env6Inc;
			}
			if (unlikely(currentTimbre->env1.isDead(&envState1) && currentTimbre->env2.isDead(&envState2) && currentTimbre->env3.isDead(&envState3)  && currentTimbre->env4.isDead(&envState4) && currentTimbre->env5.isDead(&envState5) && currentTimbre->env6.isDead(&envState6))) {
				endNoteOrBeginNextOne();
			}
		 }
		 break;

	} // End switch
}

void Voice::setCurrentTimbre(Timbre *timbre) {

    struct LfoParams* lfoParams[] = { &timbre->getParamRaw()->lfoOsc1, &timbre->getParamRaw()->lfoOsc2, &timbre->getParamRaw()->lfoOsc3};
    struct StepSequencerParams* stepseqparams[] = { &timbre->getParamRaw()->lfoSeq1, &timbre->getParamRaw()->lfoSeq2};
    struct StepSequencerSteps* stepseqs[] = { &timbre->getParamRaw()->lfoSteps1, &timbre->getParamRaw()->lfoSteps2};


    this->currentTimbre = timbre;

    matrix.init(&timbre->getParamRaw()->matrixRowState1);

    // OSC
    for (int k = 0; k < NUMBER_OF_LFO_OSC; k++) {
        float* phase = &((float*)&timbre->getParamRaw()->lfoPhases.phaseLfo1)[k];
        lfoOsc[k].init(lfoParams[k], phase, &this->matrix, (SourceEnum)(MATRIX_SOURCE_LFO1 + k), (DestinationEnum)(LFO1_FREQ + k));
    }

    // ENV
    lfoEnv[0].init(&timbre->getParamRaw()->lfoEnv1 , &this->matrix, MATRIX_SOURCE_LFOENV1, (DestinationEnum)0);
    lfoEnv2[0].init(&timbre->getParamRaw()->lfoEnv2 , &this->matrix, MATRIX_SOURCE_LFOENV2, (DestinationEnum)LFOENV2_SILENCE);

    // Step sequencer
    for (int k = 0; k< NUMBER_OF_LFO_STEP; k++) {
        lfoStepSeq[k].init(stepseqparams[k], stepseqs[k], &matrix, (SourceEnum)(MATRIX_SOURCE_LFOSEQ1+k), (DestinationEnum)(LFOSEQ1_GATE+k));
    }
}

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
}


void Voice::glideToNote(short newNote) {
    // Must glide...
    this->gliding = true;
    this->glidePhase = 0.0f;
    this->nextNote = newNote;

    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        currentTimbre->osc[k]->glideToNote(oscState[k], newNote);
    }
}

void Voice::noteOnWithoutPop(short newNote, short velocity, unsigned int index) {

    // Update index : so that few chance to be chosen again during the quick dying
    this->index = index;
    if (!this->released && (int)currentTimbre->params.engine1.numberOfVoice == 1 && currentTimbre->params.engine1.glide > 0) {
        glideToNote(newNote);
    } else {
        // update note now so that the noteOff is triggered by the new note
        this->note = newNote;
        // Quick dead !
        this->newNotePending = true;
        this->nextVelocity = velocity;
        this->nextNote = newNote;
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
    this->glidePhase += currentTimbre->params.engine1.glide * currentTimbre->params.engine1.glide * .001f;
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
    this->nextNote = 0;
    this->index = index;
    this->velocity = (float)velo * .0078125f; // divide by 127


    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        currentTimbre->osc[k]->newNote(oscState[k], newNote);
        currentTimbre->env[k]->noteOn(envState[k]);
    }

    currentTimbre->noteOn();
}

void Voice::glideNoteOff() {
    // while gliding the first note was released
    this->note = this->nextNote;
    this->nextNote = 0;
}

void Voice::noteOff() {
    this->note = 0;
    this->released = true;
    this->nextNote = 0;
    this->gliding = false;
    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        currentTimbre->env[k]->noteOff(envState[k]);
        currentTimbre->lfo[k]->noteOff();
    }
}

void Voice::killNow() {
    this->note = 0;
    this->playing = false;
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

    if (playing) {
        float *sample = currentTimbre->getSampleBlock();

        currentTimbre->calculateFrequencyWithMatrix(oscState);
        int numberOfOscillators = algoInformation[(int)currentTimbre->params.engine1.algo].osc;

        env1Value = this->env1ValueMem;
        envNextValue = currentTimbre->env1.getNextAmp(&envState1);
        env1Inc = (envNextValue - env1Value) * .03125f;  // divide by 32
        this->env1ValueMem = envNextValue;

        env2Value = this->env2ValueMem;
        envNextValue = currentTimbre->env2.getNextAmp(&envState2);
        env2Inc = (envNextValue - env2Value) * .03125f;
        this->env2ValueMem = envNextValue;

        env3Value = this->env3ValueMem;
        envNextValue = currentTimbre->env3.getNextAmp(&envState3);
        env3Inc = (envNextValue - env3Value) * .03125f;
        this->env3ValueMem = envNextValue;

        if (numberOfOscillators >= 4 ) {
            env4Value = this->env4ValueMem;
            envNextValue = currentTimbre->env4.getNextAmp(&envState4);
            env4Inc = (envNextValue - env4Value) * .03125f;
            this->env4ValueMem = envNextValue;
            if (numberOfOscillators == 6 ) {
                env5Value = this->env5ValueMem;
                envNextValue = currentTimbre->env5.getNextAmp(&envState5);
                env5Inc = (envNextValue - env5Value) * .03125f;
                this->env5ValueMem = envNextValue;
                env6Value = this->env6ValueMem;
                envNextValue = currentTimbre->env6.getNextAmp(&envState6);
                env6Inc = (envNextValue - env6Value) * .03125f;
                this->env6ValueMem = envNextValue;

            }
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
                float currentSample = currentTimbre->osc1.getNextSample(&oscState1)  * this->velocity * env1Value;

                currentSample *= currentTimbre->mix1;

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

            for (int k =0; k< BLOCK_SIZE; k++) {
                float freq3 = osc3Values[k] * env3Value * oscState3.frequency;

                oscState2.frequency =  freq3 * currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;
                float currentSample = currentTimbre->osc2.getNextSample(&oscState2)* env2Value * currentTimbre->mix2 * this->velocity * .5f;

                *sample++  += currentSample * currentTimbre->pan2Left;
                *sample--  += currentSample * currentTimbre->pan2Right;

                oscState1.frequency =  freq3 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
                currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * this->velocity * .5f;

                *sample++  += currentSample * currentTimbre->pan1Left;
                *sample++  += currentSample * currentTimbre->pan1Right;

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

            for (int k = 0; k< BLOCK_SIZE; k++) {
                float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

                oscState3.frequency =  freq4 * currentTimbre->modulationIndex4 + oscState3.mainFrequencyPlusMatrix;
                float freq3 = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * oscState3.frequency;

                oscState2.frequency =  freq4 * currentTimbre->modulationIndex2 +  freq3 * currentTimbre->modulationIndex3 + oscState2.mainFrequencyPlusMatrix;
                float currentSample = currentTimbre->osc2.getNextSample(&oscState2) *  env2Value *  currentTimbre->mix2 * this->velocity * .5f;

                *sample++  += currentSample * currentTimbre->pan2Left;
                *sample--  += currentSample * currentTimbre->pan2Right;

                oscState1.frequency =  freq3 * currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;
                currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * this->velocity * .5f;

                *sample++  += currentSample * currentTimbre->pan1Left;
                *sample++  += currentSample * currentTimbre->pan1Right;

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
                float currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * this->velocity * currentTimbre->mix1;

                *sample++  += currentSample * currentTimbre->pan1Left;
                *sample++  += currentSample * currentTimbre->pan1Right;

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

            for (int k =0; k< BLOCK_SIZE; k++) {

                float freq4 = osc4Values[k] * env4Value * oscState4.frequency;

                oscState3.frequency =  freq4 * currentTimbre->modulationIndex3 + oscState3.mainFrequencyPlusMatrix;

                float currentSample = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix3 * this->velocity * .3333f;

                *sample++  += currentSample * currentTimbre->pan3Left;
                *sample--  += currentSample * currentTimbre->pan3Right;

                oscState2.frequency =  freq4*currentTimbre->modulationIndex2 + oscState2.mainFrequencyPlusMatrix;

                currentSample = currentTimbre->osc2.getNextSample(&oscState2) * env2Value * currentTimbre->mix2 * this->velocity * .3333f;

                *sample++  += currentSample * currentTimbre->pan2Left;
                *sample--  += currentSample * currentTimbre->pan2Right;

                oscState1.frequency =  freq4*currentTimbre->modulationIndex1 + oscState1.mainFrequencyPlusMatrix;

                currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * this->velocity * .3333f;

                *sample++  += currentSample * currentTimbre->pan1Left;
                *sample++  += currentSample * currentTimbre->pan1Right;

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

            for (int k =0; k< BLOCK_SIZE; k++) {

                float freq4 = osc4Values[k] * env4Value;
                freq4 *=  oscState4.frequency;

                float freq2 = osc2Values[k] * env2Value;
                freq2 *=  oscState2.frequency;

                oscState6.frequency = freq4 * currentTimbre->modulationIndex4 +  oscState6.mainFrequencyPlusMatrix;
                float freq6 = currentTimbre->osc6.getNextSample(&oscState6) * env6Value * oscState6.frequency;

                oscState5.frequency = freq6 * currentTimbre->modulationIndex3 +  oscState5.mainFrequencyPlusMatrix;
                float currentSample = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix3 * .333f * this->velocity;

                *sample++  += currentSample  * currentTimbre->pan3Right;
                *sample--  += currentSample * currentTimbre->pan3Left;

                oscState3.frequency = freq4 * currentTimbre->modulationIndex2 +  oscState3.mainFrequencyPlusMatrix;
                currentSample = currentTimbre->osc3.getNextSample(&oscState3) * env3Value * currentTimbre->mix2 * .333f * this->velocity;

                *sample++  += currentSample  * currentTimbre->pan2Right;
                *sample--  += currentSample * currentTimbre->pan2Left;

                oscState1.frequency = freq2 * currentTimbre->modulationIndex1 +  oscState1.mainFrequencyPlusMatrix;
                currentSample = currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * .333f * this->velocity;

                *sample++  += currentSample  * currentTimbre->pan1Right;
                *sample++  += currentSample * currentTimbre->pan1Left;

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
                float currentSample =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * .5f * this->velocity;

                *sample++  += currentSample  * currentTimbre->pan1Right;
                *sample--  += currentSample * currentTimbre->pan1Left;


                oscState5.frequency = freq6 * currentTimbre->modulationIndex4 +  oscState5.mainFrequencyPlusMatrix;
                currentSample = currentTimbre->osc5.getNextSample(&oscState5) * env5Value * currentTimbre->mix2 * .5f * this->velocity;

                *sample++ += currentSample  * currentTimbre->pan2Right;
                *sample++  += currentSample * currentTimbre->pan2Left;

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

            for (int k =0; k< BLOCK_SIZE; k++) {

                float freq6 = osc6Values[k] * env6Value;
                freq6 *=  oscState6.frequency;

                float freq3 = osc3Values[k] * env3Value;
                freq3 *=  oscState3.frequency;

                float freq2 = osc2Values[k] * env2Value;
                freq2 *=  oscState2.frequency;

                oscState1.frequency =  freq2*currentTimbre->modulationIndex1 + freq3*currentTimbre->modulationIndex2 + oscState1.mainFrequencyPlusMatrix;
                float currentSample =  currentTimbre->osc1.getNextSample(&oscState1) * env1Value * currentTimbre->mix1 * .5f * this->velocity;

                *sample++  += currentSample  * currentTimbre->pan1Right;
                *sample--  += currentSample * currentTimbre->pan1Left;

                oscState5.frequency =  freq6 * currentTimbre->modulationIndex4 + oscState5.mainFrequencyPlusMatrix;
                float freq5 = currentTimbre->osc5.getNextSample(&oscState5) * env5Value;
                freq5 *=  oscState5.frequency;

                oscState4.frequency = freq5 * currentTimbre->modulationIndex3 +  oscState4.mainFrequencyPlusMatrix;

                currentSample = currentTimbre->osc4.getNextSample(&oscState4) * env4Value * currentTimbre->mix2 * .5f * this->velocity;

                *sample++ += currentSample  * currentTimbre->pan2Right;
                *sample++  += currentSample * currentTimbre->pan2Left;

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

        } // End switch
    }
}




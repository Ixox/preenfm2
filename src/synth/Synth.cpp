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

#include <stdlib.h>
#include "Synth.h"
#include "Menu.h"
#include "stm32f4xx_rng.h"


extern float noise[32];

Synth::Synth(void) {
}

Synth::~Synth(void) {
}

void Synth::init() {
	int numberOfVoices[]= { 5, 1, 1, 1};
    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        for (int k=0; k<sizeof(struct OneSynthParams)/sizeof(float); k++) {
            ((float*)&timbres[t].params)[k] = ((float*)&preenMainPreset)[k];
        }
        timbres[t].params.engine1.numberOfVoice = numberOfVoices[t];
        timbres[t].init(t);
        for (int v=0; v<MAX_NUMBER_OF_VOICES; v++) {
        	timbres[t].initVoicePointer(v, &voices[v]);
        }
    }

    newTimbre(0);
    this->writeCursor = 0;
    this->readCursor = 0;
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        voices[k].init(&timbres[0], &timbres[1], &timbres[2], &timbres[3]);
    }
    rebuidVoiceTimbre();
    refreshNumberOfOsc();
}


void Synth::noteOn(int timbre, char note, char velocity) {
	timbres[timbre].noteOn(note, velocity);
}

void Synth::noteOff(int timbre, char note) {
	timbres[timbre].noteOff(note);
}

void Synth::setHoldPedal(int timbre, int value) {
	timbres[timbre].setHoldPedal(value);
}

void Synth::allNoteOff(int timbre) {
    int numberOfVoices = timbres[timbre].params.engine1.numberOfVoice;
    for (int k = 0; k < numberOfVoices; k++) {
        // voice number k of timbre
        int n = timbres[timbre].voiceNumber[k];
        voices[n].noteOff();
    }
}

void Synth::allSoundOff(int timbre) {
    int numberOfVoices = timbres[timbre].params.engine1.numberOfVoice;
    for (int k = 0; k < numberOfVoices; k++) {
        // voice number k of timbre
        int n = timbres[timbre].voiceNumber[k];
        voices[n].killNow();
    }
}

void Synth::allSoundOff() {
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        voices[k].killNow();
    }
}

bool Synth::isPlaying() {
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        if (voices[k].isPlaying()) {
            return true;
        }
    }
    return false;
}


void Synth::buildNewSampleBlock() {
    float *currentBlock = &samples[writeCursor];
    float nvi = this->numberOfVoiceInverse;
    float toAdd = 131071.0f;

    // If we are loading we must not play, the voices assignation can be broken
	// Noise... part
	int noiseIndex = 0;
	for (int k=0; k<16; k++) {
		if (RNG_GetFlagStatus(RNG_FLAG_DRDY) != RESET) {
			/* Get a 32bit Random number */
			random32bit = RNG_GetRandomNumber();
		} else {
			random32bit = 214013 * random32bit + 2531011;
		}

		float value1 = random32bit & 0xffff;
		float value2 = random32bit >> 16;
		float ratioValue = .000030518f;
		noise[noiseIndex++] = value1 * ratioValue - 1.0f; // value between -1 and 1.
		noise[noiseIndex++] = value2 * ratioValue - 1.0f; // value between -1 and 1.
	}


	for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
		timbres[t].prepareForNextBlock();
		if (likely(timbres[t].voiceNumber[0] != -1)) {
			// glide only happens on first voice of any
			this->voices[timbres[t].voiceNumber[0]].glide();
		}
	}

	// render all voices in their timbre sample block...
	// 16 voices
	int k=0;
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();
	this->voices[k++].nextBlock();


	// Add timbre per timbre because gate and eventual other effect are per timbre
	timbres[0].fxAfterBlock();
	timbres[1].fxAfterBlock();
	timbres[2].fxAfterBlock();
	timbres[3].fxAfterBlock();
	float *sampleFromTimbre1 = timbres[0].getSampleBlock();
	float *sampleFromTimbre2 = timbres[1].getSampleBlock();
	float *sampleFromTimbre3 = timbres[2].getSampleBlock();
	float *sampleFromTimbre4 = timbres[3].getSampleBlock();
	float *cb = &samples[writeCursor];
	// 64 adress to copy
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	// 8
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
	*cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;

    if (unlikely(writeCursor == 192)) {
        writeCursor = 0;
        return;
    } else {
        writeCursor += 64;
        return;
    }
}

void Synth::beforeNewParamsLoad(int timbre) {
    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        for (int v=0; v<MAX_NUMBER_OF_VOICES; v++) {
        	timbres[t].voiceNumber[v] = -1;
        }
    }
    // Stop all voices
    allSoundOff();
};

void Synth::afterNewParamsLoad(int timbre) {
    timbres[timbre].afterNewParamsLoad();
    // Reset to 0 the number of voice then try to set the right value
    int numberOfVoice = timbres[timbre].params.engine1.numberOfVoice;
    timbres[timbre].params.engine1.numberOfVoice = 0;


    // refresh a first time with numberofvoice = 0
    refreshNumberOfOsc();

    int freeOsc = MAX_NUMBER_OF_OPERATORS - this->numberOfOsc;
    float voicesMax = (float)freeOsc / (float)algoInformation[(int)timbres[timbre].params.engine1.algo].osc ;

    if (numberOfVoice > voicesMax) {
    	timbres[timbre].params.engine1.numberOfVoice = voicesMax;
    } else {
    	timbres[timbre].params.engine1.numberOfVoice = numberOfVoice;
    }
    // Refresh again so that the value is up to date
    refreshNumberOfOsc();
    rebuidVoiceTimbre();
}

void Synth::afterNewComboLoad() {
    for (int t=0; t<NUMBER_OF_TIMBRES ; t++) {
    	timbres[t].afterNewParamsLoad();
    }
    rebuidVoiceTimbre();
    refreshNumberOfOsc();
}

int Synth::getFreeVoice() {
	// Loop on all voice
    for (int voice=0; voice< MAX_NUMBER_OF_VOICES; voice++) {
        bool used = false;

        for (int t=0; t<NUMBER_OF_TIMBRES && !used; t++) {
            int interVoice = 0;
            for (int v=0; v<MAX_NUMBER_OF_VOICES && interVoice!=-1 && !used; v++) {
                interVoice = timbres[t].voiceNumber[v];
                if (interVoice == voice) {
                    used = true;
                }
            }
        }
        if (!used) {
            return voice;
        }
    }
    return -1;
}

// can prevent some value change...
void Synth::checkNewParamValue(int timbre, int currentRow, int encoder, float oldValue, float *newValue) {
    if (unlikely(currentRow == ROW_ENGINE)) {

        int freeOsc = MAX_NUMBER_OF_OPERATORS - numberOfOsc;
        if (unlikely(encoder == ENCODER_ENGINE_ALGO)) {
            // If one voice exactly and not enough free osc to change algo
        	// If more than 1 voice, it's OK, the number of voice will be reduced later.
            if (timbres[timbre].params.engine1.numberOfVoice < 1.5f && timbres[timbre].params.engine1.numberOfVoice > 0.5f
                    &&  freeOsc < (algoInformation[(int)(*newValue)].osc - algoInformation[(int)oldValue].osc)) {
                // not enough free osc
                *newValue = oldValue;
            }
        }
        if (unlikely(encoder == ENCODER_ENGINE_VOICE)) {

            // Increase number of voice ?
            if ((*newValue)> oldValue) {
                if (freeOsc < ((*newValue) - oldValue) * algoInformation[(int)timbres[timbre].params.engine1.algo].osc) {
                    *newValue = oldValue;
                }
            }
        }
    }
}

void Synth::newParamValue(int timbre, SynthParamType type, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {

    if (unlikely(type == SYNTH_PARAM_TYPE_ENGINE )) {
        if (currentRow == ROW_ENGINE) {
            if (encoder == ENCODER_ENGINE_ALGO) {
                refreshNumberOfOsc();
                fixMaxNumberOfVoices(timbre);
            } else if (encoder == ENCODER_ENGINE_VOICE) {
                if (newValue > oldValue) {
                    for (int v=(int)oldValue; v < (int)newValue; v++) {
                    	timbres[timbre].voiceNumber[v] = getFreeVoice();
                        this->numberOfVoices ++;
                    }
                    this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;
                    refreshNumberOfOsc();
                } else {
                    for (int v=(int)newValue; v < (int)oldValue; v++) {
                        voices[timbres[timbre].voiceNumber[v]].killNow();
                        timbres[timbre].voiceNumber[v] = -1;
                        this->numberOfVoices --;
                    }
                    if (this->numberOfVoices > 0) {
                        this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;
                    } else {
                        this->numberOfVoiceInverse =  131071.0f;
                        // Redraw same timbre....
                        synthState->propagateNewTimbre(timbre);
                    }
                    refreshNumberOfOsc();
                }

            }
            /* DEBUG TEST setHoldPedal...
            else if (encoder == ENCODER_ENGINE_VELOCITY) {
            	if (newValue == 6) {
            		setHoldPedal(0, 80);
            	} else if (newValue == 7) {
            		setHoldPedal(0, 20);
            	}
            }
            */
        } else if (currentRow == ROW_ARPEGGIATOR1) {
        	if (encoder == ENCODER_ARPEGGIATOR_CLOCK) {
        		timbres[timbre].setArpeggiatorClock((uint8_t) newValue);
        	}
        	if (encoder == ENCODER_ARPEGGIATOR_BPM) {
        		timbres[timbre].setNewBPMValue((uint8_t) newValue);
        	}
        } else if (currentRow == ROW_ARPEGGIATOR2) {
        	if (encoder == ENCODER_ARPEGGIATOR_LATCH) {
        		timbres[timbre].setLatchMode((uint8_t) newValue);
        	}
        }
    } else if (type == SYNTH_PARAM_TYPE_ENV) {
        switch (currentRow) {
        case ROW_ENV1a:
            timbres[timbre].env1.reloadADSR(encoder);
            break;
        case ROW_ENV1b:
            timbres[timbre].env1.reloadADSR(encoder + 4);
            break;
        case ROW_ENV2a:
            timbres[timbre].env2.reloadADSR(encoder);
            break;
        case ROW_ENV2b:
            timbres[timbre].env2.reloadADSR(encoder + 4);
            break;
        case ROW_ENV3a:
            timbres[timbre].env3.reloadADSR(encoder);
            break;
        case ROW_ENV3b:
            timbres[timbre].env3.reloadADSR(encoder + 4);
            break;
        case ROW_ENV4a:
            timbres[timbre].env4.reloadADSR(encoder);
            break;
        case ROW_ENV4b:
            timbres[timbre].env4.reloadADSR(encoder + 4);
            break;
        case ROW_ENV5a:
            timbres[timbre].env5.reloadADSR(encoder);
            break;
       case ROW_ENV5b:
    	    timbres[timbre].env5.reloadADSR(encoder + 4);
            break;
        case ROW_ENV6a:
            timbres[timbre].env6.reloadADSR(encoder);
            break;
        case ROW_ENV6b:
            timbres[timbre].env6.reloadADSR(encoder + 4);
            break;
        }
    } else if (type == SYNTH_PARAM_TYPE_MATRIX && encoder == ENCODER_MATRIX_DEST) {
        // Reset old destination
        timbres[timbre].matrix.resetDestination(oldValue);
    } else if (type == SYNTH_PARAM_TYPE_LFO) {
        timbres[timbre].lfo[currentRow - ROW_LFOOSC1]->valueChanged(encoder);
    }

}


// synth is the only one who knows timbres
void Synth::newTimbre(int timbre)  {
//    allParameterRows.row[ROW_ENGINE]->params[ENCODER_ENGINE_VOICE].maxValue = this->numberOfVoicesMax[timbre];
//    allParameterRows.row[ROW_ENGINE]->params[ENCODER_ENGINE_VOICE].numberOfValues = this->numberOfVoicesMax[timbre];
    this->synthState->setParamsAndTimbre(&timbres[timbre].params, timbre);
}

void Synth::newMidiConfig(int menuSelect, char newValue) {
    /*
    if (menuSelect == MIDICONFIG_SYNTHMODE) {
        if (newValue == SYNTH_MODE_SINGLE) {

            for (int k = 0; k<MAX_NUMBER_OF_VOICES; k++) {
                if (voices[k].getCurrentTimbre() > 0 ) {
                    voices[k].killNow();
                }
            }
            numberOfTimbres = 1;
        } else {
            int voicesNumber[] = { 4, 2, 1, 1};
            numberOfTimbres = 4;
            for (int t=0; t<numberOfTimbres; t++) {
                timbres[t].params.engine1.numberOfVoice = voicesNumber[t];
            }
        }
        newTimbre(0);
        rebuidVoiceTimbre();
        refreshNumberOfOsc();
        fixMaxNumberOfVoices(0);
    }
    */
}

/*
 * Return false if had to change number of voice
 *
 */
bool Synth::fixMaxNumberOfVoices(int timbre) {
    int freeOsc = MAX_NUMBER_OF_OPERATORS - this->numberOfOsc;

    float voicesMax = timbres[timbre].params.engine1.numberOfVoice + .001f + (float)freeOsc / (float)algoInformation[(int)timbres[timbre].params.engine1.algo].osc ;

    if (this->timbres[timbre].params.engine1.numberOfVoice > voicesMax) {
        int oldValue = this->timbres[timbre].params.engine1.numberOfVoice;
        this->timbres[timbre].params.engine1.numberOfVoice = voicesMax;
        ParameterDisplay *params = &allParameterRows.row[ROW_ENGINE]->params[ENCODER_ENGINE_VOICE];
        synthState->propagateNewParamValue(timbre, ROW_ENGINE, ENCODER_ENGINE_VOICE, params, oldValue, voicesMax );
        return false;
    }

    return true;
}



void Synth::rebuidVoiceTimbre() {
    int voices = 0;

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        int nv = timbres[t].params.engine1.numberOfVoice;

        for (int v=0; v < nv; v++) {
        	timbres[t].voiceNumber[v] = voices++;
        }
        for (int v=nv; v < NUMBER_OF_TIMBRES;  v++) {
        	timbres[t].voiceNumber[v] = -1;
        }
    }

    // update globel number of Voices
    this->numberOfVoices = voices;
    this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;
}

void Synth::refreshNumberOfOsc() {
    this->numberOfOsc = 0;

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        int nv = timbres[t].params.engine1.numberOfVoice + .0001f;
		this->numberOfOsc += algoInformation[(int)timbres[t].params.engine1.algo].osc * nv;
    }
}



void Synth::setNewValueFromMidi(int timbre, int row, int encoder, float newValue) {
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    int index = row * NUMBER_OF_ENCODERS + encoder;
    float oldValue = ((float*)this->timbres[timbre].getParamRaw())[index];
    this->timbres[timbre].setNewValue(index, param, newValue);
    this->synthState->propagateNewParamValueFromExternal(timbre, row, encoder, param, oldValue, newValue);
}


#ifdef DEBUG

// ========================== DEBUG ========================
void Synth::debugVoice() {

	lcd.setRealTimeAction(true);
	lcd.clearActions();
	lcd.clear();

    int numberOfVoices = timbres[0].params.engine1.numberOfVoice;

    for (int k = 0; k < numberOfVoices && k < 4; k++) {
        // voice number k of timbre
        int n = timbres[0].voiceNumber[k];

        lcd.setCursor(0, k);
        lcd.print((int)voices[n].getNote());

        lcd.setCursor(4, k);
        lcd.print((int)voices[n].getNextPendingNote());

        lcd.setCursor(12, k);
        lcd.print((int)voices[n].getIndex());

        lcd.setCursor(17, k);
        lcd.print((int) voices[n].isReleased());
		lcd.print((int) voices[n].isPlaying());
    }
	lcd.setRealTimeAction(false);
}
#endif

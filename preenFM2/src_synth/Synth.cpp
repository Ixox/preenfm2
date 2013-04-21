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

#include <stdlib.h>
#include "Synth.h"
#include "Menu.h"
#include "stm32f4xx_rng.h"

//#include "LiquidCrystal.h"
//extern LiquidCrystal lcd;

extern float noise[32];

Synth::Synth(void) {
}

Synth::~Synth(void) {
}

void Synth::init() {

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        for (int k=0; k<sizeof(struct OneSynthParams)/sizeof(float); k++) {
            ((float*)&timbres[t].params)[k] = ((float*)&presets[t])[k];
        }
        timbres[t].init();
    }

    newTimbre(0);
    this->writeCursor = 0;
    this->readCursor = 0;
    this->numberOfVoices = timbres[0].params.engine1.numberOfVoice;
    this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        voices[k].init(&timbres[0], &timbres[1], &timbres[2], &timbres[3]);
    }
    rebuidVoiceTimbre();
    refreshNumberOfOsc();
    fixMaxNumberOfVoices(0);
}



void Synth::noteOn(int timbre, char note, char velocity) {
    if (note<24 || note >107) {
        return;
    }

    int zeroVelo = (16 - timbres[timbre].params.engine1.velocity) * 8;
    int newVelocity = zeroVelo + velocity * (128 - zeroVelo) / 128;

    unsigned int indexMin = (unsigned int)2147483647;
    int voiceToUse = -1;

    int numberOfVoices = timbres[timbre].params.engine1.numberOfVoice;

    for (int k = 0; k < numberOfVoices; k++) {
        // voice number k of timbre
        int n = voiceTimbre[timbre][k];

        // same note.... ?
        if (voices[n].getNote() == note) {
            voices[n].noteOnWithoutPop(note, newVelocity, voiceIndex++);
            return;
        }

        if (!voices[n].isPlaying()) {
            voices[n].noteOn(timbre, note, newVelocity, voiceIndex++);
            return;
        }

        if (voices[n].isReleased()) {
            int indexVoice = voices[n].getIndex();
            if (indexVoice < indexMin) {
                indexMin = indexVoice;
                voiceToUse = n;
            }
        }
    }

    if (voiceToUse == -1) {
        for (int k = 0; k < numberOfVoices; k++) {
            // voice number k of timbre
            int n = voiceTimbre[timbre][k];
            int indexVoice = voices[n].getIndex();
            if (indexVoice < indexMin) {
                indexMin = indexVoice;
                voiceToUse = n;
            }
        }
    }
    voices[voiceToUse].noteOnWithoutPop(note, newVelocity, voiceIndex++);
}

void Synth::noteOff(int timbre, char note) {
    int numberOfVoices = timbres[timbre].params.engine1.numberOfVoice;

    for (int k = 0; k < numberOfVoices; k++) {
        // voice number k of timbre
        int n = voiceTimbre[timbre][k];

        if (voices[n].getNextNote() == 0) {
            if (voices[n].getNote() == note) {
                voices[n].noteOff();
                return;
            }
        } else {
            // if gliding and releasing first note
            if (voices[n].getNote() == note) {
                voices[n].glideNoteOff();
                return;
            }
            // if gliding and releasing next note
            if (voices[n].getNextNote() == note) {
                voices[n].glideToNote(voices[n].getNote());
                voices[n].glideNoteOff();
                return;
            }
        }
    }
}

void Synth::allNoteOff() {
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        voices[k].noteOff();
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


int gCpt = 0;

void Synth::buildNewSampleBlock() {
    float *currentBlock = &samples[writeCursor];

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
        noise[noiseIndex++] = value1 * .000030518f - 1.0f; // value between -1 and 1.
        noise[noiseIndex++] = value2 * .000030518f - 1.0f; // value between -1 and 1.
    }

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        timbres[t].prepareForNextBlock();
        if (timbres[t].params.engine1.numberOfVoice > 0) {
            // glide only happens on first voice of any
            this->voices[voiceTimbre[t][0]].glide();
        }
    }

    // render all voices in their timbre sample block...
    for (int k = 0; k < MAX_NUMBER_OF_VOICES; ) {
        this->voices[k++].nextBlock();
        this->voices[k++].nextBlock();
        this->voices[k++].nextBlock();
        this->voices[k++].nextBlock();
    }


    float nvi = this->numberOfVoiceInverse;
    float toAdd = 131071.0f;

    // Add timbre per timbre because gate and eventual other effect are per timbre
    /*
    if (numberOfTimbres == 1) {
        timbres[0].fxAfterBlock();
        float *sampleFromTimbre = timbres[0].getSampleBlock();
        float *cb = &samples[writeCursor];
        float *cbMax = &cb[64];
        while (cb < cbMax) {
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
            *cb++ = *sampleFromTimbre++  * nvi  + toAdd;
        }
    }
    else {
    */
    timbres[0].fxAfterBlock();
    timbres[1].fxAfterBlock();
    timbres[2].fxAfterBlock();
    timbres[3].fxAfterBlock();
    float *sampleFromTimbre1 = timbres[0].getSampleBlock();
    float *sampleFromTimbre2 = timbres[1].getSampleBlock();
    float *sampleFromTimbre3 = timbres[2].getSampleBlock();
    float *sampleFromTimbre4 = timbres[3].getSampleBlock();
    float *cb = &samples[writeCursor];
    float *cbMax = &cb[64];
    while (cb < cbMax) {
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
        *cb++ = (*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) * nvi + toAdd;
    }
//    }


/*
    // gate on timbre 0 only
    // Gate algo !!
    float gate = timbres[0].matrix.getDestination(MAIN_GATE);

//    if (gate > 0) {

        float targetGate = (1 - gate);
        targetGate = targetGate < 0.0f ? 0.0f : (targetGate > 1.0f ? 1.0f : targetGate);

        float currentGate = timbres[0].currentGate;

        float gateInc = (targetGate - currentGate) * .03125f; // in 32 steps should be able to go from currentGate  to gate...


        for (int k=0; k<32; k++) {
            currentGate += gateInc;
//            float multiplyGate = (1 - currentGate);
//            currentBlock[k*2] =  currentBlock[k*2] * multiplyGate;
//            currentBlock[k*2+1] =  currentBlock[k*2+1] * multiplyGate;

            currentBlock[k*2] =  currentBlock[k*2] * currentGate;
            currentBlock[k*2+1] =  currentBlock[k*2+1] * currentGate;
        }
        timbres[0].currentGate = targetGate;
//    }


    for (int k = 0; k < MAX_NUMBER_OF_VOICES; k++) {
        if (voices[k].getCurrentTimbre() != 0) {
            this->voices[k].nextBlock();
        }
    }
*/

    if (writeCursor == 192) {
        writeCursor = 0;
    } else {
        writeCursor += 64;
    }


}

void Synth::checkMaxVoice(bool setEngineVoice) {
}

void Synth::afterNewParamsLoad(int timbre) {
    timbres[timbre].afterNewParamsLoad();
    checkMaxVoice(false);
    rebuidVoiceTimbre();
    refreshNumberOfOsc();
    fixMaxNumberOfVoices(timbre);
}


int Synth::getFreeVoice() {
    for (int voice=1; voice< MAX_NUMBER_OF_VOICES; voice++) {
        bool found = false;

        for (int t=0; t<NUMBER_OF_TIMBRES && !found; t++) {
            int interVoice = 0;
            for (int v=0; v<MAX_NUMBER_OF_VOICES && interVoice!=-1 && !found; v++) {
                interVoice = voiceTimbre[t][v];
                if (interVoice == voice) {
                    found = true;
                }
            }
        }
        if (!found) {
            return voice;
        }
    }
    return -1;
}

void Synth::checkNewParamValue(int timbre, int currentRow, int encoder, float oldValue, float *newValue) {
    if (currentRow == ROW_ENGINE) {

        int freeOsc = 48 - numberOfOsc;
        if (encoder == ENCODER_ENGINE_ALGO) {
            // if one voice and not enough free osc to change algo
            if (timbres[timbre].params.engine1.numberOfVoice < 1.5f
                    &&  freeOsc < (algoInformation[(int)(*newValue)].osc - algoInformation[(int)oldValue].osc)) {
                // not enough free osc
                *newValue = oldValue;
            }
        } else if (encoder == ENCODER_ENGINE_VOICE) {

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

    if (type == SYNTH_PARAM_TYPE_ENGINE ) {
        if (currentRow == ROW_ENGINE) {
            if (encoder == ENCODER_ENGINE_ALGO) {
                checkMaxVoice(true);
                refreshNumberOfOsc();
                fixMaxNumberOfVoices(timbre);
            } else if (encoder == ENCODER_ENGINE_VOICE) {
                if (newValue > oldValue) {
                    for (int v=(int)oldValue; v < (int)newValue; v++) {
                        voiceTimbre[timbre][v] = getFreeVoice();
                        this->numberOfVoices ++;
                    }
                    this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;
                    refreshNumberOfOsc();
                } else {
                    for (int v=(int)newValue; v < (int)oldValue; v++) {
                        voices[voiceTimbre[timbre][v]].killNow();
                        voiceTimbre[timbre][v] = -1;
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
        } else if (currentRow == ROW_PERFORMANCE) {
            timbres[timbre].matrix.setSource((enum SourceEnum)(MATRIX_SOURCE_CC1 + encoder), newValue);
        }
    } else if (type == SYNTH_PARAM_TYPE_ENV) {
        switch (currentRow) {
        case ROW_ENV1:
            timbres[timbre].env1.reloadADSR(encoder);
            break;
        case ROW_ENV2:
            timbres[timbre].env2.reloadADSR(encoder);
            break;
        case ROW_ENV3:
            timbres[timbre].env3.reloadADSR(encoder);
            break;
        case ROW_ENV4:
            timbres[timbre].env4.reloadADSR(encoder);
            break;
        case ROW_ENV5:
            timbres[timbre].env5.reloadADSR(encoder);
            break;
        case ROW_ENV6:
            timbres[timbre].env6.reloadADSR(encoder);
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
    int freeOsc = 48 - this->numberOfOsc;

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
    // timbre 0 if SINGLE MODE or 0,1,2,4 if COMBO MODDE
    int voices = 0;

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        int nv = timbres[t].params.engine1.numberOfVoice;

        for (int v=0; v < nv; v++) {
            voiceTimbre[t][v] = voices++;
        }
        for (int v=nv; v < NUMBER_OF_TIMBRES;  v++) {
            voiceTimbre[t][v] = -1;
        }
    }

    // update globel number of Voices
    this->numberOfVoices = voices;
    this->numberOfVoiceInverse =  131071.0f / this->numberOfVoices;


}

void Synth::refreshNumberOfOsc() {
    this->numberOfOsc = 0;

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        int nv = timbres[t].params.engine1.numberOfVoice + .0001;
        for (int v=0; v < nv; v++) {
            this->numberOfOsc += algoInformation[(int)timbres[t].params.engine1.algo].osc;
        }
    }
/*
    int freeOsc = 48 - this->numberOfOsc;
    lcd.setCursor(18,1);
    if (freeOsc < 10) {
        lcd.print(' ');
    }
    lcd.print(freeOsc);
*/
}



void Synth::setNewValueFromMidi(int timbre, int row, int encoder, int newMidiValue) {
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    int index = row * NUMBER_OF_ENCODERS + encoder;
    int oldValue = ((float*)this->timbres[timbre].getParamRaw())[index];
    float newValue = 0;
    float step = ((param->maxValue - param->minValue) / (param->numberOfValues - 1.0f));
    if (param->numberOfValues <= 128) {
        newValue = param->minValue + newMidiValue * step;
    } else {
        step *= 2;
        newValue = param->minValue + newMidiValue * step;
    }
    this->timbres[timbre].setNewValue(index, param, newValue);
    this->synthState->propagateNewParamValueFromExternal(timbre, row, encoder, param, oldValue, newValue);
}

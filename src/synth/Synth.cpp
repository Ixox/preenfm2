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


#include "hardware/dwt.h"

#ifdef DEBUG
CYCCNT_buffer cycles_all;
#endif

extern float noise[32];
float ratiosTimbre[]= { 131072.0f * 1.0f, 131072.0f * 1.0f, 131072.0f *  0.5f, 131072.0f * 0.333f, 131072.0f * 0.25f };



Synth::Synth(void) {
}

Synth::~Synth(void) {
}

void Synth::init() {
    int numberOfVoices[]= { 6, 0, 0, 0 };
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
        voices[k].init();
    }
    rebuidVoiceTimbre();
    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        timbres[t].numberOfVoicesChanged();
    }
    updateNumberOfActiveTimbres();

#ifdef CVIN
    cvin12Ready = true;
    cvin34Ready = true;
#endif

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
        if (voices[n].isPlaying() && !voices[n].isReleased()) {
            voices[n].noteOff();
        }
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


#ifdef DEBUG
int cptDisplay = 0;
float totalCycles = 0;
#endif


void Synth::buildNewSampleBlock() {
    CYCLE_MEASURE_START(cycles_all);

    // We consider the random number is always ready here...
    uint32_t random32bit = RNG_GetRandomNumber();
    noise[0] =  (random32bit & 0xffff) * .000030518f - 1.0f; // value between -1 and 1.
    noise[1] = (random32bit >> 16) * .000030518f - 1.0f; // value between -1 and 1.
    for (int noiseIndex = 2; noiseIndex<32; ) {
        random32bit = 214013 * random32bit + 2531011;
        noise[noiseIndex++] =  (random32bit & 0xffff) * .000030518f - 1.0f; // value between -1 and 1.
        noise[noiseIndex++] = (random32bit >> 16) * .000030518f - 1.0f; // value between -1 and 1.
    }

#ifdef CVIN
    cvin->updateValues();
#endif

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        timbres[t].cleanNextBlock();
        if (likely(timbres[t].params.engine1.numberOfVoice > 0)) {
            timbres[t].prepareForNextBlock();
            // need to glide ?
            if (timbres[t].voiceNumber[0] != -1 && this->voices[timbres[t].voiceNumber[0]].isGliding()) {
                this->voices[timbres[t].voiceNumber[0]].glide();
            }
        }

#ifdef CVIN
        timbres[t].setMatrixSource(MATRIX_SOURCE_CVIN1, cvin->getCvin1());
        timbres[t].setMatrixSource(MATRIX_SOURCE_CVIN2, cvin->getCvin2());
        timbres[t].setMatrixSource(MATRIX_SOURCE_CVIN3, cvin->getCvin3());
        timbres[t].setMatrixSource(MATRIX_SOURCE_CVIN4, cvin->getCvin4());
#endif
        timbres[t].prepareMatrixForNewBlock();
    }


#ifdef CVIN
    // We need matrix source in osc
    // cvin1 can trigger Instrument 1 notes
    int cvinstrument = synthState->fullState.midiConfigValue[MIDICONFIG_CVIN1_2];
    if (cvinstrument >= 0) {
        int timbreIndex = 0;
        int timbreToTrigger[4];
        switch (cvinstrument) {
            case 1:
                timbreToTrigger[timbreIndex++] = 0;
            break;
            case 2:
                timbreToTrigger[timbreIndex++] = 1;
            break;
            case 3:
                timbreToTrigger[timbreIndex++] = 2;
            break;
            case 4: 
                timbreToTrigger[timbreIndex++] = 3;
            break;
            case 5:
                timbreToTrigger[timbreIndex++] = 0;
                timbreToTrigger[timbreIndex++] = 1;
            break;
            case 6: 
                timbreToTrigger[timbreIndex++] = 0;
                timbreToTrigger[timbreIndex++] = 1;
                timbreToTrigger[timbreIndex++] = 2;
            break;
            case 7:
                timbreToTrigger[timbreIndex++] = 0;
                timbreToTrigger[timbreIndex++] = 1;
                timbreToTrigger[timbreIndex++] = 2;
                timbreToTrigger[timbreIndex++] = 3;
            break;
        }

        // CV_GATE from 0 to 100 => cvGate from 62 to 962. 
        // Which leaves some room for the histeresit algo bellow.
        int cvGate = synthState->fullState.midiConfigValue[MIDICONFIG_CV_GATE] * 9 + 62;
        if (cvin12Ready) {
            if (cvin->getGate() > cvGate) {
                cvin12Ready = false;
                for (int tk = 0; tk < timbreIndex; tk++ ) {
                    timbres[timbreToTrigger[tk]].setCvFrequency(cvin->getFrequency());
                    timbres[timbreToTrigger[tk]].noteOn(128, 127);
                    visualInfo->noteOn(timbreToTrigger[tk], true);
                }
            }
        } else {
            if (cvin->getGate() > (cvGate + 50)) {
                // Adjust frequency with CVIN2 !!! while gate is on !!
                for (int tk = 0; tk < timbreIndex; tk++ ) {
                    timbres[timbreToTrigger[tk]].setCvFrequency(cvin->getFrequency());
                    timbres[timbreToTrigger[tk]].propagateCvFreq(128);
                }
            } else if (cvin->getGate() < (cvGate - 50)) {
                for (int tk = 0; tk < timbreIndex; tk++ ) {
                    timbres[timbreToTrigger[tk]].noteOff(128);
                }
                cvin12Ready = true;
            }
        }
    }
#endif

    // render all voices in their timbre sample block...
    // 16 voices

    for (int v = 0; v < MAX_NUMBER_OF_VOICES; v++) {
        if (likely(this->voices[v].isPlaying())) {
            this->voices[v].nextBlock();
        }
    }

    // Add timbre per timbre because gate and eventual other effect are per timbre
    if (likely(timbres[0].params.engine1.numberOfVoice > 0)) {
        timbres[0].fxAfterBlock(ratioTimbre);
    }
    if (likely(timbres[1].params.engine1.numberOfVoice > 0)) {
        timbres[1].fxAfterBlock(ratioTimbre);
    }
    if (likely(timbres[2].params.engine1.numberOfVoice > 0)) {
        timbres[2].fxAfterBlock(ratioTimbre);
    }
    if (likely(timbres[3].params.engine1.numberOfVoice > 0)) {
        timbres[3].fxAfterBlock(ratioTimbre);
    }

    const float *sampleFromTimbre1 = timbres[0].getSampleBlock();
    const float *sampleFromTimbre2 = timbres[1].getSampleBlock();
    const float *sampleFromTimbre3 = timbres[2].getSampleBlock();
    const float *sampleFromTimbre4 = timbres[3].getSampleBlock();

    int *cb = &samples[writeCursor];

    float toAdd = 131071.0f;
    for (int s = 0; s < 64/4; s++) {
        *cb++ = (int)((*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) + toAdd);
        *cb++ = (int)((*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) + toAdd);
        *cb++ = (int)((*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) + toAdd);
        *cb++ = (int)((*sampleFromTimbre1++ + *sampleFromTimbre2++ + *sampleFromTimbre3++ + *sampleFromTimbre4++) + toAdd);
    }

    writeCursor = (writeCursor + 64) & 255;

    CYCLE_MEASURE_END();

#ifdef DEBUG
    if (cptDisplay++ > 500) {
        totalCycles += cycles_all.remove();

        if (cptDisplay == 600) {
            float max = SystemCoreClock * 32.0f * PREENFM_FREQUENCY_INVERSED;
            float percent = totalCycles / max;
            lcd.setCursor(14, 1);
            lcd.print('>');
            lcd.printWithOneDecimal(percent);
            lcd.print('%');
            cptDisplay = 0;
            totalCycles = 0;
        }
    }
#endif
}

void Synth::beforeNewParamsLoad(int timbre) {
    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        timbres[t].resetArpeggiator();
        for (int v=0; v<MAX_NUMBER_OF_VOICES; v++) {
            timbres[t].setVoiceNumber(v, -1);
        }
    }
    // Stop all voices
    allSoundOff();
};


int Synth::getNumberOfFreeVoicesForThisTimbre(int timbre) {
    int maxNumberOfFreeVoice = 0;
    for (int t=0 ; t< NUMBER_OF_TIMBRES; t++) {
        if (t != timbre) {
            maxNumberOfFreeVoice += timbres[t].params.engine1.numberOfVoice;
        }
    }
    maxNumberOfFreeVoice =  MAX_NUMBER_OF_VOICES - maxNumberOfFreeVoice;

    int freeOsc = getNumberOfFreeOscForThisTimbre(timbre);
    int maxNumberOfFreeVoicesWithOperators = freeOsc / algoInformation[(int)timbres[timbre].params.engine1.algo].osc ;

    return maxNumberOfFreeVoicesWithOperators < maxNumberOfFreeVoice ? maxNumberOfFreeVoicesWithOperators : maxNumberOfFreeVoice;
}

void Synth::afterNewParamsLoad(int timbre) {
    // Reset to 0 the number of voice then try to set the right value
    int numberOfVoice = timbres[timbre].params.engine1.numberOfVoice;
    int voicesMax = getNumberOfFreeVoicesForThisTimbre(timbre);
    timbres[timbre].params.engine1.numberOfVoice = numberOfVoice < voicesMax ? numberOfVoice : voicesMax;

    rebuidVoiceTimbre();
    updateNumberOfActiveTimbres();

    timbres[timbre].numberOfVoicesChanged();
    timbres[timbre].afterNewParamsLoad();
    // values to force check lfo used
    timbres[timbre].verifyLfoUsed(ENCODER_MATRIX_SOURCE, 0.0f, 1.0f);

}

void Synth::afterNewComboLoad() {


    // If combo have been saved with 16 voices
    // Reduce number of voices of timbre with more voices
    int timbreMax = -1;
    int voicesOfTimbreMax = -1;
    for (int t=0; t<NUMBER_OF_TIMBRES ; t++) {
        int numberOfVoice = timbres[t].params.engine1.numberOfVoice;
        if (numberOfVoice > voicesOfTimbreMax) {
            timbreMax = t;
            voicesOfTimbreMax = numberOfVoice;
        }
    }
    if (timbreMax >= 0) {
        int voicesMax = getNumberOfFreeVoicesForThisTimbre(timbreMax);
        timbres[timbreMax].params.engine1.numberOfVoice = voicesMax < voicesOfTimbreMax ? voicesMax : voicesOfTimbreMax;
    }

    rebuidVoiceTimbre();
    updateNumberOfActiveTimbres();

    for (int t=0; t<NUMBER_OF_TIMBRES ; t++) {
        timbres[t].numberOfVoicesChanged();
        timbres[t].afterNewParamsLoad();
        // values to force check lfo used
        timbres[t].verifyLfoUsed(ENCODER_MATRIX_SOURCE, 0.0f, 1.0f);
        //
    }
}

void Synth::updateNumberOfActiveTimbres() {
    int activeTimbres = 0;
    if (timbres[0].params.engine1.numberOfVoice > 0) {
        activeTimbres ++;
    }
    if (timbres[1].params.engine1.numberOfVoice > 0) {
        activeTimbres ++;
    }
    if (timbres[2].params.engine1.numberOfVoice > 0) {
        activeTimbres ++;
    }
    if (timbres[3].params.engine1.numberOfVoice > 0) {
        activeTimbres ++;
    }
    ratioTimbre = ratiosTimbre[activeTimbres];
}

int Synth::getFreeVoice() {
    // Loop on all voices
    for (int voice=0; voice< MAX_NUMBER_OF_VOICES; voice++) {
        bool used = false;

        for (int t=0; t< NUMBER_OF_TIMBRES && !used; t++) {
            // Must be different from 0 and -1
            int interVoice = -10;
            for (int v=0;  v < MAX_NUMBER_OF_VOICES  && !used; v++) {
                interVoice = timbres[t].voiceNumber[v];
                if (interVoice == -1) {
                    break;
                }
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
        if (unlikely(encoder == ENCODER_ENGINE_VOICE)) {
            // Increase number of voice ?
            if ((*newValue)> oldValue) {
                if ((*newValue) > getNumberOfFreeVoicesForThisTimbre(timbre)) {
                    *newValue = oldValue;
                }
            }
        }
    }
}

void Synth::newParamValue(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {
    switch (currentRow) {
    case ROW_ENGINE:
        switch (encoder) {
        case ENCODER_ENGINE_ALGO:
            fixMaxNumberOfVoices(timbre);
            break;
        case ENCODER_ENGINE_VOICE:
            if (newValue > oldValue) {
                for (int v=(int)oldValue; v < (int)newValue; v++) {
                    timbres[timbre].setVoiceNumber(v, getFreeVoice());
                }
            } else {
                for (int v=(int)newValue; v < (int)oldValue; v++) {
                    voices[timbres[timbre].voiceNumber[v]].killNow();
                    timbres[timbre].setVoiceNumber(v, -1);
                }
            }
            timbres[timbre].numberOfVoicesChanged();
            if (newValue == 0.0f || oldValue == 0.0f) {
                updateNumberOfActiveTimbres();
            }
            break;
        }
        break;
    case ROW_ARPEGGIATOR1:
        switch (encoder) {
        case ENCODER_ARPEGGIATOR_CLOCK:
            allNoteOff(timbre);
            timbres[timbre].setArpeggiatorClock((uint8_t) newValue);
            break;
        case ENCODER_ARPEGGIATOR_BPM:
            timbres[timbre].setNewBPMValue((uint8_t) newValue);
            break;
        case ENCODER_ARPEGGIATOR_DIRECTION:
            timbres[timbre].setDirection((uint8_t) newValue);
            break;
        }
        break;
    case ROW_ARPEGGIATOR2:
        if (unlikely(encoder == ENCODER_ARPEGGIATOR_LATCH)) {
            timbres[timbre].setLatchMode((uint8_t) newValue);
        }
        break;
    case ROW_ARPEGGIATOR3:
        break;
    case ROW_EFFECT:
        timbres[timbre].setNewEffecParam(encoder);
        break;
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
    case ROW_MATRIX_FIRST ... ROW_MATRIX_LAST:
        timbres[timbre].verifyLfoUsed(encoder, oldValue, newValue);
        if (encoder == ENCODER_MATRIX_DEST) {
            // Reset old destination
            timbres[timbre].resetMatrixDestination(oldValue);
        }
        break;
    case ROW_LFOOSC1 ... ROW_LFOOSC3:
    case ROW_LFOENV1 ... ROW_LFOENV2:
    case ROW_LFOSEQ1 ... ROW_LFOSEQ2:
        // timbres[timbre].lfo[currentRow - ROW_LFOOSC1]->valueChanged(encoder);
        timbres[timbre].lfoValueChange(currentRow, encoder, newValue);
        break;
    case ROW_PERFORMANCE1:
        timbres[timbre].setMatrixSource((enum SourceEnum)(MATRIX_SOURCE_CC1 + encoder), newValue);
        break;
    case ROW_MIDINOTE1CURVE:
        timbres[timbre].updateMidiNoteScale(0);
        break;
    case ROW_MIDINOTE2CURVE:
        timbres[timbre].updateMidiNoteScale(1);
        break;
    }
}


// synth is the only one who knows timbres
void Synth::newTimbre(int timbre)  {
    this->synthState->setParamsAndTimbre(&timbres[timbre].params, timbre);
}


/*
 * Return false if had to change number of voice
 *
 */
bool Synth::fixMaxNumberOfVoices(int timbre) {
    int voicesMax = getNumberOfFreeVoicesForThisTimbre(timbre) ;

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
            timbres[t].setVoiceNumber(v, voices++);
        }
        for (int v=nv; v < MAX_NUMBER_OF_VOICES;  v++) {
            timbres[t].setVoiceNumber(v, -1);
        }
    }
}

int Synth::getNumberOfFreeOscForThisTimbre(int timbre) {
    int numberOfOsc = 0;

    for (int t=0; t<NUMBER_OF_TIMBRES; t++) {
        if (t != timbre) {
            int nv = timbres[t].params.engine1.numberOfVoice + .0001f;
            numberOfOsc += algoInformation[(int)timbres[t].params.engine1.algo].osc * nv;
        }
    }

    return MAX_NUMBER_OF_OPERATORS - numberOfOsc;
}

void Synth::loadPreenFMPatchFromMidi(int timbre, int bank, int bankLSB, int patchNumber) {
    this->synthState->loadPreenFMPatchFromMidi(timbre, bank, bankLSB, patchNumber, &timbres[timbre].params);
}


void Synth::setNewValueFromMidi(int timbre, int row, int encoder, float newValue) {
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    int index = row * NUMBER_OF_ENCODERS + encoder;
    float oldValue = ((float*)this->timbres[timbre].getParamRaw())[index];

    // 2.08e : FIX CRASH when sending to many number of voices from editor !!!!
    if (unlikely(row == ROW_ENGINE)) {
        if (unlikely(encoder == ENCODER_ENGINE_VOICE)) {
            int maxNumberOfVoices = getNumberOfFreeVoicesForThisTimbre(timbre);
            if (newValue > maxNumberOfVoices) {
                newValue = maxNumberOfVoices;
            }
        }
    }


    this->timbres[timbre].setNewValue(index, param, newValue);
    float newNewValue = ((float*)this->timbres[timbre].getParamRaw())[index];
    if (oldValue != newNewValue) {
        this->synthState->propagateNewParamValueFromExternal(timbre, row, encoder, param, oldValue, newNewValue);
    }
}

void Synth::setNewStepValueFromMidi(int timbre, int whichStepSeq, int step, int newValue) {

    if (whichStepSeq < 0 || whichStepSeq > 1 || step < 0 || step > 15 || newValue < 0 || newValue > 15) {
        return;
    }

    int oldValue = this->timbres[timbre].getSeqStepValue(whichStepSeq, step);

    if (oldValue !=  newValue) {
        int oldStep = this->synthState->stepSelect[whichStepSeq];
        this->synthState->stepSelect[whichStepSeq] = step;
        if (oldStep != step) {
            this->synthState->propagateNewParamValueFromExternal(timbre, ROW_LFOSEQ1 + whichStepSeq, 2, NULL, oldStep, step);
        }
        this->timbres[timbre].setSeqStepValue(whichStepSeq, step, newValue);
        int newNewValue = this->timbres[timbre].getSeqStepValue(whichStepSeq, step);
        if (oldValue != newNewValue) {
            this->synthState->propagateNewParamValueFromExternal(timbre, ROW_LFOSEQ1 + whichStepSeq, 3, NULL, oldValue, newValue);
        }
    }
}



void Synth::setNewSymbolInPresetName(int timbre, int index, int value) {
    this->timbres[timbre].getParamRaw()->presetName[index] = value;
}



void Synth::setScalaEnable(bool enable) {
    this->synthState->setScalaEnable(enable);
}

void Synth::setScalaScale(int scaleNumber) {
    this->synthState->setScalaScale(scaleNumber);    
}

void Synth::setCurrentInstrument(int value) {
    if (value >=1 && value <= 4) {
        this->synthState->setCurrentInstrument(value);    
    }
}

void Synth::newMenuSelect(FullState* fullState) {
    if (fullState->currentMenuItem->menuState == MENU_CONFIG_SETTINGS) {
        if (fullState->menuSelect == MIDICONFIG_CVIN_A2 || fullState->menuSelect == MIDICONFIG_CVIN_A6) {
            cvin->updateFormula(synthState->fullState.midiConfigValue[MIDICONFIG_CVIN_A2], synthState->fullState.midiConfigValue[MIDICONFIG_CVIN_A6]);
        }
    }

}


#ifdef DEBUG

// ========================== DEBUG ========================
void Synth::debugVoice() {

    lcd.setRealTimeAction(true);
    lcd.clearActions();
    lcd.clear();
    int numberOfVoices = timbres[0].params.engine1.numberOfVoice;
    // HARDFAULT !!! :-)
    //    for (int k = 0; k <10000; k++) {
    //    	numberOfVoices += timbres[k].params.engine1.numberOfVoice;
    //    	timbres[k].params.engine1.numberOfVoice = 100;
    //    }

    for (int k = 0; k < numberOfVoices && k < 4; k++) {

        // voice number k of timbre
        int n = timbres[0].voiceNumber[k];

        lcd.setCursor(0, k);
        lcd.print((int)voices[n].getNote());

        lcd.setCursor(4, k);
        lcd.print((int)voices[n].getNextPendingNote());

        lcd.setCursor(8, k);
        lcd.print(n);

        lcd.setCursor(12, k);
        lcd.print((int)voices[n].getIndex());

        lcd.setCursor(18, k);
        lcd.print((int) voices[n].isReleased());
        lcd.print((int) voices[n].isPlaying());
    }
    lcd.setRealTimeAction(false);
}

void Synth::showCycles() {
    lcd.setRealTimeAction(true);
    lcd.clearActions();
    lcd.clear();

    float max = SystemCoreClock * 32.0f * PREENFM_FREQUENCY_INVERSED;
    int cycles = cycles_all.remove();
    float percent = (float)cycles * 100.0f / max;
    lcd.setCursor(10, 0);
    lcd.print('>');
    lcd.print(cycles);
    lcd.print('<');
    lcd.setCursor(10, 1);
    lcd.print('>');
    lcd.print(percent);
    lcd.print('%');
    lcd.print('<');

    /*
    lcd.setCursor( 0, 0 );
    lcd.print( "RNG: " );
    lcd.print( cycles_rng.remove() );

    lcd.setCursor( 0, 1 );
    lcd.print( "VOI: " );  lcd.print( cycles_voices1.remove() );
    lcd.print( " " ); lcd.print( cycles_voices2.remove() );

    lcd.setCursor( 0, 2 );
    lcd.print( "FX : " );
    lcd.print( cycles_fx.remove() );

    lcd.setCursor( 0, 3 );
    lcd.print( "TIM: " );
    lcd.print( cycles_timbres.remove() );

    lcd.setRealTimeAction(false);
     */
}

#endif

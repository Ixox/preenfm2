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

#ifndef SYNTH_H_
#define SYNTH_H_

#include "Timbre.h"
#include "Voice.h"
#include "LfoOsc.h"
#include "LfoEnv.h"
#include "LfoStepSeq.h"

#ifdef CVIN
#include "CVIn.h"
#include "VisualInfo.h"
#endif

#include "SynthParamListener.h"
#include "SynthStateAware.h"

#define UINT_MAX  4294967295

class Synth : public SynthParamListener, public SynthStateAware, public SynthParamChecker, public SynthMenuListener
{
public:
    Synth(void);
    virtual ~Synth(void);

    void setSynthState(SynthState* sState) {
        SynthStateAware::setSynthState(sState);
        init(sState);
    }

    void setDacNumberOfBits(uint32_t dacNumberOfBits);
#ifdef CVIN
    void setCVIn(CVIn * cvin) {
        this->cvin = cvin;
    }

    void setVisualInfo(VisualInfo *visualInfo) {
        this->visualInfo = visualInfo;
    }
#endif

    void noteOn(int timbre, char note, char velocity);
    void noteOff(int timbre, char note);
    void allNoteOff(int timbre);
    void allSoundOff();
    void allSoundOff(int timbre);
    bool isPlaying();
    void buildNewSampleBlock();
    void buildNewSampleBlockMcp4922();
    void buildNewSampleBlockCS4344(int32_t *sample);

    // Overide SynthParamListener
    void playNote(int timbreNumber, char note, char velocity) {
        noteOn(timbreNumber, note, velocity);
    }
    void stopNote(int timbreNumber, char note) {
        if (note != 0) {
            noteOff(timbreNumber, note);
        }
    }


    void newParamValueFromExternal(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {
        newParamValue(timbre, currentRow, encoder, param, oldValue, newValue);
    }

    void newParamValue(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue);
    void checkNewParamValue(int timbre, int currentRow, int encoder, float oldValue, float *newValue);

    int getFreeVoice();
    void rebuidVoiceTimbre();
    bool fixMaxNumberOfVoices(int timbre);

    int getNumberOfFreeVoicesForThisTimbre(int timbre);
    int getNumberOfFreeOscForThisTimbre(int timbre);

    void newcurrentRow(int timbre, int newcurrentRow)  {
        // Nothing to do
    }
    void newTimbre(int timbre);

    void beforeNewParamsLoad(int timbre);
    void afterNewParamsLoad(int timbre);
    void afterNewComboLoad();
    void updateNumberOfActiveTimbres();
    void showAlgo() { }
    void showIMInformation() {}


    void midiClockSetSongPosition(int songPosition) {
        // nothing to do
    }

    void midiClockContinue(int songPosition) {
        for (int t = 0; t < NUMBER_OF_TIMBRES; t++) {
            timbres[t].midiClockContinue(songPosition);
        }
    }


    void midiClockStart() {
        for (int t = 0; t < NUMBER_OF_TIMBRES; t++) {
            timbres[t].midiClockStart();
        }
    }

    void midiClockStop() {
        for (int t = 0; t < NUMBER_OF_TIMBRES; t++) {
            timbres[t].midiClockStop();
        }
    }

    void midiTick() {
        for (int t = 0; t < NUMBER_OF_TIMBRES; t++) {
            timbres[t].OnMidiClock();
        }
    }

    void midiClockSongPositionStep(int songPosition) {
        for (int t = 0; t < NUMBER_OF_TIMBRES; t++) {
            timbres[t].midiClockSongPositionStep(songPosition);
        }
    }

    uint32_t *getSample() {
        return this->samples;
    }

    inline int leftSampleAtReadCursor() const {
        return this->samples[this->readCursor];
    }

    inline int rightSampleAtReadCursor() const {
        return this->samples[this->readCursor + 1];
    }

    void incReadCursor() {
        this->readCursor = (this->readCursor + 2) & 255;
    }

    inline int getSampleCount() {
        if (this->readCursor > this->writeCursor) {
            return this->writeCursor - this->readCursor + 256;
        } else {
            return this->writeCursor - this->readCursor;
        }
    }

    Timbre* getTimbre(int timbre) {
        return &timbres[timbre];
    }

    void setNewValueFromMidi(int timbre, int row, int encoder, float newValue);
    void setNewStepValueFromMidi(int timbre, int whichStepSeq, int step, int newValue);
    void setNewSymbolInPresetName(int timbre, int index, int value);
    void loadPreenFMPatchFromMidi(int timbre, int bank, int bankLSB, int patchNumber);
    void setHoldPedal(int timbre, int value);

    void setScalaEnable(bool enable);
    void setScalaScale(int scaleNumber);

    void setCurrentInstrument(int value);

    // SynthMenuListener
    void newSynthMode(FullState* fullState) {};
    void menuBack(enum MenuState  oldMenuState, FullState* fullState) {};
    void newMenuState(FullState* fullState) {};
    void newMenuSelect(FullState* fullState);

    void updateGlobalTuningFromConfig();


#ifdef DEBUG
    void debugVoice();
    void showCycles();
#endif

    float getCpuUsage() {
        return cpuUsage;
    }

    int getPlayingNotes() {
        return playingNotes;
    }

private:
    // Called by setSynthState
    void init(SynthState* sState);

    float ratioTimbre;
    float ratioTimbreLP;

    Voice voices[MAX_NUMBER_OF_VOICES];
    Timbre timbres[NUMBER_OF_TIMBRES];

    // 4 buffer or 32 stero int = 64*4 = 256
    // sample Buffer
    volatile int readCursor;
    volatile int writeCursor;
    uint32_t samples[256];

    // gate
    float currentGate;

    float cpuUsage;
    uint32_t playingNotes;
#ifdef CVIN
    bool cvin12Ready ;
    bool cvin34Ready ;
    VisualInfo *visualInfo;
    CVIn* cvin;
    int triggeredTimbre;
#endif
};



#endif


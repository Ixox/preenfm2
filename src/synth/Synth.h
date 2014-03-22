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

#include "SynthParamListener.h"
#include "SynthStateAware.h"

#define UINT_MAX  4294967295

class Synth : public SynthParamListener, public SynthStateAware, public SynthParamChecker
{
public:
    Synth(void);
    virtual ~Synth(void);

	void setSynthState(SynthState* sState) {
		SynthStateAware::setSynthState(sState);
		init();
	}

    void noteOn(int timbre, char note, char velocity);
    void noteOff(int timbre, char note);
    void allNoteOff(int timbre);
    void allSoundOff();
    void allSoundOff(int timbre);
    bool isPlaying();
    void buildNewSampleBlock();


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
    void refreshNumberOfOsc();
    bool fixMaxNumberOfVoices(int timbre);

    void newcurrentRow(int timbre, int newcurrentRow)  {
        // Nothing to do
    }
    void newTimbre(int timbre);

    void beforeNewParamsLoad(int timbre);
    void afterNewParamsLoad(int timbre);
    void afterNewComboLoad();
    void updateNumberOfActiveTimbres();
    void showAlgo() { }

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

    inline float leftSampleAtReadCursor() {
        return this->samples[this->readCursor];
    }

    inline float rightSampleAtReadCursor() {
        return this->samples[this->readCursor + 1];
    }


    void incReadCursor() {
        this->readCursor += 2;
        if (this->readCursor == 256) {
            this->readCursor = 0;
        }
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
	void loadPreenFMPatchFromMidi(int timbre, int bank, int bankLSB, int patchNumber);
    void setHoldPedal(int timbre, int value);


#ifdef DEBUG
    void debugVoice();
#endif


private:
    // Called by setSynthState
    void init();

    float ratioTimbre;
    int numberOfOsc;
    Voice voices[MAX_NUMBER_OF_VOICES];
    Timbre timbres[NUMBER_OF_TIMBRES];

    // 4 buffer or 32 stero int = 64*4 = 256
    // sample Buffer
    volatile int readCursor;
    volatile int writeCursor;
    float samples[256];

    // gate
    float currentGate;

    // noise index
    uint32_t random32bit;
};



#endif


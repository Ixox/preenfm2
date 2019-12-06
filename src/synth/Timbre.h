/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <.> hosxe < a t > gmail.com)
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



#ifndef TIMBRE_H_
#define TIMBRE_H_

#include "Common.h"

#include "Osc.h"
#include "Env.h"
#include "Lfo.h"
#include "LfoOsc.h"
#include "LfoEnv.h"
#include "LfoEnv2.h"
#include "LfoStepSeq.h"
#include "Matrix.h"
#include "core_cmInstr.h"
#include "note_stack.h"
#include "event_scheduler.h"


extern float panTable[];
class Voice;

enum {  CLOCK_OFF,
    CLOCK_INTERNAL,
    CLOCK_EXTERNAL
};

class Timbre {
    friend class Synth;
    friend class Voice;
public:
    Timbre();
    virtual ~Timbre();
    void init(int timbreNumber, SynthState* sState);
    void setVoiceNumber(int v, int n);
    void initVoicePointer(int n, Voice* voice);
    void prepareForNextBlock();
    void cleanNextBlock();
    void prepareMatrixForNewBlock();
    void fxAfterBlock(float ratioTimbres);
    void afterNewParamsLoad();
    void setNewValue(int index, struct ParameterDisplay* param, float newValue);
    void setNewEffecParam(int encoder);
    void recomputeBPValues();
    int getSeqStepValue(int whichStepSeq, int step);
    void setSeqStepValue(int whichStepSeq, int step, int value);
    // Arpegiator
    void arpeggiatorNoteOn(char note, char velocity);
    void arpeggiatorNoteOff(char note);
    void StartArpeggio();
    void StepArpeggio();
    void Start();
    void arpeggiatorSetHoldPedal(uint8_t value);
    void setLatchMode(uint8_t value);
    void setDirection(uint8_t value);
    void setNewBPMValue(float bpm);
    void setArpeggiatorClock(float bpm);
    void resetArpeggiator();
    uint16_t getArpeggiatorPattern() const;

    void noteOn(char note, char velocity);
    void noteOff(char note);

    void preenNoteOn(char note, char velocity);
    inline void preenNoteOnUpdateMatrix(int voiceToUse, int note, int velocity);
    void preenNoteOff(char note);
    void numberOfVoicesChanged() {
        if (params.engine1.numberOfVoice > 0) {
            numberOfVoiceInverse = 1.0f / params.engine1.numberOfVoice;
        } else {
            numberOfVoiceInverse = 1.0f;
        }
    }

    void lfoValueChange(int currentRow, int encoder, float newValue);


    void setHoldPedal(int value);


    void resetMatrixDestination(float oldValue);
    void setMatrixSource(enum SourceEnum source, float newValue);
    void verifyLfoUsed(int encoder, float oldValue, float newValue);

    void midiClockStop() {
        OnMidiStop();
    }

    void midiClockContinue(int songPosition);
    void midiClockStart();
    void midiClockSongPositionStep(int songPosition);

    struct OneSynthParams * getParamRaw() {
        return &params;
    }

    float* getSampleBlock() {
        return sampleBlock;
    }

    const float* getSampleBlock() const {
        return sampleBlock;
    }



    // Needed for debuging
    char voiceNumber[MAX_NUMBER_OF_VOICES];

    // Midi note response
    // Midi Note Scale
    void updateMidiNoteScale(int scale);

    // Do matrix use LFO
    bool isLfoUsed(int lfo) {
        return  lfoUSed[lfo] > 0;
    }

private:

    // MiniPal Arpegiator
    void SendLater(uint8_t note, uint8_t velocity, uint8_t when, uint8_t tag);
    void SendScheduledNotes();
    void FlushQueue();
    void Tick();
    void OnMidiContinue();
    void OnMidiStart();
    void OnMidiStop();
    void OnMidiClock();
	void SendNote(uint8_t note, uint8_t velocity);

	int8_t timbreNumber;
    struct OneSynthParams params;
    float sampleBlock[BLOCK_SIZE * 2];
    float *sbMax;
    float numberOfVoiceInverse;
    float mixerGain;
    Voice *voices[MAX_NUMBER_OF_VOICES];
    bool holdPedal;
    int8_t lastPlayedNote;


    // 6 oscillators Max
    Osc osc1;
    Osc osc2;
    Osc osc3;
    Osc osc4;
    Osc osc5;
    Osc osc6;

    // And their 6 envelopes
    Env env1;
    Env env2;
    Env env3;
    Env env4;
    Env env5;
    Env env6;


    // Must recompute LFO steps ?
    bool recomputeNext;
    float currentGate;
    // Arpeggiator

    // TO REFACTOR
    float ticksPerSecond;
    static const float calledPerSecond = PREENFM_FREQUENCY / 32.0f;
    float ticksEveryNCalls;
    int ticksEveyNCallsInteger;



    float arpegiatorStep;
    NoteStack note_stack;
    EventScheduler event_scheduler;
    //
    //
    //    uint8_t clk_mode_;
    //    uint8_t groove_template_;
    //    uint8_t groove_amount_;
    //    uint8_t channel_;
    //    uint8_t pattern_;
    //


    uint8_t running_;
    uint8_t latch_;
    uint8_t tick_;
    uint8_t idle_ticks_;
    uint16_t bitmask_;
    int8_t current_direction_;
    int8_t current_octave_;
    int8_t current_step_;
	int8_t start_step_;
    uint8_t ignore_note_off_messages_;
    uint8_t recording_;
    // Low pass filter
    float fxParam1, fxParam2, fxParam3;
    float fxParamA1, fxParamA2;
    float fxParamB1, fxParamB2;
    float v0L, v1L;
    float v0R, v1R;
    float fxPhase;
    // save float fxParam1 to detect modification
    float fxParam1PlusMatrix;

    // lfoUsed
    uint8_t lfoUSed[NUMBER_OF_LFO];

};

#endif /* TIMBRE_H_ */

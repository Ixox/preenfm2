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

#ifndef VOICE_H_
#define VOICE_H_

#include "Common.h"
#include "Env.h"
#include "Osc.h"


class Timbre;


class Voice
{
public:
    Voice();
    ~Voice(void);

    void init(Timbre* timbre0, Timbre* timbre1, Timbre* timbre2, Timbre* timbre3);

    void nextBlock();

    void endNoteOrBeginNextOne();

    void noteOnWithoutPop(short note, short velocity, unsigned int index);
    void noteOn(short timbre, short note, short velocity, unsigned int index);
    void glideToNote(short newNote);
    void killNow();
    void noteOff();
    void glideFirstNoteOff();
    void glide();

    bool isReleased() { return this->released; }
    bool isPlaying() { return this->playing; }
    bool isNewNotePending() { return this->newNotePending; }
    unsigned int getIndex() { return this->index; }
    char getNote() { return this->note; }
    char getNextPendingNote() { return this->nextPendingNote; }
    char getNextGlidingNote() { return this->nextGlidingNote; }
    int getCurrentTimbre() { return this->voiceTimbre; }
    bool isHoldedByPedal() { return this->holdedByPedal; }
    bool setHoldedByPedal(bool holded) { this->holdedByPedal = holded; }

private:
    // voice status
    int frequency;
    bool released;
    bool playing;
    unsigned int index;
    char note;
    float velocity;
    float im1, im2, im3, im4, im5;
    //
    float freqAi, freqAo;
    float freqBi, freqBo;

    EnvData envState1;
    EnvData envState2;
    EnvData envState3;
    EnvData envState4;
    EnvData envState5;
    EnvData envState6;

    OscState oscState1;
    OscState oscState2;
    OscState oscState3;
    OscState oscState4;
    OscState oscState5;
    OscState oscState6;

    bool holdedByPedal;
    // Fixing the "plop" when all notes are buisy...
    bool newNotePending;
    char nextPendingNote;
    char nextVelocity;
    unsigned int nextIndex;

    // Gliding ?
    bool gliding;
    float glidePhase;
    char nextGlidingNote;

    // env Value
    float env1ValueMem;
    float env2ValueMem;
    float env3ValueMem;
    float env4ValueMem;
    float env5ValueMem;
    float env6ValueMem;

    int voiceTimbre;
    Timbre* timbres[4];
    Timbre* currentTimbre;

    // glide phase increment
    static float glidePhaseInc[10];

    // FM feedback...
    float feedback;
};

#endif



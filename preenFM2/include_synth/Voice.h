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

#ifndef VOICE_H_
#define VOICE_H_

#include "SynthState.h"
#include "Timbre.h"
#include "Common.h"

class Voice
{
public:
    Voice();
    ~Voice(void);

    void init(Timbre* timbre0, Timbre* timbre1, Timbre* timbre2, Timbre* timbre3);

    void nextBlock();

    void endNoteOrBeginNextOne() {
        if (newNotePending) {
            noteOn(voiceTimbre, nextNote, nextVelocity, index);
            this->newNotePending = false;
        } else {
            playing = false;
        }
        this->env1ValueMem = 0;
        this->env2ValueMem = 0;
        this->env3ValueMem = 0;
        this->env4ValueMem = 0;
        this->env5ValueMem = 0;
        this->env6ValueMem = 0;
    }

    void noteOnWithoutPop(short note, short velocity, unsigned int index);
    void noteOn(short timbre, short note, short velocity, unsigned int index);
    void glideToNote(short newNote);
    void killNow();
    void noteOff();
    void glideNoteOff();
    void glide();

    bool isReleased() { return this->released; }
    bool isPlaying() { return this->playing; }
    bool isGliding() { return this->gliding; }
    unsigned int getIndex() { return this->index; }
    char getNote() { return this->note; }
    char getNextNote() { return this->nextNote; }
    int getCurrentTimbre() { return this->voiceTimbre; }

private:
    // voice status
    int frequency;
    bool released;
    bool playing;
    unsigned int index;
    char note;
    float velocity;

    EnvData envState1;
    EnvData envState2;
    EnvData envState3;
    EnvData envState4;
    EnvData envState5;
    EnvData envState6;
    EnvData *envState[6];

    OscState oscState1;
    OscState oscState2;
    OscState oscState3;
    OscState oscState4;
    OscState oscState5;
    OscState oscState6;
    OscState *oscState[6];


    // Fixing the "plop" when all notes are buisy...
    bool newNotePending;
    char nextNote;
    char nextVelocity;
    unsigned int nextIndex;

    // Gliding ?
    bool gliding;
    float glidePhase;

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
};

#endif



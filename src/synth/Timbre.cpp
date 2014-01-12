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


#include "Timbre.h"
#include "Voice.h"

enum ArpeggiatorDirection {
  ARPEGGIO_DIRECTION_UP = 0,
  ARPEGGIO_DIRECTION_DOWN,
  ARPEGGIO_DIRECTION_UP_DOWN,
  ARPEGGIO_DIRECTION_RANDOM,
};
uint16_t lut_res_arpeggiator_patterns[]  = {
   21845,  62965,  46517,  54741,  43861,  22869,  38293,   2313,
   37449,  21065,  18761,  54553,  27499,  23387,  30583,  28087,
   22359,  28527,  30431,  43281,  28609,  53505,
};

const uint8_t midi_clock_tick_per_step[17]  = {
  192, 144, 96, 72, 64, 48, 36, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1
};

extern float noise[32];


float panTable[] = {
		0.0000, 0.0007, 0.0020, 0.0036, 0.0055, 0.0077, 0.0101, 0.0128, 0.0156, 0.0186,
		0.0218, 0.0252, 0.0287, 0.0324, 0.0362, 0.0401, 0.0442, 0.0484, 0.0527, 0.0572,
		0.0618, 0.0665, 0.0713, 0.0762, 0.0812, 0.0863, 0.0915, 0.0969, 0.1023, 0.1078,
		0.1135, 0.1192, 0.1250, 0.1309, 0.1369, 0.1430, 0.1492, 0.1554, 0.1618, 0.1682,
		0.1747, 0.1813, 0.1880, 0.1947, 0.2015, 0.2085, 0.2154, 0.2225, 0.2296, 0.2369,
		0.2441, 0.2515, 0.2589, 0.2664, 0.2740, 0.2817, 0.2894, 0.2972, 0.3050, 0.3129,
		0.3209, 0.3290, 0.3371, 0.3453, 0.3536, 0.3619, 0.3703, 0.3787, 0.3872, 0.3958,
		0.4044, 0.4131, 0.4219, 0.4307, 0.4396, 0.4485, 0.4575, 0.4666, 0.4757, 0.4849,
		0.4941, 0.5034, 0.5128, 0.5222, 0.5316, 0.5411, 0.5507, 0.5604, 0.5700, 0.5798,
		0.5896, 0.5994, 0.6093, 0.6193, 0.6293, 0.6394, 0.6495, 0.6597, 0.6699, 0.6802,
		0.6905, 0.7009, 0.7114, 0.7218, 0.7324, 0.7430, 0.7536, 0.7643, 0.7750, 0.7858,
		0.7967, 0.8076, 0.8185, 0.8295, 0.8405, 0.8516, 0.8627, 0.8739, 0.8851, 0.8964,
		0.9077, 0.9191, 0.9305, 0.9420, 0.9535, 0.9651, 0.9767, 0.9883, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000,
		1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000
} ;

// Static to all 4 timbres
unsigned int voiceIndex  __attribute__ ((section(".ccmnoload")));


Timbre::Timbre() {
    osc[0] = &osc1;
    osc[1] = &osc2;
    osc[2] = &osc3;
    osc[3] = &osc4;
    osc[4] = &osc5;
    osc[5] = &osc6;

    env[0] = &env1;
    env[1] = &env2;
    env[2] = &env3;
    env[3] = &env4;
    env[4] = &env5;
    env[5] = &env6;

    lfo[0] = &lfoOsc[0];
    lfo[1] = &lfoOsc[1];
    lfo[2] = &lfoOsc[2];
    lfo[3] = &lfoEnv[0];
    lfo[4] = &lfoEnv2[0];
    lfo[5] = &lfoStepSeq[0];
    lfo[6] = &lfoStepSeq[1];


    this->recomputeNext = true;
    this->currentGate = 0;
    this->sbMax = &this->sampleBlock[64];
    this->holdPedal = false;

    // arpegiator
    setNewBPMValue(90);
    arpegiatorStep = 0.0;
    idle_ticks_ = 96;
    running_ = 0;
    ignore_note_off_messages_ = 0;
    recording_ = 0;
    note_stack.Init();
    event_scheduler.Init();
    // Arpeggiator start
    Start();
}

Timbre::~Timbre() {
}

void Timbre::init(int timbreNumber) {
    struct EnvelopeParamsA* envParamsA[] = { &params.env1a, &params.env2a, &params.env3a, &params.env4a, &params.env5a, &params.env6a};
    struct EnvelopeParamsB* envParamsB[] = { &params.env1b, &params.env2b, &params.env3b, &params.env4b, &params.env5b, &params.env6b};
    struct OscillatorParams* oscParams[] = { &params.osc1, &params.osc2, &params.osc3, &params.osc4, &params.osc5, &params.osc6};
    struct LfoParams* lfoParams[] = { &params.lfoOsc1, &params.lfoOsc2, &params.lfoOsc3};
    struct StepSequencerParams* stepseqparams[] = { &params.lfoSeq1, &params.lfoSeq2};
    struct StepSequencerSteps* stepseqs[] = { &params.lfoSteps1, &params.lfoSteps2};

    matrix.init(&params.matrixRowState1);

    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        env[k]->init(&matrix, envParamsA[k],  envParamsB[k], (DestinationEnum)(ENV1_ATTACK + k));
        osc[k]->init(&matrix, oscParams[k], (DestinationEnum)(OSC1_FREQ + k));
    }
    // OSC
    for (int k = 0; k < NUMBER_OF_LFO_OSC; k++) {
        lfoOsc[k].init(lfoParams[k], &this->matrix, (SourceEnum)(MATRIX_SOURCE_LFO1 + k), (DestinationEnum)(LFO1_FREQ + k));
    }

    // ENV
    lfoEnv[0].init(&params.lfoEnv1 , &this->matrix, MATRIX_SOURCE_LFOENV1, (DestinationEnum)0);
    lfoEnv2[0].init(&params.lfoEnv2 , &this->matrix, MATRIX_SOURCE_LFOENV2, (DestinationEnum)LFOENV2_SILENCE);

    // Step sequencer
    for (int k = 0; k< NUMBER_OF_LFO_STEP; k++) {
        lfoStepSeq[k].init(stepseqparams[k], stepseqs[k], &matrix, (SourceEnum)(MATRIX_SOURCE_LFOSEQ1+k), (DestinationEnum)(LFOSEQ1_GATE+k));
    }
    this->timbreNumber = timbreNumber;
}

void Timbre::initVoicePointer(int n, Voice* voice) {
	voices[n] = voice;
}

void Timbre::noteOn(char note, char velocity) {
	if (params.engineApr1.clock) {
		arpeggiatorNoteOn(note, velocity);
	} else {
		preenNoteOn(note, velocity);
	}
}

void Timbre::noteOff(char note) {
	if (params.engineApr1.clock) {
		arpeggiatorNoteOff(note);
	} else {
		preenNoteOff(note);
	}
}

void Timbre::preenNoteOn(char note, char velocity) {
	if (params.engine1.numberOfVoice == 0) {
		return;
	}

	int zeroVelo = (16 - params.engine1.velocity) * 8;
	int newVelocity = zeroVelo + ((velocity * (128 - zeroVelo)) >> 7);

	unsigned int indexMin = (unsigned int)2147483647;
	int voiceToUse = -1;


	for (int k = 0; k < params.engine1.numberOfVoice; k++) {
		// voice number k of timbre
		int n = voiceNumber[k];

		// same note.... ?
		if (voices[n]->getNote() == note) {
			voices[n]->noteOnWithoutPop(note, newVelocity, voiceIndex++);
			return;
		}

		// unlikely because if it true, CPU is not full
		if (unlikely(!voices[n]->isPlaying())) {
			voices[n]->noteOn(timbreNumber, note, newVelocity, voiceIndex++);
			return;
		}

		if (voices[n]->isReleased()) {
			int indexVoice = voices[n]->getIndex();
			if (indexVoice < indexMin) {
				indexMin = indexVoice;
				voiceToUse = n;
				// NO break... We must take the elder one
			}
		}
	}

	if (voiceToUse == -1) {
		for (int k = 0; k < params.engine1.numberOfVoice; k++) {
			// voice number k of timbre
			int n = voiceNumber[k];
			int indexVoice = voices[n]->getIndex();
			if (indexVoice < indexMin && !voices[n]->isNewNotePending()) {
				indexMin = indexVoice;
				voiceToUse = n;
				// NO break... We must take the elder one
			}
		}
	}
	// All voices in newnotepending state ?
	if (voiceToUse != -1) {
		voices[voiceToUse]->noteOnWithoutPop(note, newVelocity, voiceIndex++);
	}
}


void Timbre::preenNoteOff(char note) {
  for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        // voice number k of timbre
        int n = voiceNumber[k];

        if (likely(voices[n]->getNextGlidingNote() == 0)) {
            if (voices[n]->getNote() == note) {
            	if (unlikely(holdPedal)) {
            		voices[n]->setHoldedByPedal(true);
            		return;
            	} else {
                	voices[n]->noteOff();
                    return;
            	}

            }
        } else {
            // if gliding and releasing first note
        	if (voices[n]->getNote() == note) {
				voices[n]->glideFirstNoteOff();
                return;
            }
            // if gliding and releasing next note
            if (voices[n]->getNextGlidingNote() == note) {
				voices[n]->glideToNote(voices[n]->getNote());
				voices[n]->glideFirstNoteOff();
                return;
            }
        }
    }

}


void Timbre::setHoldPedal(int value) {
	if (value <64) {
		holdPedal = false;
	    int numberOfVoices = params.engine1.numberOfVoice;
	    for (int k = 0; k < numberOfVoices; k++) {
	        // voice number k of timbre
	        int n = voiceNumber[k];
	        if (voices[n]->isHoldedByPedal()) {
	        	voices[n]->noteOff();
	        }
	    }
	    arpeggiatorSetHoldPedal(0);
	} else {
		holdPedal = true;
	    arpeggiatorSetHoldPedal(127);
	}
}




void Timbre::setNewBPMValue(float bpm) {
	ticksPerSecond = bpm * 24.0f / 60.0f;
	ticksEveryNCalls = calledPerSecond / ticksPerSecond;
	ticksEveyNCallsInteger = (int)ticksEveryNCalls;
}

void Timbre::setArpeggiatorClock(float clockValue) {
	if (clockValue == CLOCK_OFF) {
		FlushQueue();
		note_stack.Clear();
	}
	if (clockValue == CLOCK_INTERNAL) {
	    setNewBPMValue(params.engineApr1.BPM);
	}
	lcd.setCursor(12,1);
	lcd.print((int)clockValue);
}




void Timbre::prepareForNextBlock() {
	// Apeggiator clock : internal
	if (params.engineApr1.clock == CLOCK_INTERNAL) {
		arpegiatorStep+=1.0f;
		if ((arpegiatorStep) > ticksEveryNCalls) {
			arpegiatorStep -= ticksEveyNCallsInteger;
			// control by internal clock
			Tick();
		}
	}

    this->lfo[0]->nextValueInMatrix();
    this->lfo[1]->nextValueInMatrix();
    this->lfo[2]->nextValueInMatrix();
    this->lfo[3]->nextValueInMatrix();
    this->lfo[4]->nextValueInMatrix();
    this->lfo[5]->nextValueInMatrix();
    this->lfo[6]->nextValueInMatrix();

    this->matrix.computeAllFutureDestintationAndSwitch();

    updateAllModulationIndexes();
    updateAllMixOscsAndPans();

	float *sp = this->sampleBlock;
	while (sp < this->sbMax) {
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
		*sp++ = 0;
	}

}


#define GATE_INC 0.02f

void Timbre::fxAfterBlock() {
    // Gate algo !!
    float gate = this->matrix.getDestination(MAIN_GATE);
    if (likely(gate <= 0 && currentGate <= 0)) {
        return;
    }
    gate *=.72547132656922730694f; // 0 < gate < 1.0
    if (gate > 1.0f) {
        gate = 1.0f;
    }
    float incGate = (gate - currentGate) * .03125f; // ( *.03125f = / 32)
    // limit the speed.
    if (incGate > 0.002f) {
        incGate = 0.002f;
    } else if (incGate < -0.002f) {
        incGate = -0.002f;
    }

    float *sp = this->sampleBlock;
    float coef;
    while (sp < this->sbMax) {
        currentGate += incGate;
        coef = 1.0f - currentGate;
        *sp = *sp * coef;
        sp++;
        *sp = *sp * coef;
        sp++;
    }
    //    currentGate = gate;
}


void Timbre::afterNewParamsLoad() {
    this->matrix.resetSources();
    this->matrix.resetAllDestination();
    for (int k=0; k<NUMBER_OF_OPERATORS; k++) {
        for (int j=0; j<NUMBER_OF_ENCODERS; j++) {
        	this->env[k]->reloadADSR(j);
        	this->env[k]->reloadADSR(j+4);
        }
    }
    for (int k=0; k<NUMBER_OF_LFO; k++) {
        for (int j=0; j<NUMBER_OF_ENCODERS; j++) {
        	this->lfo[k]->valueChanged(j);
        }
    }
    setArpeggiatorClock(params.engineApr1.clock);
    setLatchMode(params.engineApr2.latche);
}

void Timbre::setNewValue(int index, struct ParameterDisplay* param, float newValue) {
    if (newValue > param->maxValue) {
        newValue= param->maxValue;
    } else if (newValue < param->minValue) {
        newValue= param->minValue;
    }
    ((float*)&params)[index] = newValue;
}


// Code bellowed have been adapted by Xavier Hosxe for PreenFM2
// It come from Muteable Instrument midiPAL

/////////////////////////////////////////////////////////////////////////////////
// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Arpeggiator app.



void Timbre::arpeggiatorNoteOn(char note, char velocity) {
	// CLOCK_MODE_INTERNAL
	if (params.engineApr1.clock == CLOCK_INTERNAL) {
		if (idle_ticks_ >= 96) {
			Start();
		}
		idle_ticks_ = 0;
	}

	if (latch_ && !recording_) {
		note_stack.Clear();
		recording_ = 1;
	}
	note_stack.NoteOn(note, velocity);
}


void Timbre::arpeggiatorNoteOff(char note) {
	if (ignore_note_off_messages_) {
		return;
	}
	if (!latch_) {
		note_stack.NoteOff(note);
	} else {
		if (note == note_stack.most_recent_note().note) {
			recording_ = 0;
		}
	}
}


void Timbre::OnMidiContinue() {
	if (params.engineApr1.clock == CLOCK_EXTERNAL) {
		running_ = 1;
	}
}

void Timbre::OnMidiStart() {
	if (params.engineApr1.clock == CLOCK_EXTERNAL) {
		Start();
	}
}

void Timbre::OnMidiStop() {
	if (params.engineApr1.clock == CLOCK_EXTERNAL) {
		running_ = 0;
		SendScheduledNotes();
	}
}


void Timbre::OnMidiClock() {
	if (params.engineApr1.clock == CLOCK_EXTERNAL && running_) {
		Tick();
	}
}




void Timbre::SendLater(uint8_t note, uint8_t velocity, uint8_t when, uint8_t tag) {
	event_scheduler.Schedule(note, velocity, when, tag);
}


void Timbre::SendScheduledNotes() {
  uint8_t current = event_scheduler.root();
  while (current) {
    const SchedulerEntry& entry = event_scheduler.entry(current);
    if (entry.when) {
      break;
    }
    if (entry.note != kZombieSlot) {
      if (entry.velocity == 0) {
    	  preenNoteOff(entry.note);
      } else {
    	  preenNoteOn(entry.note, entry.velocity);
      }
    }
    current = entry.next;
  }
  event_scheduler.Tick();
}


void Timbre::FlushQueue() {
  while (event_scheduler.size()) {
    SendScheduledNotes();
  }
}



void Timbre::Tick() {
	++tick_;

	if (note_stack.size()) {
		idle_ticks_ = 0;
	}
	++idle_ticks_;
	if (idle_ticks_ >= 96) {
		idle_ticks_ = 96;
	    if (params.engineApr1.clock == CLOCK_INTERNAL) {
	      running_ = 0;
	      FlushQueue();
	    }
	}

	SendScheduledNotes();

	if (tick_ >= midi_clock_tick_per_step[(int)params.engineApr2.division]) {
		tick_ = 0;
		uint16_t pattern = lut_res_arpeggiator_patterns[(int)params.engineApr2.pattern - 1];
		uint8_t has_arpeggiator_note = (bitmask_ & pattern) ? 255 : 0;
		if (note_stack.size() && has_arpeggiator_note) {
			uint8_t note = note_stack.sorted_note(current_step_).note;
			uint8_t velocity = note_stack.sorted_note(current_step_).velocity;
			note += 12 * current_octave_;
			while (note > 127) {
				note -= 12;
			}
			// If there are some Note Off messages for the note about to be triggeered
			// remove them from the queue and process them now.
			if (event_scheduler.Remove(note, 0)) {
				preenNoteOff(note);
			}
			// Send a note on and schedule a note off later.
			preenNoteOn(note, velocity);
			event_scheduler.Schedule(note, 0, midi_clock_tick_per_step[(int)params.engineApr2.duration] - 1, 0);

			StepArpeggio();
		}
		bitmask_ <<= 1;
		if (!bitmask_) {
			bitmask_ = 1;
		}
	}
}



void Timbre::StepArpeggio() {
	uint8_t num_notes = note_stack.size();
	if (params.engineApr1.direction == ARPEGGIO_DIRECTION_RANDOM) {
		uint8_t random_byte = *(uint8_t*)noise;
		current_octave_ = random_byte & 0xf;
		current_step_ = (random_byte & 0xf0) >> 4;
		while (current_octave_ >= params.engineApr1.octave) {
			current_octave_ -= params.engineApr1.octave;
		}
		while (current_step_ >= num_notes) {
			current_step_ -= num_notes;
		}
	} else {
		current_step_ += current_direction_;
		uint8_t change_octave = 0;
		if (current_step_ >= num_notes) {
			current_step_ = 0;
			change_octave = 1;
		} else if (current_step_ < 0) {
			current_step_ = num_notes - 1;
			change_octave = 1;
		}
		if (change_octave) {
			current_octave_ += current_direction_;
			if (current_octave_ >= params.engineApr1.octave || current_octave_ < 0) {
				if (params.engineApr1.direction == ARPEGGIO_DIRECTION_UP_DOWN) {
					current_direction_ = -current_direction_;
					StartArpeggio();
					if (num_notes > 1 || params.engineApr1.octave > 1) {
						StepArpeggio();
					}
				} else {
					StartArpeggio();
				}
			}
		}
	}
}

void Timbre::StartArpeggio() {
  if (current_direction_ == 1) {
    current_octave_ = 0;
    current_step_ = 0;
  } else {
    current_step_ = note_stack.size() - 1;
    current_octave_ = params.engineApr1.octave - 1;
  }
}

void Timbre::Start() {
	bitmask_ = 1;
	recording_ = 0;
	running_ = 1;
	tick_ = midi_clock_tick_per_step[(int)params.engineApr2.division] - 1;
	current_direction_ = (params.engineApr1.direction == ARPEGGIO_DIRECTION_DOWN ? -1 : 1);
	StartArpeggio();
}


void Timbre::arpeggiatorSetHoldPedal(uint8_t value) {
  if (ignore_note_off_messages_ && !value) {
    // Pedal was released, kill all pending arpeggios.
    note_stack.Clear();
  }
  ignore_note_off_messages_ = value;
}


void Timbre::setLatchMode(uint8_t value) {
    // When disabling latch mode, clear the note stack.
	latch_ = value;
    if (value == 0) {
      note_stack.Clear();
      recording_ = 0;
    }
}


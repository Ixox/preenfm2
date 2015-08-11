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

// 2014 : Modified by Xavier Hosxe for PreenFM2


#include "midipal/arpeggiator.h"
#include "midipal/event_scheduler.h"
#include "midipal/note_stack.h"


enum ArpeggiatorDirection {
  ARPEGGIO_DIRECTION_UP = 0,
  ARPEGGIO_DIRECTION_DOWN,
  ARPEGGIO_DIRECTION_UP_DOWN,
  ARPEGGIO_DIRECTION_RANDOM,
};

uint8_t arpeggiator_factory_data[11] = {
  0, 120, 0, 0, 0, 0, 1, 0, 12, 14, 0
};

uint8_t midi_clock_tick_per_step[17] = {
  192, 144, 96, 72, 64, 48, 36, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1
};


/* <static> */
uint8_t Arpeggiator::running_;

uint8_t Arpeggiator::clk_mode_;
uint8_t Arpeggiator::bpm_;
uint8_t Arpeggiator::groove_template_;
uint8_t Arpeggiator::groove_amount_;
uint8_t Arpeggiator::channel_;
uint8_t Arpeggiator::direction_;
uint8_t Arpeggiator::num_octaves_;
uint8_t Arpeggiator::pattern_;
uint8_t Arpeggiator::clock_division_;
uint8_t Arpeggiator::duration_;
uint8_t Arpeggiator::latch_;

uint8_t Arpeggiator::midi_clock_prescaler_;

uint8_t Arpeggiator::tick_;
uint8_t Arpeggiator::idle_ticks_;
uint16_t Arpeggiator::bitmask_;
int8_t Arpeggiator::current_direction_;
int8_t Arpeggiator::current_octave_;
int8_t Arpeggiator::current_step_;
uint8_t Arpeggiator::ignore_note_off_messages_;
uint8_t Arpeggiator::recording_;
/* </static> */


/* static */
void Arpeggiator::OnInit() {

  clock.Update(bpm_, groove_template_, groove_amount_);
  SetParameter(8, clock_division_);  // Force an update of the prescaler.
  clock.Start();
  idle_ticks_ = 96;
  running_ = 0;
  ignore_note_off_messages_ = 0;
  recording_ = 0;
  // XH
  clk_mode_ = CLOCK_MODE_EXTERNAL;
}


/* static */
void Arpeggiator::OnContinue() {
  if (clk_mode_ == CLOCK_MODE_EXTERNAL) {
    running_ = 1;
  }
}

/* static */
void Arpeggiator::OnStart() {
  if (clk_mode_ == CLOCK_MODE_EXTERNAL) {
    Start();
  }
}

/* static */
void Arpeggiator::OnStop() {
  if (clk_mode_ == CLOCK_MODE_EXTERNAL) {
    running_ = 0;
    //app.FlushQueue(channel_);
    // XH :
    // while (event_scheduler.size()) {
      // SendScheduledNotes(channel);
    // }
  }
}

/* static */
void Arpeggiator::OnClock() {
  if (clk_mode_ == CLOCK_MODE_EXTERNAL && running_) {
    Tick();
  }
}

/* static */
void Arpeggiator::OnInternalClockTick() {
  if (clk_mode_ == CLOCK_MODE_INTERNAL && running_) {
    app.SendNow(0xf8);
    Tick();
  }
}

/* static */
void Arpeggiator::OnNoteOn(
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  if (channel != channel_) {
    return;
  }
  note_stack.NoteOn(note, velocity);
}

/* static */
void Arpeggiator::OnNoteOff(
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  if (channel != channel_ || ignore_note_off_messages_) {
    return;
  }
    note_stack.NoteOff(note);
}

/* static */
void Arpeggiator::Tick() {
  ++tick_;

  if (note_stack.size()) {
    idle_ticks_ = 0;
  }
  ++idle_ticks_;
  if (idle_ticks_ >= 96) {
    idle_ticks_ = 96;
  }

  app.SendScheduledNotes(channel_);

  if (tick_ >= midi_clock_prescaler_) {
    tick_ = 0;
    uint16_t pattern = ResourcesManager::Lookup<uint16_t, uint8_t>(
        lut_res_arpeggiator_patterns,
        pattern_);
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
        app.Send3(0x80 | channel_, note, 0);
      }
      // Send a note on and schedule a note off later.
      app.Send3(0x90 | channel_, note, velocity);
      app.SendLater(note, 0, ResourcesManager::Lookup<uint8_t, uint8_t>(
          midi_clock_tick_per_step, duration_) - 1);
      StepArpeggio();
    }
    bitmask_ <<= 1;
    if (!bitmask_) {
      bitmask_ = 1;
    }
  }
}

/* static */
void Arpeggiator::Start() {
  running_ = 1;
  bitmask_ = 1;
  tick_ = midi_clock_prescaler_ - 1;
  current_direction_ = (direction_ == ARPEGGIO_DIRECTION_DOWN ? -1 : 1);
  recording_ = 0;
  StartArpeggio();
}

/* static */
void Arpeggiator::StartArpeggio() {
  if (current_direction_ == 1) {
    current_octave_ = 0;
    current_step_ = 0;
  } else {
    current_step_ = note_stack.size() - 1;
    current_octave_ = num_octaves_ - 1;
  }
}

/* static */
void Arpeggiator::OnControlChange(uint8_t channel, uint8_t cc, uint8_t value) {
  if (channel != channel_ || cc != kHoldPedal) {
    return;
  }
  if (ignore_note_off_messages_ && !value) {
    // Pedal was released, kill all pending arpeggios.
    note_stack.Clear();
  }
  ignore_note_off_messages_ = value;
}

/* static */
void Arpeggiator::StepArpeggio() {
  uint8_t num_notes = note_stack.size();
  if (direction_ == ARPEGGIO_DIRECTION_RANDOM) {
    uint8_t random_byte = Random::GetByte();
    current_octave_ = random_byte & 0xf;
    current_step_ = (random_byte & 0xf0) >> 4;
    while (current_octave_ >= num_octaves_) {
      current_octave_ -= num_octaves_;
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
      if (current_octave_ >= num_octaves_ || current_octave_ < 0) {
        if (direction_ == ARPEGGIO_DIRECTION_UP_DOWN) {
          current_direction_ = -current_direction_;
          StartArpeggio();
          if (num_notes > 1 || num_octaves_ > 1) {
            StepArpeggio();
          }
        } else {
          StartArpeggio();
        }
      }
    }
  }
}

/* static */
void Arpeggiator::SetParameter(uint8_t key, uint8_t value) {
  static_cast<uint8_t*>(&clk_mode_)[key] = value;
  if (key < 4) {
    clock.Update(bpm_, groove_template_, groove_amount_);
  }
  midi_clock_prescaler_ = ResourcesManager::Lookup<uint8_t, uint8_t>(
      midi_clock_tick_per_step, clock_division_);
  if (key == 5) {
    // When changing the arpeggio direction, reset the pattern.
    current_direction_ = (direction_ == ARPEGGIO_DIRECTION_DOWN ? -1 : 1);
    StartArpeggio();
  }
  if (key == 10) {
    // When disabling latch mode, clear the note stack.
    if (value == 0) {
      note_stack.Clear();
      recording_ = 0;
    }
  }
}

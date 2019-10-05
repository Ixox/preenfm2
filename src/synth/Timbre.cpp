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


#include <math.h>
#include "Timbre.h"
#include "Voice.h"

#define INV127 .00787401574803149606f
#define INV16 .0625f
// Regular memory
float midiNoteScale[2][NUMBER_OF_TIMBRES][128];

/*
#include "LiquidCrystal.h"
extern LiquidCrystal lcd;
void myVoiceError(char info, int t, int t2) {
    lcd.setRealTimeAction(true);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print('!');
    lcd.print(info);
    lcd.print(t);
    lcd.print(' ');
    lcd.print(t2);
    while (true) {};
}

...
        if (voiceNumber[k] < 0) myVoiceError('A', voiceNumber[k], k);

*/

//#define DEBUG_ARP_STEP
enum ArpeggiatorDirection {
    ARPEGGIO_DIRECTION_UP = 0,
    ARPEGGIO_DIRECTION_DOWN,
    ARPEGGIO_DIRECTION_UP_DOWN,
    ARPEGGIO_DIRECTION_PLAYED,
    ARPEGGIO_DIRECTION_RANDOM,
    ARPEGGIO_DIRECTION_CHORD,
   /*
    * ROTATE modes rotate the first note played, e.g. UP: C-E-G -> E-G-C -> G-C-E -> repeat
    */
    ARPEGGIO_DIRECTION_ROTATE_UP, ARPEGGIO_DIRECTION_ROTATE_DOWN, ARPEGGIO_DIRECTION_ROTATE_UP_DOWN,
   /*
    * SHIFT modes rotate and extend with transpose, e.g. UP: C-E-G -> E-G-C1 -> G-C1-E1 -> repeat
    */
    ARPEGGIO_DIRECTION_SHIFT_UP, ARPEGGIO_DIRECTION_SHIFT_DOWN, ARPEGGIO_DIRECTION_SHIFT_UP_DOWN,

    ARPEGGIO_DIRECTION_COUNT
};

// TODO Maybe add something like struct ArpDirectionParams { dir, can_change, use_start_step }

inline static int __getDirection( int _direction ) {
	switch( _direction ) {
	case ARPEGGIO_DIRECTION_DOWN:
	case ARPEGGIO_DIRECTION_ROTATE_DOWN:
	case ARPEGGIO_DIRECTION_SHIFT_DOWN:
		return -1;
	default:
		return 1;
	}
}

inline static int __canChangeDir( int _direction ) {
	switch( _direction ) {
	case ARPEGGIO_DIRECTION_UP_DOWN:
	case ARPEGGIO_DIRECTION_ROTATE_UP_DOWN:
	case ARPEGGIO_DIRECTION_SHIFT_UP_DOWN:
		return 1;
	default:
		return 0;
	}
}

inline static int __canTranspose( int _direction ) {
	switch( _direction ) {
	case ARPEGGIO_DIRECTION_SHIFT_UP:
	case ARPEGGIO_DIRECTION_SHIFT_DOWN:
	case ARPEGGIO_DIRECTION_SHIFT_UP_DOWN:
		return 1;
	default:
		return 0;
	}
}

//for bitwise manipulations
#define FLOAT2SHORT 32768.f
#define SHORT2FLOAT 1./32768.f

#define RATIOINV 1./131072.f

#define SVFRANGE 1.23f
#define SVFOFFSET 0.151f
#define SVFGAINOFFSET 0.3f

#define LP2OFFSET -0.045f
#define min(a,b)                ((a)<(b)?(a):(b))

extern float noise[32];

inline 
float expf_fast(float a) {
  //https://github.com/ekmett/approximate/blob/master/cbits/fast.c
  union { float f; int x; } u;
  u.x = (int) (12102203 * a + 1064866805);
  return u.f;
}
inline
float sqrt3(const float x)  
{
  union
  {
    int i;
    float x;
  } u;

  u.x = x;
  u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
  return u.x;
} 
inline
float tanh2(float x)
{
	return x / (fabsf(x) + 3.f / (2.f + x * x));
}
inline
float sat25(float x)
{ 
	return x * (1.f - fabsf(x * 0.25f));
}
inline
float sat33(float x)
{
	if (unlikely(fabsf(x) > 2.58f))
		return 0;
	return x * (1 - x * x * 0.15f);
}
inline
float clamp(float d, float min, float max) {
  const float t = unlikely(d < min) ? min : d;
  return unlikely(t > max) ? max : t;
}
inline 
float satSin(float x, float drive, uint_fast16_t pos) {
	//https://www.desmos.com/calculator/pdsqpi5lp6
	return x + clamp(x, -0.5f * drive, drive) * sinTable[((uint_fast16_t)(fabsf(x) * 360) + pos) & 0x3FF];
}
//https://www.musicdsp.org/en/latest/Other/120-saturation.html
inline
float sigmoid(float x)
{
	return x * (1.5f - 0.5f * x * x);
}
float fold(float x4) {
	// https://www.desmos.com/calculator/ge2wvg2wgj
	// x4 = x / 4
	return (fabsf(x4 + 0.25f - roundf(x4 + 0.25f)) - 0.25f);
}
inline
float wrap(float x) {
	return x - floorf(x);
}
inline 
float window(float x) {
	if (x < 0 || x > 1) {
		return 0;
	} else if (x < 0.2f) {
		return sqrt3( x * 5 );
	} else if (x > 0.8f) {
		return sqrt3(1 - (x-0.8f) * 5);
	} else {
		return 1;
	}
}

enum NewNoteType {
	NEW_NOTE_FREE = 0,
	NEW_NOTE_RELEASE,
	NEW_NOTE_OLD,
	NEW_NOTE_NONE
};


arp_pattern_t lut_res_arpeggiator_patterns[ ARPEGGIATOR_PRESET_PATTERN_COUNT ]  = {
  ARP_PATTERN(21845), ARP_PATTERN(62965), ARP_PATTERN(46517), ARP_PATTERN(54741),
  ARP_PATTERN(43861), ARP_PATTERN(22869), ARP_PATTERN(38293), ARP_PATTERN(2313),
  ARP_PATTERN(37449), ARP_PATTERN(21065), ARP_PATTERN(18761), ARP_PATTERN(54553),
  ARP_PATTERN(27499), ARP_PATTERN(23387), ARP_PATTERN(30583), ARP_PATTERN(28087),
  ARP_PATTERN(22359), ARP_PATTERN(28527), ARP_PATTERN(30431), ARP_PATTERN(43281),
  ARP_PATTERN(28609), ARP_PATTERN(53505)
};

uint16_t Timbre::getArpeggiatorPattern() const
{
  const int pattern = (int)params.engineArp2.pattern;
  if ( pattern < ARPEGGIATOR_PRESET_PATTERN_COUNT )
    return ARP_PATTERN_GETMASK(lut_res_arpeggiator_patterns[ pattern ]);
  else
    return ARP_PATTERN_GETMASK( params.engineArpUserPatterns.patterns[ pattern - ARPEGGIATOR_PRESET_PATTERN_COUNT ] );
}

const uint8_t midi_clock_tick_per_step[17]  = {
  192, 144, 96, 72, 64, 48, 36, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1
};

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


    this->recomputeNext = true;
	this->currentGate = 0;
    this->sbMax = &this->sampleBlock[64];
    this->holdPedal = false;
    this->lastPlayedNote = 0;
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


    // Init FX variables
    v0L = v1L = v2L = v3L = v4L = v5L = v0R = v1R = v2R = v3R = v4R = v5R = 0.0f;
    fxParam1PlusMatrix = -1.0;
}

Timbre::~Timbre() {
}

void Timbre::init(int timbreNumber) {


	env1.init(&params.env1a,  &params.env1b, 0, &params.engine1.algo);
	env2.init(&params.env2a,  &params.env2b, 1, &params.engine1.algo);
	env3.init(&params.env3a,  &params.env3b, 2, &params.engine1.algo);
	env4.init(&params.env4a,  &params.env4b, 3, &params.engine1.algo);
	env5.init(&params.env5a,  &params.env5b, 4, &params.engine1.algo);
	env6.init(&params.env6a,  &params.env6b, 5, &params.engine1.algo);

	osc1.init(&params.osc1, OSC1_FREQ);
	osc2.init(&params.osc2, OSC2_FREQ);
	osc3.init(&params.osc3, OSC3_FREQ);
	osc4.init(&params.osc4, OSC4_FREQ);
	osc5.init(&params.osc5, OSC5_FREQ);
	osc6.init(&params.osc6, OSC6_FREQ);

    this->timbreNumber = timbreNumber;

    for (int s=0; s<2; s++) {
        for (int n=0; n<128; n++) {
            midiNoteScale[s][timbreNumber][n] = INV127 * (float)n;
        }
    }
    for (int lfo=0; lfo<NUMBER_OF_LFO; lfo++) {
        lfoUSed[lfo] = 0;
    }

}

void Timbre::setVoiceNumber(int v, int n) {
	voiceNumber[v] = n;
	if (n >=0) {
		voices[n]->setCurrentTimbre(this);
	}
}


void Timbre::initVoicePointer(int n, Voice* voice) {
	voices[n] = voice;
}

void Timbre::noteOn(char note, char velocity) {
	if (params.engineArp1.clock) {
		arpeggiatorNoteOn(note, velocity);
	} else {
		preenNoteOn(note, velocity);
	}
}

void Timbre::noteOff(char note) {
	if (params.engineArp1.clock) {
		arpeggiatorNoteOff(note);
	} else {
		preenNoteOff(note);
	}
}

int cptHighNote = 0;

void Timbre::preenNoteOn(char note, char velocity) {

	int iNov = (int) params.engine1.numberOfVoice;
	if (unlikely(iNov == 0)) {
		return;
	}

	unsigned int indexMin = (unsigned int)2147483647;
	int voiceToUse = -1;

	int newNoteType = NEW_NOTE_NONE;

	for (int k = 0; k < iNov; k++) {
		// voice number k of timbre
		int n = voiceNumber[k];

        if (unlikely(voices[n]->isNewNotePending())) {
            continue;
        }

		// same note = priority 1 : take the voice immediatly
		if (unlikely(voices[n]->isPlaying() && voices[n]->getNote() == note)) {

#ifdef DEBUG_VOICE
		lcd.setRealTimeAction(true);
		lcd.setCursor(16,1);
		lcd.print(cptHighNote++);
		lcd.setCursor(16,2);
		lcd.print("S:");
		lcd.print(n);
#endif

            preenNoteOnUpdateMatrix(n, note, velocity);
            voices[n]->noteOnWithoutPop(note, velocity, voiceIndex++);
            this->lastPlayedNote = n;
			return;
		}

		// unlikely because if it true, CPU is not full
		if (unlikely(newNoteType > NEW_NOTE_FREE)) {
			if (!voices[n]->isPlaying()) {
				voiceToUse = n;
				newNoteType = NEW_NOTE_FREE;
			}

			if (voices[n]->isReleased()) {
				int indexVoice = voices[n]->getIndex();
				if (indexVoice < indexMin) {
					indexMin = indexVoice;
					voiceToUse = n;
					newNoteType = NEW_NOTE_RELEASE;
				}
			}
		}
	}

	if (voiceToUse == -1) {
		for (int k = 0; k < iNov; k++) {
			// voice number k of timbre
			int n = voiceNumber[k];
			int indexVoice = voices[n]->getIndex();
			if (indexVoice < indexMin && !voices[n]->isNewNotePending()) {
				newNoteType = NEW_NOTE_OLD;
				indexMin = indexVoice;
				voiceToUse = n;
			}
		}
	}
	// All voices in newnotepending state ?
	if (voiceToUse != -1) {
#ifdef DEBUG_VOICE
		lcd.setRealTimeAction(true);
		lcd.setCursor(16,1);
		lcd.print(cptHighNote++);
		lcd.setCursor(16,2);
		switch (newNoteType) {
			case NEW_NOTE_FREE:
				lcd.print("F:");
				break;
			case NEW_NOTE_OLD:
				lcd.print("O:");
				break;
			case NEW_NOTE_RELEASE:
				lcd.print("R:");
				break;
		}
		lcd.print(voiceToUse);
#endif


		preenNoteOnUpdateMatrix(voiceToUse, note, velocity);

		switch (newNoteType) {
		case NEW_NOTE_FREE:
			voices[voiceToUse]->noteOn(note, velocity, voiceIndex++);
			break;
		case NEW_NOTE_OLD:
		case NEW_NOTE_RELEASE:
			voices[voiceToUse]->noteOnWithoutPop(note, velocity, voiceIndex++);
			break;
		}

		this->lastPlayedNote = voiceToUse;
	}
}

void Timbre::preenNoteOnUpdateMatrix(int voiceToUse, int note, int velocity) {
    // Update voice matrix with midi note and velocity
    voices[voiceToUse]->matrix.setSource(MATRIX_SOURCE_NOTE1, midiNoteScale[0][timbreNumber][note]);
    voices[voiceToUse]->matrix.setSource(MATRIX_SOURCE_NOTE2, midiNoteScale[1][timbreNumber][note]);
    voices[voiceToUse]->matrix.setSource(MATRIX_SOURCE_VELOCITY, INV127*velocity);
}

void Timbre::preenNoteOff(char note) {
	int iNov = (int) params.engine1.numberOfVoice;
	for (int k = 0; k < iNov; k++) {
		// voice number k of timbre
		int n = voiceNumber[k];

		// Not playing = free CPU
		if (unlikely(!voices[n]->isPlaying())) {
			continue;
		}

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
	    setNewBPMValue(params.engineArp1.BPM);
	}
	if (clockValue == CLOCK_EXTERNAL) {
		// Let's consider we're running
		running_ = 1;
	}
}


void Timbre::prepareForNextBlock() {

	// Apeggiator clock : internal
	if (params.engineArp1.clock == CLOCK_INTERNAL) {
		arpegiatorStep+=1.0f;
		if (unlikely((arpegiatorStep) > ticksEveryNCalls)) {
			arpegiatorStep -= ticksEveyNCallsInteger;
			Tick();
		}
	}
}

void Timbre::cleanNextBlock() {

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


void Timbre::prepareMatrixForNewBlock() {
    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->prepareMatrixForNewBlock();
    }
}


#define GATE_INC 0.02f

void Timbre::fxAfterBlock(float ratioTimbres) {

    // Gate algo !!
    float gate = voices[this->lastPlayedNote]->matrix.getDestination(MAIN_GATE);
    if (unlikely(gate > 0 || currentGate > 0)) {
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
    	for (int k=0 ; k< BLOCK_SIZE ; k++) {
			currentGate += incGate;
			coef = 1.0f - currentGate;
			*sp = *sp * coef;
			sp++;
			*sp = *sp * coef;
			sp++;
		}
    //    currentGate = gate;
    }

    float matrixFilterFrequency = voices[this->lastPlayedNote]->matrix.getDestination(FILTER_FREQUENCY);
	float numberVoicesAttn = 1 - (params.engine1.numberOfVoice * 0.04f * ratioTimbres * RATIOINV);

    // LP Algo
    int effectType = params.effect.type;
    float gainTmp =  params.effect.param3 * numberOfVoiceInverse * ratioTimbres;
    mixerGain = 0.02f * gainTmp + .98f  * mixerGain;

    switch (effectType) {
    case FILTER_LP:
    {
    	float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
    	fxParamTmp *= fxParamTmp;

    	// Low pass... on the Frequency
    	fxParam1 = (fxParamTmp + 9.0f * fxParam1) * .1f;
    	if (unlikely(fxParam1 > 1.0f)) {
    		fxParam1 = 1.0f;
    	}
    	if (unlikely(fxParam1 < 0.0f)) {
    		fxParam1 = 0.0f;
    	}

    	float pattern = (1 - fxParam2 * fxParam1);

    	float *sp = this->sampleBlock;
    	float localv0L = v0L;
    	float localv1L = v1L;
    	float localv0R = v0R;
    	float localv1R = v1R;

    	for (int k=BLOCK_SIZE ; k--; ) {

    		// Left voice
    		localv0L =  pattern * localv0L  -  (fxParam1) * (localv1L + *sp);
    		localv1L =  pattern * localv1L  +  (fxParam1) * localv0L;

    		*sp = localv1L * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;

    		// Right voice
    		localv0R =  pattern * localv0R  -  (fxParam1) * (localv1R + *sp);
    		localv1R =  pattern * localv1R  +  (fxParam1) * localv0R;

    		*sp = localv1R * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;
    	}
    	v0L = localv0L;
    	v1L = localv1L;
    	v0R = localv0R;
    	v1R = localv1R;
    }
    break;
    case FILTER_HP:
    {
    	float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
    	fxParamTmp *= fxParamTmp;

    	// Low pass... on the Frequency
    	fxParam1 = (fxParamTmp + 9.0f * fxParam1) * .1f;
    	if (unlikely(fxParam1 > 1.0)) {
    		fxParam1 = 1.0;
    	}
        if (unlikely(fxParam1 < 0.0f)) {
            fxParam1 = 0.0f;
        }
    	float pattern = (1 - fxParam2 * fxParam1);

    	float *sp = this->sampleBlock;
    	float localv0L = v0L;
    	float localv1L = v1L;
    	float localv0R = v0R;
    	float localv1R = v1R;

    	for (int k=0 ; k < BLOCK_SIZE ; k++) {

    		// Left voice
    		localv0L =  pattern * localv0L  -  (fxParam1) * (localv1L + *sp);
    		localv1L =  pattern * localv1L  +  (fxParam1) * localv0L;

    		*sp = (*sp - localv1L) * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;

    		// Right voice
    		localv0R =  pattern * localv0R  -  (fxParam1) * (localv1R + *sp);
    		localv1R =  pattern * localv1R  +  (fxParam1) * localv0R;

    		*sp = (*sp - localv1R) * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;
    	}
    	v0L = localv0L;
    	v1L = localv1L;
    	v0R = localv0R;
    	v1R = localv1R;

    }
	break;
    case FILTER_BASS:
    {
    	// From musicdsp.com
    	//    	Bass Booster
    	//
    	//    	Type : LP and SUM
    	//    	References : Posted by Johny Dupej
    	//
    	//    	Notes :
    	//    	This function adds a low-passed signal to the original signal. The low-pass has a quite wide response.
    	//
    	//    	selectivity - frequency response of the LP (higher value gives a steeper one) [70.0 to 140.0 sounds good]
    	//    	ratio - how much of the filtered signal is mixed to the original
    	//    	gain2 - adjusts the final volume to handle cut-offs (might be good to set dynamically)

    	//static float selectivity, gain1, gain2, ratio, cap;
    	//gain1 = 1.0/(selectivity + 1.0);
    	//
    	//cap= (sample + cap*selectivity )*gain1;
    	//sample = saturate((sample + cap*ratio)*gain2);

    	float *sp = this->sampleBlock;
    	float localv0L = v0L;
    	float localv0R = v0R;

    	for (int k=0 ; k < BLOCK_SIZE ; k++) {

    		localv0L = ((*sp) + localv0L * fxParam1) * fxParam3;
    		(*sp) = ((*sp) + localv0L * fxParam2) * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;

    		localv0R = ((*sp) + localv0R * fxParam1) * fxParam3;
    		(*sp) = ((*sp) + localv0R * fxParam2) * mixerGain;

    		if (unlikely(*sp > ratioTimbres)) {
    			*sp = ratioTimbres;
    		}
    		if (unlikely(*sp < -ratioTimbres)) {
    			*sp = -ratioTimbres;
    		}

    		sp++;
    	}
    	v0L = localv0L;
    	v0R = localv0R;

    }
    break;
    case FILTER_MIXER:
    {
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

    	float pan = fxParam1 * 2 - 1.0f ;
    	float *sp = this->sampleBlock;
    	float sampleR, sampleL;
    	if (pan <= 0) {
        	float onePlusPan = 1 + pan;
        	float minusPan = - pan;
        	for (int k=BLOCK_SIZE ; k--; ) {
				sampleL = *(sp);
				sampleR = *(sp + 1);

				*sp = (sampleL + sampleR * minusPan) * mixerGain;
				sp++;
				*sp = sampleR * onePlusPan * mixerGain;
				sp++;
			}
    	} else if (pan > 0) {
        	float oneMinusPan = 1 - pan;
        	float adjustedmixerGain = (pan * .5) * mixerGain;
        	for (int k=0 ; k < BLOCK_SIZE ; k++) {
				sampleL = *(sp);
				sampleR = *(sp + 1);

				*sp = sampleL * oneMinusPan * mixerGain;
				sp++;
				*sp = (sampleR + sampleL * pan) * mixerGain;
				sp++;
			}
    	}
    }
    break;
    case FILTER_CRUSHER:
    {
        // Algo from http://www.musicdsp.org/archive.php?classid=4#139
        // Lo-Fi Crusher
        // Type : Quantizer / Decimator with smooth control
        // References : Posted by David Lowenfels

        //        function output = crusher( input, normfreq, bits );
        //            step = 1/2^(bits);
        //            phasor = 0;
        //            last = 0;
        //
        //            for i = 1:length(input)
        //               phasor = phasor + normfreq;
        //               if (phasor >= 1.0)
        //                  phasor = phasor - 1.0;
        //                  last = step * floor( input(i)/step + 0.5 ); %quantize
        //               end
        //               output(i) = last; %sample and hold
        //            end
        //        end


        float fxParamTmp = params.effect.param1 + matrixFilterFrequency +.005f;
        if (unlikely(fxParamTmp > 1.0)) {
            fxParamTmp = 1.0;
        }
        if (unlikely(fxParamTmp < 0.005f)) {
            fxParamTmp = 0.005f;
        }
        fxParamA1 = (fxParamTmp + 9.0f * fxParamA1) * .1f;
        // Low pass... on the Sampling rate
        register float fxFreq = fxParamA1;

        register float *sp = this->sampleBlock;

        register float localPhase = fxPhase;

        //        localPower = fxParam1 = pow(2, (int)(1.0f + 15.0f * params.effect.param2));
        //        localStep = fxParam2 = 1 / fxParam1;

        register float localPower = fxParam1;
        register float localStep = fxParam2;

        register float localv0L = v0L;
        register float localv0R = v0R;


        for (int k=0 ; k < BLOCK_SIZE ; k++) {
            localPhase += fxFreq;
            if (unlikely(localPhase >= 1.0f)) {
                localPhase -= 1.0f;
                // Simulate floor by making the conversion always positive
                // simplify version
                register int iL =  (*sp) * localPower + .75f;
                register int iR =  (*(sp + 1)) * localPower + .75f;
                localv0L = localStep * iL;
                localv0R = localStep * iR;
            }

            *sp++ = localv0L * mixerGain;
            *sp++ = localv0R * mixerGain;
        }
        v0L = localv0L;
        v0R = localv0R;
        fxPhase = localPhase;

    }
    break;
    case FILTER_BP:
    {
//        float input;                    // input sample
//        float output;                   // output sample
//        float v;                        // This is the intermediate value that
//                                        //    gets stored in the delay registers
//        float old1;                     // delay register 1, initialized to 0
//        float old2;                     // delay register 2, initialized to 0
//
//        /* filter coefficients */
//        omega1  = 2 * PI * f/srate; // f is your center frequency
//        sn1 = (float)sin(omega1);
//        cs1 = (float)cos(omega1);
//        alpha1 = sn1/(2*Qvalue);        // Qvalue is none other than Q!
//        a0 = 1.0f + alpha1;     // a0
//        b0 = alpha1;            // b0
//        b1 = 0.0f;          // b1/b0
//        b2= -alpha1/b0          // b2/b0
//        a1= -2.0f * cs1/a0;     // a1/a0
//        a2= (1.0f - alpha1)/a0;          // a2/a0
//        k = b0/a0;
//
//        /* The filter code */
//
//        v = k*input - a1*old1 - a2*old2;
//        output = v + b1*old1 + b2*old2;
//        old2 = old1;
//        old1 = v;

        // fxParam1 v
        //

        float fxParam1PlusMatrixTmp = params.effect.param1 + matrixFilterFrequency;
        if (unlikely(fxParam1PlusMatrixTmp > 1.0f)) {
            fxParam1PlusMatrixTmp = 1.0f;
        }
        if (unlikely(fxParam1PlusMatrixTmp < 0.0f)) {
            fxParam1PlusMatrixTmp = 0.0f;
        }

        if (fxParam1PlusMatrix != fxParam1PlusMatrixTmp) {
            fxParam1PlusMatrix = fxParam1PlusMatrixTmp;
        	recomputeBPValues(params.effect.param2, fxParam1PlusMatrix * fxParam1PlusMatrix);
        }

        float localv0L = v0L;
        float localv0R = v0R;
        float localv1L = v1L;
        float localv1R = v1R;
        float *sp = this->sampleBlock;

        for (int k=0 ; k < BLOCK_SIZE ; k++) {
            float localV = fxParam1 /* k */ * (*sp) - fxParamA1 * localv0L - fxParamA2 * localv1L;
            *sp++ = (localV + /* fxParamB1 (==0) * localv0L  + */ fxParamB2 * localv1L) * mixerGain;
            if (unlikely(*sp > ratioTimbres)) {
                *sp = ratioTimbres;
            }
            if (unlikely(*sp < -ratioTimbres)) {
                *sp = -ratioTimbres;
            }
            localv1L = localv0L;
            localv0L = localV;

            localV = fxParam1 /* k */ * (*sp) - fxParamA1 * localv0R - fxParamA2 * localv1R;
            *sp++ = (localV + /* fxParamB1 (==0) * localv0R + */ fxParamB2 * localv1R) * mixerGain;
            if (unlikely(*sp > ratioTimbres)) {
                *sp = ratioTimbres;
            }
            if (unlikely(*sp < -ratioTimbres)) {
                *sp = -ratioTimbres;
            }
            localv1R = localv0R;
            localv0R = localV;


        }

        v0L = localv0L;
        v0R = localv0R;
        v1L = localv1L;
        v1R = localv1R;

        break;
    }
case FILTER_LP2:
    {
		float fxParamTmp = LP2OFFSET + params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float f = (fxParam1);
    	const float pattern = (1 - fxParam2 * f);

    	float *sp = this->sampleBlock;
    	float localv0L = v0L;
    	float localv1L = v1L;
    	float localv0R = v0R;
    	float localv1R = v1R;

    	for (int k=BLOCK_SIZE ; k--; ) {

			// Left voice
			localv0L = pattern * localv0L - f * sat25(localv1L + *sp);
			localv1L = pattern * localv1L + f * localv0L;

			localv0L = pattern * localv0L - f * (localv1L + *sp);
			localv1L = pattern * localv1L + f * localv0L;

			*sp++ = clamp(localv1L * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			localv0R = pattern * localv0R - f * sat25(localv1R + *sp);
			localv1R = pattern * localv1R + f * localv0R;

			localv0R = pattern * localv0R - f * (localv1R + *sp);
			localv1R = pattern * localv1R + f * localv0R;

			*sp++ = clamp(localv1R * mixerGain, -ratioTimbres, ratioTimbres);
		}
    	v0L = localv0L;
    	v1L = localv1L;
    	v0R = localv0R;
    	v1R = localv1R;
    }
    break;
case FILTER_HP2:
    {
		float fxParamTmp = LP2OFFSET + params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float f = (fxParam1);
        const float pattern = (1 - fxParam2 * f);

        float *sp = this->sampleBlock;
        float localv0L = v0L;
        float localv1L = v1L;
        float localv0R = v0R;
        float localv1R = v1R;

        for (int k=0 ; k < BLOCK_SIZE ; k++) {

			// Left voice
			localv0L = pattern * localv0L + f * sat33(-localv1L + *sp);
			localv1L = pattern * localv1L + f * localv0L;

			localv0L = pattern * localv0L + f * (-localv1L + *sp);
			localv1L = pattern * localv1L + f * localv0L;

			*sp++ = clamp((*sp - localv1L) * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			localv0R = pattern * localv0R + f * sat33(-localv1R + *sp);
			localv1R = pattern * localv1R + f * localv0R;

			localv0R = pattern * localv0R + f * (-localv1R + *sp);
			localv1R = pattern * localv1R + f * localv0R;

			*sp++ = clamp((*sp - localv1R) * mixerGain, -ratioTimbres, ratioTimbres);
		}
        v0L = localv0L;
        v1L = localv1L;
        v0R = localv0R;
        v1R = localv1R;

    }
    break;
case FILTER_BP2:
{
    float fxParam1PlusMatrixTmp = clamp(params.effect.param1 + matrixFilterFrequency, 0, 1);

    if (fxParam1PlusMatrix != fxParam1PlusMatrixTmp) {
        fxParam1PlusMatrix = fxParam1PlusMatrixTmp;
        recomputeBPValues(params.effect.param2, fxParam1PlusMatrix * fxParam1PlusMatrix);
    }

    float localv0L = v0L;
    float localv0R = v0R;
    float localv1L = v1L;
    float localv1R = v1R;
    float *sp = this->sampleBlock;
    float in,temp,localV;

    for (int k=0 ; k < BLOCK_SIZE ; k++) {
		//Left
		in = fxParam1 * sat33(*sp);
		temp = in - fxParamA1 * localv0L - fxParamA2 * localv1L;
		localv1L = localv0L;
		localv0L = temp;
		localV = (temp + (in - fxParamA1 * localv0L - fxParamA2 * localv1L)) * 0.5f;
		*sp++ = clamp((localV + fxParamB2 * localv1L) * mixerGain, -ratioTimbres, ratioTimbres);

		localv1L = localv0L;
		localv0L = localV;

		//Right
		in = fxParam1 * sat33(*sp);
		temp = in - fxParamA1 * localv0R - fxParamA2 * localv1R;
		localv1R = localv0R;
		localv0R = temp;
		localV = (temp + (in - fxParamA1 * localv0R - fxParamA2 * localv1R)) * 0.5f;
		*sp++ = clamp((localV + fxParamB2 * localv1R) * mixerGain, -ratioTimbres, ratioTimbres);

		localv1R = localv0R;
		localv0R = localV;
	}

    v0L = localv0L;
    v0R = localv0R;
    v1L = localv1L;
    v1R = localv1R;

    break;
}
case FILTER_LP3:
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(1 - fxParam2 * 0.999f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;

	for (int k = BLOCK_SIZE; k--;)	{
		// Left voice

		lowL += f * bandL;
		bandL += f * (scale * (*sp) - lowL - fb * sat25(bandL));
		
		lowL += f * bandL;
		bandL += f * (scale * (*sp) - lowL - fb * (bandL));

		*sp++ = clamp(lowL * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice

		lowR += f * bandR;
		bandR += f * (scale * (*sp) - lowR - fb * sat25(bandR));

		lowR += f * bandR;
		bandR += f * (scale * (*sp) - lowR - fb * (bandR));

		*sp++ = clamp(lowR * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_HP3: 
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(1 - fxParam2 * 0.999f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;

	for (int k=BLOCK_SIZE ; k--; ) {
		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * sat33(bandL);
		bandL = f * highL + bandL;

		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * bandL;
		bandL = f * highL + bandL;

		*sp++ = clamp(highL * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * sat33(bandR);
		bandR = f * highR + bandR;

		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * bandR;
		bandR = f * highR + bandR;

		*sp++ = clamp(highR * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_BP3: 
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(0.5f - fxParam2 * 0.5f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * sat25(bandL);
		bandL = f * highL + bandL;
		*sp++ = clamp(bandL * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * sat25(bandR);
		bandR = f * highR + bandR;
		*sp++ = clamp(bandR * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_PEAK:
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(1 - fxParam2 * 0.999f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * bandL;
		bandL = f * highL + bandL;

		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * sat33(bandL);
		bandL = f * highL + bandL;

		*sp++ = clamp((bandL + highL + lowL) * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * bandR;
		bandR = f * highR + bandR;

		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * sat33(bandR);
		bandR = f * highR + bandR;

		*sp++ = clamp((bandR + highR + lowR) * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_NOTCH:
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(1 - fxParam2 * 0.6f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET) * mixerGain;

	for (int k=0 ; k < BLOCK_SIZE; k++) {

		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * bandL;
		bandL = f * highL + bandL;
		*sp++ = clamp((highL + lowL) * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * bandR;
		bandR = f * highR + bandR;
		*sp++ = clamp((highR + lowR) * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_BELL:
{
	//filter algo from Andrew Simper
	//https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	//A = 10 ^ (db / 40)
	const float A = (tanh2(fxParam2 * 2) * 1.5f) + 0.5f;

	const float res = 0.6f;
	const float k = 1 / (0.0001f + res * A);
	const float g = 0.0001f + (fxParam1);
	const float a1 = 1 / (1 + g * (g + k));
	const float a2 = g * a1;
	const float a3 = g * a2;
	const float amp = k * (A * A - 1);

	float *sp = this->sampleBlock;

	float ic1eqL = v0L, ic2eqL = v1L;
	float ic1eqR = v0R, ic2eqR = v1R;
	float v1, v2, v3;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		v3 = (*sp) - ic2eqL;
		v1 = a1 * ic1eqL + a2 * v3;
		v2 = ic2eqL + a2 * ic1eqL + a3 * v3;
		ic1eqL = 2 * v1 - ic1eqL;
		ic2eqL = 2 * v2 - ic2eqL;

		*sp++ = clamp((*sp + (amp * v1)) * mixerGain, -ratioTimbres, ratioTimbres);

		// Right voice
		v3 = (*sp) - ic2eqR;
		v1 = a1 * ic1eqR + a2 * v3;
		v2 = ic2eqR + a2 * ic1eqR + a3 * v3;
		ic1eqR = 2 * v1 - ic1eqR;
		ic2eqR = 2 * v2 - ic2eqR;

		*sp++ = clamp((*sp + (amp * v1)) * mixerGain, -ratioTimbres, ratioTimbres);
	}

	v0L = ic1eqL;
	v1L = ic2eqL;
	v0R = ic1eqR;
	v1R = ic2eqR;
}
break;
case FILTER_LOWSHELF:
{
	//filter algo from Andrew Simper
	//https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	//A = 10 ^ (db / 40)
	const float A = (tanh2(fxParam2 * 2) * 1) + 0.5f;

	const float res = 0.5f;
	const float k = 1 / (0.0001f + res);
	const float g = 0.0001f + (fxParam1);
	const float a1 = 1 / (1 + g * (g + k));
	const float a2 = g * a1;
	const float a3 = g * a2;
	const float m1 = k * (A - 1);
	const float m2 = (A * A - 1);

	float *sp = this->sampleBlock;

	float ic1eqL = v0L, ic2eqL = v1L;
	float ic1eqR = v0R, ic2eqR = v1R;
	float v1, v2, v3;

	const float svfGain = mixerGain * 0.8f;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		v3 = (*sp) - ic2eqL;
		v1 = a1 * ic1eqL + a2 * v3;
		v2 = ic2eqL + a2 * ic1eqL + a3 * v3;
		ic1eqL = 2 * v1 - ic1eqL;
		ic2eqL = 2 * v2 - ic2eqL;

		*sp++ = clamp((*sp + (m1 * v1 + m2 * v2)) * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		v3 = (*sp) - ic2eqR;
		v1 = a1 * ic1eqR + a2 * v3;
		v2 = ic2eqR + a2 * ic1eqR + a3 * v3;
		ic1eqR = 2 * v1 - ic1eqR;
		ic2eqR = 2 * v2 - ic2eqR;

		*sp++ = clamp((*sp + (m1 * v1 + m2 * v2)) * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = ic1eqL;
	v1L = ic2eqL;
	v0R = ic1eqR;
	v1R = ic2eqR;
}
break;
case FILTER_HIGHSHELF:
{
	//filter algo from Andrew Simper
	//https://cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	//A = 10 ^ (db / 40)
	const float A = (tanh2(fxParam2 * 2) * 1) + 0.5f;

	const float res = 0.5f;
	const float k = 1 / (0.0001f + res);
	const float g = 0.0001f + (fxParam1);
	const float a1 = 1 / (1 + g * (g + k));
	const float a2 = g * a1;
	const float a3 = g * a2;
	const float m0 = A * A;
	const float m1 = k * (A - 1) * A;
	const float m2 = (1 - A * A);

	float *sp = this->sampleBlock;

	float ic1eqL = v0L, ic2eqL = v1L;
	float ic1eqR = v0R, ic2eqR = v1R;
	float v1, v2, v3;

	const float svfGain = mixerGain * 0.8f;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		v3 = (*sp) - ic2eqL;
		v1 = a1 * ic1eqL + a2 * v3;
		v2 = ic2eqL + a2 * ic1eqL + a3 * v3;
		ic1eqL = 2 * v1 - ic1eqL;
		ic2eqL = 2 * v2 - ic2eqL;

		*sp++ = clamp((m0 * *sp + (m1 * v1 + m2 * v2)) * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		v3 = (*sp) - ic2eqR;
		v1 = a1 * ic1eqR + a2 * v3;
		v2 = ic2eqR + a2 * ic1eqR + a3 * v3;
		ic1eqR = 2 * v1 - ic1eqR;
		ic2eqR = 2 * v2 - ic2eqR;

		*sp++ = clamp((m0 * *sp + (m1 * v1 + m2 * v2)) * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = ic1eqL;
	v1L = ic2eqL;
	v0R = ic1eqR;
	v1R = ic2eqR;
}
break;
case FILTER_LPHP:
{
	float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;

	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const int mixWet = (params.effect.param1 * 122);
	const float mixA = (1 + fxParam2 * 2) * panTable[122 - mixWet];
	const float mixB = 2.5f * panTable[5 + mixWet];

	const float f = fxParam1 * fxParam1 * 1.5f;
	const float pattern = (1 - fxParam2 * f);

	float *sp = this->sampleBlock;
	float localv0L = v0L;
	float localv1L = v1L;
	float localv0R = v0R;
	float localv1R = v1R;
	const float gain = mixerGain * 1.3f;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		localv0L = pattern * localv0L + (f * (-localv1L + (*sp)));
		localv1L = pattern * localv1L + f * localv0L;
		*sp++ = clamp(sat33(((localv1L * mixA) + ((*sp - localv1L) * mixB))) * gain, -ratioTimbres, ratioTimbres);

		// Right voice
		localv0R = pattern * localv0R + (f * (-localv1R + (*sp)));
		localv1R = pattern * localv1R + f * localv0R;
		*sp++ = clamp(sat33(((localv1R * mixA) + ((*sp - localv1R) * mixB))) * gain, -ratioTimbres, ratioTimbres);
	}
	v0L = localv0L;
	v1L = localv1L;
	v0R = localv0R;
	v1R = localv1R;
}
break;
case FILTER_BPds:
{
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam1 * fxParam1 * SVFRANGE;
	const float fb = sqrt3(0.5f - fxParam2 * 0.5f);
	const float scale = sqrt3(fb);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;

	const float sat = 1 + fxParam2;

	for (int k=BLOCK_SIZE ; k--; ) {

		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * (*sp) - lowL - fb * bandL;
		bandL = (f * highL) + bandL;
		*sp++ = clamp(sat25((bandL * sat)) * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * (*sp) - lowR - fb * bandR;
		bandR = (f * highR) + bandR;
		*sp++ = clamp(sat25((bandR * sat)) * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
}
break;
case FILTER_LPWS:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float a = 1.f - fxParam1;
		const float b = 1.f - a;

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv0R = v0R;

		const int mixWet = (params.effect.param2 * 127);
		const float mixA = panTable[mixWet] * mixerGain;
		const float mixB = panTable[127 - mixWet] * mixerGain;

		for (int k=BLOCK_SIZE ; k--; ) {
			// Left voice
			localv0L = (*sp  * b) + (localv0L * a);
			*sp++ = clamp((sigmoid(sat25(localv0L * 2.0f)) * mixA + (mixB * (*sp))), -ratioTimbres, ratioTimbres);

			// Right voice
			localv0R = (*sp  * b) + (localv0R * a);
			*sp++ = clamp((sigmoid(sat25(localv0R * 2.0f)) * mixA + (mixB * (*sp))), -ratioTimbres, ratioTimbres);
		}
    	v0L = localv0L;
    	v0R = localv0R;
	}
	break;
case FILTER_TILT:
    {
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float f1 = clamp((fxParam1), 0.001f, 1); // allpass F
		float coef1 = (1.0f - f1) / (1.0f + f1);

		const float res = 0.85f;

		const float amp = 19.93f;
		const float gain = (params.effect.param2 - 0.5f);
		const float gfactor = 10;
		float g1, g2;
		if (gain > 0) {
			g1 = -gfactor * gain;
			g2 = gain;
		} else {
			g1 = -gain;
			g2 = gfactor * gain;
		};

		//two separate gains
		const float lgain = expf_fast(g1 / amp) - 1.f;
		const float hgain = expf_fast(g2 / amp) - 1.f;

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv0R = v0R;
		float localv1L = v1L;
		float localv1R = v1R;

		float _ly1L = v2L, _ly1R = v2R;
		float _ly2L = v3L, _ly2R = v3R;
		float _lx1L = v4L, _lx1R = v4R;

		for (int k=BLOCK_SIZE ; k--; ) {
			// Left voice
			_ly1L = coef1 * (_ly1L + *sp) - _lx1L; // allpass
			_lx1L = *sp;

			localv0L = res * localv0L - (fxParam1) * localv1L + sigmoid(_ly1L);
			localv1L = res * localv1L + (fxParam1) * localv0L;
			*sp++ = clamp((*sp + lgain * (localv1L) + hgain * (*sp - localv1L)) * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			_ly1R = coef1 * (_ly1R + *sp) - _lx1R; // allpass
			_lx1R = *sp;

			localv0R = res * localv0R - (fxParam1) * localv1R + sigmoid(_ly1R);
			localv1R = res * localv1R + (fxParam1) * localv0R;
			*sp++ = clamp((*sp + lgain * (localv1R) + hgain * (*sp - localv1R)) * mixerGain, -ratioTimbres, ratioTimbres);
		}

        v0L = localv0L;
        v0R = localv0R;
        v1L = localv1L;
        v1R = localv1R;

		v2L = _ly1L; v2R = _ly1R;
		v3L = _ly2L; v3R = _ly2R;
		v4L = _lx1L; v4R = _lx1R;

    }
    break;
case FILTER_STEREO:
{
	float fxParamTmp = fabsf(params.effect.param1 + matrixFilterFrequency);
	fxParam1 = ((fxParamTmp + 19.0f * fxParam1) * .05f);

	float OffsetTmp = fabsf(params.effect.param2);
	fxParam2 = ((OffsetTmp + 19.0f * fxParam2) * .05f);

	const float offset = fxParam2 * 0.66f - 0.33f;

	const float f1 = clamp((fxParam1), 0.001f, 0.99f) * 1.8f - 0.9f;
	const float f2 = clamp((fxParam1 + offset), 0.001f, 0.99f) * 1.8f - 0.9f;
	const float f3 = clamp((fxParam1 + offset * 2), 0.001f, 0.99f) * 1.8f - 0.9f;;

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;
	float lowL2 = v2L, highL2 = 0, bandL2 = v3L;
	float lowR2 = v2R, highR2 = 0, bandR2 = v3R;
	float lowL3 = v4L, highL3 = 0, bandL3 = v5L;
	float lowR3 = v4R, highR3 = 0, bandR3 = v5R;
	float out;

	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		
		lowL = (*sp) + f1 * (bandL = lowL - f1 * (*sp));
		highL = (bandL + (*sp)) * 0.5f;

		lowL2 = highL + f2 * (bandL2 = (lowL2) - f2 * highL);
		highL2 = ((bandL2) + highL) * 0.5f;

		lowL3 = lowL2 + f3 * (bandL3 = lowL3 - f3 * lowL2);
		highL3 = (bandL3 + highL2) * 0.5f;

		*sp++ = clamp(highL3 * mixerGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = (*sp) + f1 * (bandR = lowR - f1 * (*sp));
		highR = (bandR + (*sp)) * 0.5f;

		lowR2 = lowR + f2 * (bandR2 = (lowR2) - f2 * lowR);
		highR2 = ((bandR2) + highR) * 0.5f;

		lowR3 = lowR2 + f3 * (bandR3 = lowR3 - f3 * lowR2);
		highR3 = (bandR3 + highR2) * 0.5f;

		*sp++ = clamp(highR3 * mixerGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;

	v2L = lowL2;
	v3L = bandL2;
	v2R = lowR2;
	v3R = bandR2;

	v4L = lowL3;
	v5L = bandL3;
	v4R = lowR3;
	v5R = bandR3;
}
break;
case FILTER_SAT:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float f1 = clamp((fxParam1), 0.001f, 1); // allpass F
		float coef1 = (1.0f - f1) / (1.0f + f1);

		float *sp = this->sampleBlock;

		float _ly1L = v0L, _ly1R = v0R;
		float _lx1L = v3L, _lx1R = v3R;
		float filterOut, in;

		const float a = 0.945f - fxParam2 * 0.599999f;
		const float b = 1.f - a;

		const float threshold = (sqrt3(fxParam1) * 0.4f) * numberVoicesAttn;
		const float thresTop = (threshold + 1) * 0.5f;
		const float invT = 1.f / thresTop;

		for (int k=BLOCK_SIZE ; k--; ) {
			//LEFT
			in = sigmoid(*sp);
			if(fabsf(in) > threshold) {
				in = in > 1 ? thresTop : in * invT;
			}
			_ly1L = coef1 * (_ly1L + in) - _lx1L; // allpass
			_lx1L = in;
			filterOut = (in * b) + (_ly1L * a); // lowpass
			*sp++ = clamp((*sp - filterOut) * mixerGain, -ratioTimbres, ratioTimbres);

			//RIGHT
			in = sigmoid(*sp);
			if(fabsf(in) > threshold) {
				in = in > 1 ? thresTop : in * invT;
			}
			_ly1R = coef1 * (_ly1R + in) - _lx1R; // allpass
			_lx1R = in;
			filterOut = (in * b) + (_ly1R * a); // lowpass
			*sp++ = clamp((*sp - filterOut) * mixerGain, -ratioTimbres, ratioTimbres);
		}

		v0L = _ly1L; v0R = _ly1R;
		v3L = _lx1L; v3R = _lx1R;
	}
	break;
case FILTER_SIGMOID:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv1L = v1L;
		float localv0R = v0R;
		float localv1R = v1R;

		float f = fxParam2 * 0.5f + 0.12f;
		float pattern = (1 - 0.7f * f);

		int drive = (27 + sqrt3(fxParam1) * 100);
		float gain = 1.1f + 44 * panTable[drive];
		float gainCorrection = (1.2f - sqrt3(panTable[64 + (drive >> 1)] * 0.6f));
		float bias = -0.1f + (fxParam1 * 0.2f);

		for (int k=BLOCK_SIZE ; k--; ) {
			// Left voice
			localv0L = tanh2(bias + sat33(*sp) * gain) * gainCorrection;
			localv0L = pattern * localv0L + f * (*sp - localv1L);
			localv1L = pattern * localv1L + f * localv0L;

			*sp++ = clamp((*sp - localv1L) * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			localv0R = tanh2(bias + sat33(*sp) * gain) * gainCorrection;
			localv0R = pattern * localv0R + f * (*sp - localv1R);
			localv1R = pattern * localv1R + f * localv0R;

			*sp++ = clamp((*sp - localv1R) * mixerGain, -ratioTimbres, ratioTimbres);
		}

    	v0L = localv0L;
    	v0R = localv0R;
        v1L = localv1L;
        v1R = localv1R;
	}
	break;
case FILTER_FOLD:
	{
		//https://www.desmos.com/calculator/ge2wvg2wgj

		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv1L = v1L;
		float localv0R = v0R;
		float localv1R = v1R;

		float f = fxParam2;
		float f4 = f * 4;//optimisation
		float pattern = (1 - 0.6f * f);

		float drive = sqrt3(fxParam1);
		float gain = (1 + 52 * (drive)) * 0.25f;
		float finalGain = (1 - (drive / (drive + 0.05f)) * 0.7f) * mixerGain;

		for (int k=BLOCK_SIZE ; k--; ) {
			//LEFT
			localv0L = pattern * localv0L - f * localv1L + f4 * fold(*sp * gain);
			localv1L = pattern * localv1L + f * localv0L;
			*sp++ = clamp(localv1L * finalGain, -ratioTimbres, ratioTimbres);

			//RIGHT
			localv0R = pattern * localv0R - f * localv1R + f4 * fold(*sp * gain);
			localv1R = pattern * localv1R + f * localv0R;
			*sp++ = clamp(localv1R * finalGain, -ratioTimbres, ratioTimbres);
		}

        v0L = localv0L;
        v0R = localv0R;
        v1L = localv1L;
        v1R = localv1R;
	}
	break;
case FILTER_WRAP:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv1L = v1L;
		float localv0R = v0R;
		float localv1R = v1R;

		float f = fxParam2;
		float pattern = (1 - 0.6f * f);

		float drive = sqrt3(fxParam1);
		float gain = (1 + 4 * (drive));
		float finalGain = (1 - sqrt3(drive) * 0.6f) * mixerGain;

		for (int k=BLOCK_SIZE ; k--; ) {
			//LEFT
			localv0L = pattern * localv0L + f * (wrap(*sp * gain) - localv1L);
			localv1L = pattern * localv1L + f * localv0L;
			*sp++ = clamp(localv1L * finalGain, -ratioTimbres, ratioTimbres);

			//RIGHT
			localv0R = pattern * localv0R + f * (wrap(*sp * gain) - localv1R);
			localv1R = pattern * localv1R + f * localv0R;
			*sp++ = clamp(localv1R * finalGain, -ratioTimbres, ratioTimbres);
		}

        v0L = localv0L;
        v0R = localv0R;
        v1L = localv1L;
        v1R = localv1R;
	}
	break;
case FILTER_XOR:
		{
			float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
			fxParamTmp *= fxParamTmp;

			// Low pass... on the Frequency
			fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

			float pos;
			float p1sq = sqrt3(fxParam1);
			if(p1sq > 0.5f) {
				pos = ((1 - panTable[127 - (int)(p1sq * 64)]) * 2) - 1;
			} else {
				pos = (panTable[(int)(p1sq * 64)] * 2) - 1;
			}

			float *sp = this->sampleBlock;
			float localv0L = v0L;
			float localv0R = v0R;

			const float a = 0.95f - fxParam2 * 0.5f;
			const float b = 1 - a;

			int digitsA, digitsB;
			const float threshold = (0.66f - p1sq * 0.66f) * numberVoicesAttn;

		for (int k=BLOCK_SIZE ; k--; ) {

				if(fabsf(*sp) > threshold) {
					localv0L = (*sp) - pos * (localv0L + pos * (*sp));
					digitsA = FLOAT2SHORT * (*sp);
					digitsB = FLOAT2SHORT * localv0L;
					localv0L = SHORT2FLOAT * roundf(digitsA ^ digitsB);
				} else {
					localv0L = *sp;
				}

				localv0L = (*sp * b) + (localv0L * a);
				*sp++ = clamp(localv0L * mixerGain, -ratioTimbres, ratioTimbres);

				if(fabsf(*sp) > threshold) {
					localv0R = (*sp) - pos * (localv0R + pos * (*sp));
					digitsA = FLOAT2SHORT * (*sp);
					digitsB = FLOAT2SHORT * localv0R;
					localv0R = SHORT2FLOAT * roundf(digitsA ^ digitsB);
				} else {
					localv0R = *sp;
				}

				localv0R = (*sp  * b) + (localv0R * a);
				*sp++ = clamp(localv0R * mixerGain, -ratioTimbres, ratioTimbres);
			}

			v0L = localv0L;
			v0R = localv0R;
	}
	break;
case FILTER_TEXTURE1:
		{
			float fxParamTmp = (params.effect.param1 + matrixFilterFrequency);
			fxParamTmp *= fxParamTmp;

			// Low pass... on the Frequency
			fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

			float *sp = this->sampleBlock;
			float lowL = v0L, highL = 0, bandL = v1L;
			float lowR = v0R, highR = 0, bandR = v1R;

			const float f = (fxParam1 * fxParam1) * 0.5f;
			const float fb = fxParam2;
			const float scale = sqrt3(fb);

			const int highBits = 0xFFFFFD4F;
			const int lowBits = ~(highBits);

			const int ll = (int)(fxParam1 * lowBits);
			int digitsL, digitsR;
			int lowDigitsL, lowDigitsR;

		for (int k=BLOCK_SIZE ; k--; ) {
				//LEFT
				lowL = lowL + f * bandL;
				highL = scale * (*sp) - lowL - fb * bandL;
				bandL = f * highL + bandL;

				digitsL = FLOAT2SHORT * bandL;
				lowDigitsL = (digitsL & lowBits);
				bandL = SHORT2FLOAT * (float)((digitsL & highBits) ^ ((lowDigitsL ^ ll) & 0x1FFF));

				*sp++ = clamp(highL * mixerGain, -ratioTimbres, ratioTimbres);

				//RIGHT
				lowR = lowR + f * bandR;
				highR = scale * (*sp) - lowR - fb * bandR;
				bandR = f * highR + bandR;

				digitsR = FLOAT2SHORT * bandR;
				lowDigitsR = (digitsR & lowBits);
				bandR = SHORT2FLOAT * (float)((digitsR & highBits) ^ ((lowDigitsR ^ ll) & 0x1FFF));

				*sp++ = clamp(highR * mixerGain, -ratioTimbres, ratioTimbres);
			}

			v0L = lowL;
			v1L = bandL;
			v0R = lowR;
			v1R = bandR;
	}
    break;
case FILTER_TEXTURE2:
    {
		float fxParamTmp = (params.effect.param1 + matrixFilterFrequency);
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		float *sp = this->sampleBlock;
		float lowL = v0L, highL = 0, bandL = v1L;
		float lowR = v0R, highR = 0, bandR = v1R;

		const float f = (fxParam1 * fxParam1) * 0.5f;
		const float fb = fxParam2;
		const float scale = sqrt3(fb);

		const int highBits = 0xFFFFFFAF;
		const int lowBits = ~(highBits);

		const int ll = fxParam1 * lowBits;
		int digitsL, digitsR;
		int lowDigitsL, lowDigitsR;

		for (int k=BLOCK_SIZE ; k--; ) {
			//LEFT
			digitsL = FLOAT2SHORT * (bandL);
			lowDigitsL = (digitsL & lowBits);
			bandL = SHORT2FLOAT * (float)((digitsL & highBits) ^ ((lowDigitsL * ll) & 0x1FFF));

			lowL = lowL + f * bandL;
			highL = scale * (*sp) - lowL - fb * bandL;
			bandL = f * highL + bandL;

			*sp++ = clamp(bandL * mixerGain, -ratioTimbres, ratioTimbres);

			//RIGHT
			digitsR = FLOAT2SHORT * (bandR);
			lowDigitsR = (digitsR & lowBits);
			bandR = SHORT2FLOAT * (float)((digitsR & highBits) ^ ((lowDigitsR * ll) & 0x1FFF));

			lowR = lowR + f * bandR;
			highR = scale * (*sp) - lowR - fb * bandR;
			bandR = f * highR + bandR;

			*sp++ = clamp(bandR * mixerGain, -ratioTimbres, ratioTimbres);
		}

		v0L = lowL;
		v1L = bandL;
		v0R = lowR;
		v1R = bandR;
    }
    break;
case FILTER_LPXOR:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float a = 1.f - fxParam1;
		const float b = 1.f - a;

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv0R = v0R;
		float digitized;

		float drive = (fxParam2 * fxParam2);
		float gain = (1 + 2 * drive);

		int digitsAL, digitsBL, digitsAR, digitsBR;
		digitsAL = digitsBL = digitsAR = digitsBR = 0;

		const uint_fast8_t random = (*(uint_fast8_t *)noise) & 0xff;
		if (random > 252) {
			fxParam1 += ((random & 1) * 0.007874015748031f);
		}

		const short bitmask = 0xfac;

		for (int k = BLOCK_SIZE; k--;) {
			// Left voice
			digitsAL = FLOAT2SHORT * fold(localv0L * gain);
			digitsBL = FLOAT2SHORT * (*sp);
			digitized = SHORT2FLOAT * (float)(digitsAL ^ (digitsBL & bitmask));
			localv0L = (*sp * b) + (digitized * a);

			*sp++ = clamp(localv0L * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			digitsAR = FLOAT2SHORT * fold(localv0R * gain);
			digitsBR = FLOAT2SHORT * (*sp);
			digitized = SHORT2FLOAT * (float)(digitsAR ^ (digitsBR & bitmask));
			localv0R = (*sp * b) + (digitized * a);

			*sp++ = clamp(localv0R * mixerGain, -ratioTimbres, ratioTimbres);
		}
    	v0L = localv0L;
    	v0R = localv0R;
	}
	break;
case FILTER_LPXOR2:
	{
		float fxParamTmp = params.effect.param1 + matrixFilterFrequency;
		fxParamTmp *= fxParamTmp;

		// Low pass... on the Frequency
		fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

		const float a = 1.f - fxParam1;
		const float b = 1.f - a;

		float *sp = this->sampleBlock;
		float localv0L = v0L;
		float localv0R = v0R;
		float digitized;

		float drive = (fxParam2 * fxParam2);
		float gain = (1 + 8 * (drive)) * 0.25f;

		int digitsAL, digitsBL, digitsAR, digitsBR;
		digitsAL = digitsBL = digitsAR = digitsBR = 0;

		const uint8_t random = (*(uint8_t *)noise) & 0xff;
		if (random > 252) {
			fxParam1 += ((random & 1) * 0.007874015748031f);
		}

		const short bitmask = 0xfba - (int)(fxParam1 * 7);

		for (int k=BLOCK_SIZE ; k--; ) {
			// Left voice
			digitsAL = FLOAT2SHORT * localv0L;
			digitsBL = FLOAT2SHORT * fold(*sp * gain);
			digitized = SHORT2FLOAT * (float)((digitsAL | (digitsBL & bitmask)));
			localv0L = (*sp * b) + (digitized * a);

			*sp++ = clamp(localv0L * mixerGain, -ratioTimbres, ratioTimbres);

			// Right voice
			digitsAR = FLOAT2SHORT * localv0R;
			digitsBR = FLOAT2SHORT * fold(*sp * gain);
			digitized = SHORT2FLOAT * (float)((digitsAR | (digitsBR & bitmask)));
			localv0R = (*sp * b) + (digitized * a);

			*sp++ = clamp(localv0R * mixerGain, -ratioTimbres, ratioTimbres);
		}
    	v0L = localv0L;
    	v0R = localv0R;
	}
	break;
case FILTER_LPSIN:
    {
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam2 * fxParam2 * SVFRANGE;
	const float fb = 0.45f;
	const float scale = sqrt3(fb);
	const int pos = (int)(fxParam1 * 2060);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;
	float drive = fxParam1 * fxParam1 * 0.25f;
	
	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * satSin(*sp, drive, pos) - lowL - fb * bandL;
		bandL = f * highL + bandL;

		*sp++ = clamp( lowL * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * satSin(*sp, drive, pos) - lowR - fb * bandR;
		bandR = f * highR + bandR;

		*sp++ = clamp( lowR * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
    }
    break;
case FILTER_HPSIN:
    {
	float fxParamTmp = SVFOFFSET + params.effect.param1 + matrixFilterFrequency;
	fxParamTmp *= fxParamTmp;
	// Low pass... on the Frequency
	fxParam1 = clamp((fxParamTmp + 9.0f * fxParam1) * .1f, 0, 1);

	const float f = fxParam2 * fxParam2 * SVFRANGE;
	const float fb = 0.94f;
	const float scale = sqrt3(fb);
	const int pos = (int)(fxParam1 * 2060);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;

	const float svfGain = (1 + SVFGAINOFFSET + fxParam2 * fxParam2 * 0.75f) * mixerGain;
	float drive = fxParam1 * fxParam1 * 0.25f;
	
	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		lowL = lowL + f * bandL;
		highL = scale * satSin(*sp, drive, pos) - lowL - fb * bandL;
		bandL = f * highL + bandL;

		*sp++ = clamp( highL * svfGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f * bandR;
		highR = scale * satSin(*sp, drive, pos) - lowR - fb * bandR;
		bandR = f * highR + bandR;

		*sp++ = clamp( highR * svfGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;
    }
    break;
case FILTER_TRIPLENOTCH:
{
	//https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
	float fxParamTmp = (params.effect.param1 + matrixFilterFrequency);
	fxParamTmp *= fxParamTmp;
	fxParam1 = ((fxParamTmp + 9.0f * fxParam1) * .1f);

	float OffsetTmp = fabsf(params.effect.param2);
	fxParam2 = ((OffsetTmp + 9.0f * fxParam2) * .1f);

	const float offset = fxParam2 * 0.88f - 0.44f;

	const float f1L = clamp((fxParam1), 0.001f, 1);
	const float f2L = clamp((fxParam1 + offset), 0.001f, 1);
	const float f3L = clamp((fxParam1 + offset * 2), 0.001f, 1);

	const float lrDelta = offset * 0.05f;
	const float f1R = clamp((f1L + lrDelta), 0.001f, 1);
	const float f2R = clamp((f2L + lrDelta), 0.001f, 1);
	const float f3R = clamp((f3L + lrDelta), 0.001f, 1);

	float *sp = this->sampleBlock;
	float lowL = v0L, highL = 0, bandL = v1L;
	float lowR = v0R, highR = 0, bandR = v1R;
	float lowL2 = v2L, highL2 = 0, bandL2 = v3L;
	float lowR2 = v2R, highR2 = 0, bandR2 = v3R;
	float lowL3 = v4L, highL3 = 0, bandL3 = v5L;
	float lowR3 = v4R, highR3 = 0, bandR3 = v5R;

	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		lowL = lowL + f1L * bandL;
		highL = (*sp) - lowL - bandL;
		bandL = f1L * highL + bandL;

		lowL2 = lowL2 + f2L * bandL2;
		highL2 = (highL + lowL) - lowL2 - bandL2;
		bandL2 = f2L * (highL2) + bandL2;

		lowL3 = lowL3 + f3L * bandL3;
		highL3 = (highL2 + lowL2) - lowL3 - bandL3;
		bandL3 = f3L * highL3 + bandL3;

		*sp++ = clamp((highL3 + lowL3) * mixerGain, -ratioTimbres, ratioTimbres);

		// Right voice
		lowR = lowR + f1R * bandR;
		highR = (*sp) - lowR - bandR;
		bandR = f1R * highR + bandR;

		lowR2 = lowR2 + f2R * bandR2;
		highR2 = (highR + lowR) - lowR2 - bandR2;
		bandR2 = f2R * (highR2) + bandR2;

		lowR3 = lowR3 + f3R * bandR3;
		highR3 = (highR2 + lowR2) - lowR3 - bandR3;
		bandR3 = f3R * highR3 + bandR3;

		*sp++ = clamp((highR3 + lowR3) * mixerGain, -ratioTimbres, ratioTimbres);
	}

	v0L = lowL;
	v1L = bandL;
	v0R = lowR;
	v1R = bandR;

	v2L = lowL2;
	v3L = bandL2;
	v2R = lowR2;
	v3R = bandR2;

	v4L = lowL3;
	v5L = bandL3;
	v4R = lowR3;
	v5R = bandR3;
}
break;
case FILTER_AP3 :
{
	//http://denniscronin.net/dsp/vst.html
	float fxParamTmp = (params.effect.param1 + matrixFilterFrequency);
	fxParamTmp *= fxParamTmp;
	fxParam1 = ((fxParamTmp + 9.0f * fxParam1) * .1f);

	float OffsetTmp = fabsf(params.effect.param2);
	fxParam2 = ((OffsetTmp + 9.0f * fxParam2) * .1f);

	const float offset = fxParam2 * 0.66f - 0.33f;

	const float f1L = clamp((fxParam1), 0.001f, 1);
	const float f2L = clamp((fxParam1 + offset), 0.001f, 1);
	const float f3L = clamp((fxParam1 + offset * 2), 0.001f, 1);
	float coef1L = (1.0f - f1L) / (1.0f + f1L);
	float coef2L = (1.0f - f2L) / (1.0f + f2L);
	float coef3L = (1.0f - f3L) / (1.0f + f3L);

	const float lrDelta = offset * 0.05f;
	const float f1R = clamp((f1L + lrDelta), 0.001f, 1);
	const float f2R = clamp((f2L + lrDelta), 0.001f, 1);
	const float f3R = clamp((f3L + lrDelta), 0.001f, 1);
	float coef1R = (1.0f - f1R) / (1.0f + f1R);
	float coef2R = (1.0f - f2R) / (1.0f + f2R);
	float coef3R = (1.0f - f3R) / (1.0f + f3R);

	float *sp = this->sampleBlock;

	float _ly1L = v0L, _ly1R = v0R;
	float _ly2L = v1L, _ly2R = v1R;
	float _ly3L = v2L, _ly3R = v2R;
	float _lx1L = v3L, _lx1R = v3R;
	float _lx2L = v4L, _lx2R = v4R;
	float _lx3L = v5L, _lx3R = v5R;

	float _feedback = 0.1f;
	float inmix;
	float finalGain = mixerGain * 0.5f;

	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		inmix = (*sp) + _feedback * _ly2L;

		_ly1L = coef1L * (_ly1L + inmix) - _lx1L; // do 1st filter
		_lx1L = inmix;
		_ly2L = coef2L * (_ly2L + _ly1L) - _lx2L; // do 2nd filter
		_lx2L = _ly1L;
		_ly3L = coef3L * (_ly3L + _ly2L) - _lx3L; // do 3rd filter
		_lx3L = _ly2L;

		*sp++ = clamp(((*sp) + _ly3L) * mixerGain, -ratioTimbres, ratioTimbres);

		// Right voice
		inmix = (*sp) + _feedback * _ly2R;

		_ly1R = coef1R * (_ly1R + inmix) - _lx1R; // do 1st filter
		_lx1R = inmix;
		_ly2R = coef2R * (_ly2R + _ly1R) - _lx2R; // do 2nd filter
		_lx2R = _ly1R;
		_ly3R = coef3R * (_ly3R + _ly2R) - _lx3R; // do 3rd filter
		_lx3R = _ly2R;

		*sp++ = clamp(((*sp) + _ly3R) * mixerGain, -ratioTimbres, ratioTimbres);
	}

	v0L = _ly1L; v0R = _ly1R;
	v1L = _ly2L; v1R = _ly2R;
	v2L = _ly3L; v2R = _ly3R;
	v3L = _lx1L; v3R = _lx1R;
	v4L = _lx2L; v4R = _lx2R;
	v5L = _lx3L; v5R = _lx3R;
}
break;
case FILTER_AP3B :
{
	float fxParamTmp = (params.effect.param1 + matrixFilterFrequency);
	fxParamTmp *= fxParamTmp;
	fxParam1 = ((fxParamTmp + 9.0f * fxParam1) * .1f);

	float OffsetTmp = fabsf(params.effect.param2);
	fxParam2 = ((OffsetTmp + 9.0f * fxParam2) * .1f);

	const float offset = fxParam2 * 0.66f - 0.33f;

	const float f1L = clamp((fxParam1), 0.001f, 1);
	const float f2L = clamp((fxParam1 + offset), 0.001f, 1);
	const float f3L = clamp((fxParam1 + offset * 2), 0.001f, 1);
	float coef1L = (1.0f - f1L) / (1.0f + f1L);
	float coef2L = (1.0f - f2L) / (1.0f + f2L);
	float coef3L = (1.0f - f3L) / (1.0f + f3L);

	const float lrDelta = offset * 0.05f;
	const float f1R = clamp((f1L + lrDelta), 0.001f, 1);
	const float f2R = clamp((f2L + lrDelta), 0.001f, 1);
	const float f3R = clamp((f3L + lrDelta), 0.001f, 1);
	float coef1R = (1.0f - f1R) / (1.0f + f1R);
	float coef2R = (1.0f - f2R) / (1.0f + f2R);
	float coef3R = (1.0f - f3R) / (1.0f + f3R);

	float *sp = this->sampleBlock;

	float _ly1L = v0L, _ly1R = v0R;
	float _ly2L = v1L, _ly2R = v1R;
	float _ly3L = v2L, _ly3R = v2R;
	float _lx1L = v3L, _lx1R = v3R;
	float _lx2L = v4L, _lx2R = v4R;
	float _lx3L = v5L, _lx3R = v5R;

	float _feedback = 0.68f;
	float inmix;
	float finalGain = mixerGain * 0.5f;

	for (int k = BLOCK_SIZE ; k--; ) {
		// Left voice
		inmix = (*sp) - _feedback * _ly2L;

		_ly1L = coef1L * (_ly1L + inmix) - _lx1L; // do 1st filter
		_lx1L = inmix;
		_ly2L = coef2L * (_ly2L + _ly1L) - _lx2L; // do 2nd filter
		_lx2L = _ly1L;
		_ly3L = coef3L * (_ly3L + _ly2L) - _lx3L; // do 3rd filter
		_lx3L = _ly2L;

		*sp++ = clamp(((*sp) + _ly3L) * mixerGain, -ratioTimbres, ratioTimbres);

		// Right voice
		inmix = (*sp) - _feedback * _ly2R;

		_ly1R = coef1R * (_ly1R + inmix) - _lx1R; // do 1st filter
		_lx1R = inmix;
		_ly2R = coef2R * (_ly2R + _ly1R) - _lx2R; // do 2nd filter
		_lx2R = _ly1R;
		_ly3R = coef3R * (_ly3R + _ly2R) - _lx3R; // do 3rd filter
		_lx3R = _ly2R;

		*sp++ = clamp(((*sp) + _ly3R) * mixerGain, -ratioTimbres, ratioTimbres);
	}

	v0L = _ly1L; v0R = _ly1R;
	v1L = _ly2L; v1R = _ly2R;
	v2L = _ly3L; v2R = _ly3R;
	v3L = _lx1L; v3R = _lx1R;
	v4L = _lx2L; v4R = _lx2R;
	v5L = _lx3L; v5R = _lx3R;
}
break;
case FILTER_OFF:
    {
    	// Filter off has gain...
    	float *sp = this->sampleBlock;
    	for (int k=0 ; k < BLOCK_SIZE ; k++) {
			*sp++ = (*sp) * mixerGain;
			*sp++ = (*sp) * mixerGain;
		}
    }
    break;
    default:
    	// NO EFFECT
   	break;
    }

}


void Timbre::afterNewParamsLoad() {
    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->afterNewParamsLoad();
    }

    for (int j=0; j<NUMBER_OF_ENCODERS * 2; j++) {
        this->env1.reloadADSR(j);
        this->env2.reloadADSR(j);
        this->env3.reloadADSR(j);
        this->env4.reloadADSR(j);
        this->env5.reloadADSR(j);
        this->env6.reloadADSR(j);
    }


    resetArpeggiator();
    v0L = v1L = v2L = v3L = v4L = v5L = v0R = v1R = v2R = v3R = v4R = v5R = 0.0f;
    for (int k=0; k<NUMBER_OF_ENCODERS; k++) {
        setNewEffecParam(k);
    }
    // Update midi note scale
    updateMidiNoteScale(0);
    updateMidiNoteScale(1);
}


void Timbre::resetArpeggiator() {
	// Reset Arpeggiator
	FlushQueue();
	note_stack.Clear();
	setArpeggiatorClock(params.engineArp1.clock);
	setLatchMode(params.engineArp2.latche);
}



void Timbre::setNewValue(int index, struct ParameterDisplay* param, float newValue) {
    if (newValue > param->maxValue) {
        // in v2, matrix target were removed so some values are > to max value but we need to accept it
        bool mustConstraint = true;
		if (param->valueNameOrder != NULL) {
			for (int v = 0; v < param->numberOfValues; v++) {
				if ((int)param->valueNameOrder[v] == (int)(newValue + .01)) {
					mustConstraint = false;
				}
			}
		}
        if (mustConstraint) {
            newValue= param->maxValue;
        }
    } else if (newValue < param->minValue) {
        newValue= param->minValue;
    }
    ((float*)&params)[index] = newValue;
}

int Timbre::getSeqStepValue(int whichStepSeq, int step) {

    if (whichStepSeq == 0) {
        return params.lfoSteps1.steps[step];
    } else {
        return params.lfoSteps2.steps[step];
    }
}

void Timbre::setSeqStepValue(int whichStepSeq, int step, int value) {

    if (whichStepSeq == 0) {
        params.lfoSteps1.steps[step] = value;
    } else {
        params.lfoSteps2.steps[step] = value;
    }
}

void Timbre::recomputeBPValues(float q, float fSquare) {
    //        /* filter coefficients */
    //        omega1  = 2 * PI * f/srate; // f is your center frequency
    //        sn1 = (float)sin(omega1);
    //        cs1 = (float)cos(omega1);
    //        alpha1 = sn1/(2*Qvalue);        // Qvalue is none other than Q!
    //        a0 = 1.0f + alpha1;     // a0
    //        b0 = alpha1;            // b0
    //        b1 = 0.0f;          // b1/b0
    //        b2= -alpha1/b0          // b2/b0
    //        a1= -2.0f * cs1/a0;     // a1/a0
    //        a2= (1.0f - alpha1)/a0;          // a2/a0
    //        k = b0/a0;

    // frequency must be up to SR / 2.... So 1024 * param1 :
    // 1000 instead of 1024 to get rid of strange border effect....

	//limit low values to avoid cracklings :
	if (fSquare < 0.1f && q < 0.15f) {
		q = 0.15f;
	}

	float sn1 = sinTable[(int)(12 + 1000 * fSquare)];
    // sin(x) = cos( PI/2 - x)
    int cosPhase = 500 - 1000 * fSquare;
    if (cosPhase < 0) {
        cosPhase += 2048;
    }
    float cs1 = sinTable[cosPhase];

    float alpha1 = sn1 * 12.5f;
    if (q > 0) {
        alpha1 = sn1 / ( 8 * q);
    }

    float A0 = 1.0f + alpha1;
    float A0Inv = 1 / A0;

    float B0 = alpha1;
    //fxParamB1 = 0.0f;
    fxParamB2 = - alpha1 * A0Inv;
    fxParamA1 = -2.0f * cs1 * A0Inv;
    fxParamA2 = (1.0f - alpha1) * A0Inv;

    fxParam1 = B0 * A0Inv;
}

void Timbre::setNewEffecParam(int encoder) {
	if (encoder == 0) {
   		v0L = v1L = v2L = v3L = v4L = v5L = v0R = v1R = v2R = v3R = v4R = v5R = 0.0f;

	    for (int k=1; k<NUMBER_OF_ENCODERS; k++) {
	        setNewEffecParam(k);
	    }
	}
	switch ((int)params.effect.type) {
    	case FILTER_BASS:
    		// Selectivity = fxParam1
    		// ratio = fxParam2
    		// gain1 = fxParam3
    		fxParam1 = 50 + 200 * params.effect.param1;
    		fxParam2 = params.effect.param2 * 4;
    		fxParam3 = 1.0f/(fxParam1 + 1.0f);
    		break;
    	case FILTER_HP:
    	case FILTER_LP:
        case FILTER_TILT:
		case FILTER_STEREO:
    		switch (encoder) {
    		case ENCODER_EFFECT_TYPE:
    			fxParam2 = 0.3f - params.effect.param2 * 0.3f;
    			break;
    		case ENCODER_EFFECT_PARAM1:
    			// Done in every loop
    			// fxParam1 = pow(0.5, (128- (params.effect.param1 * 128))   / 16.0);
    			break;
    		case ENCODER_EFFECT_PARAM2:
    	    	// fxParam2 = pow(0.5, ((params.effect.param2 * 127)+24) / 16.0);
    			// => value from 0.35 to 0.0
    			fxParam2 = 0.27f - params.effect.param2 * 0.27f;
    			break;
    		}
        	break;
        case FILTER_LP2:
        case FILTER_HP2:
		case FILTER_LPHP:
		    switch (encoder) {
    		case ENCODER_EFFECT_TYPE:
    			fxParam2 = 0.27f - params.effect.param2 * 0.267f;
    			break;
    		case ENCODER_EFFECT_PARAM2:
    			fxParam2 = 0.27f - params.effect.param2 * 0.267f;
    			break;
    		}
        	break;
        case FILTER_CRUSHER:
        {
            if (encoder == ENCODER_EFFECT_PARAM2) {
                fxParam1 = pow(2, (int)(1.0f + 15.0f * params.effect.param2));
                fxParam2 = 1 / fxParam1;
            }
            break;
        }
        case FILTER_BP:
        case FILTER_BP2:
        {
            fxParam1PlusMatrix = -1.0f;
            break;
        }
		case FILTER_SIGMOID:
		case FILTER_FOLD:
		case FILTER_WRAP:
		  	switch (encoder) {
    		case ENCODER_EFFECT_PARAM2:
				fxParam2 = 0.1f + (params.effect.param2 * params.effect.param2);
				break;
    		}
			break;
		case FILTER_TEXTURE1:
		case FILTER_TEXTURE2:
		  	switch (encoder) {
    		case ENCODER_EFFECT_PARAM2:
				fxParam2 = sqrt3(1 - params.effect.param2 * 0.99f);
				break;
    		}
			break;
		default:
		  	switch (encoder) {
    		case ENCODER_EFFECT_TYPE:
    			break;
    		case ENCODER_EFFECT_PARAM1:
    			break;
    		case ENCODER_EFFECT_PARAM2:
    			fxParam2 = params.effect.param2;
    			break;
    		}
	}
}

// Code bellowed have been adapted by Xavier Hosxe for PreenFM2
// It come from Muteable Instrument midiPAL

/////////////////////////////////////////////////////////////////////////////////
// Copyright 2011 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
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
	if (params.engineArp1.clock == CLOCK_INTERNAL) {
		if (idle_ticks_ >= 96 || !running_) {
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
	if (params.engineArp1.clock == CLOCK_EXTERNAL) {
		running_ = 1;
	}
}

void Timbre::OnMidiStart() {
	if (params.engineArp1.clock == CLOCK_EXTERNAL) {
		Start();
	}
}

void Timbre::OnMidiStop() {
	if (params.engineArp1.clock == CLOCK_EXTERNAL) {
		running_ = 0;
		FlushQueue();
	}
}


void Timbre::OnMidiClock() {
	if (params.engineArp1.clock == CLOCK_EXTERNAL && running_) {
		Tick();
	}
}


void Timbre::SendNote(uint8_t note, uint8_t velocity) {

	// If there are some Note Off messages for the note about to be triggeered
	// remove them from the queue and process them now.
	if (event_scheduler.Remove(note, 0)) {
		preenNoteOff(note);
	}

	// Send a note on and schedule a note off later.
	preenNoteOn(note, velocity);
	event_scheduler.Schedule(note, 0, midi_clock_tick_per_step[(int)params.engineArp2.duration] - 1, 0);
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
	    if (params.engineArp1.clock == CLOCK_INTERNAL) {
	      running_ = 0;
	      FlushQueue();
	    }
	}

	SendScheduledNotes();

	if (tick_ >= midi_clock_tick_per_step[(int)params.engineArp2.division]) {
		tick_ = 0;
		uint16_t pattern = getArpeggiatorPattern();
		uint8_t has_arpeggiator_note = (bitmask_ & pattern) ? 255 : 0;
		const int num_notes = note_stack.size();
		const int direction = params.engineArp1.direction;

		if (num_notes && has_arpeggiator_note) {
			if ( ARPEGGIO_DIRECTION_CHORD != direction ) {
				StepArpeggio();
				int step, transpose = 0;
				if ( current_direction_ > 0 ) {
					step = start_step_ + current_step_;
					if ( step >= num_notes ) {
						step -= num_notes;
						transpose = 12;
					}
				} else {
					step = (num_notes - 1) - (start_step_ + current_step_);
					if ( step < 0 ) {
						step += num_notes;
						transpose = -12;
					}
				}
#ifdef DEBUG_ARP_STEP
				lcd.setRealTimeAction(true);
				lcd.setCursor(16,0);
				lcd.print( current_direction_ > 0 ? '+' : '-' );
				lcd.print( step );
				lcd.setRealTimeAction(false);
#endif
				const NoteEntry &noteEntry = ARPEGGIO_DIRECTION_PLAYED == direction
					? note_stack.played_note(step)
					: note_stack.sorted_note(step);

				uint8_t note = noteEntry.note;
				uint8_t velocity = noteEntry.velocity;
				note += 12 * current_octave_;
				if ( __canTranspose( direction ) )
					 note += transpose;

				while (note > 127) {
					note -= 12;
				}

				SendNote(note, velocity);
			} else {
				for (int i = 0; i < note_stack.size(); ++i ) {
					const NoteEntry& noteEntry = note_stack.sorted_note(i);
					SendNote(noteEntry.note, noteEntry.velocity);
				}
			}
		}
		bitmask_ <<= 1;
		if (!bitmask_) {
			bitmask_ = 1;
		}
	}
}



void Timbre::StepArpeggio() {

	if (current_octave_ == 127) {
		StartArpeggio();
		return;
	}

	int direction = params.engineArp1.direction;
	uint8_t num_notes = note_stack.size();
	if (direction == ARPEGGIO_DIRECTION_RANDOM) {
		uint8_t random_byte = *(uint8_t*)noise;
		current_octave_ = random_byte & 0xf;
		current_step_ = (random_byte & 0xf0) >> 4;
		while (current_octave_ >= params.engineArp1.octave) {
			current_octave_ -= params.engineArp1.octave;
		}
		while (current_step_ >= num_notes) {
			current_step_ -= num_notes;
		}
	} else {
		// NOTE: We always count [0 - num_notes) here; the actual handling of direction is in Tick()

		uint8_t trigger_change = 0;
		if (++current_step_ >= num_notes) {
			current_step_ = 0;
			trigger_change = 1;
		}

		// special case the 'ROTATE' and 'SHIFT' modes, they might not change the octave until the cycle is through
		if (trigger_change && (direction >= ARPEGGIO_DIRECTION_ROTATE_UP ) ) {
			if ( ++start_step_ >= num_notes )
				start_step_ = 0;
			else
				trigger_change = 0;
		}

		if (trigger_change) {
			current_octave_ += current_direction_;
			if (current_octave_ >= params.engineArp1.octave || current_octave_ < 0) {
				if ( __canChangeDir(direction) ) {
					current_direction_ = -current_direction_;
					StartArpeggio();
					if (num_notes > 1 || params.engineArp1.octave > 1) {
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

	current_step_ = 0;
	start_step_ = 0;
	if (current_direction_ == 1) {
		current_octave_ = 0;
	} else {
		current_octave_ = params.engineArp1.octave - 1;
	}
}

void Timbre::Start() {
	bitmask_ = 1;
	recording_ = 0;
	running_ = 1;
	tick_ = midi_clock_tick_per_step[(int)params.engineArp2.division] - 1;
    current_octave_ = 127;
	current_direction_ = __getDirection( params.engineArp1.direction );
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

void Timbre::setDirection(uint8_t value) {
	// When changing the arpeggio direction, reset the pattern.
	current_direction_ = __getDirection(value);
	StartArpeggio();
}

void Timbre::lfoValueChange(int currentRow, int encoder, float newValue) {
    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->lfoValueChange(currentRow, encoder, newValue);
    }
}

void Timbre::updateMidiNoteScale(int scale) {

    int intBreakNote;
    int curveBefore;
    int curveAfter;
    if (scale == 0) {
        intBreakNote = params.midiNote1Curve.breakNote;
        curveBefore = params.midiNote1Curve.curveBefore;
        curveAfter = params.midiNote1Curve.curveAfter;
    } else {
        intBreakNote = params.midiNote2Curve.breakNote;
        curveBefore = params.midiNote2Curve.curveBefore;
        curveAfter = params.midiNote2Curve.curveAfter;
    }
    float floatBreakNote = intBreakNote;
    float multiplier = 1.0f;


    switch (curveBefore) {
    case MIDI_NOTE_CURVE_FLAT:
        for (int n=0; n < intBreakNote ; n++) {
            midiNoteScale[scale][timbreNumber][n] = 0;
        }
        break;
    case MIDI_NOTE_CURVE_M_LINEAR:
        multiplier = -1.0f;
        goto linearBefore;
    case MIDI_NOTE_CURVE_M_LINEAR2:
        multiplier = -8.0f;
        goto linearBefore;
    case MIDI_NOTE_CURVE_LINEAR2:
        multiplier = 8.0f;
        goto linearBefore;
    case MIDI_NOTE_CURVE_LINEAR:
        linearBefore:
        for (int n=0; n < intBreakNote ; n++) {
            float fn = (floatBreakNote - n);
            midiNoteScale[scale][timbreNumber][n] = fn * INV127 * multiplier;
        }
        break;
    case MIDI_NOTE_CURVE_M_EXP:
        multiplier = -1.0f;
    case MIDI_NOTE_CURVE_EXP:
        for (int n=0; n < intBreakNote ; n++) {
            float fn = (floatBreakNote - n);
            fn = fn * fn / floatBreakNote;
            midiNoteScale[scale][timbreNumber][n] = fn * INV16 * multiplier;
        }
        break;
    }

    // BREAK NOTE = 0;
    midiNoteScale[scale][timbreNumber][intBreakNote] = 0;


    float floatAfterBreakNote = 127 - floatBreakNote;
    int intAfterBreakNote = 127 - intBreakNote;


    switch (curveAfter) {
    case MIDI_NOTE_CURVE_FLAT:
        for (int n = intBreakNote + 1; n < 128 ; n++) {
            midiNoteScale[scale][timbreNumber][n] = 0;
        }
        break;
    case MIDI_NOTE_CURVE_M_LINEAR:
        multiplier = -1.0f;
        goto linearAfter;
    case MIDI_NOTE_CURVE_M_LINEAR2:
        multiplier = -8.0f;
        goto linearAfter;
    case MIDI_NOTE_CURVE_LINEAR2:
        multiplier = 8.0f;
        goto linearAfter;
    case MIDI_NOTE_CURVE_LINEAR:
        linearAfter:
        for (int n = intBreakNote + 1; n < 128 ; n++) {
            float fn = n - floatBreakNote;
            midiNoteScale[scale][timbreNumber][n] = fn  * INV127 * multiplier;
        }
        break;
    case MIDI_NOTE_CURVE_M_EXP:
        multiplier = -1.0f;
    case MIDI_NOTE_CURVE_EXP:
        for (int n = intBreakNote + 1; n < 128 ; n++) {
            float fn = n - floatBreakNote;
            fn = fn * fn / floatBreakNote;
            midiNoteScale[scale][timbreNumber][n] = fn * INV16 * multiplier;
        }
        break;
    }
/*
    lcd.setCursor(0,0);
    lcd.print((int)(midiNoteScale[timbreNumber][25] * 127.0f));
    lcd.print(" ");
    lcd.setCursor(10,0);
    lcd.print((int)(midiNoteScale[timbreNumber][intBreakNote - 5] * 127.0f));
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print((int)(midiNoteScale[timbreNumber][intBreakNote + 5] * 127.0f));
    lcd.print(" ");
    lcd.setCursor(10,1);
    lcd.print((int)(midiNoteScale[timbreNumber][102] * 127.0f));
    lcd.print(" ");
*/

}




void Timbre::midiClockContinue(int songPosition) {

    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->midiClockContinue(songPosition);
    }

    this->recomputeNext = ((songPosition&0x1)==0);
    OnMidiContinue();
}


void Timbre::midiClockStart() {

    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->midiClockStart();
    }

    this->recomputeNext = true;
    OnMidiStart();
}

void Timbre::midiClockSongPositionStep(int songPosition) {

    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->midiClockSongPositionStep(songPosition,  this->recomputeNext);
    }

    if ((songPosition & 0x1)==0) {
        this->recomputeNext = true;
    }
}


void Timbre::resetMatrixDestination(float oldValue) {
    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->matrix.resetDestination(oldValue);
    }
}

void Timbre::setMatrixSource(enum SourceEnum source, float newValue) {
    for (int k = 0; k < params.engine1.numberOfVoice; k++) {
        voices[voiceNumber[k]]->matrix.setSource(source, newValue);
    }
}


void Timbre::verifyLfoUsed(int encoder, float oldValue, float newValue) {
    // No need to recompute
    if (params.engine1.numberOfVoice == 0.0f) {
        return;
    }
    if (encoder == ENCODER_MATRIX_MUL && oldValue != 0.0f && newValue != 0.0f) {
        return;
    }

    for (int lfo=0; lfo < NUMBER_OF_LFO; lfo++) {
        lfoUSed[lfo] = 0;
    }

    MatrixRowParams* matrixRows = &params.matrixRowState1;


    for (int r = 0; r < MATRIX_SIZE; r++) {
        if (matrixRows[r].source >= MATRIX_SOURCE_LFO1 && matrixRows[r].source <= MATRIX_SOURCE_LFOSEQ2
                && matrixRows[r].mul != 0.0f
                && matrixRows[r].destination != 0.0f) {
            lfoUSed[(int)matrixRows[r].source - MATRIX_SOURCE_LFO1]++;
        }


		// Check if we have a Mtx* that would require LFO even if mul is 0
		// http://ixox.fr/forum/index.php?topic=69220.0
        if (matrixRows[r].destination >= MTX1_MUL && matrixRows[r].destination <= MTX4_MUL && matrixRows[r].mul != 0.0f && matrixRows[r].source != 0.0f) {
			int index = matrixRows[r].destination - MTX1_MUL;
	        if (matrixRows[index].source >= MATRIX_SOURCE_LFO1 && matrixRows[index].source <= MATRIX_SOURCE_LFOSEQ2 && matrixRows[index].destination != 0.0f) {
            	lfoUSed[(int)matrixRows[index].source - MATRIX_SOURCE_LFO1]++;
			}
		}

    }

    /*
    lcd.setCursor(11, 1);
    lcd.print('>');
    for (int lfo=0; lfo < NUMBER_OF_LFO; lfo++) {
        lcd.print((int)lfoUSed[lfo]);
    }
    lcd.print('<');
	*/

}

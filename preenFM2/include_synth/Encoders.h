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

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include "RingBuffer.h"
#include "EncodersListener.h"

#define HC165_CLOCK GPIO_Pin_8
#define HC165_DATA  GPIO_Pin_7
#define HC165_LOAD  GPIO_Pin_6

#define NUMBER_OF_ENCODERS 4
#define NUMBER_OF_BUTTONS 7

enum LastEncoderMove {
	LAST_MOVE_NONE = 0,
	LAST_MOVE_INC,
	LAST_MOVE_DEC
};


struct EncoderStatus {
	char value;
	bool b1;
};

class Encoders {
public:
	Encoders();
	~Encoders();
	void checkStatus();
	void checkSimpleStatus();
	int getRegisterBits();

	void insertListener(EncodersListener *listener) {
		if (firstListener!=0) {
			listener->nextListener = firstListener;
		}
		firstListener = listener;
	}

	void encoderTurned(int num, int ticks) {
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->encoderTurned(num, ticks);
		}
	}

	void buttonPressed(int num) {
		int cpt =0;
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->buttonPressed(num);
		}
	}

	void buttonLongPressed(int num) {
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->buttonLongPressed(num);
		}
	}

private:
	int action[16];
	int encoderBit1[NUMBER_OF_ENCODERS];
	int encoderBit2[NUMBER_OF_ENCODERS];
	int encoderState[NUMBER_OF_ENCODERS];
	int timerAction[NUMBER_OF_ENCODERS];

	LastEncoderMove lastMove[NUMBER_OF_ENCODERS];
    int tickSpeed[NUMBER_OF_ENCODERS];

	int buttonBit[NUMBER_OF_BUTTONS];
	bool buttonOldState[NUMBER_OF_BUTTONS];
	int buttonTimer[NUMBER_OF_BUTTONS];
	bool buttonLongPress[NUMBER_OF_BUTTONS];

	int encoderTimer;

	EncodersListener* firstListener;
};

#endif /* ENCODERS_H_ */

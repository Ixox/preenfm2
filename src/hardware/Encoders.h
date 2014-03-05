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

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include "RingBuffer.h"
#include "EncodersListener.h"

#define HC165_CLOCK GPIO_Pin_8
#define HC165_DATA  GPIO_Pin_7
#define HC165_LOAD  GPIO_Pin_6

#define NUMBER_OF_ENCODERS 4
#define NUMBER_OF_BUTTONS 8

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
	void checkStatus(int encoderType);
	void checkSimpleStatus();
	int getRegisterBits();

	void insertListener(EncodersListener *listener) {
		if (firstListener!=0) {
			listener->nextListener = firstListener;
		}
		firstListener = listener;
	}

	void encoderTurned(int encoder, int ticks) {
		if (firstButtonDown == -1) {
			for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
				listener->encoderTurned(encoder, ticks);
			}
		} else {
			for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
				listener->encoderTurnedWhileButtonPressed(encoder, ticks, firstButtonDown);
			}
			buttonUsedFromSomethingElse[firstButtonDown] = true;
		}
	}

	void encoderTurnedWileButtonDown(int encoder, int ticks) {
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->encoderTurned(encoder, ticks);
		}
	}


	void buttonPressed(int button) {
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->buttonPressed(button);
		}
	}

	void twoButtonsPressed(int button1, int button2) {
		for (EncodersListener* listener = firstListener; listener !=0; listener = listener->nextListener) {
			listener->twoButtonsPressed(button1, button2);
		}
	}

private:
	int action[2][16];
	int encoderBit1[NUMBER_OF_ENCODERS];
	int encoderBit2[NUMBER_OF_ENCODERS];
	int encoderState[NUMBER_OF_ENCODERS];
	int timerAction[NUMBER_OF_ENCODERS];

	LastEncoderMove lastMove[NUMBER_OF_ENCODERS];
    int tickSpeed[NUMBER_OF_ENCODERS];

	int buttonBit[NUMBER_OF_BUTTONS];
	int buttonTimer[NUMBER_OF_BUTTONS];
	bool buttonUsedFromSomethingElse[NUMBER_OF_BUTTONS];
	bool buttonPreviousState[NUMBER_OF_BUTTONS];
	int firstButtonDown;

	int encoderTimer;

	EncodersListener* firstListener;
};

#endif /* ENCODERS_H_ */

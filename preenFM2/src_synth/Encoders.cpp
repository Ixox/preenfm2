/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <dot> hosxe (at) g m a i l <dot> com)
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


#include "Encoders.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

Encoders::Encoders() {
//    char encoderPins[] = { 14,12,  13,1,   8,2,   7,3};
    char encoderPins[] = { 12,14, 1,13, 2,8, 3,7};
//    char buttonPins[] = { 4, 5, 11, 15, 10, 16, 9};
    char buttonPins[] = { 9, 16, 10, 15, 11, 5, 4};

    // GPIOG Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* Configure PB5 in output mode */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = HC165_CLOCK | HC165_LOAD;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitTypeDef GPIO_InitStructure2;
	GPIO_InitStructure2.GPIO_Pin = HC165_DATA;
	GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStructure2);


	GPIO_ResetBits(GPIOA, HC165_CLOCK);
	GPIO_ResetBits(GPIOA, HC165_LOAD);

	/*
			0: 0000 = 00 ; No change
			1: 0001 = 01 ; A 0>1, count up
			2: 0010 = 00; B 0>1, count down
			3: 0011 = 00 ; Both changed, invalid
			4: 0100 = 02 ; A 1>0, Down
			5: 0101 = 00 ; no change
			6: 0110 = 01 ; Invalid
			7: 0111 = 00 ;
			8: 1000 = 00
			9: 1001 = 00
			A: 1010 = 00
			B: 1011 = 00
			C: 1100 = 02
			D: 1101 = 00
			E: 1110 = 01
			F: 1111 = 00
	*/

    int actionToCopy[] = { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0};
    for (int i=0; i<16; i++) {
    	action[i] = actionToCopy[i];
    }

    firstListener= 0;


	for (int k=0; k<NUMBER_OF_ENCODERS; k++) {
		encoderBit1[k] = 1 << (encoderPins[k*2] -1);
		encoderBit2[k] = 1 << (encoderPins[k*2 + 1] -1);
		lastMove[k] = LAST_MOVE_NONE;
		tickSpeed[k] = 1;
	}

	for (int k=0; k<NUMBER_OF_BUTTONS; k++) {
		buttonBit[k] = 1 << (buttonPins[k] -1);
		buttonOldState[k] = false;
		buttonTimer[k] = -1;
		buttonLongPress[k] = false;
	}

	encoderTimer = 0;
}

Encoders::~Encoders() {
}


int Encoders::getRegisterBits() {
	// Copy the values in the HC165 registers
	GPIO_ResetBits(GPIOA, HC165_LOAD);
	GPIO_SetBits(GPIOA, HC165_LOAD);

	// Analyse the new value
	int registerBits = 0;
	for(int i=0; i<16; i++) {
		GPIO_ResetBits(GPIOA, HC165_CLOCK);
		registerBits |= (GPIO_ReadInputDataBit(GPIOA, HC165_DATA) << i) ;
		GPIO_SetBits(GPIOA, HC165_CLOCK);
	}
	return registerBits;
}

void Encoders::checkSimpleStatus() {
	int registerBits = getRegisterBits();

	for (int k=0; k<NUMBER_OF_BUTTONS; k++) {
		// button is pressed ?
		if (((registerBits & buttonBit[k]) == 0)) {
			buttonPressed(k);
		}
	}
}

void Encoders::checkStatus() {
	int registerBits = getRegisterBits();

	for (int k=0; k<NUMBER_OF_ENCODERS; k++) {
		bool b1 = ((registerBits & encoderBit1[k]) == 0);
		bool b2 = ((registerBits & encoderBit2[k]) == 0);

		encoderState[k] <<= 2;
		encoderState[k] &= 0xf;
		if (b1) {
			encoderState[k] |= 1;
		}
		if (b2) {
			encoderState[k] |= 2;
		}

		if (action[encoderState[k]] == 1 && lastMove[k]!=LAST_MOVE_DEC) {
			encoderTurned(k, tickSpeed[k]);
			tickSpeed[k] +=3;
			lastMove[k] = LAST_MOVE_INC;
			timerAction[k] = 60;
		} else if (action[encoderState[k]] == 2 && lastMove[k]!=LAST_MOVE_INC) {
			encoderTurned(k, -tickSpeed[k]);
			tickSpeed[k] +=3;
			lastMove[k] = LAST_MOVE_DEC;
			timerAction[k] = 60;
		} else {
			if (timerAction[k] > 1) {
				timerAction[k] --;
			} else if (timerAction[k] == 1) {
				timerAction[k] --;
				lastMove[k] = LAST_MOVE_NONE;
			}
			if (tickSpeed[k] > 1 && ((encoderTimer & 0x3) == 0)) {
				tickSpeed[k] = tickSpeed[k] - 1;
			}
		}
		if (tickSpeed[k]>10) {
			tickSpeed[k] = 10;
		}
	}

	for (int k=0; k<NUMBER_OF_BUTTONS; k++) {
		bool b1 = ((registerBits & buttonBit[k]) == 0);

		// button is pressed
		if (b1) {
			if (!buttonOldState[k]) {
				// New press
				buttonTimer[k] = 1;
				buttonLongPress[k] = false;
			} else {
				// still pressing
				if (buttonTimer[k] > 200) {
					buttonTimer[k] = 1;
					buttonLongPress[k] = true;
					buttonLongPressed(k);
				}
				if (buttonTimer[k]>0) {
					buttonTimer[k] ++;
				}
			}

		} else {
		// Button is not pressed
			if (buttonOldState[k]) {
				// Just released
				if (!buttonLongPress[k]) {
					// short press
					buttonPressed(k);
				}
				buttonTimer[k] = 0;
			}
		}

		buttonOldState[k] = b1;
	}
	encoderTimer++;
}

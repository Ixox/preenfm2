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


#include "LiquidCrystal.h"
extern LiquidCrystal lcd;


Encoders::Encoders() {
	// PreenFM PCB R4
    char encoderPins[] = { 12, 14, 1, 13, 2, 8, 3, 7};
    char buttonPins[] = { 9, 16, 10, 15, 11, 5, 4, 6};

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

    GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure2.GPIO_Pin = HC165_DATA;
	GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStructure2);


	GPIO_ResetBits(GPIOA, HC165_CLOCK);
	GPIO_ResetBits(GPIOA, HC165_LOAD);

	/*
			0: 0000 = 00
			1: 0001 = 00
			2: 0010 = 00
			3: 0011 = 00
			4: 0100 = 02
			5: 0101 = 00
			6: 0110 = 00
			7: 0111 = 01 // 1
			8: 1000 = 00
			9: 1001 = 00
			A: 1010 = 00
			B: 1011 = 02 // 2
			C: 1100 = 00
			D: 1101 = 00
			E: 1110 = 00
			F: 1111 = 00
	*/
	// PIC11 ... N24
    int actionToCopy[2][16] = {     /* N12 */{ 0, 0, 0, 0, 2, 0, 0, 1, 1, 0, 0, 2, 0, 0, 0, 0},
                                    /* N24 */ { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0}
    };

    for (int i=0; i<2; i++) {
        for (int j=0; j<16; j++) {
            action[i][j] = actionToCopy[i][j];
        }
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
		buttonPreviousState[k] = false;
		// > 30
		buttonTimer[k] = 31;
		buttonUsedFromSomethingElse[k] = false;
	}

	encoderTimer = 0;
	firstButtonDown = -1;

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
		bool b1 = ((registerBits & buttonBit[k]) == 0);
		// button is pressed
		if (b1) {
			if (buttonTimer[k] > 30) {
				buttonPressed(k);
			}
			buttonTimer[k]=0;
		}
		buttonTimer[k]++;
	}
}

void Encoders::checkStatus(int encoderType) {
	int registerBits = getRegisterBits();

	// target the right action row.
	// encoderType : First bit for type, second bit for inversed
	int *actionEnc = action[encoderType & 0x1];
	int inversedEnc = (encoderType & 0x2) == 0 ? 1 : -1;

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

		if (actionEnc[encoderState[k]] == 1 && lastMove[k]!=LAST_MOVE_DEC) {
			encoderTurned(k, tickSpeed[k] * inversedEnc);
			tickSpeed[k] += 3;
			lastMove[k] = LAST_MOVE_INC;
			timerAction[k] = 60;
		} else if (actionEnc[encoderState[k]] == 2 && lastMove[k]!=LAST_MOVE_INC) {
			encoderTurned(k, -tickSpeed[k] * inversedEnc);
			tickSpeed[k] += 3;
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
		if (tickSpeed[k] > 10) {
			tickSpeed[k] = 10;
		}
	}

	for (int k=0; k<NUMBER_OF_BUTTONS; k++) {
		bool b1 = ((registerBits & buttonBit[k]) == 0);

		// button is pressed
		if (b1) {
			buttonTimer[k]++;
			// just pressed ?
			if (!buttonPreviousState[k]) {
				if (firstButtonDown == -1) {
					firstButtonDown = k;
					buttonUsedFromSomethingElse[k] = false;
				} else {
					twoButtonsPressed(firstButtonDown, k);
					buttonUsedFromSomethingElse[firstButtonDown] = true;
					buttonUsedFromSomethingElse[k] = true;
				}
			}
		} else {
			// Just unpressed ?
			if (buttonPreviousState[k]) {
				if (firstButtonDown == k) {
					firstButtonDown = -1;
				}
				if (buttonPreviousState[k] && !buttonUsedFromSomethingElse[k]) {
					// Just released
					if (buttonTimer[k] > 15) {
						buttonPressed(k);
					}
				}
				buttonTimer[k]=0;
			}
		}

		buttonPreviousState[k] = b1;
	}
	encoderTimer++;
}

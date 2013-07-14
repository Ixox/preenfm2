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

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include <stdint.h>
#include "EncodersListener.h"

class LiquidCrystal;


typedef enum {
    BL_BOOTING = 0,
    BL_READING_FIRMWARE,
    BL_SHOWING_FIRMWARE,
    BL_BURNING_FIRMWARE,
    BL_SYSEX_INIT,
    BL_SYSEX_WAITING_FORMAT,
    BL_SYSEX_READING,
    BL_FINISHED
} BL_State;

class BootLoader : public EncodersListener {
public:
	BootLoader(LiquidCrystal* lcd);
	virtual ~BootLoader();

	void process();
	void initKey();
	void readFirmwares();


	void resetButtonPressed();
	void welcome();
	void USART_Config();
	int getButton() { return button; }

	bool formatFlash(int firmwareSize);
	bool sysexWaitFor(uint8_t byte);
	uint32_t sysexReadInt(int index);
	bool burnFlash();
	void sysexMode();

	// For EncodersListenr
	void encoderTurned(int encoder, int ticks);
	void encoderTurnedWhileButtonPressed(int encoder, int ticks, int button);
	void buttonPressed(int button);

private:
	LiquidCrystal* lcd;
	int button;
	BL_State state;
	char firmwareName[13];
	int firmwareSize;
	uint32_t checkSum, checkSumReceived;
	bool oneFirmwareAtLeast;

};

#endif /* BOOTLOADER_H_ */

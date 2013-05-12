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

#include "EncodersListener.h"

class LiquidCrystal;


typedef enum {
    BL_BOOTING = 0,
    BL_READING_FIRMWARE,
    BL_SHOWING_FIRMWARE,
    BL_BURNING_FIRMWARE,
    BL_FINISHED
} BL_State;

class BootLoader : public EncodersListener {
public:
	BootLoader(LiquidCrystal* lcd);
	virtual ~BootLoader();

	void process();
	void initKey();
	void readFirmwares();
	void encoderTurned(int num, int ticks);
	void buttonPressed(int number);
	void resetButtonPressed();
	void buttonLongPressed(int number);
	int getButton() { return button; }

	bool formatFlash(int firmwareSize);
	bool burnFlash();

private:
	LiquidCrystal* lcd;
	int button;
	BL_State state;
	char firmwareName[13];
	int firmwareSize;
	bool oneFirmwareAtLeast;
};

#endif /* BOOTLOADER_H_ */

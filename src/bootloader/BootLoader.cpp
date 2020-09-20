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

#include "BootLoader.h"
#include "usbd_storage_desc.h"
#include "LiquidCrystal.h"
#include "usbd_msc_core.h"
#include "Storage.h"
#include "usbKey_usr.h"
#include "RingBuffer.h"
#include "flash_if.h"
#include "usb_hcd_int.h"
#include "usb_dcd_int.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx_gpio.h"

/**
 * @brief  USB_OTG_BSP_uDelay
 *         This function provides delay time in micro sec
 * @param  usec : Value of delay required in micro sec
 * @retval None
 */
#define STM32_TICKS_PER_US          500
#define STM32_DELAY_US_MULT         (STM32_TICKS_PER_US/3)

#define BOOTLOADER_WORD 0x43211234

Storage             usbKey ;
USB_OTG_CORE_HANDLE     usbOTGDevice __ALIGN_END ;
extern USBD_Usr_cb_TypeDef storageUsrCallback;
LiquidCrystal lcd;

int readCpt = -1;
int writeCpt = -1;




// Dummy function ... UsbKey was written for PreenFM
void fillSoundBuffer() {}

// USB host interrupt
extern USB_OTG_CORE_HANDLE          usbOTGHost;
#ifdef __cplusplus
extern "C" {
#endif

void OTG_HS_IRQHandler(void) {
	USBH_OTG_ISR_Handler(&usbOTGHost);
}

void OTG_FS_IRQHandler(void) {
	USBD_OTG_ISR_Handler(&usbOTGDevice);
}

#ifdef __cplusplus
}
#endif


void uDelay (const uint32_t usec) {
	uint32_t us = usec * STM32_DELAY_US_MULT;

	/* fudge for function call overhead  */
	//us--;
	asm volatile("   mov r0, %[us]          \n\t"
			"1: subs r0, #1            \n\t"
			"   bhi 1b                 \n\t"
			:
			: [us] "r" (us)
			  : "r0");
}


BootLoader::BootLoader(LiquidCrystal* lcd) {
	this->lcd = lcd;
	this->nextListener = 0;
	this->button = 0;
	this->state = BL_BOOTING;
	this->oneFirmwareAtLeast = false;
	this->firmwareSize = 0;
	this->checkSum = 0;
	this->checkSumReceived = 0;

	encoders.insertListener(this);
	encoders.checkSimpleStatus();
}

BootLoader::~BootLoader() {
}


void BootLoader::initKey() {
	usbKey.init(0,0,0,0);
	this->lcd->setCursor(0, 0);
	this->lcd->print("   Flash Firmware   ");
	this->lcd->setCursor(4,2);
	this->lcd->print("           ");

	// New bootloader
	if (usbKey.getFirmwareFile()->firmwareInit() != COMMAND_SUCCESS) {
		this->lcd->setCursor(0, 1);
		this->lcd->print("No '/pfm2' directory");
		this->state = BL_FINISHED;
		return;
	}
	this->lcd->setCursor(0, 3);
	this->lcd->print("<- next     flash ->");
	this->state = BL_READING_FIRMWARE;
}


void BootLoader::process() {
	int res;
	switch (this->state) {
	case BL_READING_FIRMWARE:
		res = usbKey.getFirmwareFile()->readNextFirmwareName(firmwareName, &firmwareSize);
		if (res == COMMAND_SUCCESS) {
			this->oneFirmwareAtLeast = true;
			this->lcd->setCursor(4,1);
			this->lcd->print("                ");
			this->lcd->setCursor(4,1);
			this->lcd->print(firmwareName);
			this->lcd->setCursor(4,2);
			this->lcd->print("                ");
			this->lcd->setCursor(4,2);
			this->lcd->print(firmwareSize);
			this->lcd->print(" Bytes");
			this->state = BL_SHOWING_FIRMWARE;
		} else {
			if (this->oneFirmwareAtLeast == true) {
				// Loop on firmware list
				usbKey.getFirmwareFile()->firmwareInit();
			} else {
				// error message
				this->lcd->setCursor(1,1);
				this->lcd->print("No firmware on Key");
				this->state = BL_FINISHED;
			}
		}
		break;
	case BL_SHOWING_FIRMWARE:
		if (this->button != 0) {
			/*
            this->lcd->setCursor(2,2);
            this->lcd->print(this->button);
			 */
			if (this->button >= 1 && this->button<=5) {
				this->state = BL_READING_FIRMWARE;
			} else if (this->button >= 6 && this->button <= 7) {
				this->lcd->setCursor(0, 3);
				this->lcd->print("                    ");
				this->state = BL_BURNING_FIRMWARE;
			}
		}

		break;
	case BL_BURNING_FIRMWARE:
		FLASH_Unlock();
		if (formatFlash(this->firmwareSize)) {
			burnFlash();
		}
		this->state = BL_FINISHED;
		break;
	case BL_FINISHED:
		this->lcd->setCursor(0,3);
		this->lcd->print("    Rebooting...    ");
		uDelay(500000);
		doReboot();
		while (1);
		break;
	}
}




bool BootLoader::formatFlash(int firmwareSize) {
	uint16_t sector[6];
	sector[0] = FLASH_Sector_6;
	sector[1] = FLASH_Sector_7;
	sector[2] = FLASH_Sector_8;
	sector[3] = FLASH_Sector_9;
	sector[4] = FLASH_Sector_10;
	sector[5] = FLASH_Sector_11;

	bool cont = true;

	// USE FIRMWARE SIZE !!!!!!!!!!!!
	int number = firmwareSize / 131072 + 1;
	for (int k=0; k < number && cont; k++) {
		this->lcd->setCursor(2, 2);
		this->lcd->print("Formatting ");
		this->lcd->print(k +1);
		this->lcd->print(" / ");
		this->lcd->print(number);
		FLASH_Status erasestatus = FLASH_EraseSector(sector[k], VoltageRange_3);
		if (erasestatus != FLASH_COMPLETE) {
			this->lcd->setCursor(2, 2);
			this->lcd->print("# Format ERR  #");
			cont = false;
			this->state = BL_FINISHED;
		}
	}
	this->lcd->setCursor(2, 2);
	this->lcd->print("                  ");
	return cont;
}

bool BootLoader::burnFlash() {
	const int bufferSize = 1024;
	char buffer[bufferSize];
	bool readflag = true;
	int readIndex = 0;

	while (readflag) {
		int toRead = this->firmwareSize - readIndex;

		if (toRead > bufferSize) {
			toRead = bufferSize;
		} else {
			// Last iteration
			readflag = false;
		}

		/* DEBUG
        this->lcd->setCursor(12,2);
        this->lcd->print(toRead);
        this->lcd->print(" ");
		 */

		/* Read maximum 1024 byte from the selected file */
		if (usbKey.getFirmwareFile()->loadFirmwarePart(firmwareName, readIndex, buffer, toRead) != COMMAND_SUCCESS) {
			// Aouch
			this->lcd->setCursor(2,2);
			this->lcd->print("##ERR LOAD##");
			readflag = false;
			this->state = BL_FINISHED;
			continue;
		}

		/* Program flash memory */
		for (int pc = 0; pc < toRead; pc += 4) {
			/* Write word into flash memory */
			uint32_t data = *((uint32_t *) (buffer + pc));
			FLASH_Status status = FLASH_ProgramWord(APPLICATION_ADDRESS + readIndex + pc, data);
			if (status != FLASH_COMPLETE) {
				this->lcd->setCursor(2,2);
				this->lcd->print("##ERR FLASH##");
				readflag = false;
				this->state = BL_FINISHED;
				continue;
			}
		}
		readIndex += toRead;
		/* Update last programmed address value */
		float pos = 19.0f * readIndex / this->firmwareSize ;
		this->lcd->setCursor((int)pos,2);
		this->lcd->print('.');
	}

	return true;
}

void BootLoader::encoderTurned(int encoder, int ticks) {
}

void BootLoader::encoderTurnedWhileButtonPressed(int encoder, int ticks, int button) {
}

void BootLoader::twoButtonsPressed(int number1, int number2) {
}

void BootLoader::buttonPressed(int number) {
	this->button = number + 1;
}

void BootLoader::resetButtonPressed() {
	this->button = 0;
}


void BootLoader::welcome() {
	this->lcd->clear();
	this->lcd->setCursor(0,0);
	this->lcd->print("- bootloader  "PFM2_BOOTLOADER_VERSION" -" );
}



void switchLedInit() {
	// GPIOG Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* Configure PB5 in output mode */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}
void switchLedOn() {
	GPIO_SetBits(GPIOB, GPIO_Pin_6);
}

void switchLedOff() {
	GPIO_ResetBits(GPIOB, GPIO_Pin_6);
}

int BootLoader::doMainMenu()
{
	waitForButtonRelease();
	welcome();
	lcd->setCursor(0,1);
	lcd->print("Eng:USB access");
	lcd->setCursor(0,2);
	lcd->print("Op:Flash   Mtx:DFU");
	lcd->setCursor(0,3);
	lcd->print("Menu:Reboot");

	return waitForButton();
}

int BootLoader::waitForButton()
{
	while ( !getButton() ) {
		resetButtonPressed();
		encoders.checkSimpleStatus();
	}

	int button = getButton();
	resetButtonPressed();
	return button;
}

int BootLoader::checkButtons()
{
	encoders.checkSimpleStatus();
	return getButton();
}

void BootLoader::waitForButtonRelease()
{
	while ( getButton() ) {
		resetButtonPressed();
		encoders.checkSimpleStatus();
	}
}

int main(void) {
	uint32_t magicRam = 0x2001BFF0;

	switchLedInit();
	switchLedOn();

	lcd.begin(20,4);
	BootLoader bootLoader(&lcd);

	if (bootLoader.getButton() == 0 && (*(__IO uint32_t*)magicRam != BOOTLOADER_WORD)) {
		// App ready ?
		pFunction Jump_To_Application;
		uint32_t JumpAddress;

		// Stack can be on RAM or CCRAM
		if (((*(__IO uint32_t*) APPLICATION_ADDRESS) & 0x3FFC0000) == 0x20000000
				|| ((*(__IO uint32_t*) APPLICATION_ADDRESS) & 0x3FFE0000) == 0x10000000) {
			switchLedOff();
			/* Jump to user application */
			JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
			Jump_To_Application = (pFunction) JumpAddress;
			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
			Jump_To_Application();
		} else {
			lcd.begin(20,4);
			lcd.clear();
			lcd.setCursor(1, 0);
			lcd.print("Bootloader OK but");
			lcd.setCursor(1, 1);
			lcd.print("No PreenFM Firmware");
			int cpt = 0;
			while (1) {
				cpt++;
				if (cpt == 1000000) {
					switchLedOff();
				} else if (cpt == 2000000) {
					switchLedOn();
					cpt = 0;
				}
			}
		}
	}

	*(__IO uint32_t*)magicRam = 0;
	while ( true ) {
		switch( bootLoader.doMainMenu() ) {
		case 1: // ENG
			bootLoader.doUSBStorage();
			*(__IO uint32_t*)magicRam = BOOTLOADER_WORD;
			// Instead of cleaning up, just restart from main menu
			NVIC_SystemReset();
			while(1);
			break;
		case 2: // Op
			bootLoader.doUSBUpgrade();
			break;
		case 4: // Mtx
			bootLoader.doDFU();
			break;
		case 7: // Menu
			bootLoader.doReboot();
			break;
		default:
			break;
		}
	}

}

void BootLoader::doReboot() {
	waitForButtonRelease();
	int blinkCpt = 10;
	int cpt = 0;
	while (blinkCpt) {
		cpt++;
		if (cpt == 1000000) {
			switchLedOff();
		} else if (cpt == 2000000) {
			switchLedOn();
			cpt = 0;
			blinkCpt--;
		}
	}

	switchLedOff();
	uDelay(100000);
	__set_MSP(*(__IO uint32_t*) 0x08000000);
	NVIC_SystemReset();
	while (1);
}

void BootLoader::doUSBStorage()
{
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print("  Access USB Stick  ");

	// Init state
	uDelay(1000000);
	usbKey.init(0,0,0,0);
	USBD_Init(&usbOTGDevice, USB_OTG_FS_CORE_ID, &USR_storage_desc, &USBD_MSC_cb, &storageUsrCallback);

	lcd->setCursor(0,2);
	lcd->print("Unmount before exit" );
	lcd->setCursor(3,3);
	lcd->print("<-   Exit   ->" );
	encoders.checkSimpleStatus();
	waitForButtonRelease();
	while (0 == getButton() ) {
		resetButtonPressed();
		encoders.checkSimpleStatus();

		if (readCpt >=0) {
			lcd->setCursor(9,1);
			if (readCpt >= 9998) {
				lcd->print('R');
			} else if (readCpt == 0) {
				lcd->print(' ');
			}
			readCpt --;
		}
		if (writeCpt >=0) {
			lcd->setCursor(10,1);
			if (writeCpt >= 9998) {
				lcd->print('W');
			} else if (writeCpt == 0) {
				lcd->print(' ');
			}
			writeCpt --;
		}
	}
	encoders.checkSimpleStatus();
	waitForButtonRelease();

	// Nuke if from orbit, its the only way to be sure...
	// USBD_DeInit(&usbOTGDevice);	// seems to be a NOP
	// usbOTGDevice.regs.DREGS->DCTL |= 0x02;
}

void BootLoader::doUSBUpgrade()
{
	welcome();
	lcd->setCursor(1,2);
	lcd->print("Reading USB drive");
	uDelay(1000000);
	lcd->setCursor(1,2);
	lcd->print("                    ");

	// Init state
	initKey();

	while (1) {
		resetButtonPressed();
		encoders.checkStatus(0);
		process();
		USB_OTG_BSP_uDelay(1000);
	}
}


void BootLoader::doDFU()
{
	welcome();
	lcd->setCursor(1,2);
	lcd->print("!STM32F4 USB DFU!");

	uint32_t magicRam = 0x2001BFF0;
	*(__IO uint32_t*)magicRam = 0x12344321;
	pFunction jumpToBootloader  = (pFunction)*(__IO uint32_t*) 0x08000004;
	RCC_DeInit();
	__set_MSP(*(__IO uint32_t*) 0x08000000);
	jumpToBootloader();
}

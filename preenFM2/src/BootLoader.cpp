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
#include "LiquidCrystal.h"
#include "Encoders.h"
#include "UsbKey.h"
#include "usbKey_usr.h"
#include "flash_if.h"
#include "usb_hcd_int.h"

/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
#define STM32_TICKS_PER_US          500
#define STM32_DELAY_US_MULT         (STM32_TICKS_PER_US/3)


UsbKey             usbKey ;

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
}

BootLoader::~BootLoader() {
}


void BootLoader::initKey() {
    usbKey.init(0,0,0,0);
    this->lcd->setCursor(0, 0);
    this->lcd->print("   Flash Firmware   ");
    if (usbKey.firmwareInit() != COMMAND_SUCCESS) {
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
        res = usbKey.readNextFirmwareName(firmwareName, &firmwareSize);
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
                usbKey.firmwareInit();
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
        this->lcd->setCursor(3,3);
        this->lcd->print("Please Reboot ");
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
        if (usbKey.loadFirmwarePart(firmwareName, readIndex, buffer, toRead) != COMMAND_SUCCESS) {
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
    this->lcd->setCursor(0, 2);
    this->lcd->print(" New Firmware Ready ");

    return true;
}

void BootLoader::encoderTurned(int num, int ticks) {
}

void BootLoader::buttonPressed(int number) {
    this->button = number + 1;
}

void BootLoader::resetButtonPressed() {
    this->button = 0;
}

void BootLoader::buttonLongPressed(int number) {
}

// Symbol usefull in other modules... so must be global
LiquidCrystal lcd;

int main(void) {
    unsigned int encoderCpt = 0;


    BootLoader bootLoader(&lcd);
    Encoders encoders;

    encoders.insertListener(&bootLoader);
    encoders.checkSimpleStatus();
/*
    lcd.setCursor(17, 0);
    lcd.print("B:");
    lcd.print(bootLoader.getButton());
*/
    if (bootLoader.getButton() == 2) {
        // Button... flash new firmware
        lcd.begin(20,4);
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("PreenFM bootloader");
        lcd.setCursor(1,1);
        lcd.print("      v1.00");
        uDelay(1000000);
        bootLoader.initKey();

        while (1) {
            bootLoader.resetButtonPressed();
            encoders.checkStatus(0);
            bootLoader.process();
            USB_OTG_BSP_uDelay(10000);
        }
    } else if (bootLoader.getButton() == 4) {
        lcd.begin(20,4);
        lcd.clear();
        FLASH_Unlock();
        bootLoader.formatFlash(500000);
        lcd.setCursor(1, 0);
        lcd.print("PreenFM Firmware");
        lcd.setCursor(1, 1);
        lcd.print("Formatted...");
    }
    else {
        // App ready ?
        pFunction Jump_To_Application;
        uint32_t JumpAddress;

	// Stack can be on RAM or CCRAM
        if (((*(__IO uint32_t*) APPLICATION_ADDRESS) & 0x3FFC0000) == 0x20000000
		|| ((*(__IO uint32_t*) APPLICATION_ADDRESS) & 0x3FFE0000) == 0x10000000) {
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
        }
    }

    while (1)
        ;
}

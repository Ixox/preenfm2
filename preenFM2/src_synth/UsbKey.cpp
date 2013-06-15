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

#include "UsbKey.h"
#include "usbKey_usr.h"

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;



extern USB_OTG_CORE_HANDLE          usbOTGHost;
extern USBH_HOST                    usbHost;

// define in PreenFM.c
void fillSoundBuffer();

void UsbKey::init(uint8_t*timbre1, uint8_t*timbre2, uint8_t*timbre3, uint8_t*timbre4) {
    Storage::init(timbre1, timbre2, timbre3, timbre4);
    USBH_Init(&usbOTGHost, USB_OTG_HS_CORE_ID, &usbHost, &USBH_MSC_cb, &USR_Callbacks);
    commandParams.commandState = COMMAND_INIT;
    usbProcess();
}

void UsbKey::usbProcess() {
    commandParams.commandResult = COMMAND_FAILED;
    while (commandParams.commandState != COMMAND_NONE) {
        fillSoundBuffer();
        USBH_Process(&usbOTGHost, &usbHost);
    }
    for (int k=0; k<10; k++) {
        USBH_Process(&usbOTGHost, &usbHost);
    }
}

const char* UsbKey::getFileName(FILE_ENUM file) {
    switch (file) {
        case DEFAULT_COMBO:
            return DEFAULT_COMBO_NAME;
        case PATCH_BANK1:
            return PATCH_BANK1_NAME;
        case PATCH_BANK2:
            return PATCH_BANK2_NAME;
        case PATCH_BANK3:
            return PATCH_BANK3_NAME;
        case PATCH_BANK4:
            return PATCH_BANK4_NAME;
        case COMBO_BANK:
            return COMBO_BANK_NAME;
        case PROPERTIES:
            return PROPERTIES_NAME;
    }
}


int UsbKey::save(FILE_ENUM file, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_SAVE;
    commandParams.commandFileName = getFileName(file);
    commandParams.commandParam1 = bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}



int UsbKey::load(FILE_ENUM file, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_LOAD;
    commandParams.commandFileName = getFileName(file);
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}

int UsbKey::remove(FILE_ENUM file) {
    commandParams.commandState = COMMAND_DELETE;
    commandParams.commandFileName = getFileName(file);
    usbProcess();
    return commandParams.commandResult;
}


int UsbKey::checkSize(FILE_ENUM file) {
    commandParams.commandState = COMMAND_EXISTS;
    commandParams.commandFileName = getFileName(file);
    usbProcess();
    return commandParams.commandResult;
}


bool UsbKey::isFirmwareFile(char *name)  {
    if (name[0] != 'p' && name[0] != 'P') return false;
    if (name[1] != 'f' && name[1] != 'F') return false;
    if (name[2] != 'm' && name[2] != 'M') return false;

    int pointPos = -1;
    for (int k=3; k<10 && pointPos == -1; k++) {
        if (name[k] == '.') {
            pointPos = k;
        }
    }
    if (pointPos == -1) return false;
    if (name[pointPos+1] != 'b' && name[pointPos+1] != 'B') return false;
    if (name[pointPos+2] != 'i' && name[pointPos+2] != 'I') return false;
    if (name[pointPos+3] != 'n' && name[pointPos+3] != 'N') return false;

    return true;
}


int UsbKey::firmwareInit() {
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = FIRMWARE_DIR;
    usbProcess();
    return commandParams.commandResult;
}

int UsbKey::readNextFirmwareName(char *name, int *size) {
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)name;
        commandParams.commandParam2 = (void*)size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isFirmwareFile(name));
    return commandParams.commandResult;
}

int UsbKey::strlen(const char *string) {
    int k;
    for (k=0; k<1000 && string[k] != 0; k++);
    return k;
}

void UsbKey::getFullFirmwareName(char* fullName, char* fileName) {
    int pos = 0;
    for (int k =0; k < strlen(FIRMWARE_DIR); k++) {
        fullName[pos++] = FIRMWARE_DIR[k];
    }
    fullName[pos++] = '/';

    for (int k = 0; k < strlen(fileName) ; k++) {
        fullName[pos++] = fileName[k];
    }
    fullName[pos] = 0;
}

int UsbKey::loadFirmwarePart(char *fileName, int seek, void* bytes, int size) {
    char fullName[32];
    commandParams.commandState = COMMAND_LOAD;

    getFullFirmwareName(fullName, fileName);

    commandParams.commandFileName = fullName;

    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}


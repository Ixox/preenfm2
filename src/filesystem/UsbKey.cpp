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

// define in PreenFM.c
void fillSoundBuffer();

void UsbKey::init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4) {
    Storage::init(timbre1, timbre2, timbre3, timbre4);
    USBH_Init(&usbOTGHost, USB_OTG_HS_CORE_ID, &usbHost, &USBH_MSC_cb, &USR_Callbacks);
    commandParams.commandState = COMMAND_INIT;
    usbProcess();
}



#ifndef BOOTLOADER

int UsbKey::dx7Init() {
	int res;
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = DX7_DIR;
    dx7BankInitialized = true;
    usbProcess();
    if (commandParams.commandResult != COMMAND_SUCCESS) {
    	return commandParams.commandResult;
    }
    int k;
    for (k = 0; k<NUMBEROFDX7BANKS; k++) {
    	res = dx7ReadNextFileName(&dx7Bank[k]);
    	if (res != COMMAND_SUCCESS) {
    		break;
    	}
    }
	dx7NumberOfBanks = k ;
	sortBankFile(dx7Bank, dx7NumberOfBanks);
    return res;
}


int UsbKey::dx7ReadNextFileName(struct PFM2File* bank) {
	unsigned long size;
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)bank->name;
        commandParams.commandParam2 = (void*)&size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isDX7SysexFile((char*)bank->name, size));
	bank->fileType = FILE_OK;
    return commandParams.commandResult;
}

const struct PFM2File* UsbKey::getDx7Bank(int bankNumber) {
	if (!dx7BankInitialized) {
		dx7Init();
	}
	if (bankNumber < 0 || bankNumber >= dx7NumberOfBanks) {
		return &errorDX7Bank;
	}
	return &dx7Bank[bankNumber];
}

const struct PFM2File* UsbKey::getPreenFMBank(int bankNumber) {
	if (!preenFMBankInitialized) {
		preenFMBankInit();
	}
	if (bankNumber < 0 || bankNumber >= preenFMNumberOfBanks) {
		return &errorPreenFMBank;
	}
	return &preenFMBank[bankNumber];
}


int UsbKey::preenFMBankInit() {
	int res;
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = PREENFM_DIR;
    preenFMBankInitialized = true;
    usbProcess();
    if (commandParams.commandResult != COMMAND_SUCCESS) {
    	return commandParams.commandResult;
    }
    int k;
    for (k=0; k<NUMBEROFPREENFMBANKS; k++) {
    	res = preenFMBankReadNextFileName(&preenFMBank[k]);
    	if (res != COMMAND_SUCCESS) {
    		break;
    	}
    }
	preenFMNumberOfBanks = k;
	sortBankFile(preenFMBank, preenFMNumberOfBanks);
    return res;
}


int UsbKey::preenFMBankReadNextFileName(struct PFM2File* bank) {
	unsigned long size;
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)bank->name;
        commandParams.commandParam2 = (void*)&size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isPreenFMBankFile((char*)bank->name, size));
    if (bank->name[0] == '_') {
    	bank->fileType = FILE_READ_ONLY;
    } else {
    	bank->fileType = FILE_OK;
    }
    return commandParams.commandResult;
}


const struct PFM2File* UsbKey::getPreenFMCombo(int comboNumber) {
	if (!preenFMComboInitialized) {
		preenFMComboInit();
	}
	if (comboNumber < 0 || comboNumber >= preenFMNumberOfCombos) {
		return &errorPreenFMCombo;
	}
	return &preenFMCombo[comboNumber];
}


int UsbKey::preenFMComboInit() {
	int res;
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = PREENFM_DIR;
    preenFMComboInitialized = true;
    usbProcess();
    if (commandParams.commandResult != COMMAND_SUCCESS) {
    	return commandParams.commandResult;
    }
    int k;
    for (k=0; k<NUMBEROFPREENFMCOMBOS; k++) {
    	res = preenFMComboReadNextFileName(&preenFMCombo[k]);
    	if (res != COMMAND_SUCCESS) {
    		break;
    	}
    }
	preenFMNumberOfCombos = k;
	sortBankFile(preenFMCombo, preenFMNumberOfCombos);
    return res;
}


int UsbKey::preenFMComboReadNextFileName(struct PFM2File* combo) {
	unsigned long size;
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)combo->name;
        commandParams.commandParam2 = (void*)&size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isPreenFMComboFile((char*)combo->name, size));
    if (combo->name[0] == '_') {
    	combo->fileType = FILE_READ_ONLY;
    } else {
    	combo->fileType = FILE_OK;
    }
    return commandParams.commandResult;
}






// scala scale loading




#endif




#ifdef BOOTLOADER


int UsbKey::firmwareInit() {
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = FIRMWARE_DIR;
    usbProcess();
    return commandParams.commandResult;
}

bool UsbKey::isFirmwareFile(char *name)  {
    if (name[0] != 'p' && name[0] != 'P') return false;
    if (name[1] != '2') return false;

    int pointPos = -1;
    for (int k=10; k>2 && pointPos == -1; k--) {
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


int UsbKey::readNextFirmwareName(char *name, int *size) {
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)name;
        commandParams.commandParam2 = (void*)size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isFirmwareFile(name));
    return commandParams.commandResult;
}



#endif

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
#ifndef BOOTLOADER
    dx7NumberOfBanks = 0;
    dx7BankInitialized = false;
    for (int k=0; k< NUMBEROFDX7BANKS; k++) {
    	dx7Bank[k].name[0] = '\0';
    	dx7Bank[k].name[12] = '\0';
    	dx7Bank[k].fileType = FILE_EMPTY;
    }
    for (int k=0; k<NUMBEROFPREENFMBANKS; k++) {
    	preenFMBank[k].fileType = FILE_EMPTY;
    }
    for (int k=0; k<NUMBEROFPREENFMCOMBOS; k++) {
    	preenFMCombo[k].fileType = FILE_EMPTY;
    }
    char empty[] = "<Empty>\0";
    for (int k=0; k< 8; k++) {
    	errorDX7Bank.name[k] = empty[k];
    	errorPreenFMBank.name[k] = empty[k];
    	errorPreenFMCombo.name[k] = empty[k];
    }
    errorPreenFMBank.fileType = FILE_EMPTY;
    errorPreenFMCombo.fileType = FILE_EMPTY;
    errorDX7Bank.fileType = FILE_EMPTY;
#endif
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
        case PROPERTIES:
            return PROPERTIES_NAME;
    }
}




int UsbKey::load(FILE_ENUM file, int seek, void* bytes, int size) {
    return load(getFileName(file), seek, bytes, size);
}

int UsbKey::load(const char* fileName, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_LOAD;
    commandParams.commandFileName = fileName;
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
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


int UsbKey::save(const char* fileName, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_SAVE;
    commandParams.commandFileName = fileName;
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




int UsbKey::strlen(const char *string) {
    int k;
    for (k=0; k<1000 && string[k] != 0; k++);
    return k;
}

const char* UsbKey::getFullName(const char* pathName, const char* fileName) {
    int pos = 0;
    int cpt = 0;
    for (int k =0; k < strlen(pathName) && cpt++<24; k++) {
        this->fullName[pos++] = pathName[k];
    }
    this->fullName[pos++] = '/';
    cpt = 0;
    for (int k = 0; k < strlen(fileName) && cpt++<14 ; k++) {
    	this->fullName[pos++] = fileName[k];
    }
    this->fullName[pos] = 0;
    return this->fullName;
}


#ifndef BOOTLOADER
bool UsbKey::isDX7SysexFile(char *name, int size)  {
	// DX7 Dump sysex size is 4104
	if (size != 4104) {
		return false;
	}

	int pointPos = -1;
    for (int k=1; k<9 && pointPos == -1; k++) {
        if (name[k] == '.') {
            pointPos = k;
        }
    }
    if (pointPos == -1) return false;
    if (name[pointPos+1] != 's' && name[pointPos+1] != 'S') return false;
    if (name[pointPos+2] != 'y' && name[pointPos+2] != 'Y') return false;
    if (name[pointPos+3] != 'x' && name[pointPos+3] != 'X') return false;

    return true;
}

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


int UsbKey::dx7ReadNextFileName(struct BankFile* bank) {
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

const struct BankFile* UsbKey::getDx7Bank(int bankNumber) {
	if (!dx7BankInitialized) {
		dx7Init();
	}
	if (bankNumber < 0 || bankNumber >= dx7NumberOfBanks) {
		return &errorDX7Bank;
	}
	return &dx7Bank[bankNumber];
}

const struct BankFile* UsbKey::getPreenFMBank(int bankNumber) {
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


int UsbKey::preenFMBankReadNextFileName(struct BankFile* bank) {
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

bool UsbKey::isPreenFMBankFile(char *name, int size)  {
	if (size != 131072) {
		return false;
	}
	int pointPos = -1;
    for (int k=1; k<9 && pointPos == -1; k++) {
        if (name[k] == '.') {
            pointPos = k;
        }
    }
    if (pointPos == -1) return false;
    if (name[pointPos+1] != 'b' && name[pointPos+1] != 'B') return false;
    if (name[pointPos+2] != 'n' && name[pointPos+2] != 'N') return false;
    if (name[pointPos+3] != 'k' && name[pointPos+3] != 'K') return false;

    return true;
}

const struct BankFile* UsbKey::getPreenFMCombo(int comboNumber) {
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


int UsbKey::preenFMComboReadNextFileName(struct BankFile* combo) {
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

bool UsbKey::isPreenFMComboFile(char *name, int size)  {
	if (size != 525536) {
		return false;
	}

	int pointPos = -1;
    for (int k=1; k<9 && pointPos == -1; k++) {
        if (name[k] == '.') {
            pointPos = k;
        }
    }
    if (pointPos == -1) return false;
    if (name[pointPos+1] != 'c' && name[pointPos+1] != 'C') return false;
    if (name[pointPos+2] != 'm' && name[pointPos+2] != 'M') return false;
    if (name[pointPos+3] != 'b' && name[pointPos+3] != 'B') return false;

    return true;
}

const char* UsbKey::getDX7BankFullName(const char* bankName) {
	return getFullName(DX7_DIR, bankName);
}

const char* UsbKey::getPreenFMFullName(const char* bankName) {
	return getFullName(PREENFM_DIR, bankName);
}

const struct BankFile* UsbKey::addEmptyBank(const char* newBankName) {
	int k;
	for (k=0; preenFMBank[k].fileType != FILE_EMPTY && k < NUMBEROFPREENFMBANKS; k++);
	if (k == NUMBEROFPREENFMBANKS) {
		// NO EMPTY BANK....
		return NULL;
	}
	preenFMBank[k].fileType = FILE_OK;
	for (int n = 0; n < 12 ; n++) {
		preenFMBank[k].name[n] = newBankName[n];
	}
	preenFMBankInitialized = false;
	return &preenFMBank[k];
}

const struct BankFile* UsbKey::addEmptyCombo(const char* newComboName) {
	int k;
	for (k=0; preenFMCombo[k].fileType != FILE_EMPTY && k < NUMBEROFPREENFMCOMBOS; k++);
	if (k == NUMBEROFPREENFMCOMBOS) {
		// NO EMPTY COMBO....
		return NULL;
	}
	preenFMCombo[k].fileType = FILE_OK;
	for (int n = 0; n < 12 ; n++) {
		preenFMCombo[k].name[n] = newComboName[n];
	}
	preenFMComboInitialized = false;
	return &preenFMCombo[k];
}


int UsbKey::renameBank(const struct BankFile* bank, const char* newName) {
	preenFMBankInitialized = false;
	return renameFile(bank, newName);
}
int UsbKey::renameCombo(const struct BankFile* bank, const char* newName) {
	preenFMComboInitialized = false;
	return renameFile(bank, newName);
}


int UsbKey::renameFile(const struct BankFile* bank, const char* newName) {
	char fullNewBankName[40];
	const char* fullNameTmp = getPreenFMFullName(newName);
	// Don't want the logical drive (two first char)
	for (int k=2; k<40; k++) {
		fullNewBankName[k-2] = fullNameTmp[k];
	}
	commandParams.commandState = COMMAND_RENAME;
	commandParams.commandFileName = getPreenFMFullName(bank->name);
	commandParams.commandParam1 = (void*)fullNewBankName;
	usbProcess();
	return commandParams.commandResult;
}

void UsbKey::swapBankFile(struct BankFile* bankFiles, int i, int j) {
	if (i == j) {
		return;
	}
	struct BankFile tmp = bankFiles[i];
	bankFiles[i] = bankFiles[j];
	bankFiles[j] = tmp;
}

void UsbKey::sortBankFile(struct BankFile* bankFiles, int numberOfFiles) {
	for (int i=0 ; i < numberOfFiles - 1; i++) {
		int minBank = i;
		for (int j = i + 1; j < numberOfFiles; j++) {
			if (strcmp(bankFiles[minBank].name, bankFiles[j].name) > 0 ) {
				minBank = j;
			}
		}
		swapBankFile(bankFiles, i, minBank);
	}
}
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


int UsbKey::loadFirmwarePart(char *fileName, int seek, void* bytes, int size) {
    char fullName[32];
    commandParams.commandState = COMMAND_LOAD;
    commandParams.commandFileName = getFullName(FIRMWARE_DIR, fileName);
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}

unsigned int UsbKey::diskioGetSectorNumber() {
	unsigned long size;
	commandParams.commandState = DISKIO_GETSECTORNUMBER;
	commandParams.commandParam1 = &size;
	usbProcess();
	return size;
}


int UsbKey::diskioRead(uint8_t* buff, int address, int length) {
	commandParams.commandState = DISKIO_READ;
	commandParams.commandParam1 = buff;
	commandParams.commandParam2 = &address;
	commandParams.commandParamSize = length;
	usbProcess();
	return commandParams.commandResult;
}

int UsbKey::diskioWrite(uint8_t* buff, int address, int length) {
	commandParams.commandState = DISKIO_WRITE;
	commandParams.commandParam1 = buff;
	commandParams.commandParam2 = &address;
	commandParams.commandParamSize = length;
	usbProcess();
	return commandParams.commandResult;
}

#endif

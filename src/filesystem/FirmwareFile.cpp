/*
 * FirmwareFile.cpp
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#include "FirmwareFile.h"

FirmwareFile::FirmwareFile() {
	// TODO Auto-generated constructor stub

}

FirmwareFile::~FirmwareFile() {
	// TODO Auto-generated destructor stub
}

const char* FirmwareFile::getFolderName() {
	return FIRMWARE_DIR;
}

bool FirmwareFile::isCorrectFile(char *name, int size)  {
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

int FirmwareFile::loadFirmwarePart(char *fileName, int seek, void* bytes, int size) {
    char fullName[32];
    commandParams.commandState = COMMAND_LOAD;
    commandParams.commandFileName = getFullName(fileName);
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}

uint32_t FirmwareFile::diskioGetSectorNumber() {
	uint32_t size;
	commandParams.commandState = DISKIO_GETSECTORNUMBER;
	commandParams.commandParam1 = &size;
	usbProcess();
	return size;
}


int FirmwareFile::diskioRead(uint8_t* buff, uint32_t address, uint16_t length) {
	commandParams.commandState = DISKIO_READ;
	commandParams.commandParam1 = buff;
	commandParams.commandParam2 = (void*)address;
	commandParams.commandParamSize = length;
	usbProcess();
	return commandParams.commandResult;
}

int FirmwareFile::diskioWrite(uint8_t* buff, uint32_t address, uint16_t length) {
	commandParams.commandState = DISKIO_WRITE;
	commandParams.commandParam1 = buff;
	commandParams.commandParam2 = (void*)address;
	commandParams.commandParamSize = length;
	usbProcess();
	return commandParams.commandResult;
}

int FirmwareFile::firmwareInit() {
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = FIRMWARE_DIR;
    usbProcess();
    return commandParams.commandResult;
}


int FirmwareFile::readNextFirmwareName(char *name, int *size) {
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)name;
        commandParams.commandParam2 = (void*)size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isFirmwareFile(name));
    return commandParams.commandResult;
}

bool FirmwareFile::isFirmwareFile(char *name)  {
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
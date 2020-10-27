/*
 * FirmwareFile.h
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#ifndef FIRMWAREFILE_H_
#define FIRMWAREFILE_H_

#include "PreenFMFileType.h"

class FirmwareFile: public PreenFMFileType {
public:
	FirmwareFile();
	virtual ~FirmwareFile();

	int loadFirmwarePart(char *fileName, int seek, void* bytes, int size);
	uint32_t diskioGetSectorNumber();
	int diskioRead(uint8_t* buff, uint32_t address, uint16_t length);
	int diskioWrite(uint8_t* buff, uint32_t address, uint16_t length);
	int firmwareInit();
	int readNextFirmwareName(char *name, int *size);
	bool isFirmwareFile(char *name);

protected:
	const char* getFolderName();
	bool isCorrectFile(char *name, int size);

};

#endif /* FIRMWAREFILE_H_ */

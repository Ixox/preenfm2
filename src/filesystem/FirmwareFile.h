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
	unsigned int diskioGetSectorNumber();
	int diskioRead(uint8_t* buff, int address, int length);
	int diskioWrite(uint8_t* buff, int address, int length);

protected:
	const char* getFolderName();
	bool isCorrectFile(char *name, int size);

};

#endif /* FIRMWAREFILE_H_ */

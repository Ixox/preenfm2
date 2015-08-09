/*
 * DX7SysexFile.h
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#ifndef DX7SYSEXFILE_H_
#define DX7SYSEXFILE_H_

#include "PreenFMFileType.h"

class DX7SysexFile: public PreenFMFileType {
public:
	DX7SysexFile();
	virtual ~DX7SysexFile();

	uint8_t* dx7LoadPatch(const struct PFM2File* bank, int patchNumber);

protected:
	const char* getFolderName();
	bool isCorrectFile(char *name, int size);
	struct PFM2File dx7Bank[NUMBEROFDX7BANKS];

};

#endif /* DX7SYSEXFILE_H_ */

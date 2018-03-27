/*
 * PatchBank.h
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#ifndef PATCHBANK_H_
#define PATCHBANK_H_

#include "PreenFMFileType.h"
#include "SysexSender.h"

class PatchBank : public PreenFMFileType {
public:
	PatchBank();
	virtual ~PatchBank();

	void create(const char* name);
	const struct PFM2File* addEmptyBank(const char* newBankName);
	void savePreenFMPatch(const struct PFM2File* bank, int patchNumber, const struct OneSynthParams *params);
    void setArpeggiatorPartOfThePreset(short *pointer) { arpeggiatorPartOfThePreset = pointer; }
    void loadPreenFMPatch(const struct PFM2File* bank, int patchNumber, struct OneSynthParams *params);
    const char* loadPreenFMPatchName(const struct PFM2File* bank, int patchNumber);
    void sendPreenFMPatchAsSysex(const struct OneSynthParams *params);

	void decodeBufferAndApplyPreset(uint8_t* buffer, struct OneSynthParams *params);
    void setSysexSender(SysexSender* sysexSender) { this->sysexSender = sysexSender; }
#ifdef DEBUG
    void testMemoryPreset();
#endif

protected:
	const char* getFolderName();
	bool isCorrectFile(char *name, int size);

private:
    short *arpeggiatorPartOfThePreset;
    char presetName[13];
    SysexSender* sysexSender;
    struct PFM2File preenFMBank[NUMBEROFPREENFMBANKS];

};

#endif /* PATCHBANK_H_ */

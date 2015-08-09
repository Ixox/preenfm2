/*
 * PatchBank.cpp
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#include "PatchBank.h"


PatchBank::PatchBank() {
	numberOfFilesMax = NUMBEROFPREENFMBANKS;
	myFiles = preenFMBank;
}

PatchBank::~PatchBank() {
	// TODO Auto-generated destructor stub
}

const char* PatchBank::getFolderName() {
	return PREENFM_DIR;
}

bool PatchBank::isCorrectFile(char *name, int size)  {
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


void PatchBank::create(const char* name) {
	const struct PFM2File * newBank = addEmptyBank(name);
	const char* fullBankName = getFullName(name);
	if (newBank == 0) {
		return;
	}
    // back up name
    fsu->copy(reachableFlashParam.presetName, "Preset 0\0\0\0\0\0", 12);
	for (int k=0; k<128; k++) {
		fsu->addNumber(reachableFlashParam.presetName, 7, k + 1);
		savePreenFMPatch(newBank, k, &preenMainPreset);
	}

}

const struct PFM2File* PatchBank::addEmptyBank(const char* newBankName) {
	int k;
	for (k=0; myFiles[k].fileType != FILE_EMPTY && k < NUMBEROFPREENFMBANKS; k++);
	if (k == NUMBEROFPREENFMBANKS) {
		// NO EMPTY BANK....
		return NULL;
	}
	myFiles[k].fileType = FILE_OK;
	for (int n = 0; n < 12 ; n++) {
		myFiles[k].name[n] = newBankName[n];
	}
	isInitialized = false;
	return &myFiles[k];
}


void PatchBank::loadPreenFMPatch(const struct PFM2File* bank, int patchNumber, struct OneSynthParams *params) {
	const char* fullBankName = getFullName(bank->name);

    // Don't load in params directly because params is in CCM memory
    int result = load(fullBankName, patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

    if (result == 0) {
    	convertMemoryToParams(&reachableFlashParam, params, *arpeggiatorPartOfThePreset > 0);
    }
}

const char* PatchBank::loadPreenFMPatchName(const struct PFM2File* bank, int patchNumber) {
	// Get name position
	int namePosition = (int)(((unsigned int) reachableFlashParam.presetName) - (unsigned int)&reachableFlashParam);
	const char* fullBankName = getFullName(bank->name);
    int result = load(fullBankName, ALIGNED_PATCH_SIZE * patchNumber + namePosition,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void PatchBank::savePreenFMPatch(const struct PFM2File* bank, int patchNumber, const struct OneSynthParams *params) {
	const char* fullBankName = getFullName(bank->name);

    char zeros[ALIGNED_PATCH_ZERO];
    for (int k=0; k<ALIGNED_PATCH_ZERO;k++) {
        zeros[k] = 0;
    }
	convertParamsToMemory(params, &reachableFlashParam, *arpeggiatorPartOfThePreset > 0);

    // Save patch
    save(fullBankName, patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

    // Add zeros
    save(fullBankName, patchNumber * ALIGNED_PATCH_SIZE  + PFM_PATCH_SIZE,  (void*)zeros, ALIGNED_PATCH_ZERO);
}


void PatchBank::sendPreenFMPatchAsSysex(const struct OneSynthParams *params) {
	// Use Flash memory model which is not supposed to change
	convertParamsToMemory(params, &reachableFlashParam, true);

	int sysexSize = sizeof(FlashSynthParams);

    sysexSender->sendSysexByte(0xf0);
    sysexSender->sendSysexByte(0x7d);
    sysexSender->sendSysexByte(SYSEX_NEW_PFM2_BYTE_PATCH);

    uint32_t checksum = 0;
    uint32_t rest = 0;
    uint8_t  restBits = 0;

    for (int index=0; index < sizeof(FlashSynthParams); index++) {
    	uint32_t newPart = ((uint8_t*)&reachableFlashParam)[index];
    	rest = rest | (newPart << restBits);

        sysexSender->sendSysexByte(rest & 0x7f);
        checksum += rest & 0x7f;
    	rest >>= 7;
    	restBits ++;
    	if (restBits == 7) {
            sysexSender->sendSysexByte(rest);
            rest = 0;
            restBits = 0;
    	}
    }
    if (restBits > 0) {
        sysexSender->sendSysexByte(rest);
    }

    sysexSender->sendSysexByte((uint8_t) (checksum % 128));
    sysexSender->sendSysexByte(0xf7);
    sysexSender->sendSysexFinished();
}

void PatchBank::decodeBufferAndApplyPreset(uint8_t* buffer, struct OneSynthParams *params) {
	if (buffer[0] != 0x7d || buffer[1] != SYSEX_NEW_PFM2_BYTE_PATCH) {
		return;
	}

	int indexBuffer = 2;
	int indexParam = 0;
	char* decodeParams = (char*)&reachableFlashParam;
    uint8_t restBits = 0;
    uint32_t rest = 0;
	while (buffer[indexBuffer] != 0xf7 && indexParam <= sizeof(struct FlashSynthParams)) {
		rest = rest | (buffer[indexBuffer++] << restBits);
		restBits += 7;
		if (restBits >= 8) {
			decodeParams[indexParam++] = rest & 0xff;
			rest >>= 8;
			restBits -= 8;
		}
	}

	// Apply to param with arpegiator
	convertMemoryToParams(&reachableFlashParam, params, true);
}



#ifdef DEBUG

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

extern unsigned int preenTimer;
struct OneSynthParams reachableParam;


void PatchBank::testMemoryPreset() {
	lcd.setRealTimeAction(true);
	struct OneSynthParams tmpParam;
	unsigned char* test = (unsigned char*)&tmpParam;
	for (int k=0; k< sizeof(struct OneSynthParams); k++) {
		test[k] = ((k + preenTimer) % 200) + 34;
	}

	tmpParam.engineArp1.BPM = (preenTimer % 200) + 15;
	convertParamsToMemory(&tmpParam, &reachableFlashParam, true);
	convertMemoryToParams(&reachableFlashParam, &reachableParam, true);

	fsu->copyFloat((float*)&tmpParam.performance1, (float*)&reachableParam.performance1, 4);

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print((int)sizeof(struct OneSynthParams));
	lcd.setCursor(10,0);
	lcd.print((int)sizeof(struct FlashSynthParams));

	for (int k=0; k< sizeof(struct OneSynthParams); k++) {
		unsigned char c1 = ((unsigned char*)&reachableParam)[k];
		unsigned char c2 = test[k];
		lcd.setCursor(0,1);
		lcd.print(k);
		lcd.print(":");
		lcd.print((int)c1);
		lcd.print("=");
		lcd.print((int)c2);
		lcd.print("?    ");
		if (c1 != c2) {
			lcd.setCursor(2,8);
			lcd.print("NO !!(");
			lcd.print((int)((unsigned char*)&reachableFlashParam)[k]);
			lcd.print(")");
			break;
		}
	}
}
#endif



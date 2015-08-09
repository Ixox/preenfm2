/*
 * ComboBank.cpp
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#include "ComboBank.h"

ComboBank::ComboBank() {
	numberOfFilesMax = NUMBEROFPREENFMCOMBOS;
	myFiles = preenFMCombo;
    // numberOfFilesMax has been initialized in the concrete class
    for (int k = 0; k < numberOfFilesMax; k++) {
    	myFiles[k].fileType = FILE_EMPTY;
    }
}

ComboBank::~ComboBank() {
	// TODO Auto-generated destructor stub
}

void ComboBank::init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4) {
	this->timbre[0] = timbre1;
	this->timbre[1] = timbre2;
	this->timbre[2] = timbre3;
	this->timbre[3] = timbre4;
}

const char* ComboBank::getFolderName() {
	return PREENFM_DIR;
}

bool ComboBank::isCorrectFile(char *name, int size)  {
	if (size != 525536 && size != 525632) {
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



void ComboBank::removeDefaultCombo() {
    remove(DEFAULT_COMBO);
}

void ComboBank::saveDefaultCombo() {
    // Default : no name
    // delete to be sure the size is OK...
    remove(DEFAULT_COMBO);
    for (int timbre = 0; timbre < 4; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam, true);
    	save(DEFAULT_COMBO, ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
    }
}

bool ComboBank::loadDefaultCombo() {
    // default : non ame

    if (checkSize(DEFAULT_COMBO) != (ALIGNED_PATCH_SIZE*3+PFM_PATCH_SIZE)) {
        return false;
    }

    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(DEFAULT_COMBO,  ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

        if (result == 0) {
        	convertMemoryToParams(&reachableFlashParam, this->timbre[timbre], true);
        }
    }
    return true;
}




void ComboBank::createComboBank(const char* name) {

	const struct PFM2File * newBank = addEmptyCombo(name);
	const char* fullBankName = getFullName(name);

	if (newBank == 0) {
		return;
	}

	convertParamsToMemory(&preenMainPreset, &reachableFlashParam, true);

	char comboName[12];
    fsu->copy(comboName,  "Combo \0\0\0\0\0\0", 12);
    fsu->copy(reachableFlashParam.presetName, "Preset 0\0\0\0\0\0", 12);
	for (int comboNumber=0; comboNumber<128; comboNumber++) {

        fsu->addNumber(comboName, 6, comboNumber + 1);
		save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber , comboName, 12);

        for (int timbre = 0; timbre < 4; timbre++)  {
        	fsu->addNumber(reachableFlashParam.presetName, 7, timbre + 1);
            save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
        }
	}
}


void ComboBank::loadPreenFMCombo(const struct PFM2File* combo, int comboNumber) {
	const char* fullBankName = getFullName(combo->name);
    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(fullBankName,  (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

        if (result == 0) {
        	convertMemoryToParams(&reachableFlashParam, this->timbre[timbre], true);
        }
    }
}

const char* ComboBank::loadPreenFMComboName(const struct PFM2File* combo, int comboNumber) {
	const char* fullBankName = getFullName(combo->name);
    int result = load(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void ComboBank::savePreenFMCombo(const struct PFM2File* combo, int comboNumber, char* comboName) {
	const char* fullBankName = getFullName(combo->name);
    save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)comboName, 12);

    for (int timbre = 0; timbre < 4; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam, true);
        save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
    }
}


const struct PFM2File* ComboBank::addEmptyCombo(const char* newComboName) {
	int k;
	for (k=0; myFiles[k].fileType != FILE_EMPTY && k < NUMBEROFPREENFMCOMBOS; k++);
	if (k == NUMBEROFPREENFMCOMBOS) {
		// NO EMPTY COMBO....
		return NULL;
	}
	myFiles[k].fileType = FILE_OK;
	for (int n = 0; n < 12 ; n++) {
		myFiles[k].name[n] = newComboName[n];
	}
	isInitialized = false;
	return &myFiles[k];
}



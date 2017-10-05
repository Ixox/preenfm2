/*
 * ComboBank.cpp
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#include "ComboBank.h"


extern char file_zeros[ALIGNED_PATCH_ZERO];


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

void ComboBank::init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4,
        struct OneSynthParams*timbre5, struct OneSynthParams*timbre6, struct OneSynthParams*timbre7, struct OneSynthParams*timbre8) {
	this->timbre[0] = timbre1;
	this->timbre[1] = timbre2;
	this->timbre[2] = timbre3;
	this->timbre[3] = timbre4;
    this->timbre[4] = timbre5;
    this->timbre[5] = timbre6;
    this->timbre[6] = timbre7;
    this->timbre[7] = timbre8;
}

const char* ComboBank::getFolderName() {
	return PREENFM_DIR;
}

bool ComboBank::isCorrectFile(char *name, int size)  {

    if (size != ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES) {
        // return false;
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
    if (name[pointPos+3] != '8') return false;

    return true;
}



void ComboBank::removeDefaultCombo() {
    remove(DEFAULT_COMBO);
}

void ComboBank::saveDefaultCombo() {
    // Default : no name
    // delete to be sure the size is OK...
    remove(DEFAULT_COMBO);
    for (int timbre = 0; timbre < NUMBER_OF_TIMBRES; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam, true);
    	save(DEFAULT_COMBO, ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
        save(DEFAULT_COMBO, ALIGNED_PATCH_SIZE * timbre  + PFM_PATCH_SIZE,  (void*)file_zeros, ALIGNED_PATCH_ZERO);
    }
}

bool ComboBank::loadDefaultCombo() {
    // default : non ame

    if (checkSize(DEFAULT_COMBO) < 0 ) {
        return false;
    }

    for (int timbre = 0; timbre < NUMBER_OF_TIMBRES; timbre++)  {
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
		save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber , comboName, 12);

        for (int timbre = 0; timbre < NUMBER_OF_TIMBRES; timbre++)  {
        	fsu->addNumber(reachableFlashParam.presetName, 7, timbre + 1);
            save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
            save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre + PFM_PATCH_SIZE,  (void*)file_zeros, ALIGNED_PATCH_ZERO);
        }
	}
}


void ComboBank::loadPreenFMCombo(const struct PFM2File* combo, int comboNumber) {
	const char* fullBankName = getFullName(combo->name);
    for (int timbre = 0; timbre < NUMBER_OF_TIMBRES; timbre++)  {
        int result = load(fullBankName,  (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

        if (result == 0) {
        	convertMemoryToParams(&reachableFlashParam, this->timbre[timbre], true);
        }
    }
}

const char* ComboBank::loadPreenFMComboName(const struct PFM2File* combo, int comboNumber) {
	const char* fullBankName = getFullName(combo->name);
    int result = load(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber ,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void ComboBank::savePreenFMCombo(const struct PFM2File* combo, int comboNumber, char* comboName) {
	const char* fullBankName = getFullName(combo->name);
    save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber ,  (void*)comboName, 12);

    for (int timbre = 0; timbre < NUMBER_OF_TIMBRES; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam, true);
        save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
        save(fullBankName, (ALIGNED_PATCH_SIZE * NUMBER_OF_TIMBRES + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre + PFM_PATCH_SIZE,  (void*)file_zeros, ALIGNED_PATCH_ZERO);
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



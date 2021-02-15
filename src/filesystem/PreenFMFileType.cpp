/*
 * PreenFMFileType.cpp
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#include "PreenFMFileType.h"

// Set param in memmory reachable with USB : static is OK
struct FlashSynthParams reachableFlashParam;

char propertyFile [PROPERTY_FILE_SIZE];
extern void displayFileError(char* msg);

PreenFMFileType::PreenFMFileType() {
	isInitialized = false;
    numberOfFiles = 0;
	// init error file
    char empty[] = "<Empty>\0";
    for (int k=0; k< 8; k++) {
    	errorFile.name[k] = empty[k];
    }
    errorFile.fileType = FILE_EMPTY;
}

PreenFMFileType::~PreenFMFileType() {
	// TODO Auto-generated destructor stub
}

const char* PreenFMFileType::getFileName(FILE_ENUM file) {
    switch (file) {
        case DEFAULT_COMBO:
            return DEFAULT_COMBO_NAME;
        case PROPERTIES:
            return PROPERTIES_NAME;
        case SCALA_CONFIG:
            return SCALA_CONFIG_NAME;
    }
}


const char* PreenFMFileType::getFullName(const char* fileName) {
	const char* pathName = getFolderName();
    int pos = 0;
    int cpt = 0;
    for (int k =0; k < fsu->strlen(pathName) && cpt++<24; k++) {
        this->fullName[pos++] = pathName[k];
    }
    this->fullName[pos++] = '/';
    cpt = 0;
    for (int k = 0; k < fsu->strlen(fileName) && cpt++<14 ; k++) {
    	this->fullName[pos++] = fileName[k];
    }
    this->fullName[pos] = 0;
    return this->fullName;
}



// == USB Comand

int PreenFMFileType::remove(FILE_ENUM file) {
    commandParams.commandState = COMMAND_DELETE;
    commandParams.commandFileName = getFileName(file);
    usbProcess();
    return commandParams.commandResult;
}



int PreenFMFileType::load(FILE_ENUM file, int seek, void* bytes, int size) {
    return load(getFileName(file), seek, bytes, size);
}

int PreenFMFileType::load(const char* fileName, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_LOAD;
    commandParams.commandFileName = fileName;
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}


int PreenFMFileType::save(FILE_ENUM file, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_SAVE;
    commandParams.commandFileName = getFileName(file);
    commandParams.commandParam1 = bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}


int PreenFMFileType::save(const char* fileName, int seek, void* bytes, int size) {
    commandParams.commandState = COMMAND_SAVE;
    commandParams.commandFileName = fileName;
    commandParams.commandParam1 = (void*)bytes;
    commandParams.commandParamSize = size;
    commandParams.commandSeek = seek;
    usbProcess();
    return commandParams.commandResult;
}




int PreenFMFileType::checkSize(FILE_ENUM file) {
    return checkSize(getFileName(file));
}

int PreenFMFileType::checkSize(const char* fileName) {
    commandParams.commandState = COMMAND_EXISTS;
    commandParams.commandFileName = fileName;
    usbProcess();
    return commandParams.commandResult;
}


void PreenFMFileType::closeDir() {
    commandParams.commandState = COMMAND_CLOSE_DIR;
    usbProcess();
}

void PreenFMFileType::usbProcess() {
    commandParams.commandResult = COMMAND_FAILED;
    int trys = 1000000;
    while (commandParams.commandState != COMMAND_NONE && (trys--) >0) {
        USBH_Process(&usbOTGHost, &usbHost);
    }
    for (int k=0; k<10; k++) {
        USBH_Process(&usbOTGHost, &usbHost);
    }
    if (trys == 0) {
        char *msg = "0 : No response";
        const char* hex = "0123456789ABCDEFGHIJKL";
        msg[0] = '0' + hex[commandParams.commandState];
        displayFileError(msg);
    }
}

bool PreenFMFileType::sendInitCommand() {
    commandParams.commandResult = COMMAND_FAILED;
   	commandParams.commandState = COMMAND_INIT;
    int trys = 500000;
    while (commandParams.commandState != COMMAND_NONE && (trys--) > 0) {
        USBH_Process(&usbOTGHost, &usbHost);
    }
    for (int k = 0; k < 10; k++) {
        USBH_Process(&usbOTGHost, &usbHost);
    }
    return (commandParams.commandState == COMMAND_NONE);
}

// Init bank

int PreenFMFileType::initFiles() {
    for (int k = 0; k < numberOfFilesMax; k++) {
    	myFiles[k].fileType = FILE_EMPTY;
    }

	int res;
    commandParams.commandState = COMMAND_OPEN_DIR;
    commandParams.commandFileName = getFolderName();
    usbProcess();
    if (commandParams.commandResult != COMMAND_SUCCESS) {
    	return commandParams.commandResult;
    }
    int k;
    for (k = 0; k<numberOfFilesMax; k++) {
    	res = readNextFile(&myFiles[k]);
    	if (res != COMMAND_SUCCESS) {
    		break;
    	}
    }

    closeDir();

	numberOfFiles = k ;
	sortFiles(myFiles, numberOfFiles);

    isInitialized = true;

    return res;
}


int PreenFMFileType::readNextFile(struct PFM2File* bank) {
	unsigned long size;
    do {
        commandParams.commandState = COMMAND_NEXT_FILE_NAME;
        commandParams.commandParam1 = (void*)bank->name;
        commandParams.commandParam2 = (void*)&size;
        usbProcess();
    }  while (commandParams.commandResult == COMMAND_SUCCESS && !isCorrectFile((char*)bank->name, size));
    if (bank->name[0] == '_') {
    	bank->fileType = FILE_READ_ONLY;
    } else {
    	bank->fileType = FILE_OK;
    }
    return commandParams.commandResult;
}

const struct PFM2File* PreenFMFileType::getFile(int fileNumber) {
	if (!isInitialized) {
		initFiles();
	}
	if (fileNumber < 0 || fileNumber >= numberOfFiles) {
		return &errorFile;
	}
	return &myFiles[fileNumber];
}

int PreenFMFileType::getFileIndex(const char* name) {
	if (!isInitialized) {
		initFiles();
	}
	for (int k=0; k< numberOfFiles; k++) {
		if (fsu->str_cmp(name, myFiles[k].name) == 0) {
			return k;
		}
	}
	return -1;
}

int PreenFMFileType::getFileIndex(const struct PFM2File* file) {
	if (!isInitialized) {
		initFiles();
	}
	for (int k=0; k< numberOfFiles; k++) {
		if (&myFiles[k] == file) {
			return k;
		}
	}
	return -1;
}


int PreenFMFileType::renameFile(const struct PFM2File* bank, const char* newName) {
	isInitialized = false;
	char fullNewBankName[40];
	const char* fullNameTmp = getFullName(newName);
	// Don't want the logical drive (two first char)
	for (int k=2; k<40; k++) {
		fullNewBankName[k-2] = fullNameTmp[k];
	}
	commandParams.commandState = COMMAND_RENAME;
	commandParams.commandFileName = getFullName(bank->name);
	commandParams.commandParam1 = (void*)fullNewBankName;
	usbProcess();
	return commandParams.commandResult;
}

void PreenFMFileType::swapFiles(struct PFM2File* bankFiles, int i, int j) {
	if (i == j) {
		return;
	}
	struct PFM2File tmp = bankFiles[i];
	bankFiles[i] = bankFiles[j];
	bankFiles[j] = tmp;
}

void PreenFMFileType::sortFiles(struct PFM2File* bankFiles, int numberOfFiles) {
	for (int i=0 ; i < numberOfFiles - 1; i++) {
		int minBank = i;
		for (int j = i + 1; j < numberOfFiles; j++) {
			if (strcmp(bankFiles[minBank].name, bankFiles[j].name) > 0 ) {
				minBank = j;
			}
		}
		swapFiles(bankFiles, i, minBank);
	}
}


void PreenFMFileType::convertParamsToMemory(const struct OneSynthParams* params, struct FlashSynthParams* memory, bool saveArp) {
	// First engine line

	fsu->copyFloat((float*)&params->engine1, (float*)&memory->engine1, 4);
	fsu->copyFloat((float*)&params->engine2, (float*)&memory->engine2, 4);
	
	if (saveArp) {
		fsu->copyFloat((float*)&params->engineArp1, (float*)&memory->engineArp1, 4 * 2);
		memory->engineArpUserPatterns = params->engineArpUserPatterns;
	} else {
		memory->engineArp1.clock = 0;
		memory->engineArp1.BPM = 90;
		memory->engineArp1.octave = 1;
		memory->engineArp2.pattern = 2;
		memory->engineArp2.division = 12;
		memory->engineArp2.duration = 14;
		memory->engineArp2.latche = 0;
		for ( int p = 0; p< ARRAY_SIZE(memory->engineArpUserPatterns.patterns); ++p )
		  memory->engineArpUserPatterns.patterns[ p ] = 0;
	}

	memory->flashEngineIm1.modulationIndex1 = params->engineIm1.modulationIndex1;
	memory->flashEngineIm1.modulationIndex2 = params->engineIm1.modulationIndex2;
	memory->flashEngineIm1.modulationIndex3 = params->engineIm2.modulationIndex3;
	memory->flashEngineIm1.modulationIndex4 = params->engineIm2.modulationIndex4;
	memory->flashEngineIm2.modulationIndex5 = params->engineIm3.modulationIndex5;
	memory->flashEngineIm2.modulationIndex6 = params->engineIm3.modulationIndex6;
	memory->flashEngineIm2.notUsed1 = 0.0f;
	memory->flashEngineIm2.notUsed2 = 0.0f;

	memory->flashEngineVeloIm1.modulationIndexVelo1 = params->engineIm1.modulationIndexVelo1;
	memory->flashEngineVeloIm1.modulationIndexVelo2 = params->engineIm1.modulationIndexVelo2;
	memory->flashEngineVeloIm1.modulationIndexVelo3 = params->engineIm2.modulationIndexVelo3;
	memory->flashEngineVeloIm1.modulationIndexVelo4 = params->engineIm2.modulationIndexVelo4;
	memory->flashEngineVeloIm2.modulationIndexVelo5 = params->engineIm3.modulationIndexVelo5;
	memory->flashEngineVeloIm2.modulationIndexVelo6 = params->engineIm3.modulationIndexVelo6;
	memory->flashEngineVeloIm2.notUsed1 = 0.0f;
	memory->flashEngineVeloIm2.notUsed1 = 0.0f;

	fsu->copyFloat((float*)&params->engineMix1,(float*)&memory->engineMix1 , 4 * 3);
	fsu->copyFloat((float*)&params->effect,(float*)&memory->effect , 4);
	fsu->copyFloat((float*)&params->osc1,(float*)&memory->osc1 , 4 * 6);
	fsu->copyFloat((float*)&params->env1a, (float*)&memory->env1a, 4 * 6 * 2);
	fsu->copyFloat((float*)&params->matrixRowState1, (float*)&memory->matrixRowState1, 4 * 12);
	fsu->copyFloat((float*)&params->lfoOsc1, (float*)&memory->lfoOsc1, 4 * 3);
	fsu->copyFloat((float*)&params->lfoEnv1, (float*)&memory->lfoEnv1, 4);
	fsu->copyFloat((float*)&params->lfoEnv2, (float*)&memory->lfoEnv2, 4);
	fsu->copyFloat((float*)&params->lfoSeq1, (float*)&memory->lfoSeq1, 4 * 2);

    fsu->copyFloat((float*)&params->midiNote1Curve, (float*)&memory->midiNote1Curve, 4);
    fsu->copyFloat((float*)&params->midiNote2Curve, (float*)&memory->midiNote2Curve, 4);
    fsu->copyFloat((float*)&params->lfoPhases, (float*)&memory->lfoPhases, 4);

    for (int s=0; s<16; s++) {
		memory->lfoSteps1.steps[s] = params->lfoSteps1.steps[s];
		memory->lfoSteps2.steps[s] = params->lfoSteps2.steps[s];
	}
	for (int s=0; s<13; s++) {
		memory->presetName[s] = params->presetName[s];
	}
}


void PreenFMFileType::convertMemoryToParams(const struct FlashSynthParams* memory, struct OneSynthParams* params, bool loadArp) {
	// First engine line
	fsu->copyFloat((float*)&memory->engine1, (float*)&params->engine1, 4);

	fsu->copyFloat((float*)&memory->engine2, (float*)&params->engine2, 4);
	if (params->engine2.pfmVersion == 0.0f) {
		params->engine2.playMode = 1.0f;
		params->engine2.unisonDetune = .12f;
		params->engine2.unisonSpread = .5f;
	} else if (params->engine2.pfmVersion == 1.0f) {
		// preenfm3 ?
		if (params->engine2.playMode == 0.0f) { // Pfm3 mono
			params->engine1.numberOfVoice = 1; // One voice
			params->engine2.playMode = 1.0f; // poly set to (but not used)
		} else {
			params->engine1.numberOfVoice = 3;
			params->engine2.playMode = params->engine1.numberOfVoice;
		}
	}
	// Set version to 2
	params->engine2.pfmVersion = 2.0f;

	if (loadArp) {
		fsu->copyFloat((float*)&memory->engineArp1, (float*)&params->engineArp1, 4 * 2);
		params->engineArpUserPatterns = memory->engineArpUserPatterns;
	}

	params->engineIm1.modulationIndex1 = memory->flashEngineIm1.modulationIndex1;
	params->engineIm1.modulationIndex2 = memory->flashEngineIm1.modulationIndex2;
	params->engineIm2.modulationIndex3 = memory->flashEngineIm1.modulationIndex3;
	params->engineIm2.modulationIndex4 = memory->flashEngineIm1.modulationIndex4;
	params->engineIm3.modulationIndex5 = memory->flashEngineIm2.modulationIndex5;
	params->engineIm3.modulationIndex6 = memory->flashEngineIm2.modulationIndex6;
	params->engineIm1.modulationIndexVelo1 = memory->flashEngineVeloIm1.modulationIndexVelo1;
	params->engineIm1.modulationIndexVelo2 = memory->flashEngineVeloIm1.modulationIndexVelo2;
	params->engineIm2.modulationIndexVelo3 = memory->flashEngineVeloIm1.modulationIndexVelo3;
	params->engineIm2.modulationIndexVelo4 = memory->flashEngineVeloIm1.modulationIndexVelo4;
	params->engineIm3.modulationIndexVelo5 = memory->flashEngineVeloIm2.modulationIndexVelo5;
	params->engineIm3.modulationIndexVelo6 = memory->flashEngineVeloIm2.modulationIndexVelo6;

	fsu->copyFloat((float*)&memory->engineMix1,(float*)&params->engineMix1 , 4 * 3);
	fsu->copyFloat((float*)&memory->effect,(float*)&params->effect , 4);
	fsu->copyFloat((float*)&memory->osc1,(float*)&params->osc1 , 4 * 6);
	fsu->copyFloat((float*)&memory->env1a, (float*)&params->env1a, 4 * 6 * 2);
	fsu->copyFloat((float*)&memory->matrixRowState1, (float*)&params->matrixRowState1, 4 * 12);
	fsu->copyFloat((float*)&memory->lfoOsc1, (float*)&params->lfoOsc1, 4 * 3);
	fsu->copyFloat((float*)&memory->lfoEnv1, (float*)&params->lfoEnv1, 4);
	fsu->copyFloat((float*)&memory->lfoEnv2, (float*)&params->lfoEnv2, 4);
	fsu->copyFloat((float*)&memory->lfoSeq1, (float*)&params->lfoSeq1, 4 * 2);

	fsu->copyFloat((float*)&memory->lfoPhases, (float*)&params->lfoPhases, 4);
    fsu->copyFloat((float*)&memory->midiNote1Curve, (float*)&params->midiNote1Curve, 4);
    fsu->copyFloat((float*)&memory->midiNote2Curve, (float*)&params->midiNote2Curve, 4);

	for (int s=0; s<16; s++) {
		params->lfoSteps1.steps[s] = memory->lfoSteps1.steps[s];
		params->lfoSteps2.steps[s] = memory->lfoSteps2.steps[s];
	}
	for (int s=0; s<13; s++) {
		params->presetName[s] = memory->presetName[s];
	}

	params->performance1.perf1 = 0.0f;
	params->performance1.perf2 = 0.0f;
	params->performance1.perf3 = 0.0f;
	params->performance1.perf4 = 0.0f;

	// Initialized not initialize params in memory
    if (params->engineArp1.BPM < 10) {
		params->engineArp1.clock = 0;
    	params->engineArp1.BPM = 90;
    	params->engineArp1.octave = 1;
    	params->engineArp2.pattern = 2;
    	params->engineArp2.division = 12;
    	params->engineArp2.duration = 14;
    	params->engineArp2.latche = 0;
    }

    if (params->effect.type == 0.0f && params->effect.param1 == 0.0f && params->effect.param2 == 0.0f && params->effect.param3 == 0.0f) {
    	params->effect.param1 = 0.5f;
    	params->effect.param2 = 0.5f;
    	params->effect.param3 = 1.0f;
    }

    if (params->midiNote1Curve.breakNote == 0.0f && params->midiNote1Curve.curveAfter == 0.0f && params->midiNote1Curve.curveBefore == 0.0f) {
        // Default compatibility value
        // FLAT 0 +Lin
        params->midiNote1Curve.curveAfter  = 1;
    }
    if (params->midiNote2Curve.breakNote == 0.0f && params->midiNote2Curve.curveAfter == 0.0f && params->midiNote2Curve.curveBefore == 0.0f) {
        // Default compatibility value
        // FLAT 0 +Lin
        params->midiNote2Curve.curveBefore = 4;
        params->midiNote2Curve.curveAfter  = 1;
        params->midiNote2Curve.breakNote = 60;
    }
}


int PreenFMFileType::bankBaseLength(const char* bankName) {
	int k;
	for (k=0; k<8 && bankName[k]!=0 && bankName[k]!='.'; k++);
	return k;
}

bool PreenFMFileType::nameExists(const char* bankName) {
	int nameLength = bankBaseLength(bankName);
	for (int b=0; getFile(b)->fileType != FILE_EMPTY && b<numberOfFilesMax; b++) {
		const struct PFM2File* pfmb = getFile(b);
		if (nameLength != bankBaseLength(pfmb->name)) {
			continue;
		}
		bool sameName = true;
		for (int n=0; n < nameLength && sameName; n++) {
			// Case insensitive...
			char c1 = bankName[n];
			char c2 = pfmb->name[n];
			if (c1 >= 'a' && c1<='z') {
				c1 = 'A' + c1 - 'a';
			}
			if (c2 >= 'a' && c2<='z') {
				c2 = 'A' + c2 - 'a';
			}
			if (c1 != c2) {
				sameName = false;
			}
		}
		if (sameName) {
			return true;
		}
	}
	return false;
}



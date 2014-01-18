
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

#include <stdint.h>
#include "Storage.h"
#include "Menu.h"
#include "PresetUtil.h"


// Set param in memmory reachable with USB : static is OK
struct FlashSynthParams reachableFlashParam;
struct OneSynthParams reachableParam;
uint8_t dx7PackedPatch[DX7_PACKED_PATCH_SIZED];

void Storage::init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4) {
    timbre[0] = timbre1;
    timbre[1] = timbre2;
    timbre[2] = timbre3;
    timbre[3] = timbre4;
}

void Storage::copy(char* dest, const char* source, int length) {
    for (int k=0; k<length; k++) {
        dest[k] = source[k];
    }
}

void Storage::addNumber(char* name, int offset, int number) {
    int div = 100;
    while (div > 0) {
        int digit = number / div;
        name[offset++] = '0' + digit;
        number -= digit * div;
        div /= 10;
    }
    name[offset] = '\0';
}

void Storage::removeDefaultCombo() {
    remove(DEFAULT_COMBO);
}

void Storage::saveDefaultCombo() {
    // Default : no name
    // delete to be sure the size is OK...
    remove(DEFAULT_COMBO);
    for (int timbre = 0; timbre < 4; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam);
    	save(DEFAULT_COMBO, ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
    }
}

bool Storage::loadDefaultCombo() {
    // default : non ame

    if (checkSize(DEFAULT_COMBO) != (ALIGNED_PATCH_SIZE*3+PFM_PATCH_SIZE)) {
        return false;
    }

    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(DEFAULT_COMBO,  ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

        if (result == 0) {
        	convertMemoryToParams(&reachableFlashParam, this->timbre[timbre]);
        }
    }
    return true;
}


void Storage::createPatchBank(const char* name) {
#ifndef BOOTLOADER
	const struct BankFile * newBank = addEmptyBank(name);
	const char* fullBankName = getPreenFMFullName(name);
	if (newBank == 0) {
		return;
	}
    // back up name
    copy(reachableFlashParam.presetName, "Preset 0\0\0\0\0\0", 12);
	for (int k=0; k<128; k++) {
        addNumber(reachableFlashParam.presetName, 7, k + 1);
		savePreenFMPatch(newBank, k, &preenMainPreset);
	}
#endif
}


void Storage::createComboBank(const char* name) {
#ifndef BOOTLOADER
	const struct BankFile * newBank = addEmptyCombo(name);
	const char* fullBankName = getPreenFMFullName(name);

	if (newBank == 0) {
		return;
	}

	convertParamsToMemory(&preenMainPreset, &reachableFlashParam);

	char comboName[12];
    copy(comboName,  "Combo \0\0\0\0\0\0", 12);
    copy(reachableFlashParam.presetName, "Preset 0\0\0\0\0\0", 12);
	for (int comboNumber=0; comboNumber<128; comboNumber++) {

        addNumber(comboName, 6, comboNumber + 1);
		save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber , comboName, 12);

        for (int timbre = 0; timbre < 4; timbre++)  {
            addNumber(reachableFlashParam.presetName, 7, timbre + 1);
            save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
        }
	}

#endif
#ifdef DONOTEXISTATALL
	char comboName[12];
    copy(comboName,  "Combo \0\0\0\0\0\0", 12);

    for (int combo = 0; combo < 128; combo++) {
        addNumber(comboName, 6, combo + 1);
        saveCombo(combo, comboName);
    }
#endif
}

void Storage::loadConfig(char* midiConfig) {
    char* reachableProperties = (char*)&reachableFlashParam;
    if (checkSize(PROPERTIES) != MIDICONFIG_SIZE) {
        return;
    }
    // Don't load in params directly because params is in CCM memory
    int result = load(PROPERTIES, 0,  reachableProperties, MIDICONFIG_SIZE);
    if (result == 0) {
        for (int k=0; k<MIDICONFIG_SIZE; k++) {
           midiConfig[k] = reachableProperties[k];
        }
    }
}

void Storage::saveConfig(const char* midiConfig) {
    char* reachableProperties = (char*)&reachableFlashParam;
    for (int k=0; k<MIDICONFIG_SIZE; k++) {
        reachableProperties[k] = midiConfig[k];
    }
    // delete it so that we're sure the new one has the right size...
    remove(PROPERTIES);
    save(PROPERTIES, 0,  reachableProperties, MIDICONFIG_SIZE);
}

// NEW mechanism ===

uint8_t* Storage::dx7LoadPatch(const struct BankFile* bank, int patchNumber) {
	const char* fullBankName = getDX7BankFullName(bank->name);
    int result = load(fullBankName, patchNumber * DX7_PACKED_PATCH_SIZED + 6,  (void*)dx7PackedPatch, DX7_PACKED_PATCH_SIZED);
    if (result >0) {
    	return (uint8_t*)0;
    }
    return dx7PackedPatch;
}



void Storage::loadPreenFMPatch(const struct BankFile* bank, int patchNumber, struct OneSynthParams *params) {
	const char* fullBankName = getPreenFMFullName(bank->name);

    // Don't load in params directly because params is in CCM memory
    int result = load(fullBankName, patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

    if (result == 0) {
    	convertMemoryToParams(&reachableFlashParam, params);
    }
}

const char* Storage::loadPreenFMPatchName(const struct BankFile* bank, int patchNumber) {
	// Get name position
	int namePosition = (int)(((unsigned int) reachableFlashParam.presetName) - (unsigned int)&reachableFlashParam);
	const char* fullBankName = getPreenFMFullName(bank->name);
    int result = load(fullBankName, ALIGNED_PATCH_SIZE * patchNumber + namePosition,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void Storage::savePreenFMPatch(const struct BankFile* bank, int patchNumber, const struct OneSynthParams *params) {
	const char* fullBankName = getPreenFMFullName(bank->name);

    char zeros[ALIGNED_PATCH_ZERO];
    for (int k=0; k<ALIGNED_PATCH_ZERO;k++) {
        zeros[k] = 0;
    }
	convertParamsToMemory(params, &reachableFlashParam);

    // Save patch
    save(fullBankName, patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

    // Add zeros
    save(fullBankName, patchNumber * ALIGNED_PATCH_SIZE  + PFM_PATCH_SIZE,  (void*)zeros, ALIGNED_PATCH_ZERO);
}

void Storage::loadPreenFMCombo(const struct BankFile* combo, int comboNumber) {
	const char* fullBankName = getPreenFMFullName(combo->name);
    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(fullBankName,  (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);

        if (result == 0) {
        	convertMemoryToParams(&reachableFlashParam, this->timbre[timbre]);
        }
    }
}
const char* Storage::loadPreenFMComboName(const struct BankFile* combo, int comboNumber) {
	const char* fullBankName = getPreenFMFullName(combo->name);
    int result = load(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void Storage::savePreenFMCombo(const struct BankFile* combo, int comboNumber, char* comboName) {
	const char* fullBankName = getPreenFMFullName(combo->name);
    save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)comboName, 12);

    for (int timbre = 0; timbre < 4; timbre++)  {
    	convertParamsToMemory(this->timbre[timbre], &reachableFlashParam);
        save(fullBankName, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableFlashParam, PFM_PATCH_SIZE);
    }
}



void Storage::saveBank(const char* newBankName, const uint8_t* sysexTmpMem) {
#ifndef BOOTLOADER
	const struct BankFile * newBank = addEmptyBank(newBankName);
	if (newBank == 0) {
		return;
	}
	for (int k=0; k<128; k++) {
		if (sysexTmpMem[8 + PATCH_SIZE_PFM2 * 128] == '2') {
			PresetUtil::convertCharArrayToSynthState(8 + sysexTmpMem + PATCH_SIZE_PFM2 * k, &oneSynthParamsTmp);
		} else /* MUST BE '1' */ {
			PresetUtil::convertPFM1CharArrayToSynthState(sysexTmpMem + PFM1_PATCH_SIZE * k, &oneSynthParamsTmp, false);
		}
		savePreenFMPatch(newBank, k, &oneSynthParamsTmp);
	}
#endif
}

int Storage::bankBaseLength(const char* bankName) {
	int k;
	for (k=0; k<8 && bankName[k]!=0 && bankName[k]!='.'; k++);
	return k;
}

bool Storage::bankNameExist(const char* bankName) {
	int nameLength = bankBaseLength(bankName);
	for (int b=0; getPreenFMBank(b)->fileType != FILE_EMPTY && b<NUMBEROFPREENFMBANKS; b++) {
		const struct BankFile* pfmb = getPreenFMBank(b);
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

bool Storage::comboNameExist(const char* comboName) {
	int nameLength = bankBaseLength(comboName);
	for (int b=0; getPreenFMCombo(b)->fileType != FILE_EMPTY && b<NUMBEROFPREENFMCOMBOS; b++) {
		const struct BankFile* pfmc = getPreenFMCombo(b);
		if (nameLength != bankBaseLength(pfmc->name)) {
			continue;
		}
		bool sameName = true;
		for (int n=0; n < nameLength && sameName; n++) {
			// Case insensitive...
			char c1 = comboName[n];
			char c2 = pfmc->name[n];
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


void  Storage::copyFloat(float* source, float* dest, int n) {
	for (int k=0; k<n; k++) {
		dest[k] = source[k];
	}
}


void Storage::convertParamsToMemory(const struct OneSynthParams* params, struct FlashSynthParams* memory) {
	// First engine line
	copyFloat((float*)&params->engine1, (float*)&memory->engine1, 4);
	copyFloat((float*)&params->engineApr1, (float*)&memory->engineApr1, 4 * 2);

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

	copyFloat((float*)&params->engineMix1,(float*)&memory->engineMix1 , 4 * 3);
	copyFloat((float*)&params->effect,(float*)&memory->effect , 4);
	copyFloat((float*)&params->osc1,(float*)&memory->osc1 , 4 * 6);
	copyFloat((float*)&params->env1a, (float*)&memory->env1a, 4 * 6 * 2);
	copyFloat((float*)&params->matrixRowState1, (float*)&memory->matrixRowState1, 4 * 12);
	copyFloat((float*)&params->lfoOsc1, (float*)&memory->lfoOsc1, 4 * 3);
	copyFloat((float*)&params->lfoEnv1, (float*)&memory->lfoEnv1, 4);
	copyFloat((float*)&params->lfoEnv2, (float*)&memory->lfoEnv2, 4);
	copyFloat((float*)&params->lfoSeq1, (float*)&memory->lfoSeq1, 4 * 2);
	for (int s=0; s<16; s++) {
		memory->lfoSteps1.steps[s] = params->lfoSteps1.steps[s];
		memory->lfoSteps2.steps[s] = params->lfoSteps2.steps[s];
	}
	for (int s=0; s<13; s++) {
		memory->presetName[s] = params->presetName[s];
	}
}


void Storage::convertMemoryToParams(const struct FlashSynthParams* memory, struct OneSynthParams* params) {
	// First engine line
	copyFloat((float*)&memory->engine1, (float*)&params->engine1, 4);
	copyFloat((float*)&memory->engineApr1, (float*)&params->engineApr1, 4 * 2);

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

	copyFloat((float*)&memory->engineMix1,(float*)&params->engineMix1 , 4 * 3);
	copyFloat((float*)&memory->effect,(float*)&params->effect , 4);
	copyFloat((float*)&memory->osc1,(float*)&params->osc1 , 4 * 6);
	copyFloat((float*)&memory->env1a, (float*)&params->env1a, 4 * 6 * 2);
	copyFloat((float*)&memory->matrixRowState1, (float*)&params->matrixRowState1, 4 * 12);
	copyFloat((float*)&memory->lfoOsc1, (float*)&params->lfoOsc1, 4 * 3);
	copyFloat((float*)&memory->lfoEnv1, (float*)&params->lfoEnv1, 4);
	copyFloat((float*)&memory->lfoEnv2, (float*)&params->lfoEnv2, 4);
	copyFloat((float*)&memory->lfoSeq1, (float*)&params->lfoSeq1, 4 * 2);
	for (int s=0; s<16; s++) {
		params->lfoSteps1.steps[s] = memory->lfoSteps1.steps[s];
		params->lfoSteps2.steps[s] = memory->lfoSteps2.steps[s];
	}
	for (int s=0; s<13; s++) {
		params->presetName[s] = memory->presetName[s];
	}

    if (params->engineApr1.BPM < 10) {
    	params->engineApr1.BPM = 90;
    	params->engineApr1.octave = 1;
    	params->engineApr2.pattern = 1;
    	params->engineApr2.division = 12;
    	params->engineApr2.duration = 14;
    }

//    if (params->effect.param3 == 0.0f) {
//    	params->effect.param1 = 0.5f;
//    	params->effect.param2 = 0.5f;
//    	params->effect.param3 = 0.5f;
//    }

}




#ifdef DEBUG
#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

extern unsigned int preenTimer;


void Storage::testMemoryPreset() {
	lcd.setRealTimeAction(true);
	struct OneSynthParams tmpParam;
	unsigned char* test = (unsigned char*)&tmpParam;
	for (int k=0; k< sizeof(struct OneSynthParams); k++) {
		test[k] = ((k + preenTimer) % 200) + 34;
	}

	tmpParam.engineApr1.BPM = (preenTimer % 200) + 15;
	convertParamsToMemory(&tmpParam, &reachableFlashParam);
	convertMemoryToParams(&reachableFlashParam, &reachableParam);

	lcd.clear();
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

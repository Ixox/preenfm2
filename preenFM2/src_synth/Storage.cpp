
/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier.hosxe@gmail.com)
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

// Set param in mememory reachable with USB : static is OK
struct OneSynthParams reachableParam;

void Storage::init(uint8_t*timbre1, uint8_t*timbre2, uint8_t*timbre3, uint8_t*timbre4) {
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
        for (int k=0; k<PFM_PATCH_SIZE; k++) {
            ((uint8_t*)&reachableParam)[k] = this->timbre[timbre][k];
        }
        save(DEFAULT_COMBO, ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableParam, PFM_PATCH_SIZE);
    }
}

bool Storage::loadDefaultCombo() {
    // default : non ame

    if (checkSize(DEFAULT_COMBO) != (ALIGNED_PATCH_SIZE*3+PFM_PATCH_SIZE)) {
        return false;
    }

    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(DEFAULT_COMBO,  ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableParam, PFM_PATCH_SIZE);

        if (result == 0) {
            for (int k=0; k<PFM_PATCH_SIZE; k++) {
               this->timbre[timbre][k] = ((uint8_t*)&reachableParam)[k];
            }
        }
    }
    return true;
}

const char* Storage::readPresetName(int patchNumber) {
    int result = load(PATCH_BANK1, ALIGNED_PATCH_SIZE * patchNumber + PFM_PATCH_SIZE - 16,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

const char* Storage::readComboName(int comboNumber) {
    int result = load(COMBO_BANK, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)presetName, 12);
    presetName[12] = 0;
    return presetName;
}

void Storage::loadPatch(int bankNumber, int patchNumber, struct OneSynthParams *params) {
    // Don't load in params directly because params is in CCM memory
    int result = load((FILE_ENUM)(PATCH_BANK1 + bankNumber), patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableParam, PFM_PATCH_SIZE);

    if (result == 0) {
        for (int k=0; k<PFM_PATCH_SIZE; k++) {
           ((uint8_t*)params)[k] = ((uint8_t*)&reachableParam)[k];
        }
    }
}

void Storage::loadCombo(int comboNumber) {

    for (int timbre = 0; timbre < 4; timbre++)  {
        int result = load(COMBO_BANK,  (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre ,  (void*)&reachableParam, PFM_PATCH_SIZE);

        if (result == 0) {
            for (int k=0; k<PFM_PATCH_SIZE; k++) {
               this->timbre[timbre][k] = ((uint8_t*)&reachableParam)[k];
            }
        }
    }
}

void Storage::savePatch(int bankNumber, int patchNumber, struct OneSynthParams *params) {
    char zeros[ALIGNED_PATCH_ZERO];

    for (int k=0; k<ALIGNED_PATCH_ZERO;k++) {
        zeros[k] = 0;
    }

    for (int k=0; k<PFM_PATCH_SIZE; k++) {
        ((uint8_t*)&reachableParam)[k] = ((uint8_t*)params)[k];
    }

    // Save patch
    save((FILE_ENUM)(PATCH_BANK1 + bankNumber), patchNumber * ALIGNED_PATCH_SIZE,  (void*)&reachableParam, PFM_PATCH_SIZE);

    // Add zeros
    save((FILE_ENUM)(PATCH_BANK1 + bankNumber), patchNumber * ALIGNED_PATCH_SIZE  + PFM_PATCH_SIZE,  (void*)zeros, ALIGNED_PATCH_ZERO);
}

void Storage::saveCombo(int comboNumber, const char*comboName) {

    save(COMBO_BANK, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber ,  (void*)comboName, 12);

    for (int timbre = 0; timbre < 4; timbre++)  {
        for (int k=0; k<PFM_PATCH_SIZE; k++) {
            ((uint8_t*)&reachableParam)[k] = this->timbre[timbre][k];
        }
        save(COMBO_BANK, (ALIGNED_PATCH_SIZE * 4 + 12) * comboNumber +  12 + ALIGNED_PATCH_SIZE * timbre,  (void*)&reachableParam, PFM_PATCH_SIZE);
    }
}

void Storage::createPatchBank() {
    /*
    for (int k=0; k<PFM_PATCH_SIZE; k++) {
        ((uint8_t*)&reachableParam)[k] = ((uint8_t*)presets)[k];
    }
    // back up name
    copy(reachableParam.presetName, "Preset 0\0\0\0\0\0", 12);
    for (int patch = 0; patch < 128; patch++) {
        addNumber(reachableParam.presetName, 7, patch + 1);
        savePatch(0, patch, &reachableParam);
        savePatch(1, patch, &reachableParam);
        savePatch(2, patch, &reachableParam);
        savePatch(3, patch, &reachableParam);
    }
    */
}


void Storage::createComboBank() {
    char comboName[12];
    copy(comboName,  "Combo \0\0\0\0\0\0", 12);

    for (int combo = 0; combo < 128; combo++) {
        addNumber(comboName, 6, combo + 1);
        saveCombo(combo, comboName);
    }
}

void Storage::loadConfig(char* midiConfig) {
    char* reachableProperties = (char*)&reachableParam;
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
    char* reachableProperties = (char*)&reachableParam;
    for (int k=0; k<MIDICONFIG_SIZE; k++) {
        reachableProperties[k] = midiConfig[k];
    }
    // delete it so that we're sure the new one has the right size...
    remove(PROPERTIES);
    save(PROPERTIES, 0,  reachableProperties, MIDICONFIG_SIZE);
}

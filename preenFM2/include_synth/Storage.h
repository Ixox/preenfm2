/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe
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



#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "Common.h"

enum FILE_ENUM {
    DEFAULT_COMBO = 0,
    PATCH_BANK1,
    PATCH_BANK2,
    PATCH_BANK3,
    PATCH_BANK4,
    COMBO_BANK,
    PROPERTIES,
    FIRMWARE
};

#define PFM_PATCH_SIZE sizeof(struct OneSynthParams)
#define ALIGNED_PATCH_SIZE 1024
#define ALIGNED_PATCH_ZERO ALIGNED_PATCH_SIZE-PFM_PATCH_SIZE
#define ALIGNED_COMBO_SIZE ALIGNED_PATCH_SIZE*4+13


class Storage {
public:
    virtual ~Storage() {}
    void saveDefaultCombo();
    void loadDefaultCombo();
    void removeDefaultCombo();
    void createPatchBank();
    void createComboBank();
    void loadPatch(int bankNumber, int patchNumber, struct OneSynthParams *params);
    void loadCombo(int comboNumber);
    void savePatch(int bankNumber, int patchNumber, struct OneSynthParams *params);
    void saveCombo(int comboNumber, const char*comboName);
    const char* readPresetName(int patchNumber);
    const char* readComboName(int comboNumber);
    void loadConfig(char* midiConfig);
    void saveConfig(const char* midiConfig);

    // Virtual
    virtual void init(uint8_t*timbre1, uint8_t*timbre2, uint8_t*timbre3, uint8_t*timbre4);


private:
    // Pure Virtual
    virtual int save(FILE_ENUM file,int seek, void* bytes, int size) = 0;
    virtual int load(FILE_ENUM file, int seek, void* bytes, int size) = 0;
    virtual int remove(FILE_ENUM file) = 0;
    // checkSize must return -1 if file does not exist
    virtual int checkSize(FILE_ENUM file) = 0;
    void addNumber(char* name, int offset, int number);
    void copy(char* dest, const char* source, int length);

    char presetName[13];
    uint8_t* timbre[4];
};

#endif /*__STORAGE_H__*/



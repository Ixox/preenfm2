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



struct DX7Bank {
	char name[13];
	FileType fileType;
};

struct PreenFMBank {
	char name[13];
	FileType fileType;
};

struct PreenFMCombo {
	char name[13];
	FileType fileType;
};

#define PFM_PATCH_SIZE sizeof(struct OneSynthParams)
#define ALIGNED_PATCH_SIZE 1024
#define ALIGNED_PATCH_ZERO ALIGNED_PATCH_SIZE-PFM_PATCH_SIZE
#define ALIGNED_COMBO_SIZE ALIGNED_PATCH_SIZE*4+13

#define DX7_PACKED_PATCH_SIZED 128
#define DX7_UNPACKED_PATCH_SIZED 155


class Storage {
public:
    virtual ~Storage() {}
    void saveDefaultCombo();
    bool loadDefaultCombo();
    void removeDefaultCombo();
    void createPatchBank();
    void createComboBank();
    void loadPatch(int bankNumber, int patchNumber, struct OneSynthParams *params);
    void loadCombo(int comboNumber);
    void savePatch(int bankNumber, int patchNumber, struct OneSynthParams *params);
    void saveCombo(int comboNumber, const char*comboName);
    const char* readPresetName(int bankNumber, int patchNumber);
    const char* readComboName(int comboNumber);
    void loadConfig(char* midiConfig);
    void saveConfig(const char* midiConfig);

    void saveBank(const char* newBankName, const uint8_t* sysexTmpMem);
    bool bankNameExist(const char* bankName);
    int bankBaseLength(const char* bankName);

    virtual const struct DX7Bank* getDx7Bank(int bankNumber) = 0;
    uint8_t* dx7LoadPatch(const struct DX7Bank* bank, int patchNumber);

    virtual const struct PreenFMBank* getPreenFMBank(int bankNumber) = 0;
    void loadPreenFMPatch(const struct PreenFMBank* bank, int patchNumber, struct OneSynthParams *params);
    const char* loadPreenFMPatchName(const struct PreenFMBank* bank, int patchNumber);
    void savePreenFMPatch(const struct PreenFMBank* bank, int patchNumber, struct OneSynthParams *params);

    virtual const struct PreenFMCombo* getPreenFMCombo(int comboNumber) = 0;
    void loadPreenFMCombo(const struct PreenFMCombo* combo, int comboNumber);
    const char* loadPreenFMComboName(const struct PreenFMCombo* combo, int comboNumber);
    void savePreenFMCombo(const struct PreenFMCombo* combo, int comboNumber, char* comboName);

    // Virtual
    virtual void init(uint8_t*timbre1, uint8_t*timbre2, uint8_t*timbre3, uint8_t*timbre4);


private:
    // Pure Virtual
    virtual const struct PreenFMBank*  addEmptyBank(const char* newBankName) = 0;
    virtual const char* getDX7BankFullName(const char* bankName) = 0;
    virtual const char* getPreenFMFullName(const char* bankName) = 0;
    void dx7patchUnpack(uint8_t *packed_patch, uint8_t *unpacked_patch);
    virtual int save(FILE_ENUM file,int seek, void* bytes, int size) = 0;
    virtual int save(const char* fileName, int seek, void* bytes, int size) = 0;
    virtual int load(FILE_ENUM file, int seek, void* bytes, int size) = 0;
    virtual int load(const char* fileName, int seek, void* bytes, int size) = 0;
    virtual int remove(FILE_ENUM file) = 0;
    // checkSize must return -1 if file does not exist
    virtual int checkSize(FILE_ENUM file) = 0;
    void addNumber(char* name, int offset, int number);
    void copy(char* dest, const char* source, int length);

    char presetName[13];
    uint8_t* timbre[4];
};

#endif /*__STORAGE_H__*/



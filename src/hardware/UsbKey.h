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



#ifndef __USBKEY_H__
#define __USBKEY_H__

#include "Common.h"
#include "Storage.h"

#include "usbKey_usr.h"

extern USB_OTG_CORE_HANDLE          usbOTGHost;
extern USBH_HOST                    usbHost;


#define DEFAULT_COMBO_NAME       "0:/pfm2/DfltCmbo.pfm"
#define PATCH_BANK1_NAME         "0:/pfm2/Bank1.pfm"
#define PATCH_BANK2_NAME         "0:/pfm2/Bank2.pfm"
#define PATCH_BANK3_NAME         "0:/pfm2/Bank3.pfm"
#define PATCH_BANK4_NAME         "0:/pfm2/Bank4.pfm"
#define COMBO_BANK_NAME          "0:/pfm2/Combo1.pfm"
#define PROPERTIES_NAME          "0:/pfm2/Settings.pfm"

#define FIRMWARE_DIR             "0:/pfm2"
#define DX7_DIR                  "0:/pfm2/dx7"
#define PREENFM_DIR              "0:/pfm2"

#define PFM_PATCH_SIZE sizeof(struct OneSynthParams)
#define USB_PATCH_SIZE 1024
#define USB_PATCH_ZERO USB_PATCH_SIZE-PFM_PATCH_SIZE
#define USB_COMBO_SIZE USB_PATCH_SIZE*4+13


class UsbKey : public Storage {
public:
    void init(uint8_t*timbre1, uint8_t*timbre2, uint8_t*timbre3, uint8_t*timbre4);

    // get firmware name available on disk
    // not in storage.. specific to USB
    // Used by bootloader

    const struct BankFile* getDx7Bank(int bankNumber);
    const struct BankFile* getPreenFMBank(int bankNumber);
    const struct BankFile* getPreenFMCombo(int comboNumber);


    int firmwareInit();
    int readNextFirmwareName(char *name, int*size);
    int loadFirmwarePart(char *fileName, int seek, void* bytes, int size);

    int renameBank(const struct BankFile* bank, const char* newName);
    int renameCombo(const struct BankFile* bank, const char* newName);

#ifdef BOOTLOADER
    unsigned int diskioGetSectorNumber();
    int diskioRead(uint8_t* buff, int address, int lenght);
    int diskioWrite(uint8_t* buff, int address, int lenght);
#endif

private:
    int renameFile(const struct BankFile* bank, const char* newName);

    void sortBankFile(struct BankFile* bankFiles, int numberOfFiles);
	void swapBankFile(struct BankFile* bankFiles, int i, int j);

	const struct BankFile*  addEmptyBank(const char* newBankName);
	const struct BankFile*  addEmptyCombo(const char* newBankName);

    const char* getDX7BankFullName(const char* bankName);
    const char* getPreenFMFullName(const char* bankName);
    void usbProcess();
    int save(FILE_ENUM file, int seek, void* bytes, int size);
    int save(const char* fileName, int seek, void* bytes, int size);
    int load(FILE_ENUM file, int seek, void* bytes, int size);
    int load(const char* fileName, int seek, void* bytes, int size);
   	int remove(FILE_ENUM file);
    int checkSize(FILE_ENUM file);

    const char* getFileName(FILE_ENUM file);
    bool isFirmwareFile(char *name);

    int dx7Init();
    int dx7ReadNextFileName(struct BankFile* bank);
    bool isDX7SysexFile(char *name, int size);

    int preenFMBankInit();
    int preenFMBankReadNextFileName(struct BankFile* bank);
    bool isPreenFMBankFile(char *name, int size);

    int preenFMComboInit();
    int preenFMComboReadNextFileName(struct BankFile* combo);
    bool isPreenFMComboFile(char *name, int size);


    const char* getFullName(const char* pathName, const char* fileName) ;
    int strlen(const char *string);

    bool dx7BankInitialized;
    struct BankFile dx7Bank[NUMBEROFDX7BANKS];
    int dx7NumberOfBanks;

    bool preenFMBankInitialized;
    struct BankFile preenFMBank[NUMBEROFPREENFMBANKS];
    int preenFMNumberOfBanks;

    bool preenFMComboInitialized;
    struct BankFile preenFMCombo[NUMBEROFPREENFMCOMBOS];
    int preenFMNumberOfCombos;

    struct BankFile errorDX7Bank;
    struct BankFile errorPreenFMBank;
    struct BankFile errorPreenFMCombo;

    char fullName[40];
};

#endif /*__USBKEY_H__*/



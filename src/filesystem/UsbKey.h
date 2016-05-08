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





class UsbKey : public Storage {
public:
    void init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4);

    // get firmware name available on disk
    // not in storage.. specific to USB
    // Used by bootloader

    const struct PFM2File* getDx7Bank(int bankNumber);
    const struct PFM2File* getPreenFMBank(int bankNumber);
    const struct PFM2File* getPreenFMCombo(int comboNumber);
    const struct PFM2File* getScalaScaleFile(int scaleNumber);

    int renameBank(const struct PFM2File* bank, const char* newName);
    int renameCombo(const struct PFM2File* bank, const char* newName);

#ifdef BOOTLOADER
    int firmwareInit();
    int readNextFirmwareName(char *name, int*size);
    int loadFirmwarePart(char *fileName, int seek, void* bytes, int size);
    unsigned int diskioGetSectorNumber();
    int diskioRead(uint8_t* buff, int address, int lenght);
    int diskioWrite(uint8_t* buff, int address, int lenght);
    bool isFirmwareFile(char *name);
#endif

private:
    int renameFile(const struct PFM2File* bank, const char* newName);

    void sortBankFile(struct PFM2File* bankFiles, int numberOfFiles);
	void swapBankFile(struct PFM2File* bankFiles, int i, int j);

	const struct PFM2File*  addEmptyBank(const char* newBankName);
	const struct PFM2File*  addEmptyCombo(const char* newBankName);

    const char* getDX7BankFullName(const char* bankName);
    const char* getPreenFMFullName(const char* bankName);
    const char* getScalaFileFullName(const char* bankName);

    void usbProcess();
    int save(FILE_ENUM file, int seek, void* bytes, int size);
    int save(const char* fileName, int seek, void* bytes, int size);
    int load(FILE_ENUM file, int seek, void* bytes, int size);
    int load(const char* fileName, int seek, void* bytes, int size);
    int checkSize(FILE_ENUM file);
    int checkSize(const char* filename);

    const char* getFileName(FILE_ENUM file);

    int dx7Init();
    int dx7ReadNextFileName(struct PFM2File* bank);
    bool isDX7SysexFile(char *name, int size);

    int preenFMBankInit();
    int preenFMBankReadNextFileName(struct PFM2File* bank);
    bool isPreenFMBankFile(char *name, int size);

    int preenFMComboInit();
    int preenFMComboReadNextFileName(struct PFM2File* combo);
    bool isPreenFMComboFile(char *name, int size);

    int scalaScaleInit();
    int scalaScaleReadNextFileName(struct PFM2File* scala);
    bool isScalaScaleFile(char *name, int size);


    const char* getFullName(const char* pathName, const char* fileName) ;
    int strlen(const char *string);

#ifndef BOOTLOADER
    bool dx7BankInitialized;
    struct PFM2File dx7Bank[NUMBEROFDX7BANKS];
    int dx7NumberOfBanks;

    bool preenFMBankInitialized;
    struct PFM2File preenFMBank[NUMBEROFPREENFMBANKS];
    int preenFMNumberOfBanks;

    bool preenFMComboInitialized;
    struct PFM2File preenFMCombo[NUMBEROFPREENFMCOMBOS];
    int preenFMNumberOfCombos;

    bool scalaScaleInitialized;
    struct PFM2File scalaScaleFile[NUMBEROFSCALASCALEFILES];
    int scalaScaleNumberOfFiles;


    struct PFM2File errorDX7Bank;
    struct PFM2File errorPreenFMBank;
    struct PFM2File errorPreenFMCombo;
    struct PFM2File errorScalaScale;
#endif

};

#endif /*__USBKEY_H__*/

/*
 * Copyright 2011 Xavier Hosxe
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



#ifndef __USBKEY_H__
#define __USBKEY_H__

#include "Common.h"
#include "Storage.h"

#define DEFAULT_COMBO_NAME       "0:/pfm2/DfltCmbo.pfm"
#define PATCH_BANK1_NAME         "0:/pfm2/Bank1.pfm"
#define PATCH_BANK2_NAME         "0:/pfm2/Bank2.pfm"
#define PATCH_BANK3_NAME         "0:/pfm2/Bank3.pfm"
#define PATCH_BANK4_NAME         "0:/pfm2/Bank4.pfm"
#define COMBO_BANK_NAME          "0:/pfm2/Combo1.pfm"
#define PROPERTIES_NAME          "0:/pfm2/Settings.pfm"

#define FIRMWARE_DIR             "0:/pfm2"

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
    int firmwareInit();
    int readNextFirmwareName(char *name, int*size);
    int loadFirmwarePart(char *fileName, int seek, void* bytes, int size);




private:
    void usbProcess();
    int save(FILE_ENUM file, int seek, void* bytes, int size);
    int load(FILE_ENUM file, int seek, void* bytes, int size);
    int remove(FILE_ENUM file);
    int checkSize(FILE_ENUM file);

    void getFullFirmwareName(char* fullName, char* fileName) ;
    int strlen(const char *string);
    const char* getFileName(FILE_ENUM file);

    bool isFirmwareFile(char *name);
};

#endif /*__USBKEY_H__*/



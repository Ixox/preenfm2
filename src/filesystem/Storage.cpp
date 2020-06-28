
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
#include <math.h>

#include "Storage.h"
#include "Menu.h"


// regular memory 128kb
char lineBuffer[512];

void Storage::init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4) {
    USBH_Init(&usbOTGHost, USB_OTG_HS_CORE_ID, &usbHost, &USBH_MSC_cb, &USR_Callbacks);
	commandParams.commandState = COMMAND_INIT;
#if !defined(BOOTLOADER)
	// Use any object for usbProcess...
	comboBank.usbProcess();
    comboBank.setFileSystemUtils(&fsu);
    comboBank.init(timbre1, timbre2, timbre3, timbre4);
    scalaFile.setFileSystemUtils(&fsu);
    configurationFile.setFileSystemUtils(&fsu);
    configurationFile.setScalaFile(&scalaFile);
    dx7SysexFile.setFileSystemUtils(&fsu);
    patchBank.setFileSystemUtils(&fsu);
    userWaveForm.setFileSystemUtils(&fsu);
#endif
}



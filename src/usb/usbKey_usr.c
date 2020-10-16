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





/*
 * CallBack user file for  USB Key access from PreenFM 2
 * Copied and adapted from ST firmware library
 */
#include "usbKey_usr.h"
#include "diskio.h"

#include "LiquidCrystal.h"
extern LiquidCrystal lcd;

// Cannot be in class : used in PreenFM_irq.c and in usbh_msc_fatfs.c
USB_OTG_CORE_HANDLE          usbOTGHost;
USBH_HOST                    usbHost;

FATFS fatfs;
FRESULT res;
FILINFO fileinfo;
FIL fileR;
DIR dir;


struct usbCommandParam commandParams;




USBH_Usr_cb_TypeDef USR_Callbacks = { USBH_USR_Init,
        USBH_USR_DeInit,
        USBH_USR_DeviceAttached,
        USBH_USR_ResetDevice,
        USBH_USR_DeviceDisconnected,
        USBH_USR_OverCurrentDetected,
        USBH_USR_DeviceSpeedDetected,
        USBH_USR_Device_DescAvailable,
        USBH_USR_DeviceAddressAssigned,
        USBH_USR_Configuration_DescAvailable,
        USBH_USR_Manufacturer_String,
        USBH_USR_Product_String,
        USBH_USR_SerialNum_String,
        USBH_USR_EnumerationDone,
        USBH_USR_UserInput,
        USBH_USR_MSC_Application,
        USBH_USR_DeviceNotSupported,
        USBH_USR_UnrecoveredError };





/**
 * @brief  USBH_USR_Init
 *         Displays the message on LCD for host lib initialization
 * @param  None
 * @retval None
 */
void USBH_USR_Init(void) {
}

/**
 * @brief  USBH_USR_DeviceAttached
 *         Displays the message on LCD on device attached
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceAttached(void) {
}

/**
 * @brief  USBH_USR_UnrecoveredError
 * @param  None
 * @retval None
 */
void USBH_USR_UnrecoveredError() {
	/* Toggle Red LED in infinite loop */
/*    lcd.setCursor(0,3);
    lcd.print(err);
	Fail_Handler("#UE : ");
	*/
}

/**
 * @brief  USBH_DisconnectEvent
 *         Device disconnect event
 * @param  None
 * @retval Staus
 */
void USBH_USR_DeviceDisconnected(void) {
}

/**
 * @brief  USBH_USR_ResetUSBDevice
 * @param  None
 * @retval None
 */
void USBH_USR_ResetDevice(void) {
}

/**
 * @brief  USBH_USR_DeviceSpeedDetected
 *         Displays the message on LCD for device speed
 * @param  Device speed:
 * @retval None
 */
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed) {
	if ((DeviceSpeed != HPRT0_PRTSPD_FULL_SPEED)
			&& (DeviceSpeed != HPRT0_PRTSPD_LOW_SPEED)) {
		/* Toggle Red LED in infinite loop: USB device disconnected */
	    Fail_Handler("#Speed#");
	}
}

/**
 * @brief  USBH_USR_Device_DescAvailable
 * @param  device descriptor
 * @retval None
 */
void USBH_USR_Device_DescAvailable(void *DeviceDesc) {
}

/**
 * @brief  USBH_USR_DeviceAddressAssigned
 *         USB device is successfully assigned the Address
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceAddressAssigned(void) {
}

/**
 * @brief  USBH_USR_Conf_Desc
 * @param  Configuration descriptor
 * @retval None
 */
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
		USBH_InterfaceDesc_TypeDef *itfDesc, USBH_EpDesc_TypeDef *epDesc) {
}

/**
 * @brief  USBH_USR_Manufacturer_String
 * @param  Manufacturer String
 * @retval None
 */
void USBH_USR_Manufacturer_String(void *ManufacturerString) {
}

/**
 * @brief  USBH_USR_Product_String
 * @param  Product String
 * @retval None
 */
void USBH_USR_Product_String(void *ProductString) {
}

/**
 * @brief  USBH_USR_SerialNum_String
 * @param  SerialNum_String
 * @retval None
 */
void USBH_USR_SerialNum_String(void *SerialNumString) {
}

/**
 * @brief  EnumerationDone
 *         User response request is displayed to ask application jump to class
 * @param  None
 * @retval None
 */
void USBH_USR_EnumerationDone(void) {
}

/**
 * @brief  USBH_USR_DeviceNotSupported
 *         Device is not supported
 * @param  None
 * @retval None
 */
void USBH_USR_DeviceNotSupported(void) {
}

/**
 * @brief  USBH_USR_UserInput
 *         User Action for application state entry
 * @param  None
 * @retval USBH_USR_Status : User response for key button
 */
USBH_USR_Status USBH_USR_UserInput(void) {
    // XH : 1/3 of seconde to stabilise
    // Important Prevent USB from failing
    int us = 50000000 / 3;

    /* fudge for function call overhead  */
    //us--;
    asm volatile("   mov r0, %[us]          \n\t"
                 "1: subs r0, #1            \n\t"
                 "   bhi 1b                 \n\t"
                 :
                 : [us] "r" (us)
                 : "r0");


	/* callback for Key botton: set by software in this case */
	return USBH_USR_RESP_OK;
}

/**
 * @brief  USBH_USR_OverCurrentDetected
 *         Over Current Detected on VBUS
 * @param  None
 * @retval None
 */
void USBH_USR_OverCurrentDetected(void) {
}

/**
 * @brief  USBH_USR_MSC_Application
 *         Demo application for IAP thru USB mass storage
 * @param  None
 * @retval Staus
 */


void displayFileError(char* msg) {
    lcd.setRealTimeAction(true);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("# ");
    lcd.print(msg);
    lcd.print(" #");
    lcd.setCursor(0,2);
    lcd.print("Usb drive likely not");
    lcd.setCursor(0,3);
    lcd.print("compatible.");

    lcd.setCursor(0,1);
    for (int k = 0; k < 10; k++) {
        USBH_USR_UserInput();
        lcd.print("..");
    }
}

int USBH_USR_MSC_Application(void) {
    unsigned int numberOfBytes;

    switch (commandParams.commandState) {
    case COMMAND_INIT:
        if (f_mount(&fatfs, "0:/", 0) != FR_OK) {
            displayFileError("MOUNT ERROR");
        }
        commandParams.commandState = COMMAND_NONE;
        commandParams.commandResult = COMMAND_SUCCESS;
        break;

    case COMMAND_LOAD:
        if (f_open(&fileR, commandParams.commandFileName,  FA_READ) != FR_OK) {
            displayFileError("OPEN ERROR");
        }
        f_lseek(&fileR, commandParams.commandSeek);
        f_read (&fileR, commandParams.commandParam1, commandParams.commandParamSize, &numberOfBytes);
        if (numberOfBytes != commandParams.commandParamSize) {
            displayFileError("READ ERROR");
        }
        if (f_close(&fileR) != FR_OK) {
            displayFileError("CLOSE ERROR");
        }
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_SAVE:
        if (f_open(&fileR, commandParams.commandFileName,  FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
            displayFileError("OPEN ERROR");
        }
        f_lseek(&fileR, commandParams.commandSeek);
        f_write(&fileR, commandParams.commandParam1, commandParams.commandParamSize, &numberOfBytes);
        if (numberOfBytes != commandParams.commandParamSize) {
            displayFileError("WRITE ERROR");
        }
        if (f_close(&fileR) != FR_OK) {
            displayFileError("CLOSE ERROR");
        }
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_DELETE:
        // Don't look at the result....
        f_unlink(commandParams.commandFileName);
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_EXISTS:
        if (f_stat(commandParams.commandFileName, &fileinfo) == FR_OK) {
            commandParams.commandResult = fileinfo.fsize;
        } else {
            commandParams.commandResult = -1;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_OPEN_DIR:
        if (f_opendir ( &dir, commandParams.commandFileName) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
        } else {
            commandParams.commandResult = COMMAND_SUCCESS;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_CLOSE_DIR:
        if (f_closedir ( &dir) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
        } else {
            commandParams.commandResult = COMMAND_SUCCESS;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_NEXT_FILE_NAME:
        if (f_readdir ( &dir, &fileinfo) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
            commandParams.commandState = COMMAND_NONE;
            break;
        }
        if (fileinfo.fname[0] == 0) {
            commandParams.commandResult = COMMAND_FAILED;
            commandParams.commandState = COMMAND_NONE;
            break;
        }
        for (int k=0; k<13; k++) {
            ((char*)commandParams.commandParam1)[k] = fileinfo.fname[k];
        }
        *((unsigned long*)commandParams.commandParam2) = fileinfo.fsize;
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_RENAME:
        if (f_rename(commandParams.commandFileName, (const char*)commandParams.commandParam1) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
        } else {
            commandParams.commandResult = COMMAND_SUCCESS;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
// #ifdef BOOTLOADER
    // Low level only accessible by bootloader in mass storage device mode.
    case DISKIO_GETSECTORNUMBER:
    	disk_initialize(0);
    	if ((numberOfBytes = disk_ioctl(0, GET_SECTOR_COUNT, (unsigned long*)commandParams.commandParam1)) != RES_OK) {
            commandParams.commandResult = COMMAND_FAILED;
            *((int*)commandParams.commandParam1) = 0;
    	} else {
            commandParams.commandResult = COMMAND_SUCCESS;
    	}
        commandParams.commandState = COMMAND_NONE;
    	break;
    case DISKIO_READ:
	  	if (disk_read(0, (BYTE*)commandParams.commandParam1, (DWORD)*((int*)commandParams.commandParam2), (BYTE)commandParams.commandParamSize) != RES_OK) {
	  		commandParams.commandResult = COMMAND_FAILED;
	  	} else {
	  		commandParams.commandResult = COMMAND_SUCCESS;
	  	}
        commandParams.commandState = COMMAND_NONE;
    	break;
    case DISKIO_WRITE:
	  	if (disk_write(0, (BYTE*)commandParams.commandParam1, (DWORD)*((int*)commandParams.commandParam2), (BYTE)commandParams.commandParamSize) != RES_OK) {
	  		commandParams.commandResult = COMMAND_FAILED;
	  	} else {
	  		commandParams.commandResult = COMMAND_SUCCESS;
	  	}
        commandParams.commandState = COMMAND_NONE;
    	break;
// #endif
    case COMMAND_NONE:
        break;
    }
	return 0;
}

/**
 * @brief  USBH_USR_DeInit
 *         Deint User state and associated variables
 * @param  None
 * @retval None
 */
void USBH_USR_DeInit(void) {
}

/**
 * @brief  This function handles the program fail.
 * @param  None
 * @retval None
 */
void Fail_Handler(const char*msg) {
	lcd.setCursor(0, 0);
	lcd.print(msg);
	while (1) {	}
}


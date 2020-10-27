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


#include "./Menu.h"


#ifndef CVIN
#define CVIN_STRING
#else
#define CVIN_STRING "_cv"
#endif


const char* midiChannels [] = { "All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
const char* eccChannels [] = { "None", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
const char* yesNo [] = { "No", "Yes" };
const char* midiReceives[] = { "None", "CC", "NRPN", "CC & NRPN" };
const char* midiSends [] = { "None", "CC", "NRPN" };
const char* bootOptions [] = { "Default", "Bank", "Combo", "DX7" };
const char* encoderType [] = { "12", "24", "12i", "24i" };
const char* usbMidi[] = { "Off", "In", "In/Out" };
const char* operatorButton[] = { "New", "Old" };
const char* version[] = { PFM2_VERSION""CVIN_STRING};
const char* oledSaver[] = { "Off", "2m", "5m", "10m", "1h" };

const char* globalTuning[] = { "G" };
const char* inst1[] = { "Off", "1", "2", "3", "4", "12", "123", "1234", "seq", "rand", "cv3"};

const struct MidiConfig midiConfig[]  = {
#ifdef CVIN
        {
                "CV inst: ",
                "cvinst",
                11,
                inst1
        },
        {
                "CV A(33): ",
                "cva33",
                1023,
                0
        },
        {
                "CV A(81): ",
                "cva81",
                1023,
                0
        },
        {
                "CV GATE %: ",
                "cvgate",
                100,
                0
        },
#endif
        {
                "Usb Midi: ",
                "usbmidi",
                3,
                usbMidi
        },
        {
                "Midi ch. 1: ",
                "midi1",
                17,
                midiChannels
        },
        {
                "Midi ch. 2: ",
                "midi2",
                17,
                midiChannels
        },
        {
                "Midi ch. 3: ",
                "midi3",
                17,
                midiChannels
        },
        {
                "Midi ch. 4: ",
                "midi4",
                17,
                midiChannels
        },
        {
                "Current ch. : ",
                "midiglobal",
                17,
                eccChannels
        },
        {
                "Global ch. : ",
                "midicurrent",
                17,
                eccChannels
        },
        {
                "Midi thru: ",
                "miditrhu",
                2,
                yesNo
        },
        {
                "Receives: ",
                "midireceives",
                4,
                midiReceives
        },
        {
                "Send: ",
                "midisend",
                3,
                midiSends
        },
        {
                "Tuning: ",
                "tuning",
                101,
                globalTuning
        },
        {
                "Program ch: ",
                "programchange",
                2,
                yesNo
        },
        {
                "Boot: ",
                "boot",
                4,
                bootOptions
        },
        {
                "Op Buttons: ",
                "opbuttons",
                2,
                operatorButton
        },
        {
                "Rot. Enc.: ",
                "encoders",
                4,
                encoderType
        },
        {
                "Test Note: ",
                "testnote",
                127,
                0
        },
        {
                "Test Velocity: ",
                "testvelocity",
                127,
                0
        },
        {
                "Clock Led: ",
                "clockled",
                2,
                yesNo
        },
        {
                "Arp in Preset: ",
                "arpinpreset",
                2,
                yesNo
        },
        {
                "Oled Saver: ",
                "screensaver",
                5,
                oledSaver,
        },
        {
                "Unlinked edit: ",
                "unlinkededit",
                2,
                yesNo
        },
        {
                "Boot sound: ",
                "bootsound",
                2,
                yesNo
        },
        // Don't count in MIDICONFIG_SIZE
        {
                "V: ",
                "",
                1,
                version
        }

};

const struct MenuItem allMenus[]  = {
        {
                MAIN_MENU,
                "",
                true,
                3,
                {MENU_LOAD, MENU_SAVE, MENU_TOOLS }
        },
        // === LOAD
        {
                MENU_LOAD,
                "Load",
                true,
                4,
                {MENU_LOAD_SELECT_BANK, MENU_LOAD_SELECT_COMBO, MENU_LOAD_SELECT_DX7_BANK, MENU_LOAD_RANDOMIZER}
        },
        {
                MENU_LOAD_SELECT_BANK,
                "Bank",
                false,
                NUMBEROFPREENFMBANKS,
                {MENU_LOAD_SELECT_BANK_PRESET}
        },
        {
                MENU_LOAD_SELECT_COMBO,
                "Combo",
                false,
                NUMBEROFPREENFMCOMBOS,
                {MENU_LOAD_SELECT_COMBO_PRESET}
        },
        {
                MENU_LOAD_SELECT_DX7_BANK,
                "DX7",
                false,
                NUMBEROFDX7BANKS,
                {MENU_LOAD_SELECT_DX7_PRESET}
        },
        {
                MENU_LOAD_RANDOMIZER,
                "Rand",
                false,
                4,
                {MENU_DONE}
        },
        {
                MENU_LOAD_SELECT_BANK_PRESET,
                "",
                false,
                128,
                {MENU_DONE}
        },
        {
                MENU_LOAD_SELECT_COMBO_PRESET,
                "",
                false,
                128,
                {MENU_DONE}
        },
        {
                MENU_LOAD_SELECT_DX7_PRESET,
                "",
                false,
                32,
                {MENU_DONE}
        },
        // === SAVE
        {
                MENU_SAVE,
                "Save",
                true,
                4,
                {MENU_SAVE_SELECT_BANK, MENU_SAVE_SELECT_COMBO, MENU_DEFAULT_COMBO, MENU_SAVE_SYSEX_PATCH }
        },
        {
                MENU_SAVE_SELECT_BANK,
                "Bank",
                false,
                NUMBEROFPREENFMBANKS,
                {MENU_SAVE_SELECT_BANK_PRESET}
        },
        {
                MENU_SAVE_SELECT_COMBO,
                "Combo",
                false,
                NUMBEROFPREENFMCOMBOS,
                {MENU_SAVE_SELECT_COMBO_PRESET}
        },
        {
                MENU_SAVE_SELECT_BANK_PRESET,
                "",
                false,
                128,
                {MENU_SAVE_ENTER_PRESET_NAME}
        },
        {
                MENU_SAVE_SELECT_COMBO_PRESET,
                "",
                false,
                128,
                {MENU_SAVE_ENTER_COMBO_NAME}
        },
        {
                MENU_DEFAULT_COMBO,
                "Defl",
                true,
                2,
                {MENU_DEFAULT_COMBO_SAVE, MENU_DEFAULT_COMBO_RESET}
        },
        {
                MENU_DEFAULT_COMBO_SAVE,
                "Save",
                false,
                0,
                {MENU_DONE}
        },
        {
                MENU_DEFAULT_COMBO_RESET,
                "Clear",
                false,
                0,
                {MENU_DONE}
        },
        {
                MENU_SAVE_ENTER_PRESET_NAME,
                "Enter name",
                false,
                12,
                {MENU_DONE}
        },
        {
                MENU_SAVE_ENTER_COMBO_NAME,
                "Enter name",
                false,
                12,
                {MENU_DONE}
        },
        // === SYSEX
        {
                MENU_SAVE_SYSEX_PATCH,
                "SyX",
                false,
                0,
                {MENU_DONE}
        },
        // === DONE
        {
                MENU_DONE,
                "Done",
                false,
                0,
                {MENU_DONE}
        },
        // === CANCELED
        {
                MENU_CANCEL,
                "Canceled",
                false,
                0,
                {MENU_CANCEL}
        },
        // === CANCELED
        {
                MENU_ERROR,
                "Error",
                false,
                0,
                {MENU_CANCEL}
        },
        // == In progress
        {
                MENU_IN_PROGRESS,
                "",
                false,
                0,
                {MENU_IN_PROGRESS}
        },
        // ==  CONFIG
        {
                MENU_TOOLS,
                "Tools",
                true,
                4,
                {MENU_CONFIG_SETTINGS, MENU_CREATE, MENU_RENAME, MENU_SCALA}
        },
        {
                MENU_CONFIG_SETTINGS,
                "Set",
                false,
                MIDICONFIG_SIZE + 1,
                {MENU_CONFIG_SETTINGS_SAVE}
        },
        {
                MENU_CONFIG_SETTINGS_SAVE,
                "",
                false,
                MENU_CONFIG_SETTINGS,
                {MENU_DONE}
        },
        {
                MENU_CREATE,
                "Crea",
                true,
                2,
                { MENU_CREATE_BANK, MENU_CREATE_COMBO }
        },
        {
                MENU_CREATE_BANK,
                "Bank",
                false,
                8,
                {MENU_DONE}
        },
        {
                MENU_CREATE_COMBO,
                "Combo",
                false,
                8,
                {MENU_DONE}
        },
        {
                MENU_RENAME,
                "Ren",
                true,
                3,
                {MENU_RENAME_PATCH, MENU_RENAME_SELECT_BANK, MENU_RENAME_SELECT_COMBO}
        },
        {
                MENU_RENAME_PATCH,
                "Patch",
                false,
                12,
                {MENU_DONE}
        },
        {
                MENU_RENAME_SELECT_BANK,
                "Bank",
                false,
                32,
                {MENU_RENAME_BANK}
        },
        {
                MENU_RENAME_BANK,
                "",
                false,
                8,
                {MENU_DONE}
        },
        {
                MENU_RENAME_SELECT_COMBO,
                "Combo",
                false,
                8,
                {MENU_RENAME_COMBO}
        },
        {
                MENU_RENAME_COMBO,
                "",
                false,
                8,
                {MENU_DONE}
        },
        // SCALA
        {
                MENU_SCALA,
                "Scala",
                true,
                4,
                {MENU_SCALA_ENABLE, MENU_SCALA_FILENAME, MENU_SCALA_FREQUENCY, MENU_SCALA_MAPPING}
        },
        {
                MENU_SCALA_ENABLE,
                "Enbl",
                false,
                2,
                {MENU_DONE}
        },
        {
                MENU_SCALA_FILENAME,
                "Scal",
                false,
                NUMBEROFSCALASCALEFILES,
                {MENU_DONE}
        },
        {
                MENU_SCALA_FREQUENCY,
                "Frq",
                false,
                10000,
                {MENU_DONE}
        },
        {
                MENU_SCALA_MAPPING,
                "Map",
                false,
                2,
                {MENU_DONE}
        }

};


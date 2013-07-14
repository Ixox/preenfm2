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



const char* midiChannels [] = { "All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
const char* eccChannels [] = { "None", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
const char* yesNo [] = { "No", "Yes" };
const char* midiReceives[] = { "None", "CC", "NRPN", "CC & NRPN" };
const char* midiSends [] = { "None", "CC", "NRPN" };
const char* bootOptions [] = { "Default", "Bank1", "Bank2", "Bank3", "Bank4", "Combo","Internal" };
const char* encoderType [] = { "12", "24" };

const struct MidiConfig midiConfig[]  = {
        {
                "Midi ch. 1: ",
                17,
                midiChannels
        },
        {
                "Midi ch. 2: ",
                17,
                midiChannels
        },
        {
                "Midi ch. 3: ",
                17,
                midiChannels
        },
        {
                "Midi ch. 4: ",
                17,
                midiChannels
        },
        {
                "Midi thru: ",
                2,
                yesNo
        },
        {
                "Receives: ",
                4,
                midiReceives
        },
        {
                "Send: ",
                3,
                midiSends
        },
        {
                "R.Time SysEx: ",
                2,
                yesNo
        },
        {
                "Test Note: ",
                127,
                0
        },
        {
                "Test Velocity: ",
                127,
                0
        },
        {
                "Boot: ",
                7,
                bootOptions
        },
        {
                "Rot. Enc.: ",
                2,
                encoderType
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
                3,
                {MENU_LOAD_USER_SELECT_BANK, MENU_LOAD_DX7_SELECT_BANK, MENU_MIDI_SYSEX_GET}
        },
        {
                MENU_LOAD_USER_SELECT_BANK,
                "User",
                false,
                5,
                {MENU_LOAD_USER_SELECT_PRESET}
        },
        {
        		MENU_LOAD_DX7_SELECT_BANK,
                "DX7",
                false,
                300,
                {MENU_LOAD_DX7_SELECT_PRESET}
        },
        {
                MENU_LOAD_USER_SELECT_PRESET,
                "",
                false,
                128,
                {MENU_DONE}
        },
        {
        		MENU_LOAD_DX7_SELECT_PRESET,
                """",
                false,
                32,
                {MENU_DONE}
        },
        {
                MENU_MIDI_SYSEX_GET,
                "SysEx",
                false,
                0,
                {MENU_DONE}
        },
        // === SAVE
        {
                MENU_SAVE,
                "Save",
                true,
                3,
                {MENU_SAVE_SELECT_USER_BANK, MENU_DEFAULT_COMBO, MENU_MIDI_SYSEX_DUMP }
        },
        {
                MENU_SAVE_SELECT_USER_BANK,
                "User",
                false,
                5,
                {MENU_SAVE_SELECT_PRESET}
        },
        {
                MENU_SAVE_SELECT_PRESET,
                "",
                false,
                128,
                {MENU_SAVE_ENTER_NAME}
        },
        {
                MENU_DEFAULT_COMBO,
                "Deflt",
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
                MENU_SAVE_ENTER_NAME,
                "Enter name",
                false,
                12,
                {MENU_DONE}
        },
        // === SYSEX
        {
                MENU_MIDI_SYSEX_DUMP,
                "SysEx",
                true,
                2,
                {MENU_MIDI_PATCH_DUMP, MENU_MIDI_BANK_SELECT_DUMP}
        },
        {
                MENU_MIDI_PATCH_GET,
                "Patch",
                false,
                0,
                {MENU_DONE}
        },
        {
                MENU_MIDI_BANK_SELECT_DUMP,
                "Bank",
                false,
                5,
                {MENU_DONE}
        },
        {
                MENU_MIDI_PATCH_DUMP,
                "Patch",
                false,
                0,
                {MENU_DONE}
        },


        // === DONE
        {
                MENU_DONE,
                "",
                false,
                0,
                {MENU_DONE}
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
                3,
                {MENU_CONFIG_SETTINGS, MENU_RENAME_PATCH, MENU_FORMAT_BANK}
        },
        {
                MENU_CONFIG_SETTINGS,
                "Set",
                false,
                MIDICONFIG_SIZE,
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
                MENU_FORMAT_BANK,
                "Format",
                false,
                27,
                {MENU_DONE}
        },
        {
                MENU_RENAME_PATCH,
                "Name",
                false,
                12,
                {MENU_DONE}
        },

        // ==================== SAVE BANK
        {
                MENU_SAVE_BANK,
                "",
                false,
                4,
                {MENU_SAVE_BANK_CONFIRM}
        },
        {
                MENU_SAVE_BANK_CONFIRM,
                "",
                false,
                0,
                {MENU_DONE}
        }

};


/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <.> hosxe < a t > gmail.com)
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


#ifndef MENU_H_
#define MENU_H_

#define INTERNAL_LAST_BANK 71
#include "Storage.h"

enum {
    MIDICONFIG_USB = 0,
    MIDICONFIG_CHANNEL1,
    MIDICONFIG_CHANNEL2,
    MIDICONFIG_CHANNEL3,
    MIDICONFIG_CHANNEL4,
    MIDICONFIG_CURRENT_INSTRUMENT,
    MIDICONFIG_GLOBAL,
    MIDICONFIG_THROUGH,
    MIDICONFIG_RECEIVES,
    MIDICONFIG_SENDS,
    MIDICONFIG_PROGRAM_CHANGE,
    MIDICONFIG_BOOT_START,
    MIDICONFIG_OP_OPTION,
    MIDICONFIG_ENCODER,
    MIDICONFIG_TEST_NOTE,
    MIDICONFIG_TEST_VELOCITY,
    MIDICONFIG_LED_CLOCK,
    MIDICONFIG_ARPEGGIATOR_IN_PRESET,
    MIDICONFIG_OLED_SAVER,
    MIDICONFIG_UNLINKED_EDITING,
    MIDICONFIG_BOOT_SOUND,
    MIDICONFIG_SIZE
};

enum SynthEditMode {
    SYNTH_MODE_EDIT = 0,
    SYNTH_MODE_MENU
};

enum MidiConfigUSB {
    USBMIDI_OFF = 0,
    USBMIDI_IN,
    USBMIDI_IN_AND_OUT
};

enum MenuState {
    MAIN_MENU = 0,
    MENU_LOAD,
    MENU_SAVE_SELECT_BANK,
    MENU_SAVE_SELECT_BANK_PRESET,
    MENU_SAVE_SELECT_COMBO,
    MENU_SAVE_SELECT_COMBO_PRESET,
    MENU_LOAD_SELECT_BANK,
    MENU_LOAD_SELECT_BANK_PRESET,
    MENU_LOAD_SELECT_DX7_BANK,
    MENU_LOAD_RANDOMIZER,
    MENU_LOAD_SELECT_DX7_PRESET,
    MENU_LOAD_SELECT_COMBO,
    MENU_LOAD_SELECT_COMBO_PRESET,
    MENU_SAVE,
    MENU_SAVE_ENTER_PRESET_NAME,
    MENU_SAVE_ENTER_COMBO_NAME,
    MENU_SAVE_SYSEX_PATCH,
    MENU_CONFIG_SETTINGS,
    MENU_CONFIG_SETTINGS_SAVE,
    MENU_MIDI_BANK,
    MENU_MIDI_BANK_GET,
    MENU_MIDI_PATCH,
    MENU_MIDI_PATCH_GET,
    MENU_DONE,
    MENU_CANCEL,
    MENU_IN_PROGRESS,
    MENU_ERROR,
    MENU_TOOLS,
    MENU_CREATE,
    MENU_CREATE_BANK,
    MENU_CREATE_COMBO,
    MENU_DEFAULT_COMBO,
    MENU_DEFAULT_COMBO_SAVE,
    MENU_DEFAULT_COMBO_RESET,
    MENU_RENAME,
    MENU_RENAME_PATCH,
    MENU_RENAME_SELECT_BANK,
    MENU_RENAME_BANK,
    MENU_RENAME_SELECT_COMBO,
    MENU_RENAME_COMBO,
    MENU_SCALA,
    MENU_SCALA_ENABLE,
    MENU_SCALA_FILENAME,
    MENU_SCALA_FREQUENCY,
    MENU_SCALA_MAPPING,
    LAST_MENU
} ;



struct MenuItem {
    MenuState menuState;
    const char* name;
    bool hasSubMenu;
    short maxValue;
    MenuState subMenu[4];
};

struct Randomizer {
    // lcd->print("OpFr EnvT IM   Modl");
    char Oper;
    char EnvT;
    char IM;
    char Modl;
};


struct FullState {
    SynthEditMode synthMode;
    int menuSelect;
    unsigned char previousMenuSelect;
    const MenuItem* currentMenuItem;
    char name[13];

    unsigned char firstMenu;
    unsigned char loadWhat;
    unsigned char saveWhat;
    unsigned char toolsWhat;
    unsigned char scalaWhat;
    unsigned char  menuPosition[5];
    char  midiConfigValue[MIDICONFIG_SIZE + 1];

    unsigned char preenFMBankNumber;
    unsigned char preenFMPresetNumber;
    const struct PFM2File* preenFMBank;

    unsigned char preenFMComboNumber;
    unsigned char preenFMComboPresetNumber;
    const struct PFM2File* preenFMCombo;

    short dx7BankNumber;
    unsigned char dx7PresetNumber;
    const struct PFM2File* dx7Bank;

    struct ScalaScaleConfig scalaScaleConfig;

    struct Randomizer randomizer;
};

struct MidiConfig {
    const char* title;
    const char* nameInFile;
    unsigned char maxValue;
    const char** valueName;
};


extern const struct MenuItem allMenus[];
extern const struct MidiConfig midiConfig[];

class MenuItemUtil {
public:
    static const MenuItem* getMenuItem(MenuState ms) {
        const MenuItem* item = &allMenus[0];
        int cpt = 0;
        while (item->menuState != LAST_MENU) {
            if (item->menuState == ms) {
                return item;
            }
            cpt ++;
            item = &allMenus[cpt];
        }
        return 0;
    }

    static const MenuItem* getParentMenuItem(MenuState ms) {
        // MENU_DONE exception -> return itself to block back button
        if (ms == MENU_DONE || ms == MENU_CANCEL || ms == MENU_ERROR) {
            return getMenuItem(ms);
        }
        const MenuItem* item = &allMenus[0];
        int cpt = 0;
        while (item->menuState != LAST_MENU) {
            for (int k=0; k<4; k++) {
                if (item->subMenu[k] == ms) {
                    return item;
                }
            }
            cpt ++;
            item = &allMenus[cpt];
        }
        return 0;
    }

    static int getParentMenuSelect(MenuState ms) {
        const MenuItem* item = &allMenus[0];
        int cpt = 0;
        while (item->menuState != LAST_MENU) {
            for (int k=0; k<4; k++) {
                if (item->subMenu[k] == ms) {
                    return k;
                }
            }
            cpt ++;
            item = &allMenus[cpt];
        }
        return 0;
    }
};

#endif /* MENU_H_ */

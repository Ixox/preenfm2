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


enum {
    MIDICONFIG_CHANNEL1 = 0,
    MIDICONFIG_CHANNEL2,
    MIDICONFIG_CHANNEL3,
    MIDICONFIG_CHANNEL4,
    MIDICONFIG_THROUGH,
    MIDICONFIG_RECEIVES,
    MIDICONFIG_SENDS,
    MIDICONFIG_REALTIME_SYSEX,
    MIDICONFIG_TEST_NOTE,
    MIDICONFIG_TEST_VELOCITY,
    MIDICONFIG_BOOT_START,
    MIDICONFIG_ECHANNEL,
    MIDICONFIG_ECC1,
    MIDICONFIG_ECC2,
    MIDICONFIG_ECC3,
    MIDICONFIG_ECC4,
    MIDICONFIG_ENCODER,
    MIDICONFIG_SIZE,
    // To remove later
};

enum SynthEditMode {
    SYNTH_MODE_EDIT = 0,
    SYNTH_MODE_MENU
};

enum MenuState {
    MAIN_MENU = 0,
    MENU_LOAD,
    MENU_SAVE_SELECT_USER_BANK,
    MENU_SAVE_SELECT_PRESET,
    MENU_LOAD_USER_SELECT_BANK,
    MENU_LOAD_USER_SELECT_PRESET,
    MENU_LOAD_DX7_SELECT_BANK,
    MENU_LOAD_DX7_SELECT_PRESET,
    MENU_SAVE,
    MENU_SAVE_ENTER_NAME,
    MENU_MIDI_SYSEX_DUMP,
    MENU_MIDI_SYS_EX,
    MENU_MIDI_SYSEX_GET,
    MENU_CONFIG_SETTINGS,
    MENU_CONFIG_SETTINGS_SAVE,
    MENU_MIDI_BANK,
    MENU_MIDI_BANK_GET,
    MENU_MIDI_BANK_DUMP,
    MENU_MIDI_BANK_SELECT_DUMP,
    MENU_MIDI_PATCH,
    MENU_MIDI_PATCH_GET,
    MENU_MIDI_PATCH_DUMP,
    MENU_DONE,
    MENU_IN_PROGRESS,
    MENU_TOOLS,
    MENU_FORMAT_BANK,
    MENU_SAVE_BANK,
    MENU_SAVE_BANK_CONFIRM,
    MENU_DEFAULT_COMBO,
    MENU_DEFAULT_COMBO_SAVE,
    MENU_DEFAULT_COMBO_RESET,
    MENU_RENAME_PATCH,
    LAST_MENU
};



struct MenuItem {
    MenuState menuState;
    const char* name;
    bool hasSubMenu;
    short maxValue;
    MenuState subMenu[4];
};



struct FullState {
    SynthEditMode synthMode;
    int menuSelect;
    unsigned char previousMenuSelect;
    unsigned char bankNumber;
    unsigned char dx7BankNumber;
    unsigned char presetNumber;
    unsigned char dx7PresetNumber;
    unsigned char internalPresetNumber;
    const MenuItem* currentMenuItem;
    char name[13];
    unsigned char loadWhat;
    unsigned char saveWhat;
    unsigned char firstMenu;
    unsigned char  menuPosition[5];
    char  midiConfigValue[MIDICONFIG_SIZE];
    const char  *dx7BankName;
};

struct MidiConfig {
    const char* title;
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
        if (ms == MENU_DONE) {
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

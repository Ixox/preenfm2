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

#ifndef PRESETUTIL_H_
#define PRESETUTIL_H_

#define EEPROM_CONFIG_CHECK 86
#define EEPROM_DEFPATCH_CHECK 116


#define EEPROM1_ID 0b1010000;
#define EEPROM2_ID 0b1010001;


#define BANKSIZE 16384
#define BLOCKSIZE 4096

#define PART1_SIZE 128
#define PART2_SIZE 64

#define PATCH_SIZE PART1_SIZE+PART2_SIZE


class SynthState;
struct OneSynthParams;



// is included by SynthState so cannot be SynthStateAware...
// have to implement its own get/set synthState

class PresetUtil  {
public:
    PresetUtil();
    ~PresetUtil();

    static void setSynthState(SynthState* synthState);

    static void dumpPatch();
    static void dumpLine(const char *enums1[], int a, const char *enums2[], int b, const char *enums3[], int c, const char *enums4[], int d) ;

    static void createEmptyBanks();
    static void resetConfigAndSaveToEEPROM();
    static void saveConfigToEEPROM();
    static void loadConfigFromEEPROM();
    static void checkReadEEPROM();

    static void sendBankToSysex(int bankNumber);
    static void sendCurrentPatchToSysex();
    static void sendParamsToSysex(unsigned char* params);
    static int  readSysex(bool patchAllowed, bool bankAllowed);
    static int  readSysexPatch(unsigned char* params);
    static int  readSysexBank();


    static int  getNextMidiByte();
    static int  fillBufferWithNextMidiByte();
    static void copySynthParams(char* source, char* dest);


    static void convertSynthStateToCharArray(OneSynthParams* params, unsigned char* chars);
    static void convertCharArrayToSynthState(unsigned char* chars, OneSynthParams* params);

    static void sendNrpn(struct MidiEvent cc);
    static void sendCurrentPatchAsNrpns(int timbre);

private:
    static char readName[13];
    static SynthState * synthState;

    static unsigned char midiBuffer[1024];
    static int midiBufferWriteIndex;
    static int midiBufferReadIndex;
};

#endif /* PRESETUTIL_H_ */

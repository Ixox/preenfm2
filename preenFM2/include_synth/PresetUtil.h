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

#ifndef PRESETUTIL_H_
#define PRESETUTIL_H_





class SynthState;
class Storage;
struct OneSynthParams;
struct ParameterDisplay;


// is included by SynthState so cannot be SynthStateAware...
// have to implement its own get/set synthState

class PresetUtil  {
public:
    PresetUtil();
    ~PresetUtil();

    static void setSynthState(SynthState* synthState);
    static void setStorage(Storage* storage);

    static void dumpPatch();
    static void dumpLine(const char *enums1[], int a, const char *enums2[], int b, const char *enums3[], int c, const char *enums4[], int d) ;

    static void resetConfigAndSaveToEEPROM();
    static unsigned short getShortFromParamFloat(int row, int encoder, float value);
    static float getParamFloatFromShort(int row, int encoder, short value);

    static void saveConfigToEEPROM();
    static void loadConfigFromEEPROM();

    static void sendBankToSysex(int bankNumber);
    static void sendCurrentPatchToSysex();
    static void sendSysexByte(unsigned char byte);
    static void sendParamsToSysex(unsigned char* params);
    static int  readSysex(bool patchAllowed, bool bankAllowed);
    static int  readSysexPatch(unsigned char* params);
    static int  readSysexBank();


    static int  getNextMidiByte();
    static void copySynthParams(char* source, char* dest);


    static void convertSynthStateToCharArray(OneSynthParams* params, unsigned char* chars);
    static void convertCharArrayToSynthState(unsigned char* chars, OneSynthParams* params);

    static void sendNrpn(struct MidiEvent cc);
    static void sendCurrentPatchAsNrpns(int timbre);
    static void copyBank(int source, int dest);
private:
    static SynthState * synthState;
    static Storage * storage;
};

#endif /* PRESETUTIL_H_ */

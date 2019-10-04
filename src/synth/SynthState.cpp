/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <dot> hosxe (at) g m a i l <dot> com)
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

#include "stm32f4xx_rng.h"
#include "SynthState.h"
#include "Hexter.h"

#include "LiquidCrystal.h"
extern LiquidCrystal      lcd;

#include "Synth.h"
extern Synth synth;

#define NULL 0
// FLASH :  __attribute__ ((section (".USER_FLASH")))
// Ex : const char* nullNames [] __attribute__ ((section (".USER_FLASH")))= {};
// DISPLAY structures
const char* allChars  = "_ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789 .,;:<>&*$";

const char* nullNames []= {};
const unsigned char* nullNamesOrder = NULL;
const char* algoNames [] = { "alg1", "alg2", "alg3", "alg4", "alg5", "alg6", "alg7", "alg8", "alg9",
        "al10", "al11", "al12", "al13", "al14", "al15", "al16", "al17", "al18", "al19",
        "al20", "al21", "al22", "al23", "al24", "al25", "al26", "al27", "al28"  };

struct ParameterRowDisplay engine1ParameterRow  = {
        "Engine" ,
        { "Algo", "Velo", "Voic", "Glid" },
        {
                {ALGO1, ALGO_END-1, ALGO_END, DISPLAY_TYPE_STRINGS, algoNames, nullNamesOrder, nullNamesOrder},
                {0, 16, 17, DISPLAY_TYPE_INT, nullNames,nullNamesOrder, nullNamesOrder },
                {0, 16, 17, DISPLAY_TYPE_VOICES, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 10, 11, DISPLAY_TYPE_INT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};


const char* clockName[] = { "Off ", "Int ", "Ext " };
const char* dirName[] = { "Up  ", "Down", "U&D ", "Play", "Rand", "Chrd", "RtUp", "RtDn", "RtUD", "ShUp", "ShDn", "ShUD" };

struct ParameterRowDisplay engineArp1ParameterRow  = {
        "Arpeggiator" ,
        { "Clk ", "BPM ", "Dire" , "Octv"},
        {
                {0, 2, 3, DISPLAY_TYPE_STRINGS, clockName, nullNamesOrder, nullNamesOrder },
                {10, 240, 231, DISPLAY_TYPE_INT, nullNames,nullNamesOrder, nullNamesOrder },
                {0, 11, 12, DISPLAY_TYPE_STRINGS, dirName, nullNamesOrder, nullNamesOrder },
                {1, 3, 3, DISPLAY_TYPE_INT, nullNames,nullNamesOrder, nullNamesOrder }
        }
};

//   192, 144, 96, 72, 64, 48, 36, 32, 24, 16, 12, 8, 6, 4, 3, 2, 1

const char* divNames[] = { "2/1 ", "3/2 ", "1/1 ", "3/4 ", "2/3 ", "1/2 ", "3/8 ", "1/3 ", "1/4 ", "1/6 ", "1/8 ",
        "1/12", "1/16", "1/24", "1/32", "1/48", "1/96"};
const char* activeName[] = { "Off ", "On  " };

const char* patternName[] = { "1   ", "2   ", "3   ", "4   ", "5   ", "6   ", "7   ", "8   ", "9   ",
        "10   ", "11  ", "12  ", "13  ", "14  ", "15  ", "16  ", "17  ", "18  ", "19  ",
        "20   ", "21  ", "22  ",
        "Usr1", "Usr2", "Usr3", "Usr4" };

struct ParameterRowDisplay engineArp2ParameterRow  = {
        "Arpeggiator",
        { "Ptrn", "Divi", "Dura", "Latc" },
        {
                {0, ARPEGGIATOR_PATTERN_COUNT-1, ARPEGGIATOR_PATTERN_COUNT, DISPLAY_TYPE_STRINGS, patternName, nullNamesOrder, nullNamesOrder},
                {0, 16, 17, DISPLAY_TYPE_STRINGS, divNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 17, DISPLAY_TYPE_STRINGS, divNames, nullNamesOrder, nullNamesOrder },
                {0, 1, 2, DISPLAY_TYPE_STRINGS, activeName, nullNamesOrder, nullNamesOrder },
        }
};

struct ParameterRowDisplay engineArpPatternRow = {
        "Pattern ",
        { "    ", "    ", "    ", "    " },
        {
                {0, 0, 0, DISPLAY_TYPE_ARP_PATTERN, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 0, 0, DISPLAY_TYPE_ARP_PATTERN, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 0, 0, DISPLAY_TYPE_ARP_PATTERN, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 0, 0, DISPLAY_TYPE_ARP_PATTERN, nullNames, nullNamesOrder, nullNamesOrder },
        }
};

struct ParameterRowDisplay engineIM1ParameterRow = {
        "Modulation" ,
        { "IM1 ", "v   ", "IM2 ", "v   "},
        {
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay engineIM2ParameterRow = {
        "Modulation" ,
        { "IM3 ", "v   ", "IM4 ", "v   "},
        {
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay engineIM3ParameterRow = {
        "Modulation" ,
        { "IM5 ", "v   ", "IM6 ", "v   "},
        {
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};



struct ParameterRowDisplay engineMix1ParameterRow = {
        "Mixer" ,
        { "Mix1", "Pan1", "Mix2", "Pan2" },
        {
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
        }
};

struct ParameterRowDisplay engineMix2ParameterRow = {
        "Mixer" ,
        { "Mix3", "Pan3", "Mix4", "Pan4" },
        {
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay engineMix3ParameterRow = {
        "Mixer" ,
        { "Mix5", "Pan5", "Mix6", "Pan6" },
        {
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {-1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};


const char* fxName []=  { "Off ", "Mix ", "LP  ", "HP  ", "Bass", "BP  ", "Crsh" , "LP2 ", "HP2 ", "BP2 ", "Lp3 ", "Hp3 ", "Bp3 ", "Peak", "Notc", "Bell", "LowS", "HigS", "LpHp", "BpDs", "LPws", "Tilt", "Pann", "Sat ", "Sigm", "Fold", "Wrap", "Xor ", "Txr1", "Txr2", "LPx1", "LPx2", "LpSn", "HpSn", "Not3", "Ap3 "} ;

struct ParameterRowDisplay effectParameterRow = {
        "Filter" ,
        { "Type", "    ", "    ", "Gain" },
        {
                {0, FILTER_LAST - 1, FILTER_LAST, DISPLAY_TYPE_STRINGS, fxName, nullNamesOrder, nullNamesOrder },
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 2, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct FilterRowDisplay filterRowDisplay[FILTER_LAST] = {
        { NULL, NULL, "Gain" },
        { "Pan ", NULL, "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "LoFr", "Boos", "Gain" },
        { "Freq", "Q   ", "Gain" },
        { "Samp", "Bits", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Q   ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Amp ", "Gain" },
        { "Freq", "Amp ", "Gain" },
        { "Freq", "Amp ", "Gain" },
        { "Freq", "Res ", "Gain" },        
        { "Freq", "Res ", "Gain" },
        { "Freq", "Mix ", "Gain" },
        { "Freq", "Mod ", "Gain" },
        { "Pos ", "Sprd", "Gain" },
        { "Thrs", "Tone ", "Gain" },
        { "Driv", "Tone ", "Gain" },
        { "Driv", "Tone ", "Gain" },
        { "Driv", "Tone", "Gain" },
        { "Thrs", "Tone ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Res ", "Gain" },
        { "Freq", "Fold", "Gain" },
        { "Freq", "Fold", "Gain" },
        { "Pos ", "Freq", "Gain" },
        { "Pos ", "Freq", "Gain" },
        { "Freq", "Sprd", "Gain" },
        { "Freq", "Sprd", "Gain" }
};


const char* oscShapeNames []=  {"sin ", "saw ", "squa", "s^2 ", "szer", "spos", "rand", "off ", "Usr1", "Usr2", "Usr3", "Usr4", "Usr5", "Usr6"} ;
const unsigned char  oscShapeNamesOrder[] =        { 7, 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13};
//                                                 { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const unsigned char  oscShapeNamesPosition[] =     { 1, 2, 3, 4, 5, 6, 7, 0, 8, 9, 10, 11, 12, 13};


const char* oscTypeNames [] = { "keyb", "fixe", "kehz"};
const unsigned char  oscTypeNamesOrder[] =        { 0, 2 ,1};
const unsigned char  oscTypeNamesPosition[] =     { 0, 2, 1};


struct ParameterRowDisplay oscParameterRow = {
        "Oscillator",
        { "Shap", "FTyp", "Freq", "FTun" },
        {
                { OSC_SHAPE_SIN, OSC_SHAPE_LAST -1, OSC_SHAPE_LAST, DISPLAY_TYPE_STRINGS, oscShapeNames, oscShapeNamesOrder, oscShapeNamesPosition},
                { OSC_FT_KEYBOARD, OSC_FT_KEYHZ, 3, DISPLAY_TYPE_STRINGS, oscTypeNames, oscTypeNamesOrder, oscTypeNamesPosition},
                { 0, 16, 193, DISPLAY_TYPE_FLOAT_OSC_FREQUENCY ,  nullNames, nullNamesOrder, nullNamesOrder },
                { -9, 9, 1801, DISPLAY_TYPE_FLOAT_OSC_FREQUENCY,  nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay envParameterRow1 = {
        "Env A",
        { "Attk", "lv  ", "Deca", "lv  " },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay envParameterRow2 = {
        "Env B",
        { "Sust", "lv  ", "Rele", "lv  " },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay lfoEnvParameterRow = {
        "Free Env",
        { "Attk", "Deca", "Sust", "Rele" },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* lofEnv2Loop [] = { "No  ", "Sile", "Attk"};
struct ParameterRowDisplay lfoEnv2ParameterRow = {
        "Free Env",
        { "Sile", "Attk", "Deca", "Loop" },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { LFO_ENV2_NOLOOP, LFO_ENV2_LOOP_ATTACK, 3, DISPLAY_TYPE_STRINGS, lofEnv2Loop, nullNamesOrder, nullNamesOrder }
        }
};

const char* matrixSourceNames [] = { "None", "lfo1", "lfo2", "lfo3", "env1", "env2", "seq1", "seq2",
        "ModW", "PitB", "AftT",  "Velo", "Not1", "p1  ", "p2  ", "p3  ", "p4  ", "Not2", "Brth" } ;
const unsigned char  matrixSourceOrder[] =        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 18, 13, 14, 15, 16 };
const unsigned char  matrixSourcePosition[] =     { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 18, 13, 14};


const char* matrixDestNames [] = {
        "None", "Gate", "IM1 ", "IM2 ", "IM3 ", "IM4 ", "IM* ",
        "Mix1", "Pan1", "Mix2", "Pan2", "Mix3", "Pan3", "Mix4", "Pan4", "Mix*", "Pan*",
        "o1Fq", "o2Fq", "o3Fq", "o4Fq", "o5Fq", "o6Fq", "o*Fq",
        /*24*/ "Att1", "Att2", "Att3", "Att4", "Att5", "Att6",
        /*30*/ "AttC", "RelC",
        /*32*/ "mx01", "mx02", "mx03", "mx04",
        /*36*/ "l1Fq", "l2Fq", "l3Fq", "e2si", "s1ga", "s2ga",
        /*42*/ "Fltr",
        /*43*/ "o*Fh", "DecC",
        /*45*/ "AttM", "DecM", "RelM"

} ;
const unsigned char  matrixTargetOrder[] =        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 43, 24, 25, 26, 27, 28, 29, 30, 45, 44, 46, 31, 47, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42 };
//   Order                                        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44 };
const unsigned char  matrixTargetPosition[] =     { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30, 31, 35, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 24, 33, 32, 34, 36 };


struct ParameterRowDisplay matrixParameterRow = {
        "Matrix",
        { "Srce", "Mult", "Dest", "    " },
        {
                { MATRIX_SOURCE_NONE, MATRIX_SOURCE_MAX-1, MATRIX_SOURCE_MAX, DISPLAY_TYPE_STRINGS, matrixSourceNames, matrixSourceOrder, matrixSourcePosition},
                { -10, 10, 2001, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                // We removed 8 destination target in firmware 2.0
                { DESTINATION_NONE, DESTINATION_MAX -1, DESTINATION_MAX, DISPLAY_TYPE_STRINGS, matrixDestNames, matrixTargetOrder, matrixTargetPosition},
                { 0, 0, 0, DISPLAY_TYPE_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* lfoOscMidiClock[] =  { "M/16", "MC/8", "MC/4", "MC/2", "MClk", "MC*2", "MC*3", "MC*4", "MC*8"};
const char* lfoShapeNames [] =  { "Sin ", "Saw ",  "Tria","Squa", "Rand" } ;


struct ParameterRowDisplay lfoParameterRow = {
        "LFO",
        { "Shap", "Freq", "Bias", "KSyn" },
        {
                { LFO_SIN, LFO_TYPE_MAX-1, LFO_TYPE_MAX, DISPLAY_TYPE_STRINGS,  lfoShapeNames, nullNamesOrder, nullNamesOrder},
                { 0, LFO_FREQ_MAX + .9f, LFO_FREQ_MAX_TIMES_10 +1 +9, DISPLAY_TYPE_FLOAT_LFO_FREQUENCY, nullNames, nullNamesOrder, nullNamesOrder },
                { -1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { -0.01f, 16.0f, 1602, DISPLAY_TYPE_LFO_KSYN, nullNames, nullNamesOrder, nullNamesOrder }
        }
};



const char* lfoSeqMidiClock[] =  { "MC/4", "MC/2", "MC  ", "MC*2", "MC*4" };

struct ParameterRowDisplay lfoStepParameterRow = {
        "Step Seq",
        { "Bpm ", "Gate", "    ", "    " },
        {
                { 10 ,245, 236, DISPLAY_TYPE_STEP_SEQ_BPM, nullNames, nullNamesOrder, nullNamesOrder},
                { 0 , 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_STEP_SEQ1, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 0, 0, DISPLAY_TYPE_STEP_SEQ2, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay performanceParameterRow = {
        "   -Performance-",
        { " p1 ", " p2 ", " p3 ", " p4 " },
        {
                { -1 , 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { -1 , 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { -1 , 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { -1 , 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder}
        }
};

struct ParameterRowDisplay lfoPhaseParameterRow = {
        "LFO Phase",
        { "Lfo1", "Lfo2", "Lfo3", "    " },
        {
                { 0 , 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { 0 , 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { 0 , 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* midiNoteCurves[] =  { "Flat", "+Lin", "+Ln8", "+Exp", "-Lin", "-Ln8", "-Exp" };


struct ParameterRowDisplay midiNote1ParameterRow = {
        "Note1 Midi Scaling",
        { "Befo", "Brk ", "Afte", "    " },
        {
                { MIDI_NOTE_CURVE_FLAT, MIDI_NOTE_CURVE_MAX-1, MIDI_NOTE_CURVE_MAX, DISPLAY_TYPE_STRINGS,  midiNoteCurves, nullNamesOrder, nullNamesOrder},
                { 0 , 127, 128, DISPLAY_TYPE_INT, nullNames, nullNamesOrder, nullNamesOrder},
                { MIDI_NOTE_CURVE_FLAT, MIDI_NOTE_CURVE_MAX-1, MIDI_NOTE_CURVE_MAX, DISPLAY_TYPE_STRINGS,  midiNoteCurves, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay midiNote2ParameterRow = {
        "Note2 Midi Scaling",
        { "Befo", "Brk ", "Afte", "    " },
        {
                { MIDI_NOTE_CURVE_FLAT, MIDI_NOTE_CURVE_MAX-1, MIDI_NOTE_CURVE_MAX, DISPLAY_TYPE_STRINGS,  midiNoteCurves, nullNamesOrder, nullNamesOrder},
                { 0 , 127, 128, DISPLAY_TYPE_INT, nullNames, nullNamesOrder, nullNamesOrder},
                { MIDI_NOTE_CURVE_FLAT, MIDI_NOTE_CURVE_MAX-1, MIDI_NOTE_CURVE_MAX, DISPLAY_TYPE_STRINGS,  midiNoteCurves, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct AllParameterRowsDisplay allParameterRows = {
        {
                &engine1ParameterRow,
                &engineIM1ParameterRow,
                &engineIM2ParameterRow,
                &engineIM3ParameterRow,
                &engineMix1ParameterRow,
                &engineMix2ParameterRow,
                &engineMix3ParameterRow,
                &engineArp1ParameterRow,
                &engineArp2ParameterRow,
                &engineArpPatternRow,
                &effectParameterRow,
                &oscParameterRow,
                &oscParameterRow,
                &oscParameterRow,
                &oscParameterRow,
                &oscParameterRow,
                &oscParameterRow,
                &envParameterRow1,
                &envParameterRow2,
                &envParameterRow1,
                &envParameterRow2,
                &envParameterRow1,
                &envParameterRow2,
                &envParameterRow1,
                &envParameterRow2,
                &envParameterRow1,
                &envParameterRow2,
                &envParameterRow1,
                &envParameterRow2,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &matrixParameterRow,
                &performanceParameterRow,
                &lfoParameterRow,
                &lfoParameterRow,
                &lfoParameterRow,
                &lfoPhaseParameterRow,
                &lfoEnvParameterRow,
                &lfoEnv2ParameterRow,
                &lfoStepParameterRow,
                &lfoStepParameterRow,
                &midiNote1ParameterRow,
                &midiNote2ParameterRow
        }
};




SynthState::SynthState() {
    engineRow =  ROW_ENGINE;
    // operator works for both osc and env
    oscillatorRow  = 0;
    matrixRow = ROW_MATRIX1;
    lfoRow    = ROW_LFOOSC1;
    operatorNumber = 0;
    operatorView = 0;

    // First default preset
    fullState.synthMode = SYNTH_MODE_EDIT;
    fullState.preenFMBankNumber = 0;
    fullState.preenFMPresetNumber = 0;
    fullState.preenFMComboNumber = 0;
    fullState.preenFMComboPresetNumber = 0;
    fullState.dx7BankNumber = 0;
    fullState.dx7PresetNumber = 0;
    fullState.scalaScaleConfig.scalaFile = NULL;
    fullState.scalaScaleConfig.scalaEnabled = false;
    fullState.scalaScaleConfig.keyboardMapping = true;
    // https://en.wikipedia.org/wiki/C_(musical_note)
    // Frequency of note 60 C4 :
    fullState.scalaScaleConfig.scalaFreq = 261.6f;
    fullState.loadWhat = 0;
    fullState.saveWhat = 0;
    fullState.toolsWhat = 0;
    fullState.scalaWhat = 0;
    fullState.midiConfigValue[MIDICONFIG_USB] = 2;
	fullState.midiConfigValue[MIDICONFIG_GLOBAL] = 0;
	fullState.midiConfigValue[MIDICONFIG_CURRENT_INSTRUMENT] = 0;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL1] = 1;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL2] = 2;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL3] = 3;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL4] = 4;
    fullState.midiConfigValue[MIDICONFIG_THROUGH] = 0;
    fullState.midiConfigValue[MIDICONFIG_RECEIVES] = 3;
    fullState.midiConfigValue[MIDICONFIG_SENDS] = 1;
    fullState.midiConfigValue[MIDICONFIG_PROGRAM_CHANGE] = 1;
    fullState.midiConfigValue[MIDICONFIG_BOOT_START] = 0;
    fullState.midiConfigValue[MIDICONFIG_TEST_NOTE] = 60;
    fullState.midiConfigValue[MIDICONFIG_TEST_VELOCITY] = 120;
    fullState.midiConfigValue[MIDICONFIG_ENCODER] = 0;
    fullState.midiConfigValue[MIDICONFIG_OP_OPTION] = 0;
    fullState.midiConfigValue[MIDICONFIG_LED_CLOCK] = 1;
    fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET] = 0;
    fullState.midiConfigValue[MIDICONFIG_OLED_SAVER] = 0;
    fullState.midiConfigValue[MIDICONFIG_UNLINKED_EDITING] = 0;
    fullState.midiConfigValue[MIDICONFIG_BOOT_SOUND] = 0;
    fullState.firstMenu = 0;
    // Init randomizer values to 1
    fullState.randomizer.Oper = 1;
    fullState.randomizer.EnvT = 1;
    fullState.randomizer.IM = 1;
    fullState.randomizer.Modl = 1;

    for (int k=0; k<12; k++) {
        fullState.name[k] = 0;
    }

    // edit with timbre 0
    currentTimbre = 0;
    currentRow = 0;
    if ( fullState.midiConfigValue[MIDICONFIG_UNLINKED_EDITING] ) {
        for (int t=0; t < NUMBER_OF_TIMBRES; ++t)
            lastRowForTimbre[t] = 0;
    } else {
        for (int t=0; t < NUMBER_OF_TIMBRES; ++t)
            lastRowForTimbre[t] = -1;
    }
    stepSelect[0] = 0;
    stepSelect[1] = 0;
    patternSelect = 0;

    isPlayingNote = false;

    for (int row= 0; row <NUMBER_OF_ROWS; row++) {
        for (int param=0; param<NUMBER_OF_ENCODERS; param++) {
            struct ParameterDisplay* paramDisplay = &(allParameterRows.row[row]->params[param]);
            if (paramDisplay->numberOfValues > 1.0) {
                paramDisplay->incValue = ((paramDisplay->maxValue - paramDisplay->minValue) / (paramDisplay->numberOfValues - 1.0f));
            } else {
                paramDisplay->incValue = 0.0f;
            }
        }
    }
}

void SynthState::encoderTurnedForStepSequencer(int row, int encoder, int ticks) {
    int whichStepSeq = row - ROW_LFOSEQ1;
    StepSequencerSteps * seqSteps = &((StepSequencerSteps * )(&params->lfoSteps1))[whichStepSeq];

    if (encoder == 2) {
        int oldPos = stepSelect[whichStepSeq];
        stepSelect[whichStepSeq] += (ticks>0? 1 : -1);

        if (stepSelect[whichStepSeq]>15) {
            stepSelect[whichStepSeq] = 0;
        } else if (stepSelect[whichStepSeq]<0) {
            stepSelect[whichStepSeq] = 15;
        }

        propagateNewParamValue(currentTimbre, row, encoder, (ParameterDisplay*)NULL, oldPos, stepSelect[whichStepSeq]);

    } else if (encoder == 3) {
        char *step = &seqSteps->steps[stepSelect[whichStepSeq]];
        int oldValue = (int)(*step);

        (*step) += ticks;

        if ((*step)>15) {
            (*step) = 15;
        } else if ((*step)<0) {
            (*step) = 0;
        }

        propagateNewParamValue(currentTimbre, row, encoder, (ParameterDisplay*)NULL, oldValue, (int)(*step));
    }
}

void SynthState::encoderTurnedForArpPattern(int row, int encoder, int ticks) {
    if (encoder == 0) {
        // Encoder 0: move cursor
        int oldPos = patternSelect;
        patternSelect += (ticks > 0? 1 : -1);

        if (patternSelect>15) {
            patternSelect = 0;
        } else if (patternSelect<0) {
            patternSelect = 15;
        }

        propagateNewParamValue(currentTimbre, row, encoder, (ParameterDisplay*)NULL, oldPos, patternSelect);

    } else {
        // Change value(s)
        arp_pattern_t pattern = params->engineArpUserPatterns.patterns[ (int)params->engineArp2.pattern - ARPEGGIATOR_PRESET_PATTERN_COUNT ];
        const uint16_t oldMask = ARP_PATTERN_GETMASK( pattern );
        uint16_t newMask = oldMask;

        uint16_t bitsToModify;
        switch ( encoder ) {
        case 3: bitsToModify = 0x1 << patternSelect; break;	   // modify single note
        case 2: bitsToModify = 0x1111 << (patternSelect & 3); break; // modify all
        case 1: bitsToModify = 0xf << ((patternSelect>>2)<<2); break; // modify entire bar
        }
        if (ticks > 0) {
            newMask |= bitsToModify;
        } else {
            newMask &= ~bitsToModify;
        }

        if ( oldMask != newMask ) {
            ARP_PATTERN_SETMASK( pattern, newMask );
            params->engineArpUserPatterns.patterns[ (int)params->engineArp2.pattern - ARPEGGIATOR_PRESET_PATTERN_COUNT ] = pattern;
            propagateNewParamValue(currentTimbre, row, encoder, (ParameterDisplay*)NULL, oldMask, newMask );
        }
    }
}

void SynthState::twoButtonsPressed(int button1, int button2) {
    int oldCurrentRow = currentRow;

    switch (button1) {
    case BUTTON_BACK:
        switch (button2) {
        case BUTTON_SYNTH:
            propagateNoteOn(12);
            break;
        case BUTTON_OSC:
            propagateNoteOn(8);
            break;
        case BUTTON_ENV:
            propagateNoteOn(0);
            break;
        case BUTTON_MATRIX:
            propagateNoteOn(-8);
            break;
        case BUTTON_LFO:
            propagateNoteOn(-12);
            break;
        case BUTTON_MENUSELECT:
            propagateNoteOff();
            propagateBeforeNewParamsLoad(currentTimbre);
            propagateAfterNewComboLoad();
            break;
        }
        break;
    }

    if ((fullState.synthMode  != SYNTH_MODE_EDIT)) {
        return;
    }

    switch (button1) {
    case BUTTON_SYNTH:
        switch (button2) {
        case BUTTON_MENUSELECT:
            if (fullState.synthMode  == SYNTH_MODE_EDIT) {
                propagateShowAlgo();
            }
            break;
        case BUTTON_BACK:
            if (fullState.synthMode  == SYNTH_MODE_EDIT) {
                propagateShowIMInformation();
            }
            break;
        case BUTTON_OSC:
            currentRow = ROW_ENGINE;
            break;
        case BUTTON_ENV:
            currentRow = ROW_MODULATION1;
            break;
        case BUTTON_MATRIX:
            currentRow = ROW_ARPEGGIATOR1;
            break;
        case BUTTON_LFO:
            currentRow = ROW_EFFECT;
            break;
        }
        break;
        case BUTTON_MATRIX:
            switch (button2) {
            case BUTTON_SYNTH:
                currentRow = ROW_MATRIX1;
                break;
            case BUTTON_OSC:
                changeSynthModeRow(BUTTON_MATRIX , -3);
                break;
            case BUTTON_ENV:
                changeSynthModeRow(BUTTON_MATRIX , -1);
                onUserChangedRow();
                break;
            case BUTTON_LFO:
                currentRow = ROW_MATRIX6;
                break;
            }
            break;
            case BUTTON_LFO:
                switch (button2) {
                case BUTTON_SYNTH:
                    currentRow = ROW_LFOOSC1;
                    break;
                case BUTTON_OSC:
                    currentRow = ROW_LFOENV1;
                    break;
                case BUTTON_ENV:
                    currentRow = ROW_LFOENV2;
                    break;
#ifndef DEBUG
                case BUTTON_MATRIX:
                    currentRow = ROW_LFOSEQ1;
                    break;
#endif
                case BUTTON_MENUSELECT:
                    currentRow = ROW_PERFORMANCE1;
                    break;
                }
                break;
                case BUTTON_MENUSELECT:
                    switch (button2) {
                    case BUTTON_LFO:
                        currentRow = ROW_PERFORMANCE1;
                        break;
                    }
                    break;
    }

#ifdef DEBUG
    if (button1 == BUTTON_LFO) {
        if (button2 == BUTTON_MATRIX) {
            synth.debugVoice();
        }
        if (button2 == BUTTON_BACK) {
            synth.showCycles();
        }
        if (button2 == BUTTON_MENUSELECT) {
            storage->getPatchBank()->testMemoryPreset();
        }
    }
#endif

    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
        onUserChangedRow();
    }
}



void SynthState::encoderTurnedWhileButtonPressed(int encoder, int ticks, int button) {

    if (unlikely(fullState.synthMode == SYNTH_MODE_MENU))  {
        return;
    }

    // SYNTH_MODE_EDIT

    int oldCurrentRow = currentRow;
    switch (button) {
    case BUTTON_SYNTH:
    case BUTTON_MATRIX:
    case BUTTON_LFO:
    case BUTTON_OSC:
        changeSynthModeRow(button , ticks>0 ? 1 : -1);
        onUserChangedRow();
        break;
    case BUTTON_ENV:
    {
        if (currentRow >= ROW_ENV1a && currentRow <= ROW_ENV6b) {
            int firstRow = ROW_ENV1a;
            int envBBit = ROW_ENV1b & 0x1;
            if ((currentRow & 0x1) == envBBit) {
                firstRow = ROW_ENV1b;
            }
            int currentAlgo = (int)params->engine1.algo;
            int currentOpType = algoOpInformation[currentAlgo][(currentRow - ROW_ENV1a) >> 1];

            for (int op = 0; op < NUMBER_OF_OPERATORS; op++) {
                currentRow = firstRow + op*2;
                if (currentOpType == algoOpInformation[currentAlgo][op]) {
                    // Same operator type : carrier or modulation
                    encoderTurned(encoder, ticks);
                }
            }
            // Put back current row
            currentRow = oldCurrentRow;
        } else if (currentRow >= ROW_MODULATION1 && currentRow <= ROW_MODULATION3) {
            int imMax = algoInformation[(int)params->engine1.algo].im;
            // Im or v ?
            if ((encoder & 0x1) == 0) {
                for (int m = 0; m < imMax; m++) {
                    currentRow = ROW_MODULATION1 + (m >> 1);
                    encoderTurned(ENCODER_ENGINE_IM1 + (m & 0x1) * 2, ticks);
                }
            } else {
                for (int m = 0; m < imMax; m++) {
                    currentRow = ROW_MODULATION1 + (m >> 1);
                    encoderTurned(ENCODER_ENGINE_IM1_VELOCITY + (m & 0x1) * 2, ticks);
                }
            }
            currentRow = oldCurrentRow;
        } else if (currentRow >= ROW_OSC_MIX1 && currentRow <= ROW_OSC_MIX3) {
            int mixMax = algoInformation[(int)params->engine1.algo].mix;
            // Mix or Pan ?
            if ((encoder & 0x1) == 0) {
                for (int m = 0; m < mixMax; m++) {
                    currentRow = ROW_OSC_MIX1 + (m >> 1);
                    encoderTurned(ENCODER_ENGINE_MIX1 + (m & 0x1) * 2, ticks);
                }
            } else {
                for (int m = 0; m < mixMax; m++) {
                    currentRow = ROW_OSC_MIX1 + (m >> 1);
                    encoderTurned(ENCODER_ENGINE_PAN1 + (m & 0x1) * 2, ticks);
                }
            }
            currentRow = oldCurrentRow;
        }
        break;
    }
    case BUTTON_BACK:
        encoderTurned(encoder, ticks * 10);
        break;
    case BUTTON_ENCODER:
    case BUTTON_MENUSELECT:
    {
        if (currentRow == ROW_ENGINE_FIRST) {
            // Nothing if first engine row
            return;
        }
        struct ParameterDisplay* param = &(allParameterRows.row[currentRow]->params[encoder]);
        const struct OneSynthParams* defaultParams = &defaultPreset;
        int num = encoder + currentRow * NUMBER_OF_ENCODERS;
        float &value = ((float*)params)[num];
        float oldValue = value;
        float newValue = ((float*)defaultParams)[num];
        value = newValue;
        propagateNewParamValue(currentTimbre, currentRow, encoder, param, oldValue, newValue);
        break;
    }
    }
    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
    }
}



bool SynthState::newRandomizerValue(int encoder, int ticks) {
    bool changed = false;
    char oldValue = 0;
    char newValue = 0;
    switch (encoder) {
    case 0:
        oldValue = fullState.randomizer.Oper;
        fullState.randomizer.Oper += (ticks > 0 ? 1 : -1);
        fullState.randomizer.Oper = fullState.randomizer.Oper > 3 ? 3 : fullState.randomizer.Oper;
        fullState.randomizer.Oper = fullState.randomizer.Oper < 0 ? 0 : fullState.randomizer.Oper;
        newValue = fullState.randomizer.Oper;
        break;
    case 1:
        oldValue = fullState.randomizer.EnvT;
        fullState.randomizer.EnvT += (ticks > 0 ? 1 : -1);
        fullState.randomizer.EnvT = fullState.randomizer.EnvT > 3 ? 3 : fullState.randomizer.EnvT;
        fullState.randomizer.EnvT = fullState.randomizer.EnvT < 0 ? 0 : fullState.randomizer.EnvT;
        newValue = fullState.randomizer.EnvT;
        break;
    case 2:
        oldValue = fullState.randomizer.IM;
        fullState.randomizer.IM += (ticks > 0 ? 1 : -1);
        fullState.randomizer.IM = fullState.randomizer.IM > 3 ? 3 : fullState.randomizer.IM;
        fullState.randomizer.IM = fullState.randomizer.IM < 0 ? 0 : fullState.randomizer.IM;
        newValue = fullState.randomizer.IM;
        break;
    case 3:
        oldValue = fullState.randomizer.Modl;
        fullState.randomizer.Modl += (ticks > 0 ? 1 : -1);
        fullState.randomizer.Modl = fullState.randomizer.Modl > 3 ? 3 : fullState.randomizer.Modl;
        fullState.randomizer.Modl = fullState.randomizer.Modl < 0 ? 0 : fullState.randomizer.Modl;
        newValue = fullState.randomizer.Modl;
        break;
    }
    return newValue != oldValue;
}


void SynthState::encoderTurned(int encoder, int ticks) {
    if (fullState.synthMode == SYNTH_MODE_EDIT) {

        // Step sequencer special case
        if (unlikely(currentRow == ROW_LFOSEQ1 || currentRow == ROW_LFOSEQ2)) {
            if (encoder >= 2) {
                encoderTurnedForStepSequencer(currentRow, encoder, ticks);
                return;
            }
        };
        if (unlikely(currentRow == ROW_ARPEGGIATOR3)) {
            encoderTurnedForArpPattern(currentRow, encoder, ticks);
            return;
        }

        int num = encoder + currentRow * NUMBER_OF_ENCODERS;
        struct ParameterDisplay* param = &(allParameterRows.row[currentRow]->params[encoder]);
        float newValue;
        float oldValue;

        if (param->displayType == DISPLAY_TYPE_STRINGS) {
            // Do not use encoder acceleration
            ticks = ticks > 0 ? 1 : -1;
        }

        if (param->valueNameOrder == NULL) {
            // Not string parameters

            // floating point test to be sure numberOfValues is different from 1.
            // for voices when number of voices forced to 0
            if (param->numberOfValues < 1.5) {
                return;
            }


            float &value = ((float*)params)[num];
            oldValue = value;

            float inc = param->incValue;

            // Slow down LFO frequency
            if (unlikely(param->displayType == DISPLAY_TYPE_FLOAT_LFO_FREQUENCY)) {
                if (oldValue < 1.0f) {
                    inc = inc * .1f;
                }
            }

            int tickIndex = (value - param->minValue) / inc + .0005f + ticks;
            newValue = param->minValue + tickIndex * inc;
            propagateNewParamCheck(encoder, oldValue, &newValue);
            if (newValue > param->maxValue) {
                newValue = param->maxValue;
            }
            if (newValue < param->minValue) {
                newValue = param->minValue;
            }
            value = newValue;
        } else {
            float *value = &((float*)params)[num];
            int index;
            newValue = oldValue = (*value);

            // Must use newValue (int) so that the minValue comparaison works
            // Is there any other order than the default one
            int pos = param->valueNameOrderReversed[(int)(*value)];
            if (ticks>0 && pos < param->maxValue) {
                newValue = param->valueNameOrder[pos+1];
            }
            if (ticks<0 && pos>param->minValue) {
                newValue = param->valueNameOrder[pos-1];
            }

            (*value) = (float)newValue;
        }
        if (newValue != oldValue) {
            propagateNewParamValue(currentTimbre, currentRow, encoder, param, oldValue, newValue);
        }
    } else if (fullState.synthMode == SYNTH_MODE_MENU) {
        int oldMenuSelect = fullState.menuSelect;

        if (unlikely(fullState.currentMenuItem->menuState == MENU_LOAD_RANDOMIZER)) {

            if (newRandomizerValue(encoder, ticks)) {
                // Only propagate if value really changed
                propagateNewMenuSelect();
            }
            return;
        }

        switch (encoder) {
        case 0:
            if (unlikely(fullState.currentMenuItem->menuState == MENU_SCALA_FREQUENCY)) {
                fullState.scalaScaleConfig.scalaFreq += .1f * (float)ticks;
                propagateNewMenuSelect();
                oldMenuSelect ++;
            } else {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 1;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 1;
                }
            }
            break;
        case 1:
            if (unlikely(fullState.currentMenuItem->menuState == MENU_SCALA_FREQUENCY)) {
                fullState.scalaScaleConfig.scalaFreq += 1.0f * (float)ticks;
                propagateNewMenuSelect();
                fullState.menuSelect = (fullState.menuSelect == 1) ? 2 : 1;
            } else if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = (fullState.name[fullState.menuSelect] + (ticks>0? 1: -1));
                if (fullState.name[fullState.menuSelect]<28) {
                    fullState.name[fullState.menuSelect]=28;
                }
                if (fullState.name[fullState.menuSelect]> 53) {
                    fullState.name[fullState.menuSelect] = 53;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->maxValue >= 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 5;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 5;
                }
            } else {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 1;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 1;
                }
            }
            break;
        case 2:
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = (fullState.name[fullState.menuSelect] + (ticks>0? 1: -1));
                if (fullState.name[fullState.menuSelect]<1) {
                    fullState.name[fullState.menuSelect]=1;
                }
                if (fullState.name[fullState.menuSelect]> 26) {
                    fullState.name[fullState.menuSelect] = 26;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->maxValue >= 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 10;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 10;
                }
            } else {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 1;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 1;
                }
            }
            break;
        case 3:
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = (fullState.name[fullState.menuSelect] + (ticks>0? 1: -1));
                if (fullState.name[fullState.menuSelect]<0) {
                    fullState.name[fullState.menuSelect]=0;
                }
                if (fullState.name[fullState.menuSelect]>= getLength(allChars)) {
                    fullState.name[fullState.menuSelect]= getLength(allChars)-1;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->menuState == MENU_CONFIG_SETTINGS) {
                fullState.midiConfigValue[fullState.menuSelect] = fullState.midiConfigValue[fullState.menuSelect] + (ticks>0? 1: -1);
                if (fullState.midiConfigValue[fullState.menuSelect] >= midiConfig[fullState.menuSelect].maxValue) {
                    fullState.midiConfigValue[fullState.menuSelect] = midiConfig[fullState.menuSelect].maxValue - 1;
                }
                if (fullState.midiConfigValue[fullState.menuSelect] < 0 ) {
                    fullState.midiConfigValue[fullState.menuSelect] = 0;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->maxValue >= 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 25;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 25;
                }
            } else {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 1;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 1;
                }
            }
            break;
        }

        if (fullState.menuSelect> fullState.currentMenuItem->maxValue - 1) {
            fullState.menuSelect = fullState.currentMenuItem->maxValue - 1;
        }
        if (fullState.menuSelect< 0) {
            fullState.menuSelect = 0;
        }

        if (fullState.menuSelect != oldMenuSelect) {
            switch (fullState.currentMenuItem->menuState) {
            case MENU_LOAD_SELECT_DX7_BANK:
                // Did we really change DX7 bank?
                if (fullState.dx7Bank != storage->getDX7SysexFile()->getFile(fullState.menuSelect)) {
                    fullState.dx7Bank = storage->getDX7SysexFile()->getFile(fullState.menuSelect);
                    fullState.dx7PresetNumber = 0;
                }
                break;
            case MENU_LOAD_SELECT_DX7_PRESET:
                loadDx7Patch(currentTimbre, fullState.dx7Bank, fullState.menuSelect, params);
                fullState.dx7PresetNumber = fullState.menuSelect;
                break;
            case MENU_LOAD_SELECT_BANK:
            case MENU_SAVE_SELECT_BANK:
                // Did we really change bank?
                if (fullState.preenFMBank != storage->getPatchBank()->getFile(fullState.menuSelect)) {
                    fullState.preenFMBank = storage->getPatchBank()->getFile(fullState.menuSelect);
                    fullState.preenFMPresetNumber = 0;
                }
                break;
            case MENU_RENAME_SELECT_BANK:
                // Did we really change bank?
                if (fullState.preenFMBank != storage->getPatchBank()->getFile(fullState.menuSelect)) {
                    fullState.preenFMBank = storage->getPatchBank()->getFile(fullState.menuSelect);
                }
                break;
            case MENU_LOAD_SELECT_COMBO:
            case MENU_SAVE_SELECT_COMBO:
                // Did we really change bank?
                if (fullState.preenFMCombo != storage->getComboBank()->getFile(fullState.menuSelect)) {
                    fullState.preenFMCombo = storage->getComboBank()->getFile(fullState.menuSelect);
                    fullState.preenFMComboPresetNumber = 0;
                }
                break;
            case MENU_RENAME_SELECT_COMBO:
                // Did we really change combo?
                if (fullState.preenFMCombo != storage->getComboBank()->getFile(fullState.menuSelect)) {
                    fullState.preenFMCombo = storage->getComboBank()->getFile(fullState.menuSelect);
                }
                break;
            case MENU_LOAD_SELECT_BANK_PRESET:
                loadPreenFMPatch(currentTimbre, fullState.preenFMBank, fullState.menuSelect, params);
                fullState.preenFMPresetNumber = fullState.menuSelect;
                break;
            case MENU_SAVE_SELECT_BANK_PRESET:
                fullState.preenFMPresetNumber = fullState.menuSelect;
                break;
            case MENU_LOAD_SELECT_COMBO_PRESET:
            case MENU_SAVE_SELECT_COMBO_PRESET:
                fullState.preenFMComboPresetNumber = fullState.menuSelect;
                break;
            case MENU_SCALA_MAPPING:
                fullState.scalaScaleConfig.keyboardMapping = (fullState.menuSelect == 0);
                storage->getScalaFile()->applyScalaScale(&fullState.scalaScaleConfig);
                break;
            case MENU_SCALA_FREQUENCY:
                storage->getScalaFile()->applyScalaScale(&fullState.scalaScaleConfig);
                break;
            case MENU_SCALA_FILENAME:
                // Did we really change bank?
                if (fullState.scalaScaleConfig.scalaFile != storage->getScalaFile()->getFile(fullState.menuSelect)) {
                    fullState.scalaScaleConfig.scalaFile = storage->getScalaFile()->getFile(fullState.menuSelect);
                }
                storage->getScalaFile()->loadScalaScale(&fullState.scalaScaleConfig);
                break;
            case MENU_SCALA_ENABLE:
                fullState.scalaScaleConfig.scalaEnabled = (fullState.menuSelect > 0);
                if (fullState.scalaScaleConfig.scalaEnabled) {
                    storage->getScalaFile()->loadScalaScale(&fullState.scalaScaleConfig);
                } else {
                    storage->getScalaFile()->clearScalaScale();
                }
                break;
            }
            propagateNewMenuSelect();
        }
    }
}


void SynthState::setScalaEnable(bool enable) {
    fullState.scalaScaleConfig.scalaEnabled = enable;
    if (!enable) {
        storage->getScalaFile()->clearScalaScale();
    } else if (fullState.scalaScaleConfig.scalaFile->fileType != FILE_EMPTY) {
        storage->getScalaFile()->loadScalaScale(&fullState.scalaScaleConfig);
    }
}

void SynthState::setScalaScale(int scaleNumber) {
    fullState.scalaScaleConfig.scalaFile = storage->getScalaFile()->getFile(scaleNumber);

    if (fullState.scalaScaleConfig.scalaFile->fileType != FILE_EMPTY && fullState.scalaScaleConfig.scalaEnabled) {
        storage->getScalaFile()->loadScalaScale(&fullState.scalaScaleConfig);
    }
}

void SynthState::loadPreenFMPatch(int timbre, PFM2File const *bank, int patchNumber, struct OneSynthParams* params) {
    storeTestNote();
    propagateNoteOff();
    propagateBeforeNewParamsLoad(timbre);
    storage->getPatchBank()->loadPreenFMPatch(bank, patchNumber, params);
    propagateAfterNewParamsLoad(timbre);
    restoreTestNote();
}

void SynthState::loadDx7Patch(int timbre, PFM2File const *bank, int patchNumber, struct OneSynthParams* params) {
    storeTestNote();
    propagateNoteOff();
    propagateBeforeNewParamsLoad(timbre);
    hexter->loadHexterPatch(storage->getDX7SysexFile()->dx7LoadPatch(bank, patchNumber), params);
    propagateAfterNewParamsLoad(timbre);
    restoreTestNote();
}

void SynthState::loadPreenFMCombo(PFM2File const *bank, int patchNumber) {
    propagateBeforeNewParamsLoad(currentTimbre);
    storage->getComboBank()->loadPreenFMCombo(bank, patchNumber);
    // Update and clean all timbres
    this->currentTimbre = 0;
    propagateNewTimbre(currentTimbre);
    propagateAfterNewComboLoad();
}


void SynthState::loadPreenFMPatchFromMidi(int timbre, int bank, int bankLSB, int patchNumber, struct OneSynthParams* params) {
    switch (bank) {
    case 0:
    {
        PFM2File const *bank = storage->getPatchBank()->getFile(bankLSB);
        if (bank->fileType != FILE_EMPTY) {
            loadPreenFMPatch(timbre, bank, patchNumber, params);
        }
    }
    break;
    case 1:
    {
        PFM2File const *bank = storage->getComboBank()->getFile(bankLSB);
        if (bank->fileType != FILE_EMPTY) {
            loadPreenFMCombo(bank, patchNumber);
        }
    }
    break;
    case 2:
    case 3:
    case 4:
    {
        int dx7bank = bank - 2;
        PFM2File const *bank = storage->getDX7SysexFile()->getFile(bankLSB + dx7bank * 128);
        if (bank->fileType != FILE_EMPTY) {
            loadDx7Patch(timbre, bank, patchNumber, params);
        }
    }
    break;
    }
}


void SynthState::resetDisplay() {
    fullState.synthMode = SYNTH_MODE_EDIT;
    currentRow = 0;
    propagateNewSynthMode();
}

bool SynthState::isCurrentRowAvailable() const {

    switch (currentRow) {
    case ROW_OSC_MIX2:
        return algoInformation[(int)params->engine1.algo].mix > 2;
    case ROW_OSC_MIX3:
        return algoInformation[(int)params->engine1.algo].mix > 4;
    case ROW_MODULATION1:
        return algoInformation[(int)params->engine1.algo].im != 0;
    case ROW_MODULATION2:
        return algoInformation[(int)params->engine1.algo].im > 2;
    case ROW_MODULATION3:
        return algoInformation[(int)params->engine1.algo].im > 4;
    case ROW_ARPEGGIATOR2:
        return params->engineArp1.clock > 0;
    case ROW_ARPEGGIATOR3:
        return params->engineArp1.clock > 0 && params->engineArp2.pattern >= ARPEGGIATOR_PRESET_PATTERN_COUNT;
    }
    return true;
}

int SynthState::getRowFromOperator() {
    switch (operatorView) {
    case 0:
        return ROW_OSC_FIRST + operatorNumber;
        break;
    case 1:
        return ROW_ENV_FIRST + operatorNumber * 2;
        break;
    case 2:
        return ROW_ENV_FIRST + operatorNumber * 2 + 1;
        break;
    }
}


void SynthState::changeSynthModeRow(int button, int step) {
    unsigned char lastBecauseOfAlgo;

    switch (button) {
    case BUTTON_SYNTH:
        if (currentRow<ROW_ENGINE_FIRST || currentRow>ROW_ENGINE_LAST) {
            currentRow = engineRow;
        } else {
            do {
                currentRow += step;
                if (currentRow>ROW_ENGINE_LAST) {
                    currentRow = ROW_ENGINE_FIRST;
                } else if (currentRow<ROW_ENGINE_FIRST) {
                    currentRow = ROW_ENGINE_LAST;
                }
            } while (!isCurrentRowAvailable());
        }
        engineRow = currentRow;
        break;
    case BUTTON_OSC:
        if (this->fullState.midiConfigValue[MIDICONFIG_OP_OPTION] == 0) {
            // New UI - operator number
            lastBecauseOfAlgo = algoInformation[(int)params->engine1.algo].osc - 1;
            if (currentRow < ROW_OSC_FIRST || currentRow > ROW_ENV_LAST) {
                // Nothing to do.;.
            } else {
                operatorNumber += step;
            }
            if (operatorNumber > lastBecauseOfAlgo) {
                operatorNumber = 0;
            } else if (operatorNumber < 0) {
                operatorNumber = lastBecauseOfAlgo;
            }
            currentRow = getRowFromOperator();
        } else {
            // Old UI
            lastBecauseOfAlgo = ROW_OSC_FIRST + algoInformation[(int)params->engine1.algo].osc - 1;
            if (currentRow<ROW_OSC_FIRST || currentRow>lastBecauseOfAlgo) {
                currentRow = ROW_OSC_FIRST + oscillatorRow;
            } else {
                currentRow += step;
                envelopeRow = (currentRow - ROW_OSC_FIRST)*2;
            }
            if (currentRow>lastBecauseOfAlgo) {
                currentRow = ROW_OSC_FIRST;
            } else if (currentRow<ROW_OSC_FIRST) {
                currentRow = lastBecauseOfAlgo;
            }
            oscillatorRow = currentRow - ROW_OSC_FIRST;
        }
        break;
    case BUTTON_ENV:
        if (this->fullState.midiConfigValue[MIDICONFIG_OP_OPTION] == 0) {
            // New UI - op / env1 / env2 for the current operator
            // New UI - operator number
            lastBecauseOfAlgo = algoInformation[(int)params->engine1.algo].osc - 1;
            if (currentRow < ROW_OSC_FIRST || currentRow > ROW_ENV_LAST) {
                // Nothing to do.;.
            } else {
                operatorView += step;
                if (operatorView > 2) {
                    operatorView = 0;
                } else if (operatorView < 0) {
                    operatorView = 2;
                }
            }
            if (operatorNumber > lastBecauseOfAlgo) {
                operatorNumber = 0;
            } else if (operatorNumber < 0) {
                operatorNumber = lastBecauseOfAlgo;
            }
            currentRow = getRowFromOperator();
        } else {
            lastBecauseOfAlgo = ROW_ENV_FIRST - 1 + algoInformation[(int)params->engine1.algo].osc  * 2;
            if (currentRow<ROW_ENV_FIRST || currentRow>lastBecauseOfAlgo) {
                currentRow = ROW_ENV_FIRST + envelopeRow;
            } else {
                currentRow += step;
                oscillatorRow = (currentRow - ROW_ENV_FIRST) >> 1;
            }
            if (currentRow>lastBecauseOfAlgo) {
                currentRow = ROW_ENV_FIRST;
            } else if (currentRow<ROW_ENV_FIRST) {
                currentRow = lastBecauseOfAlgo;
            }
            envelopeRow = currentRow - ROW_ENV_FIRST;
        }
        break;
    case BUTTON_MATRIX:
        if (currentRow<ROW_MATRIX_FIRST || currentRow>ROW_MATRIX_LAST) {
            currentRow = matrixRow;
        } else {
            currentRow += step;
            if (currentRow>ROW_MATRIX_LAST) {
                currentRow = ROW_MATRIX_FIRST;
            } else if (currentRow<ROW_MATRIX_FIRST) {
                currentRow = ROW_MATRIX_LAST;
            }
        }
        matrixRow = currentRow;
        break;
    case BUTTON_LFO:
        if (currentRow<ROW_LFO_FIRST || currentRow>ROW_LFO_LAST) {
            currentRow = lfoRow;
        } else {
            currentRow += step;
            if (currentRow>ROW_LFO_LAST) {
                currentRow = ROW_LFO_FIRST;
            } else if (currentRow<ROW_LFO_FIRST) {
                currentRow = ROW_LFO_LAST;
            }
        }
        lfoRow = currentRow;
        break;
    }
}

bool SynthState::isEnterNameState(int currentItem) {
    return currentItem == MENU_SAVE_ENTER_PRESET_NAME
            || currentItem == MENU_RENAME_PATCH
            || currentItem == MENU_SAVE_ENTER_COMBO_NAME
            || currentItem == MENU_RENAME_BANK
            || currentItem == MENU_RENAME_COMBO
            || currentItem == MENU_CREATE_BANK
            || currentItem == MENU_CREATE_COMBO;
}


void SynthState::setCurrentInstrument(int value) {
    setLastRowForTimbre( currentTimbre, currentRow ); // remember row for when we return to this timbre
    if (value == 0) {
        currentTimbre++;
    } else if (value <= 4) {
        currentTimbre = value - 1;
    } else {
        return;
    }
    currentTimbre &= (NUMBER_OF_TIMBRES-1);
    propagateNewTimbre(currentTimbre);

    int last = getLastRowForTimbre( currentTimbre );
    if ( last >= 0 )
        currentRow = last;
    if ( !isCurrentRowAvailable() && currentRow >= ROW_ENGINE_FIRST && currentRow <= ROW_ENGINE_LAST) {
        changeSynthModeRow( BUTTON_SYNTH, -1 );
    }
}

void SynthState::buttonPressed(int button) {
    SynthEditMode oldSynthMode = fullState.synthMode;
    int oldCurrentRow = currentRow;
    MenuState oldMenuState = fullState.currentMenuItem->menuState;

    if (fullState.synthMode == SYNTH_MODE_EDIT)  {
        switch (button) {
        case BUTTON_SYNTH:
        case BUTTON_OSC:
        case BUTTON_ENV:
        case BUTTON_MATRIX:
        case BUTTON_LFO:
            changeSynthModeRow(button , 1);
            onUserChangedRow();
            break;
        case BUTTON_ENCODER:
            currentRow = ROW_PERFORMANCE1;
            break;
        case BUTTON_MENUSELECT:
            fullState.synthMode = SYNTH_MODE_MENU;
            fullState.menuSelect = fullState.firstMenu;
            // allow undo event after trying some patches
            copySynthParams((char*)params, (char*)&backupParams);
            fullState.currentMenuItem = MenuItemUtil::getMenuItem(MAIN_MENU);
            break;
        case BUTTON_BACK:
        {
            // select next instrument as current one
            setCurrentInstrument(0);
        }
        break;
        }
    } else {
        // Any button when done is display makes the synth go back to edit mode.
        // MENU MODE

        // Special treatment for Randomizer
        if (unlikely(fullState.currentMenuItem->menuState == MENU_LOAD_RANDOMIZER)) {
            if (button != BUTTON_MENUSELECT && button != BUTTON_BACK) {
                storeTestNote();
                propagateNoteOff();
                propagateBeforeNewParamsLoad(currentTimbre);
                randomizePreset();
                propagateAfterNewParamsLoad(currentTimbre);
                restoreTestNote();
                return;
            }
        }

        switch (button) {
        case BUTTON_MENUSELECT:
            fullState.currentMenuItem = afterButtonPressed();
            break;
        case BUTTON_BACK:
        {
            enum MenuState oldState = fullState.currentMenuItem->menuState;
            fullState.currentMenuItem = menuBack();
            propagateMenuBack(oldState);
            break;
        }
        case BUTTON_LFO:
        {
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = 0;
                propagateNewMenuSelect();
            } else {
                propagateNoteOn(-12);
            }
            break;
        }
        case BUTTON_MATRIX:
        {
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = 27;
                propagateNewMenuSelect();
            } else {
                propagateNoteOn(-8);
            }
            break;
        }
        case BUTTON_ENV:
        {
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = 55;
                propagateNewMenuSelect();
            } else {
                propagateNoteOn(0);
            }
            break;
        }
        case BUTTON_OSC:
        {
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
                fullState.name[fullState.menuSelect] = 66;
                propagateNewMenuSelect();
            } else {
                propagateNoteOn(8);
            }
            break;
        }
        case BUTTON_SYNTH:
        {
            propagateNoteOn(12);
        }
        }
    }

    if (oldSynthMode != fullState.synthMode) {
        propagateNewSynthMode();
        return;
    }
    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
    }
    if (oldMenuState != fullState.currentMenuItem->menuState) {
        propagateNewMenuState();
    }
}



const MenuItem* SynthState::afterButtonPressed() {
    const MenuItem* rMenuItem = 0;
    int length, k;
    const MenuItem *cmi;

    if (fullState.currentMenuItem->hasSubMenu ) {
        rMenuItem = MenuItemUtil::getMenuItem(fullState.currentMenuItem->subMenu[fullState.menuSelect]);
    } else {
        rMenuItem = MenuItemUtil::getMenuItem(fullState.currentMenuItem->subMenu[0]);
    }

    // ------------------------
    // Previous state switch

    switch (fullState.currentMenuItem->menuState) {
    case MAIN_MENU:
        fullState.firstMenu = fullState.menuSelect;
        break;
    case MENU_SAVE_SELECT_BANK:
        if (fullState.preenFMBank->fileType != FILE_OK) {
            return fullState.currentMenuItem;
        }
        fullState.preenFMBankNumber = fullState.menuSelect;
        break;
    case MENU_RENAME_SELECT_BANK:
    case MENU_LOAD_SELECT_BANK:
        if (fullState.preenFMBank->fileType == FILE_EMPTY) {
            return fullState.currentMenuItem;
        }
        fullState.preenFMBankNumber = fullState.menuSelect;
        break;
    case MENU_SAVE_SELECT_COMBO:
        if (fullState.preenFMCombo->fileType != FILE_OK) {
            return fullState.currentMenuItem;
        }
        fullState.preenFMComboNumber = fullState.menuSelect;
        break;
    case MENU_RENAME_SELECT_COMBO:
    case MENU_LOAD_SELECT_COMBO:
        if (fullState.preenFMCombo->fileType == FILE_EMPTY) {
            return fullState.currentMenuItem;
        }
        fullState.preenFMComboNumber = fullState.menuSelect;
        break;
    case MENU_LOAD_SELECT_DX7_BANK:
        if (fullState.dx7Bank->fileType != FILE_OK) {
            return fullState.currentMenuItem;
        }
        fullState.dx7BankNumber = fullState.menuSelect;
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        loadPreenFMCombo(fullState.preenFMCombo, fullState.menuSelect);
        copySynthParams((char*)params, (char*)&backupParams);
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
    case MENU_LOAD_SELECT_BANK_PRESET:
    case MENU_LOAD_RANDOMIZER:
        copySynthParams((char*)params, (char*)&backupParams);
        break;
    case MENU_SAVE_SELECT_BANK_PRESET:
        for (int k=0; k<12; k++) {
            fullState.name[k] = 0;
        }
        for (int k=0; k<12 && params->presetName[k] != 0; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (params->presetName[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        break;
    case MENU_SAVE_SELECT_COMBO_PRESET:
    {
        // const char* comboName = storage->readComboName(fullState.menuSelect);
        const char* comboName = storage->getComboBank()->loadPreenFMComboName(fullState.preenFMCombo, fullState.preenFMComboPresetNumber);
        for (int k=0; k<12 && comboName[k] != 0; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (comboName[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        break;
    }
    case MENU_RENAME_PATCH:
        for (length=12; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            params->presetName[k] = allChars[(int)fullState.name[k]];
        }
        params->presetName[length] = '\0';
        break;
    case MENU_SAVE_ENTER_PRESET_NAME:
        for (length=12; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            params->presetName[k] = allChars[(int)fullState.name[k]];
        }
        params->presetName[length] = '\0';
        storage->getPatchBank()->savePreenFMPatch(fullState.preenFMBank, fullState.preenFMPresetNumber, params);
        break;
    case MENU_SAVE_ENTER_COMBO_NAME:
        for (length=12; fullState.name[length-1] == 0; length--);
        char comboName[12];
        for (int k=0; k<length; k++) {
            comboName[k] = allChars[(int)fullState.name[k]];
        }
        comboName[length] = '\0';
        storage->getComboBank()->savePreenFMCombo(fullState.preenFMCombo, fullState.preenFMComboPresetNumber, comboName);
        break;
    case MENU_SAVE_SYSEX_PATCH:
        //PresetUtil::sendCurrentPatchToSysex();
        storage->getPatchBank()->sendPreenFMPatchAsSysex(params);
        break;
    case MENU_RENAME_COMBO:
        for (length=0; length<8 && fullState.name[length]!=0; length++) {
            fullState.name[length] =  allChars[fullState.name[length]];
        }
        fullState.name[length++] = '.';
        fullState.name[length++] = 'c';
        fullState.name[length++] = 'm';
        fullState.name[length++] = 'b';
        fullState.name[length++] = '\0';
        if (storage->getComboBank()->renameFile(fullState.preenFMCombo, fullState.name) > 0) {
            rMenuItem = MenuItemUtil::getMenuItem(MENU_ERROR);
        }

        break;
    case MENU_RENAME_BANK:
        for (length=0; length<8 && fullState.name[length]!=0; length++) {
            fullState.name[length] =  allChars[fullState.name[length]];
        }
        fullState.name[length++] = '.';
        fullState.name[length++] = 'b';
        fullState.name[length++] = 'n';
        fullState.name[length++] = 'k';
        fullState.name[length++] = '\0';
        if (storage->getPatchBank()->renameFile(fullState.preenFMBank, fullState.name) > 0) {
            rMenuItem = MenuItemUtil::getMenuItem(MENU_ERROR);
        }
        break;
    case MENU_CANCEL:
    case MENU_DONE:
    case MENU_ERROR:
        fullState.synthMode = SYNTH_MODE_EDIT;
        break;
    case MENU_CREATE_BANK:
        // Must create the bank here....
        for (length=8; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            fullState.name[k] = allChars[(int)fullState.name[k]];
        }
        fullState.name[length++] = '.';
        fullState.name[length++] = 'b';
        fullState.name[length++] = 'n';
        fullState.name[length++] = 'k';
        fullState.name[length] = '\0';
        if (!storage->getPatchBank()->nameExists(fullState.name)) {
            cmi = fullState.currentMenuItem;
            // Update display while formating
            lcd.setRealTimeAction(true);
            fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
            propagateNewMenuState();
            storage->getPatchBank()->create(fullState.name);
            lcd.setRealTimeAction(false);
            fullState.currentMenuItem = cmi;
        } else {
            rMenuItem = MenuItemUtil::getMenuItem(MENU_ERROR);
        }
        break;
    case MENU_CREATE_COMBO:

        // Must create the combo here....
        for (length=8; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            fullState.name[k] = allChars[(int)fullState.name[k]];
        }
        fullState.name[length++] = '.';
        fullState.name[length++] = 'c';
        fullState.name[length++] = 'm';
        fullState.name[length++] = 'b';
        fullState.name[length] = '\0';

        if (!storage->getComboBank()->nameExists(fullState.name)) {
            cmi = fullState.currentMenuItem;
            // Update display while formating
            lcd.setRealTimeAction(true);
            fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
            propagateNewMenuState();
            storage->getComboBank()->createComboBank(fullState.name);
            lcd.setRealTimeAction(false);
            fullState.currentMenuItem = cmi;
        } else {
            rMenuItem = MenuItemUtil::getMenuItem(MENU_ERROR);
        }
        break;
    case MENU_CONFIG_SETTINGS_SAVE:
        storage->getConfigurationFile()->saveConfig(fullState.midiConfigValue);
        break;
    case MENU_DEFAULT_COMBO_SAVE:
        storage->getComboBank()->saveDefaultCombo();
        break;
    case MENU_DEFAULT_COMBO_RESET:
        storage->getComboBank()->removeDefaultCombo();
        break;
    case MENU_LOAD:
        fullState.loadWhat = fullState.menuSelect;
        break;
    case MENU_SAVE:
        fullState.saveWhat = fullState.menuSelect;
        break;
    case MENU_TOOLS:
        fullState.toolsWhat = fullState.menuSelect;
        break;
    case MENU_SCALA:
        fullState.scalaWhat = fullState.menuSelect;
        break;
    case MENU_SCALA_FILENAME:
        storage->getConfigurationFile()->saveScalaConfig(&fullState.scalaScaleConfig);
        if (fullState.scalaScaleConfig.scalaFile->fileType != FILE_OK) {
            return fullState.currentMenuItem;
        }
        break;
    case MENU_SCALA_ENABLE:
        storage->getConfigurationFile()->saveScalaConfig(&fullState.scalaScaleConfig);
        break;
    case MENU_SCALA_MAPPING:
    case MENU_SCALA_FREQUENCY:
        storage->getConfigurationFile()->saveScalaConfig(&fullState.scalaScaleConfig);
        break;
    default:
        break;
    }

    // ---------------------
    // Next state switch

    switch (rMenuItem->menuState) {
    case MENU_CREATE_COMBO:
    {
        const char *bankName = "COMBO01";
        int length = 7;
        for (int k=0; k<length ; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (bankName[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        for (int k=length; k<8; k++) {
            fullState.name[k] = 0;
        }
        fullState.menuSelect = 0;
        break;
    }
    case MENU_CREATE_BANK:
    {
        const char *bankName = "BANK01";
        int length = 6;
        for (int k=0; k<length ; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (bankName[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        for (int k=length; k<8; k++) {
            fullState.name[k] = 0;
        }
        fullState.menuSelect = 0;
        break;
    }
    case MENU_RENAME_COMBO:
        for (int k=0; k<8; k++) {
            fullState.name[k] = 0;
        }
        for (int k=0; k<8 && fullState.preenFMCombo->name[k]!='.'; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (fullState.preenFMCombo->name[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        fullState.menuSelect = 0;
        break;
    case MENU_RENAME_BANK:
        for (int k=0; k<8; k++) {
            fullState.name[k] = 0;
        }
        for (int k=0; k<8 && fullState.preenFMBank->name[k]!='.'; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (fullState.preenFMBank->name[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        fullState.menuSelect = 0;
        break;
    case MENU_RENAME_PATCH:
        for (int k=0; k<12; k++) {
            fullState.name[k] = 0;
        }
        for (int k=0; k<12 && params->presetName[k] != 0; k++) {
            for (int j=0; j<getLength(allChars); j++) {
                if (params->presetName[k] == allChars[j]) {
                    fullState.name[k] = j;
                }
            }
        }
        fullState.menuSelect = 0;
        break;
    case MENU_SAVE_SELECT_BANK_PRESET:
        fullState.menuSelect = fullState.preenFMPresetNumber;
        break;
    case MENU_SAVE_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboPresetNumber;
        break;
    case MENU_LOAD_SELECT_BANK_PRESET:
        loadPreenFMPatch(currentTimbre, fullState.preenFMBank, fullState.preenFMPresetNumber, params);
        fullState.menuSelect = fullState.preenFMPresetNumber;
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboPresetNumber;
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
        loadDx7Patch(currentTimbre, fullState.dx7Bank, fullState.dx7PresetNumber, params);
        fullState.menuSelect = fullState.dx7PresetNumber;
        break;
    case MENU_TOOLS:
        fullState.menuSelect = fullState.toolsWhat;
        break;
    case MENU_LOAD:
        fullState.menuSelect = fullState.loadWhat;
        break;
    case MENU_SAVE:
        fullState.menuSelect = fullState.saveWhat;
        break;
    case MENU_SCALA:
        fullState.menuSelect = fullState.scalaWhat;
        break;
    case MENU_RENAME_SELECT_BANK:
    case MENU_LOAD_SELECT_BANK:
    case MENU_SAVE_SELECT_BANK:
        fullState.menuSelect = fullState.preenFMBankNumber;
        fullState.preenFMBank = storage->getPatchBank()->getFile(fullState.menuSelect);
        break;
    case MENU_RENAME_SELECT_COMBO:
    case MENU_LOAD_SELECT_COMBO:
    case MENU_SAVE_SELECT_COMBO:
        fullState.menuSelect = fullState.preenFMComboNumber;
        fullState.preenFMCombo = storage->getComboBank()->getFile(fullState.menuSelect);
        break;
    case MENU_LOAD_SELECT_DX7_BANK:
        fullState.menuSelect = fullState.dx7BankNumber;
        fullState.dx7Bank = storage->getDX7SysexFile()->getFile(fullState.menuSelect);
        break;
    case MENU_SCALA_FILENAME:
        fullState.menuSelect = 0;
        if (fullState.scalaScaleConfig.scalaFile != NULL) {
            int scalaIndex = storage->getScalaFile()->getFileIndex(fullState.scalaScaleConfig.scalaFile);
            // can be <0 if file has been removed
            if (scalaIndex >= 0) {
                fullState.menuSelect = scalaIndex;
            }
        } else {
            fullState.scalaScaleConfig.scalaFile = storage->getScalaFile()->getFile(0);
        }
        break;
    case MENU_SCALA_MAPPING:
        fullState.menuSelect = fullState.scalaScaleConfig.keyboardMapping ? 0 : 1;
        break;
    case MENU_SCALA_ENABLE:
        fullState.menuSelect = fullState.scalaScaleConfig.scalaEnabled ? 1 : 0;
        break;
    default:
        fullState.menuSelect = 0;
    }

    // Save menu select for menuBack Action
    fullState.previousMenuSelect = fullState.menuSelect;
    return rMenuItem;
}


const MenuItem* SynthState::menuBack() {
    const MenuItem* rMenuItem = 0;

    // default menuSelect value
    fullState.menuSelect = MenuItemUtil::getParentMenuSelect(fullState.currentMenuItem->menuState);

    switch (fullState.currentMenuItem->menuState) {
    case MENU_SAVE_ENTER_PRESET_NAME:
        fullState.menuSelect = fullState.preenFMPresetNumber;
        break;
    case MENU_SAVE_ENTER_COMBO_NAME:
        fullState.menuSelect = fullState.preenFMComboNumber;
        break;
    case MENU_RENAME_BANK:
    case MENU_SAVE_SELECT_BANK_PRESET:
        fullState.menuSelect = fullState.preenFMBankNumber;
        break;
    case MENU_RENAME_COMBO:
    case MENU_SAVE_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboNumber;
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboNumber;
        propagateBeforeNewParamsLoad(currentTimbre);
        copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad(currentTimbre);
        break;
    case MENU_LOAD_RANDOMIZER:
        // Put back original preset
        propagateNoteOff();
        propagateBeforeNewParamsLoad(currentTimbre);
        copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad(currentTimbre);
        break;
    case MENU_LOAD_SELECT_BANK_PRESET:
        propagateNoteOff();
        fullState.menuSelect = fullState.preenFMBankNumber;
        propagateBeforeNewParamsLoad(currentTimbre);
        copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad(currentTimbre);
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
        propagateNoteOff();
        fullState.menuSelect = fullState.dx7BankNumber;
        propagateBeforeNewParamsLoad(currentTimbre);
        copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad(currentTimbre);
        break;
    case MAIN_MENU:
        fullState.synthMode = SYNTH_MODE_EDIT;
        // put back old patch (has been overwritten if a new patch has been loaded)
        break;
    }
    rMenuItem = MenuItemUtil::getParentMenuItem(fullState.currentMenuItem->menuState);
    return rMenuItem;
}





void SynthState::propagateAfterNewParamsLoad(int timbre) {
    for (SynthParamListener* listener = firstParamListener; listener !=0; listener = listener->nextListener) {
        listener->afterNewParamsLoad(timbre);
    }
}

void SynthState::propagateAfterNewComboLoad() {
    for (SynthParamListener* listener = firstParamListener; listener !=0; listener = listener->nextListener) {
        listener->afterNewComboLoad();
    }
}

void SynthState::propagateNewTimbre(int timbre) {
    propagateNoteOff();
    for (SynthParamListener* listener = firstParamListener; listener !=0; listener = listener->nextListener) {
        listener->newTimbre(timbre);
    }
}

void SynthState::tempoClick() {
    if (fullState.synthMode == SYNTH_MODE_MENU) {
        if (fullState.currentMenuItem->menuState == MENU_DONE
                || fullState.currentMenuItem->menuState == MENU_ERROR
                || fullState.currentMenuItem->menuState == MENU_CANCEL) {
            if (doneClick > 4) {
                fullState.synthMode = SYNTH_MODE_EDIT;
                propagateNewSynthMode();
            }
            doneClick ++;
        }
    } else {
        doneClick = 0;
    }
}

void SynthState::setParamsAndTimbre(struct OneSynthParams *newParams, int newCurrentTimbre) {
    this->params = newParams;
    this->currentTimbre = newCurrentTimbre;
}


void SynthState::copySynthParams(char* source, char* dest) {
    for (int k=0; k<sizeof(struct OneSynthParams); k++) {
        dest[k] = source[k];
    }
}


void SynthState::analyseSysexBuffer(uint8_t *buffer) {
    propagateBeforeNewParamsLoad(currentTimbre);
    storage->getPatchBank()->decodeBufferAndApplyPreset(buffer, params);
    propagateAfterNewParamsLoad(currentTimbre);
}

void SynthState::onUserChangedRow() {

    if ( !fullState.midiConfigValue[MIDICONFIG_UNLINKED_EDITING] ) {
        // Reset the row so it uses the same for each instrument
        for (int t = 0; t < NUMBER_OF_TIMBRES; ++t )
            lastRowForTimbre[t] = -1;
    }
}


int SynthState::getLastRowForTimbre( int timbre ) const
{
    if ( fullState.midiConfigValue[MIDICONFIG_UNLINKED_EDITING] )
        return lastRowForTimbre[ timbre ];
    else
        return lastRowForTimbre[ 0 ];
}

void SynthState::setLastRowForTimbre( int timbre, int row )
{
    if ( fullState.midiConfigValue[MIDICONFIG_UNLINKED_EDITING] ) {
        lastRowForTimbre[ timbre ] = row;
    } else {
        // Only remember row if there currently isn't one set; this means
        // we are cycling through the instruments and want to return to
        // the row that was set when the cycle started. This saved row is
        // invalidated when the user manually changes row.
        if ( lastRowForTimbre[ 0 ] < 0 )
            lastRowForTimbre[ 0 ] = row;
    }
}


/*
 * Randomizer
 */

int getRandomInt(int max) {
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)   {
    };
    return RNG_GetRandomNumber() % max;
}

float getRandomFloat(float min, float max) {
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY)== RESET)   {
    };
    float f = ((float)(RNG_GetRandomNumber() % 100000)) / 100000.0f;
    return f * (max - min) + min;
}


float getRandomShape(int operatorRandom) {
    int shape = getRandomInt(7);
    switch (operatorRandom) {
    case 1:
        if (shape > 2) {
            shape = 0;
        }
        break;
    case 2:
        shape = getRandomInt(8);
        if (shape == 6) { // Rand
            shape = 0;
        }
        break;
    case 3:
        break;
    }
    return shape;
}

float getRandomFrequencyType(int operatorRandom) {
    int freqType = getRandomInt(32);
    if (freqType > 1) { // Keyboard 5 times out of 6
        freqType = 0;
    }
    return freqType;
}

float getRandomFrequency(int operatorRandom) {
    float random1Frequency[] = { .5f, 1.0f, 2.0f, 4.0f};
    float random2Frequency[] = { .25, .5f, 1.0f, 1.5, 2.0f, 3.0f, 4.0f};
    float freq = 0;
    switch (operatorRandom) {
    case 1:
        freq = random1Frequency[getRandomInt(4)];
        break;
    case 2:
        freq = random2Frequency[getRandomInt(7)];
        break;
    case 3:
        freq = getRandomInt(24) * .25 + .25;
        break;
    }
    return freq;
}

float getFineTune(int operatorRandom) {
    float fineTune = 0;
    switch (operatorRandom) {
    case 2:
        fineTune = getRandomInt(10) * .01 - .05;
        if (fineTune < 0.03 || fineTune > 0.03) {
            fineTune = 0;
        }
        break;
    case 3:
        fineTune = getRandomInt(20) * .01 - .05;
        if (fineTune < 0.07 || fineTune > 0.07) {
            fineTune = 0;
        }
        break;
    }
    return fineTune;
}

void SynthState::randomizePreset() {
    int operatorRandom = fullState.randomizer.Oper;
    int envelopeTypeRandom = fullState.randomizer.EnvT;
    int imRandom = fullState.randomizer.IM;
    int modulationRandom = fullState.randomizer.Modl;


    // general

    params->engine1.velocity = 8;


    if (operatorRandom > 0) {
        params->engineMix1.mixOsc1 = getRandomFloat(0.6f, 1.0f);
        params->engineMix1.mixOsc2 = getRandomFloat(0.6f, 1.0f);
        params->engineMix2.mixOsc3 = getRandomFloat(0.6f, 1.0f);
        params->engineMix2.mixOsc4 = getRandomFloat(0.6f, 1.0f);
        params->engineMix3.mixOsc5 = getRandomFloat(0.6f, 1.0f);
        params->engineMix3.mixOsc6 = getRandomFloat(0.6f, 1.0f);

        params->engineMix1.panOsc1 = 0.0;
        params->engineMix1.panOsc2 = getRandomFloat(-0.3f, 0.3f);
        params->engineMix2.panOsc3 = getRandomFloat(-0.3f, 0.3f);
        params->engineMix2.panOsc4 = getRandomFloat(-0.5f, 0.5f);
        params->engineMix3.panOsc5 = getRandomFloat(-0.5f, 0.5f);
        params->engineMix3.panOsc6 = getRandomFloat(-0.7f, 0.7f);

        params->engine1.algo = getRandomInt(ALGO_END);
        if (getRandomInt(6) == 0) {
            params->engine1.numberOfVoice = 1;
            params->engine1.glide = getRandomInt(6) + 3;
        } else {
            params->engine1.numberOfVoice = 3;
        }
        for (int o = 0; o < 6; o++) {
            struct OscillatorParams* currentOsc =  &((struct OscillatorParams*)&params->osc1)[o];
            currentOsc->shape = getRandomShape(operatorRandom);
            currentOsc->frequencyMul = getRandomFrequency(operatorRandom);
            currentOsc->frequencyType = getRandomFrequencyType(operatorRandom);
            currentOsc->detune = getFineTune(operatorRandom);
        }

        // FX
        params->effect.param1 = getRandomFloat(0.2f, 0.8f);
        params->effect.param2 = getRandomFloat(0.2f, 0.8f);
        int effect = getRandomInt(15);
        if (effect == 1 || effect > 6) {
            params->effect.param3 = 1.0;
            params->effect.type = 0;
        } else {
            params->effect.param3 = 0.6;
            params->effect.type= effect;
        }
    }




    for (int e = 0; e < 6; e++) {
        struct EnvelopeParamsA* enva =  &((struct EnvelopeParamsA*)&params->env1a)[e * 2];
        struct EnvelopeParamsB* envb =  &((struct EnvelopeParamsB*)&params->env1b)[e * 2];

        switch (envelopeTypeRandom) {
        case 1:
            enva->attackLevel = 1.0;
            enva->attackTime = getRandomFloat(0, 0.3f);
            enva->decayLevel= getRandomFloat(0.5f, 1.0f);
            enva->decayTime = getRandomFloat(0.05, 0.5f);

            envb->sustainLevel =  getRandomFloat(0.0f, 1.0f);
            envb->sustainTime =  getRandomFloat(0.02, 1.0f);
            envb->releaseLevel = 0.0f;
            envb->releaseTime =  getRandomFloat(0.2, 5.0f);;

            break;
        case 2:
            enva->attackLevel = getRandomFloat(0.25f, 1.0f);
            enva->attackTime = getRandomFloat(0.5f, 3.0f);
            enva->decayLevel= getRandomFloat(0.5f, 1.0f);
            enva->decayTime = getRandomFloat(1.0f, 5.0f);

            envb->sustainLevel =  getRandomFloat(0.0f, 1.0f);
            envb->sustainTime =  getRandomFloat(2.0f, 5.0f);
            envb->releaseLevel = 0.0f;
            envb->releaseTime =  getRandomFloat(1.0f, 8.0f);
            break;
        case 3:
            enva->attackLevel = getRandomFloat(0, 1.0f);
            enva->attackTime = getRandomFloat(0, 1.0f);
            enva->decayLevel = getRandomFloat(0, 1.0f);
            enva->decayTime = getRandomFloat(0, 1.0f);

            envb->sustainLevel =  getRandomFloat(0, 1.0f);
            envb->sustainTime = getRandomFloat(0, 1.0f);
            envb->releaseLevel =  getRandomFloat(0, 1.0f);
            envb->releaseTime = getRandomFloat(0, 4.0f);
            break;
        }
    }

    if (imRandom > 0) {
        struct EngineIm1* im1 =  (struct EngineIm1*)&params->engineIm1;
        struct EngineIm2* im2 =  (struct EngineIm2*)&params->engineIm2;
        struct EngineIm3* im3 =  (struct EngineIm3*)&params->engineIm3;

        float min = 0;
        float max = 0;
        float minVelo = 0;
        float maxVelo = 0;

        switch (imRandom) {
        case 1:
            min = 0.25f;
            max = 2.0f;
            minVelo = 0.2f;
            maxVelo = 1.0f;
            break;
        case 2:
            min = .5f;
            max = 3.0f;
            minVelo = 1.0f;
            maxVelo = 2.0f;
            break;
        case 3:
            min = 1.0f;
            max = 5.0f;
            minVelo = 1.0f;
            maxVelo = 4.0f;
            break;
        }
        im1->modulationIndex1 = getRandomFloat(min, max);
        im1->modulationIndexVelo1 = getRandomFloat(minVelo, maxVelo);
        im1->modulationIndex2 = getRandomFloat(min, max);
        im1->modulationIndexVelo2 = getRandomFloat(minVelo, maxVelo);
        im2->modulationIndex3 = getRandomFloat(min, max);
        im2->modulationIndexVelo3 = getRandomFloat(minVelo, maxVelo);
        im2->modulationIndex4 = getRandomFloat(min, max);
        im2->modulationIndexVelo4 = getRandomFloat(minVelo, maxVelo);
        im3->modulationIndex5 = getRandomFloat(min, max);
        im3->modulationIndexVelo5 = getRandomFloat(minVelo, maxVelo);
    }

    if (modulationRandom > 0) {

        params->matrixRowState1.source = MATRIX_SOURCE_LFO1;
        params->matrixRowState2.source = MATRIX_SOURCE_LFO1;
        params->matrixRowState3.source = MATRIX_SOURCE_LFO2;
        params->matrixRowState4.source = MATRIX_SOURCE_LFO2;
        params->matrixRowState5.source = MATRIX_SOURCE_LFO3;
        params->matrixRowState6.source = MATRIX_SOURCE_LFOENV1;
        params->matrixRowState7.source = MATRIX_SOURCE_LFOENV2;
        params->matrixRowState8.source = MATRIX_SOURCE_LFOSEQ1;
        params->matrixRowState9.source = MATRIX_SOURCE_LFOSEQ2;

        params->matrixRowState10.source = MATRIX_SOURCE_MODWHEEL;
        params->matrixRowState10.mul = 2.0f;
        params->matrixRowState10.destination = INDEX_ALL_MODULATION;

        params->matrixRowState11.source = MATRIX_SOURCE_PITCHBEND;
        params->matrixRowState11.mul = 1.0f;
        params->matrixRowState11.destination = ALL_OSC_FREQ;

        params->matrixRowState12.source = MATRIX_SOURCE_AFTERTOUCH;
        params->matrixRowState12.mul = 1.0f;
        params->matrixRowState12.destination = INDEX_MODULATION1;

        for (int m = 1; m<=9; m++) {
            struct MatrixRowParams* matrixRow =  &((struct MatrixRowParams*)&params->matrixRowState1)[m - 1];
            matrixRow->mul = 0;
            matrixRow->destination = 0;
        }


        float dest[] = { INDEX_ALL_MODULATION, OSC1_FREQ, ALL_PAN, OSC2_FREQ, INDEX_MODULATION1, PAN_OSC1, INDEX_ALL_MODULATION };
        for (int i = 0; i < 2; i++) {
            struct MatrixRowParams* matrixRow =  &((struct MatrixRowParams*)&params->matrixRowState1)[getRandomInt(10)];
            matrixRow->mul = getRandomFloat(0.2f, 3.0f);
            matrixRow->destination = dest[getRandomInt(7)];
        }

        if (modulationRandom >= 2) {
            for (int i = 0; i < 3; i++) {
                struct MatrixRowParams* matrixRow =  &((struct MatrixRowParams*)&params->matrixRowState1)[getRandomInt(10)];
                matrixRow->mul = getRandomFloat(1.0f, 3.0f);
                matrixRow->destination = getRandomInt(DESTINATION_MAX);
            }
        }

        if (modulationRandom >=3) {
            for (int i = 0; i < 6; i++) {
                struct MatrixRowParams* matrixRow =  &((struct MatrixRowParams*)&params->matrixRowState1)[getRandomInt(10)];
                float mm = getRandomFloat(2.0f, 5.0f);
                matrixRow->mul = getRandomFloat(-mm, mm);
                matrixRow->destination = getRandomInt(DESTINATION_MAX);
            }
        }

        for (int o = 0; o < 3; o++) {
            struct LfoParams* osc =  &((struct LfoParams*)&params->lfoOsc1)[o];
            osc->shape = getRandomInt(5);
            osc->freq = getRandomFloat(0.2, 3 + modulationRandom*2 );
            if (getRandomInt(4) > 1) {
                osc->bias = 0;
            } else {
                osc->bias = getRandomFloat(-1.0f, 1.0f);
            }
            // PAD
            if (envelopeTypeRandom == 2) {
                osc->keybRamp = getRandomFloat(0.0f, 4.0f);
            } else {
                osc->keybRamp = getRandomFloat(0.0f, 1.0f);
            }
        }
        for (int e = 0; e < 2; e++) {
            struct EnvelopeParams* env =  &((struct EnvelopeParams*)&params->lfoEnv1)[e];
            env->attack = getRandomFloat(0.05f, 1.0f);
            env->decay = getRandomFloat(0.05f, 1.0f);
            env->sustain = getRandomFloat(0.05f, 1.0f);
            if (e==0) {
                env->release = getRandomFloat(0.05f, 1.0f);
            } else {
                params->lfoEnv2.loop = getRandomInt(2) + 1;
            }
        }

        int bpm = getRandomInt(120) + 60;
        for (int s = 0; s < 2; s++) {
            struct StepSequencerParams* stepSeq =  &((struct StepSequencerParams*)&params->lfoSeq1)[s];
            stepSeq->bpm = bpm;
            stepSeq->gate = getRandomFloat(0.25, 1);

            struct StepSequencerSteps* steps =  &((struct StepSequencerSteps*)&params->lfoSteps1)[s];
            for (int k =0; k<16; k++) {
                if ((k % 4) == 0) {
                    steps->steps[k] = getRandomInt(5) + 11;
                } else {
                    steps->steps[k] = getRandomInt(10) ;
                }
            }
        }
    }
}

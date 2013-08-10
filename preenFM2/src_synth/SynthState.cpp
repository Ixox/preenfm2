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

#include "SynthState.h"
#include "Hexter.h"

#include "LiquidCrystal.h"
extern LiquidCrystal      lcd;


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

struct ParameterRowDisplay engineIM1ParameterRow = {
        "Modulation" ,
        { "IM1 ", "IM2 ", "IM3 ", "IM4 "},
        {
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay engineIM2ParameterRow = {
        "Modulation" ,
        { "IM5 ", "IM6 ", "IM7 ", "IM8 "},
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



const char* oscShapeNames []=  {"sin ", "saw ", "squa", "s^2 ", "szer", "spos", "rand", "off "} ;



const char* oscTypeNames [] = { "keyb", "fixe"};
struct ParameterRowDisplay oscParameterRow = {
        "Oscillator",
        { "Shap", "FTyp", "Freq", "FTun" },
        {
                { OSC_SHAPE_SIN, OSC_SHAPE_LAST -1, OSC_SHAPE_LAST, DISPLAY_TYPE_STRINGS, oscShapeNames, nullNamesOrder, nullNamesOrder},
                { OSC_FT_KEYBOARD, OSC_FT_FIXE, 2, DISPLAY_TYPE_STRINGS, oscTypeNames, nullNamesOrder, nullNamesOrder},
                { 0, 16, 193, DISPLAY_TYPE_FLOAT_OSC_FREQUENCY ,  nullNames, nullNamesOrder, nullNamesOrder },
                { -1, 1, 201, DISPLAY_TYPE_FLOAT_OSC_FREQUENCY,  nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay envParameterRow1 = {
        "Enveloppe",
        { "Attk", "lv  ", "Deca", "lv " },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay envParameterRow2 = {
        "Enveloppe",
        { "Sust", "lv  ", "Rele", "lv  " },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay lfoEnvParameterRow = {
        "LFO Env",
        { "Attk", "Deca", "Sust", "Rele" },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_INT_OR_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* lofEnv2Loop [] = { "No  ", "Sile", "Attk"};
struct ParameterRowDisplay lfoEnv2ParameterRow = {
        "LFO Env",
        { "Sile", "Attk", "Deca", "Loop" },
        {
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 16, 1601, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { LFO_ENV2_NOLOOP, LFO_ENV2_LOOP_ATTACK, 3, DISPLAY_TYPE_STRINGS, lofEnv2Loop, nullNamesOrder, nullNamesOrder }
        }
};

const char* matrixSourceNames [] = { "None", "lfo1", "lfo2", "lfo3", "env1", "env2", "seq1", "seq2",
		"ModW", "PitB", "AftT",  "Velo", "Key " } ;

const char* matrixDestNames [] = {
        "None", "Gate", "IM1 ", "IM2 ", "IM3 ", "IM4 ", "IM* ",
        "Mix1", "Pan1", "Mix2", "Pan2", "Mix3", "Pan3", "Mix3", "Pan3", "Mix*", "Pan*",
        "o1Fq", "o2Fq", "o3Fq", "o4Fq", "o5Fq", "o6Fq", "o*Fq",
        "Att1", "Att2", "Att3", "Att4", "Att5", "Att6", "Att*", "Rel*",
	    "mx01", "mx02", "mx03", "mx04",
        "l1Fq", "l2Fq", "l3Fq", "e2si", "s1ga", "s2ga"
	   } ;


struct ParameterRowDisplay matrixParameterRow = {
        "Matrix",
        { "Srce", "Mult", "Dest", "    " },
        {
                { MATRIX_SOURCE_NONE, MATRIX_SOURCE_MAX-1, MATRIX_SOURCE_MAX, DISPLAY_TYPE_STRINGS, matrixSourceNames, nullNamesOrder, nullNamesOrder},
                { -10, 10, 2001, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { DESTINATION_NONE, DESTINATION_MAX-1, DESTINATION_MAX, DISPLAY_TYPE_STRINGS, matrixDestNames, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* lfoOscMidiClock[] =  { "M/16", "MC/8", "MC/4", "MC/2", "MClk", "MC*2", "MC*3", "MC*4", "MC*8"};

const char* lfoShapeNames [] =  { "Sin ", "Ramp",  "Saw ","Squa", "Rand" } ;


struct ParameterRowDisplay lfoParameterRow = {
        "LFO",
        { "Shap", "Freq", "Bias", "KSyn" },
        {
                { LFO_SIN, LFO_TYPE_MAX-1, LFO_TYPE_MAX, DISPLAY_TYPE_STRINGS,  lfoShapeNames, nullNamesOrder, nullNamesOrder},
                { 0, 24.9, 250,DISPLAY_TYPE_FLOAT_LFO_FREQUENCY, nullNames, nullNamesOrder, nullNamesOrder },
                { -1, 1, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
        }
};



const char* lfoSeqMidiClock[] =  { "MC/4", "MC/2", "MC  ", "MC*2", "MC*4" };

struct ParameterRowDisplay lfoStepParameterRow = {
        "LFO Seq",
        { "Bpm ", "Gate", "    ", "    " },
        {
                { 10 ,245, 236, DISPLAY_TYPE_STEP_SEQ_BPM, nullNames, nullNamesOrder, nullNamesOrder},
                { 0 , 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder},
                { 0, 0, 0, DISPLAY_TYPE_STEP_SEQ1, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 0, 0, DISPLAY_TYPE_STEP_SEQ2, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct AllParameterRowsDisplay allParameterRows = {
        {
                &engine1ParameterRow,
                &engineIM1ParameterRow,
                &engineIM2ParameterRow,
                &engineMix1ParameterRow,
                &engineMix2ParameterRow,
                &engineMix3ParameterRow,
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
                &lfoParameterRow,
                &lfoParameterRow,
                &lfoParameterRow,
                &lfoEnvParameterRow,
                &lfoEnv2ParameterRow,
                &lfoStepParameterRow,
                &lfoStepParameterRow
        }
};




SynthState::SynthState() {
    engineRow =  ROW_ENGINE;
    // operator works for both osc and env
    operatorRow  = 0;
    matrixRow = ROW_MATRIX1;
    lfoRow    = ROW_LFOOSC1;

    // First default preset
    fullState.synthMode = SYNTH_MODE_EDIT;
    fullState.preenFMBankNumber = 0;
    fullState.preenFMPresetNumber = 0;
    fullState.preenFMComboNumber = 0;
    fullState.preenFMComboPresetNumber = 0;
    fullState.dx7BankNumber = 0;
    fullState.dx7PresetNumber = 0;
    fullState.loadWhat = 0;
    fullState.saveWhat = 0;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL1] = 1; // all
    fullState.midiConfigValue[MIDICONFIG_CHANNEL2] = 2;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL3] = 3;
    fullState.midiConfigValue[MIDICONFIG_CHANNEL4] = 4;
    fullState.midiConfigValue[MIDICONFIG_THROUGH] = 0;
    fullState.midiConfigValue[MIDICONFIG_RECEIVES] = 3;
    fullState.midiConfigValue[MIDICONFIG_SENDS] = 1;
    fullState.midiConfigValue[MIDICONFIG_REALTIME_SYSEX] = 0;
    fullState.midiConfigValue[MIDICONFIG_BOOT_START] = 0;
    fullState.midiConfigValue[MIDICONFIG_TEST_NOTE] = 60;
    fullState.midiConfigValue[MIDICONFIG_TEST_VELOCITY] = 120;
    fullState.midiConfigValue[MIDICONFIG_ENCODER] = 0;
    fullState.firstMenu = 0;

    for (int k=0; k<12; k++) {
        fullState.name[k] = 0;
    }

    // edit with timbre 0
    currentTimbre = 0;
    currentRow = 0;


    stepSelect[0] = 0;
    stepSelect[1] = 0;

    isPlayingNote = false;

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




void SynthState::encoderTurnedWhileButtonPressed(int encoder, int ticks, int button) {
    int oldCurrentRow = currentRow;

    if (fullState.synthMode == SYNTH_MODE_EDIT)  {
    	switch (button) {
    	case BUTTON_SYNTH:
    	case BUTTON_OSC:
    	case BUTTON_ENV:
    	case BUTTON_MATRIX:
    	case BUTTON_LFO:
    		changeSynthModeRow(button , ticks>0 ? 1 : -1);
    		break;
    	case BUTTON_BACK:
    		encoderTurned(encoder, ticks * 10);
    		break;
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
    }
    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
    }
}

void SynthState::encoderTurned(int encoder, int ticks) {
    if (fullState.synthMode == SYNTH_MODE_EDIT) {

        // Step sequencer special case
        if (currentRow >= ROW_LFOSEQ1) {
    		if (encoder >= 2) {
    			encoderTurnedForStepSequencer(currentRow, encoder, ticks);
    			return;
    		}
    	};

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

			// floating point test to be sure numberOfValues is diferent from 1.
			// for voices when number of voices forced to 0
			if (param->numberOfValues < 1.5) {
				return;
			}


			float &value = ((float*)params)[num];
			oldValue = value;
			float inc = ((param->maxValue - param->minValue) / (param->numberOfValues - 1.0f));

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
    } else {
        int oldMenuSelect = fullState.menuSelect;
        if (encoder==0) {
            if (ticks>0) {
                fullState.menuSelect = fullState.menuSelect + 1;
            } else if (ticks<0) {
                fullState.menuSelect = fullState.menuSelect - 1;
            }
        } else if (encoder==1) {
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
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
        } else if (encoder==2) {
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
        } else if (encoder==3) {
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
                propagateNewMidiConfig(fullState.menuSelect, fullState.midiConfigValue[fullState.menuSelect]);
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
        		if (fullState.dx7Bank != storage->getDx7Bank(fullState.menuSelect)) {
        			fullState.dx7Bank = storage->getDx7Bank(fullState.menuSelect);
        			fullState.dx7PresetNumber = 0;
        		}
        		break;
        	case MENU_LOAD_SELECT_DX7_PRESET:
                propagateBeforeNewParamsLoad();
        		hexter->loadHexterPatch(storage->dx7LoadPatch(fullState.dx7Bank, fullState.menuSelect), params);
                propagateAfterNewParamsLoad();
                fullState.dx7PresetNumber = fullState.menuSelect;
                break;
        	case MENU_LOAD_SELECT_BANK:
        	case MENU_SAVE_SELECT_BANK:
        		// Did we really change bank?
        		if (fullState.preenFMBank != storage->getPreenFMBank(fullState.menuSelect)) {
        			fullState.preenFMBank = storage->getPreenFMBank(fullState.menuSelect);
        			fullState.preenFMPresetNumber = 0;
        		}
        		break;
        	case MENU_RENAME_SELECT_BANK:
        		// Did we really change bank?
        		if (fullState.preenFMBank != storage->getPreenFMBank(fullState.menuSelect)) {
        			fullState.preenFMBank = storage->getPreenFMBank(fullState.menuSelect);
        		}
        		break;
        	case MENU_LOAD_SELECT_COMBO:
        	case MENU_SAVE_SELECT_COMBO:
        		// Did we really change bank?
        		if (fullState.preenFMCombo != storage->getPreenFMCombo(fullState.menuSelect)) {
        			fullState.preenFMCombo = storage->getPreenFMCombo(fullState.menuSelect);
        			fullState.preenFMComboPresetNumber = 0;
        		}
        		break;
			case MENU_LOAD_SELECT_BANK_PRESET:
			case MENU_SAVE_SELECT_BANK_PRESET:
				propagateBeforeNewParamsLoad();
				storage->loadPreenFMPatch(fullState.preenFMBank, fullState.menuSelect, params);
				propagateAfterNewParamsLoad();
                fullState.preenFMPresetNumber = fullState.menuSelect;
                break;
			case MENU_LOAD_SELECT_COMBO_PRESET:
			case MENU_SAVE_SELECT_COMBO_PRESET:
                fullState.preenFMComboPresetNumber = fullState.menuSelect;
                break;
            }
            propagateNewMenuSelect();
        }
    }

}


void SynthState::resetDisplay() {
    fullState.synthMode = SYNTH_MODE_EDIT;
    currentRow = 0;
    propagateNewSynthMode();
}

bool SynthState::isCurrentRowAvailable() {

	if (algoInformation[(int)params->engine1.algo].mix <= 2) {
		if (currentRow == ROW_OSC_MIX2 ) {
			return false;
		}
	}
	if (algoInformation[(int)params->engine1.algo].mix <= 4) {
		if (currentRow == ROW_OSC_MIX3 ) {
			return false;
		}
	}
	if (algoInformation[(int)params->engine1.algo].im == 0) {
		if (currentRow == ROW_MODULATION1 ) {
			return false;
		}
	}
	if (algoInformation[(int)params->engine1.algo].im <= 4) {
		if (currentRow == ROW_MODULATION2 ) {
			return false;
		}
	}
	return true;
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
			lastBecauseOfAlgo = ROW_OSC_FIRST + algoInformation[(int)params->engine1.algo].osc - 1;
			if (currentRow<ROW_OSC_FIRST || currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_OSC_FIRST + operatorRow;
			} else {
				currentRow += step;
				envelopeRow = (currentRow - ROW_OSC_FIRST)*2;
			}
			if (currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_OSC_FIRST;
			} else if (currentRow<ROW_OSC_FIRST) {
				currentRow = lastBecauseOfAlgo;
			}
			operatorRow = currentRow - ROW_OSC_FIRST;
			break;
		case BUTTON_ENV:
			lastBecauseOfAlgo = ROW_ENV_FIRST - 1 + algoInformation[(int)params->engine1.algo].osc  * 2;
			if (currentRow<ROW_ENV_FIRST || currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_ENV_FIRST + envelopeRow;
			} else {
				currentRow += step;
				operatorRow = (currentRow - ROW_ENV_FIRST) >> 1;
			}
			if (currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_ENV_FIRST;
			} else if (currentRow<ROW_ENV_FIRST) {
				currentRow = lastBecauseOfAlgo;
			}
			envelopeRow = currentRow - ROW_ENV_FIRST;
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
			|| currentItem == MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME
			|| currentItem == MENU_RENAME_BANK;
}

void SynthState::buttonPressed(int button) {
    SynthEditMode oldSynthMode = fullState.synthMode;
    int oldCurrentRow = currentRow;
    // int oldMenuSelect = fullState.menuSelect;
    MenuState oldMenuState = fullState.currentMenuItem->menuState;


    if (fullState.synthMode == SYNTH_MODE_EDIT)  {
        switch (button) {
        case BUTTON_SYNTH:
        case BUTTON_OSC:
        case BUTTON_ENV:
        case BUTTON_MATRIX:
        case BUTTON_LFO:
        	changeSynthModeRow(button , 1);
            break;
        case BUTTON_MENUSELECT:
            fullState.synthMode = SYNTH_MODE_MENU;
            fullState.menuSelect = fullState.firstMenu;
            // allow undo event after trying some patches
            PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
            fullState.currentMenuItem = MenuItemUtil::getMenuItem(MAIN_MENU);
            break;
        case BUTTON_BACK:
            currentTimbre++;
            currentTimbre &= 3;
            propagateNewTimbre(currentTimbre);
        	break;
        }
    } else {
        // Any button when done is display makes the synth go back to edit mode.
		// MENU MODE
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
#ifdef DEBUG
		case BUTTON_DUMP:
		{
			PresetUtil::dumpPatch();
			break;
		}
#endif
		case BUTTON_LFO:
		{
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
				fullState.name[fullState.menuSelect] = 0;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_MATRIX:
		{
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
				fullState.name[fullState.menuSelect] = 27;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_ENV:
		{
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
				fullState.name[fullState.menuSelect] = 55;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_OSC:
		{
            if (isEnterNameState(fullState.currentMenuItem->menuState)) {
				fullState.name[fullState.menuSelect] = 66;
				propagateNewMenuSelect();
			}
			break;
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



void SynthState::setNewStepValue(int timbre, int whichStepSeq, int step, int newValue) {
	if (whichStepSeq <0 || whichStepSeq>1 || step <0 || step > 15 || newValue<0 || newValue>15) {
		return;
	}
	StepSequencerSteps * seqSteps = &((StepSequencerSteps * )(&params->lfoSteps1))[whichStepSeq];

	int oldValue = seqSteps->steps[step];

	if (oldValue !=  newValue) {
		int oldStep = stepSelect[whichStepSeq];
		seqSteps->steps[step] = newValue;
		stepSelect[whichStepSeq] = step;
		if (oldStep != step) {
			propagateNewParamValueFromExternal(timbre, ROW_LFOSEQ1 + whichStepSeq, 2, NULL, oldStep, stepSelect[whichStepSeq]);
		}
		propagateNewParamValueFromExternal(timbre, ROW_LFOSEQ1 + whichStepSeq, 3, NULL, oldValue, newValue);
	}

}



void SynthState::setNewValue(int timbre, int row, int encoder, float newValue) {
    // ??? STILL USEFULL
/*
    int index = row * NUMBER_OF_ENCODERS + encoder;
    struct ParameterDisplay* param = &(allParameterRows.row[row]->params[encoder]);
    int oldValue = ((float*)params)[index];
    if (newValue > param->maxValue) {
    	newValue= param->maxValue;
    } else if (newValue < param->minValue) {
    	newValue= param->minValue;
    }
    ((float*)params)[index] = newValue;
    propagateNewParamValueFromExternal(timbre, row, encoder, param, oldValue, newValue);
    */
}

const MenuItem* SynthState::afterButtonPressed() {
    const MenuItem* rMenuItem = 0;

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
    case MENU_LOAD_SELECT_BANK_PRESET:
        propagateBeforeNewParamsLoad();
        storage->loadPreenFMPatch(fullState.preenFMBank, fullState.menuSelect, params);
        PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
        propagateAfterNewParamsLoad();
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        propagateBeforeNewParamsLoad();
        storage->loadPreenFMCombo(fullState.preenFMCombo, fullState.menuSelect);
        // Update and clean all timbres
        this->currentTimbre = 0;
        propagateNewTimbre(currentTimbre);
        PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
        propagateAfterNewComboLoad();
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
    	// propagateBeforeNewParamsLoad();
		// hexter->loadHexterPatch(storage->dx7LoadPatch(&fullState.dx7Bank, fullState.menuSelect), params);
		PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
		// propagateAfterNewParamsLoad();
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
        fullState.preenFMBankNumber = fullState.menuSelect;
        break;
    case MENU_SAVE_SELECT_COMBO_PRESET:
    {
        const char* comboName = storage->readComboName(fullState.menuSelect);
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
        int length;
        for (length=12; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            params->presetName[k] = allChars[(int)fullState.name[k]];
        }
        params->presetName[length] = '\0';
        break;
    case MENU_SAVE_ENTER_PRESET_NAME:
    {
        int length;
        for (length=12; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
        	params->presetName[k] = allChars[(int)fullState.name[k]];
        }
        params->presetName[length] = '\0';
        storage->savePreenFMPatch(fullState.preenFMBank, fullState.preenFMPresetNumber, params);
        break;
    }
    case MENU_SAVE_ENTER_COMBO_NAME:
    {
        int length;
        for (length=12; fullState.name[length-1] == 0; length--);
        char comboName[12];
        for (int k=0; k<length; k++) {
        	comboName[k] = allChars[(int)fullState.name[k]];
        }
        comboName[length] = '\0';
        storage->savePreenFMCombo(fullState.preenFMCombo, fullState.preenFMComboNumber, comboName);
        break;
    }
    case MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME:
    {
    	// Must save the bank here....
        int length;
        for (length=8; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
        	fullState.name[k] = allChars[(int)fullState.name[k]];
        }
        fullState.name[length++] = '.';
        fullState.name[length++] = 'b';
        fullState.name[length++] = 'n';
        fullState.name[length++] = 'k';
        fullState.name[length] = '\0';
        if (!storage->bankNameExist(fullState.name)) {
        	const MenuItem *cmi = fullState.currentMenuItem;
        	// Update display while sending
        	lcd.setRealTimeAction(true);
        	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
        	propagateNewMenuState();
        	storage->saveBank(fullState.name, sysexTmpMem + 8);
            fullState.currentMenuItem = cmi;
        	lcd.setRealTimeAction(false);
        } else {
        	rMenuItem = MenuItemUtil::getMenuItem(MENU_SAVE_SYSEX_BANK_CONFIRM_OVERRIDE);
        }
        break;
    }
    case MENU_SAVE_SYSEX_BANK_CONFIRM_OVERRIDE:
    {
    	const MenuItem *cmi = fullState.currentMenuItem;
    	// Update display while sending
    	lcd.setRealTimeAction(true);
    	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
    	propagateNewMenuState();
    	storage->saveBank(fullState.name, sysexTmpMem + 8);
        fullState.currentMenuItem = cmi;
    	lcd.setRealTimeAction(false);
    	break;
    }
    case MENU_SAVE_SYSEX_PATCH:
        PresetUtil::sendCurrentPatchToSysex();
        break;
    case MENU_RENAME_BANK:
    {
    	int k;
    	for (k=0; k<8 && fullState.name[k]!=0; k++) {
        	fullState.name[k] =  allChars[fullState.name[k]];
    	}
    	fullState.name[k++] = '.';
    	fullState.name[k++] = 'b';
    	fullState.name[k++] = 'n';
    	fullState.name[k++] = 'k';
    	fullState.name[k++] = '\0';
    	if (storage->renameBank(fullState.preenFMBank, fullState.name) > 0) {
        	rMenuItem = MenuItemUtil::getMenuItem(MENU_ERROR);
    	}
    	break;
    }
    case MENU_SAVE_SYSEX_BANK:
    {
		if (storage->getPreenFMBank(fullState.menuSelect)->fileType == FILE_EMPTY) {
    		return fullState.currentMenuItem;
		}
    	const MenuItem *cmi = fullState.currentMenuItem;
    	// Update display while sending
    	lcd.setRealTimeAction(true);
    	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
    	propagateNewMenuState();
        PresetUtil::sendBankToSysex(fullState.menuSelect);
        fullState.currentMenuItem = cmi;
    	lcd.setRealTimeAction(false);
        break;
    }
    case MENU_CANCEL:
    case MENU_DONE:
    case MENU_ERROR:
        fullState.synthMode = SYNTH_MODE_EDIT;
        break;
    case MENU_FORMAT_BANK:
    	if (fullState.menuSelect == 25) {
        	const MenuItem *cmi = fullState.currentMenuItem;
        	// Update display while formating
        	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_IN_PROGRESS);
        	propagateNewMenuState();
            storage->createPatchBank();
            storage->createComboBank();
            fullState.currentMenuItem = cmi;
    	} else {
    		return fullState.currentMenuItem;
    	}
        break;
    case MENU_CONFIG_SETTINGS_SAVE:
        storage->saveConfig(fullState.midiConfigValue);
        break;
    case MENU_DEFAULT_COMBO_SAVE:
        storage->saveDefaultCombo();
        break;
    case MENU_DEFAULT_COMBO_RESET:
        storage->removeDefaultCombo();
        break;
    case MENU_LOAD:
        fullState.loadWhat = fullState.menuSelect;
        break;
    case MENU_SAVE:
        fullState.saveWhat = fullState.menuSelect;
        break;
     default:
        break;
    }

    // ---------------------
    // Next state switch

    switch (rMenuItem->menuState) {
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
		propagateBeforeNewParamsLoad();
		storage->loadPreenFMPatch(fullState.preenFMBank, fullState.preenFMPresetNumber, params);
		propagateAfterNewParamsLoad();
        fullState.menuSelect = fullState.preenFMPresetNumber;
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboPresetNumber;
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
		propagateBeforeNewParamsLoad();
		hexter->loadHexterPatch(storage->dx7LoadPatch(fullState.dx7Bank, fullState.dx7PresetNumber), params);
		propagateAfterNewParamsLoad();
        fullState.menuSelect = fullState.dx7PresetNumber;
        break;
    case MENU_LOAD:
        fullState.menuSelect = fullState.loadWhat;
        break;
    case MENU_SAVE:
        fullState.menuSelect = fullState.saveWhat;
        break;
    case MENU_RENAME_SELECT_BANK:
    case MENU_LOAD_SELECT_BANK:
    case MENU_SAVE_SELECT_BANK:
        fullState.menuSelect = fullState.preenFMBankNumber;
        fullState.preenFMBank = storage->getPreenFMBank(fullState.menuSelect);
        break;
    case MENU_LOAD_SELECT_COMBO:
    case MENU_SAVE_SELECT_COMBO:
        fullState.menuSelect = fullState.preenFMComboNumber;
        fullState.preenFMCombo = storage->getPreenFMCombo(fullState.menuSelect);
        break;
    case MENU_LOAD_SELECT_DX7_BANK:
        fullState.menuSelect = fullState.dx7BankNumber;
    	fullState.dx7Bank = storage->getDx7Bank(fullState.menuSelect);
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
    case MENU_SAVE_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboNumber;
        break;
    case MENU_LOAD_SELECT_BANK_PRESET:
        fullState.menuSelect = fullState.preenFMBankNumber;
        propagateBeforeNewParamsLoad();
        PresetUtil::copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad();
        break;
    case MENU_LOAD_SELECT_COMBO_PRESET:
        fullState.menuSelect = fullState.preenFMComboNumber;
        propagateBeforeNewParamsLoad();
        PresetUtil::copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad();
        break;
    case MENU_LOAD_SELECT_DX7_PRESET:
        fullState.menuSelect = fullState.dx7BankNumber;
        propagateBeforeNewParamsLoad();
        PresetUtil::copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad();
        break;
    case MENU_SAVE_SYSEX_BANK_CONFIRM_OVERRIDE:
    case MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME:
    	// CANCEL OPERATION
    	return MenuItemUtil::getMenuItem(MENU_CANCEL);
    	break;
    case MAIN_MENU:
        fullState.synthMode = SYNTH_MODE_EDIT;
        // put back old patch (has been overwritten if a new patch has been loaded)
        break;
    }
    rMenuItem = MenuItemUtil::getParentMenuItem(fullState.currentMenuItem->menuState);
    return rMenuItem;
}


void SynthState::newSysexBankReady() {
	fullState.synthMode = SYNTH_MODE_MENU;
	fullState.menuSelect = 0;
	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME);

	int k = 0;
	for (k=0; storage->getPreenFMBank(k)->fileType != FILE_EMPTY && k < NUMBEROFPREENFMBANKS; k++);
	if (k == NUMBEROFPREENFMBANKS) {
		// NO EMPTY BANK....
		// TODO : ERROR TO WRITE
		return;
	}
	for (int k=0; k<8; k++) {
		for (int j=0; j<getLength(allChars); j++) {
			if (sysexTmpMem[k] == allChars[j]) {
				fullState.name[k] = j;
			}
		}
	}

	propagateNewMenuState();
}



void SynthState::propagateAfterNewParamsLoad() {
   for (SynthParamListener* listener = firstParamListener; listener !=0; listener = listener->nextListener) {
	   listener->afterNewParamsLoad(currentTimbre);
   }
}

void SynthState::propagateAfterNewComboLoad() {
   for (SynthParamListener* listener = firstParamListener; listener !=0; listener = listener->nextListener) {
	   listener->afterNewComboLoad();
   }
}

void SynthState::propagateNewTimbre(int timbre) {
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

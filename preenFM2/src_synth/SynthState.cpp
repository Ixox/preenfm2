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
const char* algoNames [] = { "alg1", "alg2", "alg3", "alg4", "alg5", "alg6", "alg7", "alg8", "alg9", "al10", "al11", "al12", "al13", "al14", "al15", "al16", "al17"  };
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
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay engineIM2ParameterRow = {
        "Modulation" ,
        { "IM5 ", "IM6 ", "IM7 ", "IM8 "},
        {
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                {0, 16, 193, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
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
                { 0, 8, 193, DISPLAY_TYPE_FLOAT ,  nullNames, nullNamesOrder, nullNamesOrder },
                { -1, 1, 201, DISPLAY_TYPE_FLOAT,  nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay envParameterRow = {
        "Enveloppe",
        { "Attk", "Deca", "Sust", "Rele" },
        {
                { 0, 2, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

struct ParameterRowDisplay lfoEnvParameterRow = {
        "LFO Env",
        { "Attk", "Deca", "Sust", "Rele" },
        {
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 1, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_INT_OR_NONE, nullNames, nullNamesOrder, nullNamesOrder }
        }
};

const char* lofEnv2Loop [] = { "No  ", "Sile", "Attk"};
struct ParameterRowDisplay lfoEnv2ParameterRow = {
        "LFO Env",
        { "Sile", "Attk", "Deca", "Loop" },
        {
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { 0, 4, 101, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
                { LFO_ENV2_NOLOOP, LFO_ENV2_LOOP_ATTACK, 3, DISPLAY_TYPE_STRINGS, lofEnv2Loop, nullNamesOrder, nullNamesOrder }
        }
};

const char* matrixSourceNames [] = { "None", "lfo1", "lfo2", "lfo3", "env1", "env2", "seq1", "seq2",
        "PitB", "AftT", "ModW", "Velo", "Key", "CC1 ", "CC2 ", "CC3 ", "CC4 "} ;

const char* matrixDestNames [] = {
        "None", "Gate", "IM1 ", "IM2 ", "IM3 ", "IM4 ",
        "o1Fq", "o2Fq", "o3Fq", "o4Fq", "o5Fq", "o6Fq", "o*Fq",
        "l1Fq", "l2Fq", "l3Fq", "e2si", "s1ga", "s2ga",
        "Mix1", "Pan1", "Mix2", "Pan2", "Mix3", "Pan3", "Mix3", "Pan3", "Mix*", "Pan*",
        "Env1", "Env2", "Env3", "Env4", "Env5", "Env6", "Env*",
	    "mx01", "mx02", "mx03", "mx04", "mx05", "mx06", "mx07", "mx08", "mx09", "mx10", "mx11", "mx12"
	   } ;


struct ParameterRowDisplay matrixParameterRow = {
        "Matrix",
        { "Srce", "Mult", "Dest", "    " },
        {
                { MATRIX_SOURCE_NONE, MATRIX_SOURCE_MAX-1, MATRIX_SOURCE_MAX, DISPLAY_TYPE_STRINGS, matrixSourceNames, nullNamesOrder, nullNamesOrder},
                { -10, 10, 201, DISPLAY_TYPE_FLOAT, nullNames, nullNamesOrder, nullNamesOrder },
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
                { LFO_SAW, LFO_TYPE_MAX-1, 1, DISPLAY_TYPE_STRINGS,  lfoShapeNames, nullNamesOrder, nullNamesOrder},
                { 0, 24.9, 250,DISPLAY_TYPE_LFO_HZ, nullNames, nullNamesOrder, nullNamesOrder },
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
                &envParameterRow,
                &envParameterRow,
                &envParameterRow,
                &envParameterRow,
                &envParameterRow,
                &envParameterRow,
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
    fullState.bankNumber = 0;
    fullState.dx7BankNumber = 0;
    fullState.presetNumber = 0;
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
    fullState.midiConfigValue[MIDICONFIG_ECHANNEL] = 0;
    fullState.midiConfigValue[MIDICONFIG_ECC1] = 115;
    fullState.midiConfigValue[MIDICONFIG_ECC2] = 116;
    fullState.midiConfigValue[MIDICONFIG_ECC3] = 117;
    fullState.midiConfigValue[MIDICONFIG_ECC4] = 118;
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
        	// NOTE !!!!
        	break;
        }
    }
    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
    }
}

void SynthState::encoderTurned(int encoder, int ticks) {
    if (fullState.synthMode == SYNTH_MODE_EDIT) {
        int num = encoder + currentRow * NUMBER_OF_ENCODERS;

        // Step sequencer special case
        if (currentRow >= ROW_LFOSEQ1) {
    		if (encoder >= 2) {
    			encoderTurnedForStepSequencer(currentRow, encoder, ticks);
    			return;
    		}
    	};

        struct ParameterDisplay* param = &(allParameterRows.row[currentRow]->params[encoder]);
        float newValue;
        float oldValue;


		if (param->displayType == DISPLAY_TYPE_STRINGS) {
			// Do not use encoder acceleration
			ticks = ticks > 0 ? 1 : -1;
		}

		if (param->valueNameOrder == NULL) {
		    if (currentRow == ROW_MODULATION1 || currentRow == ROW_MODULATION2) {
		        // Specific rule for modulation
                float &value = ((float*)params)[num];
                oldValue = value;
                if (value < 1.0f || (value == 1.0f && ticks < 0)) {
                    int tickIndex = ((float)value / .02f) + .0005f + ticks;
                    newValue = tickIndex * .02f;
                    if (newValue > 1.0f) {
                        newValue = 1.0;
                    }
                    if (newValue < 0.0) {
                        newValue = 0.0;
                    }
                } else  {
                    float inc = .1;
                    int tickIndex = (value - 1.0f) / inc + .0005f + ticks;
                    newValue = 1.0f + tickIndex * inc;
                    if (newValue > param->maxValue) {
                        newValue = param->maxValue;
                    }
                    if (newValue < 1.0f) {
                        newValue = 1.0f;
                    }
                }
                value = newValue;
		    } else {
		        // floating point test to be sure numberOfValues is diferent from 1.
		        if (param->numberOfValues < 1.5) {
		            return;
		        }

                float &value = ((float*)params)[num];
                oldValue = value;
                float inc = ((param->maxValue - param->minValue) / (param->numberOfValues - 1.0f));
                int tickIndex = (value - param->minValue) / inc + .0005f+ ticks;
                newValue = param->minValue + tickIndex * inc;
                propagateNewParamCheck(encoder, oldValue, &newValue);
                if (newValue > param->maxValue) {
                    newValue = param->maxValue;
                }
                if (newValue < param->minValue) {
                    newValue = param->minValue;
                }
                value = newValue;
		    }
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
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
                fullState.name[fullState.menuSelect] = (fullState.name[fullState.menuSelect] + (ticks>0? 1: -1));
                if (fullState.name[fullState.menuSelect]<28) {
                    fullState.name[fullState.menuSelect]=28;
                }
                if (fullState.name[fullState.menuSelect]> 53) {
                    fullState.name[fullState.menuSelect] = 53;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->maxValue == 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 5;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 5;
                }
            }
        } else if (encoder==2) {
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
                fullState.name[fullState.menuSelect] = (fullState.name[fullState.menuSelect] + (ticks>0? 1: -1));
                if (fullState.name[fullState.menuSelect]<1) {
                    fullState.name[fullState.menuSelect]=1;
                }
                if (fullState.name[fullState.menuSelect]> 26) {
                    fullState.name[fullState.menuSelect] = 26;
                }
                propagateNewMenuSelect();
            } else if (fullState.currentMenuItem->maxValue == 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 10;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 10;
                }
            }
        } else if (encoder==3) {
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
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
            } else if (fullState.currentMenuItem->maxValue == 128) {
                if (ticks>0) {
                    fullState.menuSelect = fullState.menuSelect + 25;
                } else if (ticks<0) {
                    fullState.menuSelect = fullState.menuSelect - 25;
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
        	case MENU_LOAD_DX7_SELECT_BANK:
        		// Did we really change DX7 bank?
        		if (fullState.dx7BankName != storage->getDx7BankName(fullState.menuSelect)) {
        			fullState.dx7BankName = storage->getDx7BankName(fullState.menuSelect);
        			fullState.dx7PresetNumber = 0;
        		}
        		break;
        	case MENU_LOAD_DX7_SELECT_PRESET:
                propagateBeforeNewParamsLoad();
        		hexter->loadHexterPatch(storage->dx7LoadPatch(fullState.dx7BankName, fullState.menuSelect), params);
                propagateAfterNewParamsLoad();
                fullState.dx7PresetNumber = fullState.menuSelect;
                break;
			case MENU_LOAD_USER_SELECT_PRESET:
                if (fullState.bankNumber < 4) {
                    propagateBeforeNewParamsLoad();
                    storage->loadPatch(fullState.bankNumber, fullState.menuSelect, params);
                    propagateAfterNewParamsLoad();
                }
                fullState.presetNumber = fullState.menuSelect;
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


void SynthState::changeSynthModeRow(int button, int step) {
	unsigned char lastBecauseOfAlgo;

	switch (button) {
		case BUTTON_SYNTH:
			if (currentRow<ROW_ENGINE_FIRST || currentRow>ROW_ENGINE_LAST) {
				currentRow = engineRow;
			} else {
				currentRow += step;
				if (algoInformation[(int)params->engine1.algo].mix <= 2) {
					if (currentRow == ROW_OSC_MIX2 ) {
						currentRow += step;
					}
				}
				if (algoInformation[(int)params->engine1.algo].mix <= 4) {
					if (currentRow == ROW_OSC_MIX3 ) {
						currentRow += step;
					}
				}
				// Again so that it works in both direction
				if (algoInformation[(int)params->engine1.algo].mix <= 2) {
					if (currentRow == ROW_OSC_MIX2 ) {
						currentRow += step;
					}
				}
				if (algoInformation[(int)params->engine1.algo].im == 0) {
					if (currentRow == ROW_MODULATION1 ) {
						currentRow += step;
					}
				}
				if (algoInformation[(int)params->engine1.algo].im <= 4) {
					if (currentRow == ROW_MODULATION2 ) {
						currentRow += step;
					}
				}
				// Again so that it works in both direction
				if (algoInformation[(int)params->engine1.algo].im == 0) {
					if (currentRow == ROW_MODULATION1 ) {
						currentRow += step;
					}
				}
			}
			if (currentRow>ROW_ENGINE_LAST) {
				currentRow = ROW_ENGINE_FIRST;
			} else if (currentRow<ROW_ENGINE_FIRST) {
				currentRow = ROW_ENGINE_LAST;
			}
			engineRow = currentRow;
		break;
		case BUTTON_OSC:
			lastBecauseOfAlgo = ROW_OSC_FIRST + algoInformation[(int)params->engine1.algo].osc - 1;
			if (currentRow<ROW_OSC_FIRST || currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_OSC_FIRST + operatorRow;
			} else {
				currentRow += step;
			}
			if (currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_OSC_FIRST;
			} else if (currentRow<ROW_OSC_FIRST) {
				currentRow = lastBecauseOfAlgo;
			}
			operatorRow = currentRow - ROW_OSC_FIRST;
			break;
		case BUTTON_ENV:
			lastBecauseOfAlgo = ROW_ENV_FIRST + algoInformation[(int)params->engine1.algo].osc - 1;
			if (currentRow<ROW_ENV_FIRST || currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_ENV_FIRST + operatorRow;
			} else {
				currentRow += step;
			}
			if (currentRow>lastBecauseOfAlgo) {
				currentRow = ROW_ENV_FIRST;
			} else if (currentRow<ROW_ENV_FIRST) {
				currentRow = lastBecauseOfAlgo;
			}
			operatorRow = currentRow - ROW_ENV_FIRST;
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
			fullState.currentMenuItem = menuBack();
			propagateMenuBack();
			break;
#ifdef DEBUG
		case BUTTON_DUMP:
		{
			PresetUtil::dumpPatch();
			break;
		}
#endif
		case BUTTON_LFO:
		{
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
				fullState.name[fullState.menuSelect] = 0;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_MATRIX:
		{
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
				fullState.name[fullState.menuSelect] = 27;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_ENV:
		{
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
				fullState.name[fullState.menuSelect] = 55;
				propagateNewMenuSelect();
			}
			break;
		}
		case BUTTON_OSC:
		{
            if (fullState.currentMenuItem->menuState == MENU_SAVE_ENTER_NAME || fullState.currentMenuItem->menuState == MENU_RENAME_PATCH) {
				fullState.name[fullState.menuSelect] = 66;
				propagateNewMenuSelect();
			}
			break;
		}
		// MENU MODE
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

/*
void SynthState::buttonPressed(int button) {
    int oldCurrentRow = currentRow;

    if (fullState.synthMode == SYNTH_MODE_EDIT)  {
        switch (button) {
        case BUTTON_SYNTH:
        case BUTTON_OSC:
        case BUTTON_ENV:
        case BUTTON_MATRIX:
        case BUTTON_LFO:
        	changeSynthModeRow(button , -1);
            break;
        case BUTTON_BACK:
        	if (this->isPlayingNote) {
        		propagateNoteOn();
        	} else {
                propagateNoteOff();
        	}
        	break;
        }
    }
    if (oldCurrentRow != currentRow) {
        propagateNewCurrentRow(currentRow);
    }
}
*/


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
    case MENU_LOAD_USER_SELECT_BANK:
    case MENU_SAVE_SELECT_USER_BANK:
    case MENU_SAVE_BANK:
        fullState.bankNumber = fullState.menuSelect;
        break;
    case MENU_LOAD_DX7_SELECT_BANK:
    	fullState.dx7BankNumber = fullState.menuSelect;
    	break;
    case MENU_SAVE_BANK_CONFIRM:
    	lcd.setRealTimeAction(true);
        PresetUtil::copyBank(4, fullState.bankNumber);
    	lcd.setRealTimeAction(false);
    	break;
    case MENU_LOAD_USER_SELECT_PRESET:
        propagateBeforeNewParamsLoad();
        if (fullState.bankNumber == 4) {
            storage->loadCombo(fullState.menuSelect);
            // Update and clean all timbres
            this->currentTimbre = 0;
            propagateNewTimbre(currentTimbre);
            PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
            propagateAfterNewComboLoad();
        } else {
            storage->loadPatch(fullState.bankNumber, fullState.menuSelect, params);
            PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
            propagateAfterNewParamsLoad();
        }
        break;
    case MENU_LOAD_DX7_SELECT_PRESET:
    	// propagateBeforeNewParamsLoad();
		// hexter->loadHexterPatch(storage->dx7LoadPatch(fullState.dx7BankName, fullState.menuSelect), params);
		PresetUtil::copySynthParams((char*)params, (char*)&backupParams);
		// propagateAfterNewParamsLoad();
        break;
    case MENU_SAVE_SELECT_PRESET:
        if (fullState.bankNumber == 4) {
            const char* comboName = storage->readComboName(fullState.menuSelect);
            for (int k=0; k<12 && comboName[k] != 0; k++) {
                for (int j=0; j<getLength(allChars); j++) {
                    if (comboName[k] == allChars[j]) {
                        fullState.name[k] = j;
                    }
                }
            }
        } else {
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
        }
        fullState.presetNumber = fullState.menuSelect;
        break;
    case MENU_RENAME_PATCH:
        int length;
        for (length=12; fullState.name[length-1] == 0; length--);
        for (int k=0; k<length; k++) {
            params->presetName[k] = allChars[(int)fullState.name[k]];
        }
        params->presetName[length] = '\0';
        break;
    case MENU_SAVE_ENTER_NAME:
    {
        int length;
        for (length=12; fullState.name[length-1] == 0; length--);
        if (fullState.bankNumber == 4) {
            char comboName[12];
            for (int k=0; k<length; k++) {
                comboName[k] = allChars[(int)fullState.name[k]];
            }
            comboName[length] = '\0';
            // TODO send REAL combo name
            storage->saveCombo(fullState.presetNumber, comboName);
        } else {
            for (int k=0; k<length; k++) {
                params->presetName[k] = allChars[(int)fullState.name[k]];
            }
            params->presetName[length] = '\0';
            storage->savePatch(fullState.bankNumber, fullState.presetNumber, params);
        }
        break;
    }
    case MENU_MIDI_PATCH_DUMP:
        PresetUtil::sendCurrentPatchToSysex();
        break;
    case MENU_MIDI_BANK_SELECT_DUMP:
    {
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
    case MENU_DONE:
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
    case MENU_SAVE_SELECT_PRESET:
        fullState.menuSelect = fullState.presetNumber;
        break;
    case MENU_LOAD_USER_SELECT_PRESET:
        if (fullState.bankNumber < 4) {
            propagateBeforeNewParamsLoad();
            storage->loadPatch(fullState.bankNumber, fullState.presetNumber, params);
            propagateAfterNewParamsLoad();
        }
        fullState.menuSelect = fullState.presetNumber;
        break;
    case MENU_LOAD_DX7_SELECT_PRESET:
		propagateBeforeNewParamsLoad();
		hexter->loadHexterPatch(storage->dx7LoadPatch(fullState.dx7BankName, fullState.dx7PresetNumber), params);
		propagateAfterNewParamsLoad();
        fullState.menuSelect = fullState.dx7PresetNumber;
        break;
    case MENU_SAVE_SELECT_USER_BANK:
        fullState.menuSelect = fullState.bankNumber;
        break;
    case MENU_LOAD:
        fullState.menuSelect = fullState.loadWhat;
        break;
    case MENU_SAVE:
        fullState.menuSelect = fullState.saveWhat;
        break;
    case MENU_LOAD_USER_SELECT_BANK:
        fullState.menuSelect = fullState.bankNumber;
        break;
    case MENU_LOAD_DX7_SELECT_BANK:
        fullState.menuSelect = fullState.dx7BankNumber;
    	fullState.dx7BankName = storage->getDx7BankName(fullState.menuSelect);
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
    case MENU_SAVE_ENTER_NAME:
        fullState.menuSelect = fullState.presetNumber;
        break;
    case MENU_SAVE_SELECT_PRESET:
        fullState.menuSelect = fullState.bankNumber;
        break;
    case MENU_LOAD_USER_SELECT_PRESET:
        fullState.menuSelect = fullState.bankNumber;
        propagateBeforeNewParamsLoad();
        PresetUtil::copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad();
        break;
    case MENU_LOAD_DX7_SELECT_PRESET:
        fullState.menuSelect = fullState.dx7BankNumber;
        propagateBeforeNewParamsLoad();
        PresetUtil::copySynthParams((char*)&backupParams, (char*)params);
        propagateAfterNewParamsLoad();
        break;
    case MENU_SAVE_BANK:
    	// After back, bank should no be receivable anymore
        fullState.synthMode = SYNTH_MODE_EDIT;
    	return MenuItemUtil::getMenuItem(MENU_LOAD);
    	break;
    case MAIN_MENU:
        fullState.synthMode = SYNTH_MODE_EDIT;
        // put back old patch (has been overwritten if a new patch has been loaded)
        break;
    case MENU_LOAD_USER_SELECT_BANK:
    case MENU_MIDI_PATCH:
    case MENU_SAVE_SELECT_USER_BANK:
    case MENU_MIDI_BANK:
        break;
    }

    rMenuItem = MenuItemUtil::getParentMenuItem(fullState.currentMenuItem->menuState);
    return rMenuItem;
}


void SynthState::newBankReady() {
	fullState.synthMode = SYNTH_MODE_MENU;
	fullState.menuSelect = 0;
	fullState.currentMenuItem = MenuItemUtil::getMenuItem(MENU_SAVE_BANK);
	propagateNewSynthMode();
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
	if (fullState.synthMode == SYNTH_MODE_MENU && fullState.currentMenuItem->menuState == MENU_DONE) {
		if (doneClick > 4) {
		    fullState.synthMode = SYNTH_MODE_EDIT;
		    propagateNewSynthMode();
		}
		doneClick ++;
	} else {
		doneClick = 0;
	}
}

void SynthState::setParamsAndTimbre(struct OneSynthParams *newParams, int newCurrentTimbre) {
    this->params = newParams;
    this->currentTimbre = newCurrentTimbre;
}

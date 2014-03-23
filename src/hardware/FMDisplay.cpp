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

#include "FMDisplay.h"

#define NULL 0
extern const char* lfoSeqMidiClock[];
extern const char* lfoOscMidiClock[];

const char* stepChars  = "_123456789ABCDEF";


// = / 10000
#define TMP_FLOAT PREENFM_FREQUENCY * .0001

unsigned int screenSaverGoal[] = { 2*60 * TMP_FLOAT, 5*60*TMP_FLOAT, 10*60*TMP_FLOAT, 60*60*TMP_FLOAT };


FMDisplay::FMDisplay() {
}

FMDisplay::~FMDisplay() {
}

void FMDisplay::init(LiquidCrystal* lcd, Storage* storage) {
	refreshStatus = 0;
	this->lcd = lcd;
    this->storage = storage;

	for (int k=0; k<4; k++) {
	    presetModifed[k] = false;
	    noteOnCounter[k] = 0;
	}
	algoCounter = 0;
	currentTimbre = 0;
	customCharsInit();
	screenSaveTimer = 0;
	screenSaverMode = false;

	lcd->clear();
}



void FMDisplay::customCharsInit() {

    unsigned char midiIn[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b01110,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };

    unsigned char backslash[8] = {
            0b00000,
            0b10000,
            0b01000,
            0b00100,
            0b00010,
            0b00001,
            0b00000,
            0b00000
    };

    unsigned char minusPoint[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00011,
            0b00000,
            0b00000,
            0b00000,
            0b00000
    };

  unsigned char stepCursor[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10001,
            0b01110,
            0b00100,
            0b00100,
    };

    unsigned char stepPos[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00100,
            0b00100,
            0b00000,
    };

    unsigned char firstSteps[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10000,
            0b10000,
    };


    unsigned char thirdStep[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10100,
            0b10100,
    };

    unsigned char note[8] = {
            0b00100,
            0b00110,
            0b00101,
            0b00101,
            0b00100,
            0b11100,
            0b11100,
            0b00000,
    };



    lcd->createChar(0, midiIn);
    lcd->createChar(1, backslash);
    lcd->createChar(2, minusPoint);
    lcd->createChar(3, stepCursor);
    lcd->createChar(4, stepPos);
    lcd->createChar(5, firstSteps);
    lcd->createChar(6, thirdStep);
    lcd->createChar(7, note);
}

void FMDisplay::printValueWithSpace(int value) {
    lcd->print(value);

	if (value>99) {
		lcd->print(' ');
	} else if (value>9) {
		lcd->print("  ");
	} else if (value>-1) {
		lcd->print("   ");
	} else if (value>-10) {
		lcd->print("  ");
	} else if (value>-100) {
		lcd->print(' ');
	}
}

void FMDisplay::printFloatWithSpace(float value) {
    if (value < 0.0) {
        lcd->print((char)2);
        value = - value;
    } else {
        lcd->print(' ');
    }
    if (value < 10.0f) {
        int integer = (int) value;
        lcd->print(integer);
        lcd->print('.');
        value -= integer;
        int valueTimes100 = (int)(value*100+.0005);
        if (valueTimes100 < 10) {
            lcd->print("0");
            lcd->print(valueTimes100);
        } else {
            lcd->print(valueTimes100);
        }
    } else {
        int integer = (int) value;
        lcd->print(integer);
        lcd->print('.');
        value -= integer;
        lcd->print((int)(value*10));
    }
}

bool FMDisplay::shouldThisValueShowUp(int row, int encoder) {
    int algo = this->synthState->params->engine1.algo;
    switch (row) {
		case ROW_MODULATION1:
			if (unlikely(encoder >= 2 && algoInformation[algo].im == 1)) {
				return false;
			}
		break;
    	case ROW_MODULATION2:
			if (unlikely(encoder >= 2 && algoInformation[algo].im == 3)) {
				return false;
			}
    	break;
    	case ROW_MODULATION3:
    	    if (unlikely(encoder >= 2 && algoInformation[algo].im == 5)) {
    	    	return false;
    	    }
       	break;
    	case ROW_OSC_MIX1:
    	    if (unlikely(encoder >= 2 && algoInformation[algo].mix == 1)) {
    	    	return false;
    	    }
       	break;
    	case ROW_OSC_MIX2:
    	    if (unlikely(encoder >= 2 && algoInformation[algo].mix == 3)) {
    	    	return false;
    	    }
       	break;
    	case ROW_OSC_MIX3:
    	    if (unlikely(encoder >= 2 && algoInformation[algo].mix == 5)) {
    	    	return false;
    	    }
       	break;
    	case ROW_ENGINE:
        	if (unlikely(this->synthState->params->engine1.numberOfVoice != 1 && encoder == ENCODER_ENGINE_GLIDE)) {
                return false;
        	}
       	break;
    	case ROW_EFFECT:
    	{
        	if (unlikely(encoder == 0)) {
        		return true;
        	}
        	int effect = this->synthState->params->effect.type;
        	if (filterRowDisplay[effect].paramName[encoder -1] == NULL) {
        		return false;
        	}
    	}
        break;
    	default:
    		break;
    }


    return true;
}

void FMDisplay::updateEncoderValue(int row, int encoder, ParameterDisplay* param, float newFloatValue) {

	int newValue = (int)newFloatValue;

    if (!shouldThisValueShowUp(row, encoder)) {
        lcd->setCursor(encoder*5, 3);

        lcd->print("    ");
        return;
    }

    switch (param->displayType) {
	case DISPLAY_TYPE_STRINGS :
	    lcd->setCursor(encoder*5, 3);
	    // TO REMOVE
	    if (unlikely(newValue > param->maxValue)) {
			lcd->print("#ER#");
			break;
	    }
		lcd->print(param->valueName[newValue]);
		break;
    case DISPLAY_TYPE_FLOAT_LFO_FREQUENCY:
    	if (newFloatValue*10.0f > 240.0) {
    	    int stringIndex = newFloatValue * 10;
    		lcd->setCursor(encoder*5, 3);
    		if (likely(stringIndex <= LFO_MIDICLOCK_MC_TIME_8)) {
    			lcd->print(lfoOscMidiClock[stringIndex-241]);
    		} else {
    			lcd->print("#ER#");
    		}
    		break;
    	}
        lcd->setCursor(encoder*5 - 1, 3);
        printFloatWithSpace(newFloatValue);
        break;
    case DISPLAY_TYPE_STEP_SEQ_BPM:
        lcd->setCursor(encoder*5, 3);
    	if (unlikely(newValue > LFO_SEQ_MIDICLOCK_TIME_4)) {
			lcd->print("#ER#");
			break;
    	}
        if (newValue > 240) {
    		lcd->print(lfoSeqMidiClock[newValue-241]);
    		break;
    	}
        printValueWithSpace(newValue);
        break;
    case DISPLAY_TYPE_LFO_KSYN:
        if (newFloatValue < 0.0f) {
            lcd->setCursor(encoder*5, 3);
    		lcd->print("Off ");
    		break;
    	}
        lcd->setCursor(encoder*5 - 1, 3);
        printFloatWithSpace(newFloatValue);
        break;
    case DISPLAY_TYPE_INT_OR_NONE:
    	if (newFloatValue != 4.0f) {
            lcd->setCursor(encoder*5 - 1, 3);
            printFloatWithSpace(newFloatValue);
    	} else {
            lcd->setCursor(encoder*5, 3);
    		lcd->print("None");
    	}
        break;
    case DISPLAY_TYPE_FLOAT:
    	if (unlikely(row == ROW_PERFORMANCE1)) {
            lcd->setCursor(encoder*5, 3);
    	} else {
    		lcd->setCursor(encoder*5 - 1, 3);
    	}
        printFloatWithSpace(newFloatValue);
        break;
    	// else what follows
	case DISPLAY_TYPE_INT:
        lcd->setCursor(encoder*5, 3);
		printValueWithSpace(newValue);
		break;
	case DISPLAY_TYPE_FLOAT_OSC_FREQUENCY:
	{
		// Hack... to deal with the special case of the fixed frequency.....
		int oRow = row - ROW_OSC_FIRST;
		OscillatorParams* oParam = (OscillatorParams*)&this->synthState->params->osc1;
		OscFrequencyType ft = (OscFrequencyType)oParam[oRow].frequencyType;

		if (ft == OSC_FT_FIXE) {
			lcd->setCursor(10, 3);
			newValue = oParam[oRow].frequencyMul * 1000.0 +  oParam[oRow].detune * 100;
			if (newValue < 0) {
				newValue = 0;
			}
			printValueWithSpace(newValue);
		} else {
	        lcd->setCursor(encoder*5 - 1, 3);
			printFloatWithSpace(newFloatValue);
		}
		break;
	}
	case DISPLAY_TYPE_NONE:
        lcd->setCursor(encoder*5, 3);
		lcd->print("    ");
		break;
    case DISPLAY_TYPE_VOICES:
        lcd->setCursor(encoder*5, 3);
        printValueWithSpace(newValue);
        if (newValue == 1) {
            // if voices = 1 or 0 let's refresh the glide info
            updateEncoderValue(ENCODER_ENGINE_GLIDE+1);
            updateEncoderName(row, ENCODER_ENGINE_GLIDE);
        } else {
        	// erase glide
            lcd->setCursor(ENCODER_ENGINE_GLIDE * 5, 2);
            lcd->print("    ");
            lcd->setCursor(ENCODER_ENGINE_GLIDE * 5, 3);
            lcd->print("  ");
        }
        break;
    case DISPLAY_TYPE_STEP_SEQ1:
    {
			int whichStepSeq = row - ROW_LFOSEQ1;
			StepSequencerSteps * seqSteps = &((StepSequencerSteps * )(&this->synthState->params->lfoSteps1))[whichStepSeq];

    		int decal = (this->synthState->stepSelect[whichStepSeq] >> 3 ) * 8;
    		lcd->setCursor(12, 3);
			for (int k=0; k<4; k++) {
				lcd->print(stepChars[seqSteps->steps[k + decal] ] );
			}

			lcd->setCursor(10, 2);
			lcd->print("  ");
			lcd->setCursor(10, 3);
			lcd->print(' ');
			if (decal == 0) {
				lcd->print((char)5);
			} else {
				lcd->print((char)6);
			}

			lcd->setCursor(12 + this->synthState->stepSelect[whichStepSeq] - decal, 2);
			lcd->print((char)3);
    	}
    	break;
    case DISPLAY_TYPE_STEP_SEQ2:
    {
    		int whichStepSeq = row - ROW_LFOSEQ1;
			StepSequencerSteps * seqSteps = &((StepSequencerSteps * )(&this->synthState->params->lfoSteps1))[whichStepSeq];
			int decal = (this->synthState->stepSelect[whichStepSeq] >> 3 ) * 8;

			lcd->setCursor(16, 3);
			for (int k=4; k<8; k++) {
				lcd->print(stepChars[seqSteps->steps[k + decal]] );
			}
			lcd->setCursor(12, 2);
			lcd->print((char)4);
			lcd->setCursor(16, 2);
			lcd->print((char)4);
    }
    break;
	}
}

void FMDisplay::updateEncoderName(int row, int encoder) {
    lcd->setCursor(encoder*5, 2);

    if (!shouldThisValueShowUp(row, encoder)) {
        lcd->print("    ");
        return;
    }

    if (unlikely(row == ROW_EFFECT && encoder > 0)) {
    	int effect = this->synthState->params->effect.type;
    	lcd->print(filterRowDisplay[effect].paramName[encoder -1]);
    	return;
    }

    const struct ParameterRowDisplay* paramRow = allParameterRows.row[row];
	lcd->print(paramRow->paramName[encoder]);
}

void FMDisplay::refreshAllScreenByStep() {
	switch (this->synthState->fullState.synthMode) {
	case SYNTH_MODE_EDIT:
		switch (refreshStatus) {
		case 12:
			lcd->setCursor(3,1);
			lcd->print("               ");
			break;
		case 11:
			// erase the caracters between
			for (int k=0; k<4; k++) {
				lcd->setCursor(4+k*5,2);
				lcd->print(' ');
			}
			break;
		case 10:
			// erase the caracters between
			for (int k=0; k<4; k++) {
				lcd->setCursor(4+k*5,3);
				lcd->print(' ');
			}
			break;
		case 9:
		{
			int row = this->synthState->getCurrentRow();
			if (this->synthState->fullState.midiConfigValue[MIDICONFIG_OP_OPTION] == 0) {
				// New UI
				if (row >= ROW_OSC_FIRST && row <= ROW_ENV_LAST) {
					lcd->setCursor(0,1);
					lcd->print("Op");
					lcd->print(this->synthState->getOperatorNumber() + 1);
					lcd->print(" ");
					lcd->print(allParameterRows.row[row]->rowName);
				} else {
					lcd->setCursor(0,1);
					lcd->print(allParameterRows.row[row]->rowName);
					if (row> ROW_ENGINE_LAST && row != ROW_PERFORMANCE1) {
						lcd->print(' ');
						lcd->print(getRowNumberToDiplay(row));
					}
				}
			} else {
				lcd->setCursor(0,1);
				lcd->print(allParameterRows.row[row]->rowName);
				if (row> ROW_ENGINE_LAST && row != ROW_PERFORMANCE1) {
					lcd->print(' ');
					lcd->print(getRowNumberToDiplay(row));
				}
			}
			break;
		}
		case 5:
		case 6:
		case 7:
		case 8:
			updateEncoderName(this->synthState->getCurrentRow(), refreshStatus -5);
			break;
		default :
			updateEncoderValue(refreshStatus);
			break;
		}
		break;
	}
    refreshStatus --;
}


void FMDisplay::updateEncoderValue(int refreshStatus) {
    int row = this->synthState->getCurrentRow();
    struct ParameterDisplay param = allParameterRows.row[row]->params[refreshStatus -1];
    float newValue;
    if (row < ROW_LFOSEQ1) {
       newValue = ((float*)this->synthState->params)[row*NUMBER_OF_ENCODERS+refreshStatus -1];
    } else if (row == ROW_LFOSEQ1) {
        newValue = ((float*)&this->synthState->params->lfoSeq1)[refreshStatus -1];
    } else if (row == ROW_LFOSEQ2) {
        newValue = ((float*)&this->synthState->params->lfoSeq2)[refreshStatus -1];
    }

    updateEncoderValue(this->synthState->getCurrentRow(), refreshStatus -1, &param, newValue);
}


void FMDisplay::displayPreset() {
    FullState* fullState = &this->synthState->fullState;

    if (algoCounter > 0) {
    	// algoCounter will (should) be updated after
    	// so we don't erase useless part...
		lcd->setCursor(11,0);
		lcd->print("         ");
		lcd->setCursor(11,1);
		lcd->print("         ");
    }
    lcd->setCursor(0, 0);
    if (this->synthState->getIsPlayingNote()) {
    	lcd->print((char)7);
    } else {
    	lcd->print('I');
    }
    lcd->print((char)('0'+currentTimbre +1));
    lcd->print(' ');
    lcd->print(this->synthState->params->presetName);
    if (presetModifed[currentTimbre]) {
        lcd->print('*');
    }
}

void FMDisplay::newTimbre(int timbre) {
    currentTimbre = timbre;
	if (unlikely(wakeUpFromScreenSaver())) {
		return;
	}
    if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT) {
        lcd->clearActions();
        lcd->clear();
        displayPreset();
        refreshStatus = 12;
    }
}



// Update FMDisplay regarding the callbacks from SynthState


void FMDisplay::newParamValueFromExternal(int timbre, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {
	if (unlikely(screenSaverMode)) {
		return;
	}
    if (timbre == currentTimbre) {
        checkPresetModified(timbre);
        if (this->synthState->getSynthMode() == SYNTH_MODE_EDIT && currentRow == this->displayedRow) {
            if (currentRow >= ROW_LFOSEQ1 && encoder>1) {
                updateStepSequencer(currentRow, encoder, oldValue, newValue);
                return;
            }
            updateEncoderValue(currentRow, encoder, param, newValue);
        }
    }
}


void FMDisplay::updateStepSequencer(int currentRow, int encoder, int oldValue, int newValue) {
	int whichStepSeq = currentRow - ROW_LFOSEQ1 ;
	int decal = (this->synthState->stepSelect[whichStepSeq] >> 3 ) * 8;

	if (encoder == 3) {
		// new value to display
		lcd->setCursor(12 + this->synthState->stepSelect[whichStepSeq] - decal, 3);
		lcd->print(stepChars[newValue]);
	} else if (encoder == 2) {
		int oldDecal = (oldValue >> 3 ) * 8;
		// Change cursor
		lcd->setCursor(12 + oldValue - oldDecal, 2);
		if ((oldValue & 3) == 0) {
	    	lcd->print((char)4);
		} else {
			lcd->print(' ');
		}
		// if new part 0-7 or 8-15 let's redraw all value
		if (oldDecal != decal) {
			refreshStatus = 4;
			return;
		}

		lcd->setCursor(12 + newValue - decal, 2);
		lcd->print((char)3);
	}
}

void FMDisplay::newParamValue(int timbre, int currentRow, int encoder, ParameterDisplay* param,  float oldValue, float newValue) {
	if (wakeUpFromScreenSaver()) {
		return;
	}
    checkPresetModified(timbre);
	if (this->synthState->getSynthMode() == SYNTH_MODE_EDIT) {
		if (unlikely(currentRow != this->displayedRow)) {
			newcurrentRow(timbre, currentRow);
			return;
		}
		// if we change ROW_LFO5 it's a bit special
		if (unlikely(currentRow >= ROW_LFOSEQ1 && encoder>1)) {
			updateStepSequencer(currentRow, encoder, oldValue, newValue);
			return;
		}

		// If we change frequency type of OScillator rows, it's a bit special too....
		if (unlikely(currentRow >= ROW_OSC1 && currentRow <= ROW_OSC_LAST && encoder == ENCODER_OSC_FTYPE)) {
			if (newValue == OSC_FT_FIXE) {
				lcd->setCursor(ENCODER_OSC_FREQ * 5 + 1, 3);
				lcd->print("        ");
			}
			refreshStatus = 4;
			return;
		}

		// display algo if algo changed else if algo shows erase it...
		if (unlikely(currentRow == ROW_ENGINE && encoder == ENCODER_ENGINE_ALGO)) {
			displayAlgo(newValue);
		} else if (unlikely(algoCounter > 0)) {
			// New value with Algo on screen so we must redraw the full screen
			algoCounter = 0;
			// Refresh
			newTimbre(this->currentTimbre);
		}

		if (unlikely(currentRow == ROW_EFFECT && encoder == ENCODER_EFFECT_TYPE)) {
			refreshStatus = 8;
			return;
		}

		updateEncoderValue(currentRow, encoder, param, newValue);
	}
}

void FMDisplay::newcurrentRow(int timbre, int newcurrentRow) {
	this->displayedRow = newcurrentRow;
	if (unlikely(wakeUpFromScreenSaver())) {
		return;
	}
	if (algoCounter > 0) {
        displayPreset();
		algoCounter = 0;
	}
	refreshStatus = 12;
}


/*
 * Menu Listener
 */

void FMDisplay::newSynthMode(FullState* fullState)  {
	if (unlikely(screenSaverMode)) {
		screenSaverMode = false;
		screenSaveTimer = 0;
	}
    lcd->clearActions();
	lcd->clear();
	switch (fullState->synthMode) {
		case SYNTH_MODE_EDIT:
			displayPreset();
			refreshStatus = 12;
			break;
		case SYNTH_MODE_MENU:
			refreshStatus = 0;
			menuRow = 0;
			newMenuState(fullState);
			break;
	}
	// just in case...
	algoCounter = 0;
}

int FMDisplay::rowInc(MenuState menuState) {
	switch (menuState) {
	case MENU_SAVE_ENTER_COMBO_NAME:
	case MAIN_MENU:
	case MENU_RENAME_BANK:
	case MENU_RENAME_COMBO:
	case MENU_SAVE_ENTER_PRESET_NAME:
		return 0;
	}
	return 1;
}

void FMDisplay::newMenuState(FullState* fullState) {
	menuRow += rowInc(fullState->currentMenuItem->menuState);
	if (fullState->currentMenuItem->hasSubMenu) {
	    int pos = 0;
		for (int k=0; k<fullState->currentMenuItem->maxValue; k++) {
		    fullState->menuPosition[k] = pos;
			lcd->setCursor(pos +1, menuRow);
			const char* name = MenuItemUtil::getMenuItem(fullState->currentMenuItem->subMenu[k])->name;
			pos = pos + (getLength(name) + 1);
            lcd->print(name);
		}
	}

	switch (fullState->currentMenuItem->menuState) {
		case MENU_SAVE_ENTER_COMBO_NAME:
			lcd->setCursor(6, menuRow);
			for (int k=0;k<12; k++) {
				lcd->print(allChars[(int)fullState->name[k]]);
			}
			break;
		case MENU_SAVE_ENTER_PRESET_NAME:
			lcd->setCursor(6, menuRow);
			for (int k=0;k<12; k++) {
				lcd->print(allChars[(int)fullState->name[k]]);
			}
			break;
		case MENU_CREATE_COMBO:
		case MENU_CREATE_BANK:
		case MENU_RENAME_COMBO:
		case MENU_RENAME_BANK:
			lcd->setCursor(6, menuRow);
			lcd->print("              ");
			lcd->setCursor(6, menuRow);
			for (int k=0;k<8; k++) {
				lcd->print(allChars[(int)fullState->name[k]]);
			}
			break;
        case MENU_RENAME_PATCH:
            lcd->setCursor(6, menuRow);
            for (int k=0;k<12; k++) {
                lcd->print(allChars[(int)fullState->name[k]]);
            }
            break;
		case MENU_DEFAULT_COMBO_SAVE:
			lcd->setCursor(1, menuRow);
			lcd->print("Save to default ?");
			break;
        case MENU_DEFAULT_COMBO_RESET:
            lcd->setCursor(1, menuRow);
            lcd->print("Reset default ?");
            break;
		case MENU_CONFIG_SETTINGS:
			lcd->setCursor(1, menuRow);
			lcd->print(midiConfig[fullState->menuSelect].title);
			break;
		case MENU_CONFIG_SETTINGS_SAVE:
			lcd->setCursor(1, menuRow);
			lcd->print("Save Config ?");
			break;
		case MENU_SAVE_SYSEX_PATCH:
			lcd->setCursor(1, menuRow);
			lcd->print("Send Patch ?");
			break;
	}

	newMenuSelect(fullState);
}


void FMDisplay::displayBankSelect(int bankNumber, bool usable, const char* name) {
	eraseRow(menuRow);
	lcd->setCursor(2, menuRow);
	lcd->print(bankNumber);
	if (usable) {
		lcd->print(" - ");
	} else {
		lcd->print(" * ");
	}
	lcd->print(name);
}

void FMDisplay::displayPatchSelect(int presetNumber, const char* name) {
	eraseRow(menuRow);
	lcd->setCursor(2, menuRow);
	lcd->print(presetNumber);
	lcd->print(" - ");
	lcd->print(name);
}


void FMDisplay::newMenuSelect(FullState* fullState) {
	lcd->noCursor();

	switch(fullState->currentMenuItem->menuState) {
	case MAIN_MENU:
	case MENU_LOAD:
	case MENU_SAVE:
    case MENU_TOOLS:
	case MENU_MIDI_BANK:
	case MENU_MIDI_PATCH:
    case MENU_DEFAULT_COMBO:
	case MENU_RENAME:
	case MENU_CREATE:
		for (int k=0; k<fullState->currentMenuItem->maxValue; k++) {
			lcd->setCursor(fullState->menuPosition[k], menuRow);
			lcd->print(' ');
		}
		lcd->setCursor(fullState->menuPosition[fullState->menuSelect], menuRow);
		lcd->print(">");
		break;
	case MENU_LOAD_SELECT_BANK_PRESET:
	case MENU_LOAD_SELECT_DX7_PRESET:
		displayPatchSelect(fullState->menuSelect, this->synthState->params->presetName);
		break;
	case MENU_SAVE_SELECT_BANK_PRESET:
		displayPatchSelect(fullState->menuSelect, storage->loadPreenFMPatchName(fullState->preenFMBank, fullState->menuSelect));
		break;
	case MENU_LOAD_SELECT_COMBO_PRESET:
	case MENU_SAVE_SELECT_COMBO_PRESET:
		displayPatchSelect(fullState->menuSelect, storage->loadPreenFMComboName(fullState->preenFMCombo, fullState->menuSelect));
		break;
	case MENU_LOAD_SELECT_DX7_BANK:
		displayBankSelect(fullState->menuSelect, (fullState->dx7Bank->fileType != FILE_EMPTY), fullState->dx7Bank->name);
		break;
	case MENU_SAVE_SELECT_BANK:
		displayBankSelect(fullState->menuSelect, (fullState->preenFMBank->fileType == FILE_OK), fullState->preenFMBank->name);
		break;
	case MENU_RENAME_SELECT_BANK:
		displayBankSelect(fullState->menuSelect, (storage->getPreenFMBank(fullState->menuSelect)->fileType != FILE_EMPTY), storage->getPreenFMBank(fullState->menuSelect)->name);
		break;
	case MENU_LOAD_SELECT_BANK:
		displayBankSelect(fullState->menuSelect, (fullState->preenFMBank->fileType != FILE_EMPTY), fullState->preenFMBank->name);
		break;
	case MENU_RENAME_SELECT_COMBO:
		displayBankSelect(fullState->menuSelect, (storage->getPreenFMCombo(fullState->menuSelect)->fileType != FILE_EMPTY), storage->getPreenFMCombo(fullState->menuSelect)->name);
		break;
	case MENU_SAVE_SELECT_COMBO:
		displayBankSelect(fullState->menuSelect, (fullState->preenFMCombo->fileType == FILE_OK), fullState->preenFMCombo->name);
		break;
	case MENU_LOAD_SELECT_COMBO:
		displayBankSelect(fullState->menuSelect, (fullState->preenFMCombo->fileType != FILE_EMPTY), fullState->preenFMCombo->name);
		break;
	case MENU_DONE:
		lcd->clear();
		lcd->setCursor(8,1);
		lcd->print(fullState->currentMenuItem->name);
		break;
	case MENU_ERROR:
		lcd->clear();
		lcd->setCursor(7,1);
		lcd->print(fullState->currentMenuItem->name);
		break;
	case MENU_CANCEL:
		lcd->clear();
		lcd->setCursor(6,1);
		lcd->print(fullState->currentMenuItem->name);
		break;
	case MENU_IN_PROGRESS:
		lcd->clear();
		lcd->setCursor(3,1);
		lcd->print("In Progress...");
		break;
    case MENU_RENAME_PATCH:
        lcd->setCursor(6+fullState->menuSelect, menuRow);
        lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
        lcd->setCursor(6+fullState->menuSelect, menuRow);
        lcd->cursor();
        break;
	case MENU_RENAME_COMBO:
	case MENU_RENAME_BANK:
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->cursor();
		break;
	case MENU_SAVE_ENTER_PRESET_NAME:
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->cursor();
		break;
	case MENU_SAVE_ENTER_COMBO_NAME:
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->cursor();
		break;
	case MENU_CREATE_BANK:
	case MENU_CREATE_COMBO:
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->cursor();
		break;
	case MENU_CONFIG_SETTINGS:
		eraseRow(menuRow);
		lcd->setCursor(1, menuRow);
		lcd->print(midiConfig[fullState->menuSelect].title);
		if (midiConfig[fullState->menuSelect].valueName != NULL) {
			lcd->print(midiConfig[fullState->menuSelect].valueName[fullState->midiConfigValue[fullState->menuSelect]]);
		} else {
			lcd->print((int)fullState->midiConfigValue[fullState->menuSelect]);
			lcd->print(' ');
		}
		break;
	default:
		break;
	}
}

void FMDisplay::eraseRow(int row) {
	lcd->setCursor(0, row);
	lcd->print("                    ");
}

void FMDisplay::menuBack(enum MenuState oldMenuState, FullState* fullState) {
	if (fullState->currentMenuItem->menuState == MENU_DONE
			|| fullState->currentMenuItem->menuState == MENU_ERROR
			|| fullState->currentMenuItem->menuState == MENU_CANCEL) {
		return;
	}
	eraseRow(menuRow);
	menuRow -= rowInc(oldMenuState);
	// new menu will add it again...
	menuRow -= rowInc(fullState->currentMenuItem->menuState);
}


void FMDisplay::midiClock(bool show) {
	if (this->synthState->fullState.synthMode  == SYNTH_MODE_EDIT && algoCounter == 0 && !screenSaverMode) {
		lcd->setCursor(19,1);
		if (show) {
			lcd->print((char)7);
		} else {
			lcd->print(' ');
		}
	}
	if (show) {
		if (synthState->fullState.midiConfigValue[MIDICONFIG_LED_CLOCK] == 1) {
			GPIO_SetBits(GPIOB, GPIO_Pin_6);
		}
	} else {
		GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	}
}

void FMDisplay::noteOn(int timbre, bool show) {
	if (this->synthState->fullState.synthMode  == SYNTH_MODE_EDIT && algoCounter == 0 && !screenSaverMode) {
		if (show) {
		    if (noteOnCounter[timbre] == 0) {
		    	lcd->setCursor(16+timbre, 0);
		    	lcd->print((char)('0'+ timbre+1));
		    }
		    noteOnCounter[timbre] = 2;
		} else {
	        lcd->setCursor(16+timbre, 0);
			lcd->print(' ');
		}
	}
}

void FMDisplay::tempoClick() {
    for (int timbre=0; timbre<4; timbre++) {
        if (noteOnCounter[timbre] >0) {
            noteOnCounter[timbre]--;
            if (noteOnCounter[timbre] == 0) {
                noteOn(timbre, false);
            }
        }
    }
    if (algoCounter > 0) {
    	algoCounter --;
    	if (algoCounter == 0) {
    		// call new timbre to refresh the whole page
    		newTimbre(this->currentTimbre);
    	}
    }
	if (unlikely(synthState->fullState.midiConfigValue[MIDICONFIG_OLED_SAVER] > 0)) {
		if (!screenSaverMode) {
			if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT) {
				screenSaveTimer ++;
				if (screenSaveTimer > screenSaverGoal[synthState->fullState.midiConfigValue[MIDICONFIG_OLED_SAVER] - 1]) {
					lcd->clearActions();
					lcd->clear();
					screenSaverMode = true;
					screenSaveTimer = 0;
				}
			} else {
				screenSaveTimer = 0;
			}
		}
	}
}

void FMDisplay::afterNewParamsLoad(int timbre) {
    presetModifed[timbre] = false;
	if (currentTimbre == timbre && this->synthState->fullState.synthMode == SYNTH_MODE_EDIT && !screenSaverMode) {
		lcd->clearActions();
		lcd->clear();
		displayPreset();
		refreshStatus = 12;
	}
}

bool FMDisplay::wakeUpFromScreenSaver() {
	if (unlikely(screenSaverMode)) {
		screenSaverMode = false;
		screenSaveTimer = 0;
		displayPreset();
		refreshStatus = 12;
		return true;
	}
	screenSaveTimer = 0;
	return false;
}

void FMDisplay::displayAlgo(int algo) {
	algoCounter = 50;

	const char *da[4] = {NULL, NULL, NULL, NULL};
	int x = 13;

	switch (algo) {
	case ALGO1:
		da[1] = "  2<3  ";
		da[2] = "  \1 /  ";
		da[3] = "   1   ";
		break;
	case ALGO2:
		da[1] = "   3   ";
		da[2] = "  / \1  ";
		da[3] = "  1 2  ";
		break;
	case ALGO3:
		da[1] = " 2 3>4 ";
		da[2] = "  \1|/  ";
		da[3] = "   1   ";
		break;
	case ALGO4:
		da[1] = "  3<4  ";
		da[2] = "  |\1|";
		da[3] = "  1 2  ";
		break;
	case ALGO5:
		da[0] = "  3<4  ";
		da[1] = "  \1 /  ";
		da[2] = "   2  ";
		da[3] = "   1   ";
		break;
	case ALGO6:
		da[1] = "   4   ";
		da[2] = "  /|\1  ";
		da[3] = " 1 2 3 ";
		break;
	case ALGO7:
		da[0] = "   4   ";
		da[1] = " 2 |\1  ";
		da[2] = " | | 6 ";
		da[3] = " 1 3 5 ";
		break;
	case ALGO8:
		da[1] = " 234  6";
		da[2] = " \1|/  |";
		da[3] = "  1   5";
		break;
	case ALGO9:
		da[0] = "      6";
		da[1] = " 2 3  5";
		da[2] = " \1 /  |";
		da[3] = "  1   4";
		break;
	case ALG10:
		da[0] = "     6 ";
		da[1] = "     5 ";
		da[2] = "  2  4 ";
		da[3] = "  1  3 ";
		break;
	case ALG11:
		da[1] = "  3  6 ";
		da[2] = "  2  5 ";
		da[3] = "  1  4 ";
		break;
	case ALG12:
		da[1] = " 2 4 6 ";
		da[2] = " | | | ";
		da[3] = " 1 3 5 ";
		break;
	case ALG13:
		da[0] = "     6 ";
		da[1] = " 2 4 5 ";
		da[2] = " | |/  ";
		da[3] = " 1 3   ";
		break;
	case ALG14:
		da[0] = " 3     ";
		da[1] = " 2 5 6 ";
		da[2] = " | |/  ";
		da[3] = " 1 4   ";
		break;
	case ALG15:
		da[1] = "2 4 5 6";
		da[2] = "|  \1|/ ";
		da[3] = "1   3  ";
		break;
	case ALG16:
		da[0] = "   5 6 ";
		da[1] = "   |/  ";
		da[2] = " 2 4   ";
		da[3] = " 1 3   ";
		break;
	case ALG17:
		da[0] = "   4 6 ";
		da[1] = " 2 3 5 ";
		da[2] = "  \1|/  ";
		da[3] = "   1   ";
		break;
	case ALG18:
		da[0] = "    5-6";
		da[1] = "2 3 4  ";
		da[2] = " \1|/   ";
		da[3] = "  1    ";
		break;
	case ALG19:
		da[0] = "3      ";
		da[1] = "2   6  ";
		da[2] = "|  / \1 ";
		da[3] = "1  4 5 ";
		break;
	case ALG20:
		da[1] = " 3  5 6";
		da[2] = "/ \1 \1 /";
		da[3] = "1 2  4 ";
		break;
	case ALG21:
		da[1] = " 3   6 ";
		da[2] = "/ \1 / \1";
		da[3] = "1 2 4 5";
		break;
	case ALG22:
		da[1] = "2   6  ";
		da[2] = "|  /|\1 ";
		da[3] = "1 3 4 5";
		break;
 	case ALG23:
		da[1] = "     6 ";
		da[2] = "    /|\1";
		da[3] = "1 2 345";
		break;
	case ALG24:
		da[0] = "   5   ";
		da[1] = " 2 4   ";
		da[2] = " | |   ";
		da[3] = " 1 3 6 ";
		break;
	case ALG25:
		da[1] = "    4 6";
		da[2] = "    | |";
		da[3] = "1 2 3 5";
		break;
	case ALG26:
		da[0] = "    5  ";
		da[1] = "    4  ";
		da[2] = "    |  ";
		da[3] = "1 2 3 6";
		break;
	case ALG27:
		da[3] = "123456 ";
		break;
	case ALG28:
		da[1] = "     6 ";
		da[2] = "     | ";
		da[3] = " 12345 ";
		break;
	}

	for (int y=0; y<4; y++) {
		lcd->setCursor(11,y);
		lcd->print("| ");
		if (da[y] == NULL) {
			lcd->print("       ");
		} else {
			lcd->print(da[y]);
		}
	}
}




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
	lcd->clear();
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
    if (row == ROW_MODULATION1 && (encoder+1)> algoInformation[algo].im) {
        return false;
    } else if (row == ROW_MODULATION2 && (encoder+5)> algoInformation[algo].im) {
            return false;
    } else if (row == ROW_OSC_MIX1) {
        if ((encoder>>1) >= algoInformation[algo].mix) {
            return false;
        }
    } else if (row == ROW_OSC_MIX2) {
        if ((encoder>>1) >= (algoInformation[algo].mix-2)) {
            return false;
        }
    } else if (row == ROW_ENGINE && encoder == ENCODER_ENGINE_GLIDE && this->synthState->params->engine1.numberOfVoice >1) {
        return false;
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
		lcd->print(param->valueName[newValue]);
		break;
    case DISPLAY_TYPE_FLOAT_LFO_FREQUENCY:
    	if (newFloatValue*10.0f > 240.0) {
    	    lcd->setCursor(encoder*5, 3);
    		lcd->print(lfoOscMidiClock[(int)(newFloatValue*10.0f-241)]);
    		break;
    	}
        lcd->setCursor(encoder*5 - 1, 3);
        printFloatWithSpace(newFloatValue);
        break;
    case DISPLAY_TYPE_STEP_SEQ_BPM:
        lcd->setCursor(encoder*5, 3);
    	if (newValue > 240) {
    		lcd->print(lfoSeqMidiClock[newValue-241]);
    		break;
    	}
        printValueWithSpace(newValue);
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
        lcd->setCursor(encoder*5 - 1, 3);
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
	const struct ParameterRowDisplay* paramRow = allParameterRows.row[row];
	lcd->print(paramRow->paramName[encoder]);
}

void FMDisplay::refreshAllScreenByStep() {
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
		lcd->setCursor(0,1);
		lcd->print(allParameterRows.row[row]->rowName);
		if (row> ROW_ENGINE_LAST) {
			lcd->print(' ');
			lcd->print(getRowNumberToDiplay(row));
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
    lcd->print('I');
    lcd->print((char)('0'+currentTimbre +1));
    lcd->print(' ');
    lcd->print(this->synthState->params->presetName);
    if (presetModifed[currentTimbre]) {
        lcd->print('*');
    }
}


void FMDisplay::newTimbre(int timbre) {
    currentTimbre = timbre;
    if (this->synthState->fullState.synthMode == SYNTH_MODE_EDIT) {
        lcd->clearActions();
        lcd->clear();
        displayPreset();
        refreshStatus = 12;
    }
}



// Update FMDisplay regarding the callbacks from SynthState


void FMDisplay::newParamValueFromExternal(int timbre, SynthParamType type, int currentRow, int encoder, ParameterDisplay* param, float oldValue, float newValue) {
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

void FMDisplay::newParamValue(int timbre, SynthParamType type, int currentRow, int encoder, ParameterDisplay* param,  float oldValue, float newValue) {
    checkPresetModified(timbre);
	if (this->synthState->getSynthMode() == SYNTH_MODE_EDIT) {
		if (currentRow != this->displayedRow) {
			newcurrentRow(timbre, currentRow);
			return;
		}
		// if we change ROW_LFO5 it's a bit special
		if (currentRow >= ROW_LFOSEQ1 && encoder>1) {
			updateStepSequencer(currentRow, encoder, oldValue, newValue);
			return;
		}

		// If we change frequency type of OScillator rows, it's a bit special too....
		if (SynthState::getListenerType(currentRow)==SYNTH_PARAM_TYPE_OSC) {
			if (encoder == ENCODER_OSC_FTYPE) {
				if (newValue == OSC_FT_FIXE) {
					lcd->setCursor(ENCODER_OSC_FREQ * 5 + 1, 3);
					lcd->print("        ");
				}
				refreshStatus = 4;
				return;
			}
		}

		// display algo if algo channged else if algo shows erase it...
		if (currentRow == ROW_ENGINE && encoder == ENCODER_ENGINE_ALGO) {
			displayAlgo(newValue);
		} else if (algoCounter > 0) {
			algoCounter = 0;
			newTimbre(this->currentTimbre);
		}

		updateEncoderValue(currentRow, encoder, param, newValue);
	}
}

void FMDisplay::newcurrentRow(int timbre, int newcurrentRow) {
	if (algoCounter > 0) {
        displayPreset();
		algoCounter = 0;
	}
	refreshStatus = 12;
	this->displayedRow = newcurrentRow;
}


/*
 * Menu Listener
 */

void FMDisplay::newSynthMode(FullState* fullState)  {
    lcd->clearActions();
	lcd->clear();
	if (fullState->synthMode == SYNTH_MODE_EDIT) {
        displayPreset();
		refreshStatus = 12;
	} else {
		refreshStatus = 0;
		menuRow = 0;
		newMenuState(fullState);
	}
	// just in case...
	algoCounter = 0;
}

int FMDisplay::rowInc(MenuState menuState) {
	switch (menuState) {
	case MENU_SAVE_ENTER_COMBO_NAME:
	case MAIN_MENU:
	case MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME:
	case MENU_RENAME_BANK:
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
		case MENU_RENAME_BANK:
			lcd->setCursor(6, menuRow);
			lcd->print("              ");
			lcd->setCursor(6, menuRow);
			for (int k=0;k<8; k++) {
				lcd->print(allChars[(int)fullState->name[k]]);
			}
			break;
		case MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME:
			// Let's erase waiting sysex and bank loading progress
			eraseRow(menuRow-1);
			lcd->setCursor(3, menuRow-1);
			lcd->print(fullState->currentMenuItem->name);
			eraseRow(menuRow);
			lcd->setCursor(3, menuRow);
			for (int k=0;k<8; k++) {
				lcd->print(allChars[(int)fullState->name[k]]);
			}
			break;
        case MENU_RENAME_PATCH:
            // -1 because must erase preset name....
            lcd->setCursor(6, menuRow);
            for (int k=0;k<12; k++) {
                lcd->print(allChars[(int)fullState->name[k]]);
            }
            break;
		case MENU_SAVE_SYSEX_BANK_CONFIRM_OVERRIDE:
			eraseRow(2);
			lcd->setCursor(0, 3);
			lcd->print(fullState->currentMenuItem->name);
			break;
		case MENU_FORMAT_BANK:
			lcd->setCursor(1, menuRow);
			lcd->print("Y to format:");
			fullState->menuSelect = 14;
			lcd->setCursor(14, menuRow);
			lcd->print(allChars[fullState->menuSelect]);
			break;
		case MENU_DEFAULT_COMBO_SAVE:
			lcd->setCursor(1, menuRow);
			lcd->print("Save to default ?");
			break;
        case MENU_DEFAULT_COMBO_RESET:
            lcd->setCursor(1, menuRow);
            lcd->print("Reset default ?");
            break;
		case MENU_MIDI_SYSEX_GET:
			lcd->setCursor(1, menuRow);
			lcd->print("Waiting SysEx...");
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
	case MENU_SAVE_SYSEX:
    case MENU_DEFAULT_COMBO:
	case MENU_RENAME:
		for (int k=0; k<fullState->currentMenuItem->maxValue; k++) {
			lcd->setCursor(fullState->menuPosition[k], menuRow);
			lcd->print(' ');
		}
		lcd->setCursor(fullState->menuPosition[fullState->menuSelect], menuRow);
		lcd->print(">");
		break;
	case MENU_LOAD_SELECT_BANK_PRESET:
	case MENU_LOAD_SELECT_DX7_PRESET:
		displayPatchSelect(fullState->menuSelect + 1, this->synthState->params->presetName);
		// TO REMOVE
		lcd->setCursor(17,0);
		lcd->print((int)this->synthState->params->engine1.algo + 1);
		lcd->print(' ');
		break;
	case MENU_LOAD_SELECT_COMBO_PRESET:
		displayPatchSelect(fullState->menuSelect + 1, storage->readComboName(fullState->menuSelect));
		break;
	case MENU_SAVE_SELECT_BANK_PRESET:
		displayPatchSelect(fullState->menuSelect + 1, storage->loadPreenFMPatchName(fullState->preenFMBank, fullState->menuSelect));
		break;
	case MENU_SAVE_SELECT_COMBO_PRESET:
		displayPatchSelect(fullState->menuSelect + 1, storage->loadPreenFMComboName(fullState->preenFMCombo, fullState->menuSelect));
		break;
	case MENU_LOAD_SELECT_DX7_BANK:
		displayBankSelect(fullState->menuSelect + 1, (fullState->dx7Bank->fileType != FILE_EMPTY), fullState->dx7Bank->name);
		break;
	case MENU_SAVE_SELECT_BANK:
		displayBankSelect(fullState->menuSelect + 1, (fullState->preenFMBank->fileType == FILE_OK), fullState->preenFMBank->name);
		break;
	case MENU_RENAME_SELECT_BANK:
	case MENU_SAVE_SYSEX_BANK:
		displayBankSelect(fullState->menuSelect + 1, (storage->getPreenFMBank(fullState->menuSelect)->fileType != FILE_EMPTY), storage->getPreenFMBank(fullState->menuSelect)->name);
		break;
	case MENU_LOAD_SELECT_BANK:
		displayBankSelect(fullState->menuSelect + 1, (fullState->preenFMBank->fileType != FILE_EMPTY), fullState->preenFMBank->name);
		break;
	case MENU_SAVE_SELECT_COMBO:
		displayBankSelect(fullState->menuSelect + 1, (fullState->preenFMCombo->fileType == FILE_OK), fullState->preenFMCombo->name);
		break;
	case MENU_LOAD_SELECT_COMBO:
		displayBankSelect(fullState->menuSelect + 1, (fullState->preenFMCombo->fileType != FILE_EMPTY), fullState->preenFMCombo->name);
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
	case MENU_RENAME_BANK:
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(6+fullState->menuSelect, menuRow);
		lcd->cursor();
		break;
	case MENU_SAVE_ENTER_NEW_SYSEX_BANK_NAME:
		lcd->setCursor(3+fullState->menuSelect, menuRow);
		lcd->print(allChars[(int)fullState->name[fullState->menuSelect]]);
		lcd->setCursor(3+fullState->menuSelect, menuRow);
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
	case MENU_FORMAT_BANK:
		lcd->setCursor(14, menuRow);
		lcd->print(allChars[fullState->menuSelect]);
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
	if (this->synthState->fullState.synthMode  == SYNTH_MODE_EDIT) {
		lcd->setCursor(19,1);
		if (show) {
			lcd->print((char)7);
		} else {
			lcd->print(' ');
		}
	}
}

void FMDisplay::noteOn(int timbre, bool show) {
	if (this->synthState->fullState.synthMode  == SYNTH_MODE_EDIT) {
        lcd->setCursor(16+timbre, 0);
		if (show) {
		    noteOnCounter[timbre] = 2;
	        lcd->print((char)('0'+ timbre+1));
		} else {
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
}

void FMDisplay::displayAlgo(int algo) {
	algoCounter = 50;

	const char *da[4] = {NULL, NULL, NULL, NULL};
	int x = 13;

	switch (algo) {
	case ALGO1:
		da[1] = "  2-3  ";
		da[2] = "  \1 /  ";
		da[3] = "   1   ";
		break;
	case ALGO2:
		da[1] = "   3   ";
		da[2] = "  / \1  ";
		da[3] = "  1 2  ";
		break;
	case ALGO3:
		da[1] = " 2 3-4 ";
		da[2] = "  \1|/  ";
		da[3] = "   1   ";
		break;
	case ALGO4:
		da[1] = "  3-4  ";
		da[2] = "  |\1|";
		da[3] = "  1 2  ";
		break;
	case ALGO5:
		da[0] = "  3-4  ";
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
		da[1] = " 2 4-6 ";
		da[2] = " | | | ";
		da[3] = " 1 3 5 ";
		break;
	case ALGO8:
		da[1] = " 234  6";
		da[2] = " \1|/  |";
		da[3] = "  1   5";
		break;
	case ALGO9:
		da[0] = "      6";
		da[1] = " 2-3  5";
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
		da[2] = " | | | ";
		da[3] = " 1 3 6 ";
		break;
	case ALG25:
		da[1] = "    4 6";
		da[2] = "    | |";
		da[3] = "1 2 3 5";
		break;
	case ALG26:
		da[1] = "    5  ";
		da[1] = "    4  ";
		da[2] = "    |  ";
		da[3] = "1 2 3 6";
		break;
	case ALG27:
		da[3] = "123456 ";
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




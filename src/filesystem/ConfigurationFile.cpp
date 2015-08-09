/*
 * ConfigurationFile.cpp
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#include "ConfigurationFile.h"
#include "Menu.h"

#define SCALA_ENABLED   "scala_enabled"
#define SCALA_FILENAME  "scala_filename"
#define SCALA_FREQUENCY "scala_frequency"
#define SCALA_KEYBOARD "scala_keyboard"

extern char lineBuffer[128];

ConfigurationFile::ConfigurationFile() {
	numberOfFilesMax = 0;
}

ConfigurationFile::~ConfigurationFile() {
}

const char* ConfigurationFile::getFolderName() {
	return PREENFM_DIR;
}


void ConfigurationFile::loadConfig(char* midiConfigBytes) {
	char *line = lineBuffer;
    char* reachableProperties = propertyFile;
    int size = checkSize(PROPERTIES);
    if (size >= PROPERTY_FILE_SIZE || size == -1) {
    	// ERROR
    	return;
    }
    reachableProperties[size] = 0;

    int result = load(PROPERTIES, 0,  reachableProperties, size);
    int loop = 0;
    char *readProperties = reachableProperties;
    while (loop !=-1 && (readProperties - reachableProperties) < size) {
    	loop = fsu->getLine(readProperties, line);
    	if (line[0] != '#') {
    		fillMidiConfig(midiConfigBytes, line);
    	}
    	readProperties += loop;
    }
}

void ConfigurationFile::loadScalaConfig(struct ScalaScaleConfig *scalaScaleConfig) {
	char *line = lineBuffer;
    char* reachableProperties = propertyFile;
    int size = checkSize(SCALA_CONFIG);
    if (size >= PROPERTY_FILE_SIZE || size == -1) {
    	// file does not exist
    	return;
    }
    reachableProperties[size] = 0;

    int result = load(SCALA_CONFIG, 0,  reachableProperties, size);
    int loop = 0;
    char *readProperties = reachableProperties;
    while (loop !=-1 && (readProperties - reachableProperties) < size) {
    	loop = fsu->getLine(readProperties, line);
    	if (line[0] != '#') {
    		fillScalaConfig(scalaScaleConfig, line);
    	}
    	readProperties += loop;
    }
}

void ConfigurationFile::saveConfig(const char* midiConfigBytes) {
    int wptr = 0;
    for (int k=0; k<MIDICONFIG_SIZE; k++) {
    	propertyFile[wptr++] = '#';
    	propertyFile[wptr++] = ' ';
    	wptr += fsu->copy_string((char*)propertyFile + wptr, midiConfig[k].title);
    	propertyFile[wptr++] = '\n';
    	if (midiConfig[k].maxValue < 10 && midiConfig[k].valueName != 0) {
	    	wptr += fsu->copy_string((char*)propertyFile + wptr, "#   0=");
			for (int o=0; o<midiConfig[k].maxValue; o++) {
		    	wptr += fsu->copy_string((char*)propertyFile + wptr, midiConfig[k].valueName[o]);
				if ( o != midiConfig[k].maxValue - 1) {
			    	wptr += fsu->copy_string((char*)propertyFile + wptr, ", ");
				} else {
			    	propertyFile[wptr++] = '\n';
				}
			}
    	}
    	wptr += fsu->copy_string((char*)propertyFile + wptr, midiConfig[k].nameInFile);
    	propertyFile[wptr++] = '=';
    	wptr += fsu->printInt(propertyFile, midiConfigBytes[k]);
    	propertyFile[wptr++] = '\n';
    	propertyFile[wptr++] = '\n';
    }
    // delete it so that we're sure the new one has the right size...
    remove(PROPERTIES);
    save(PROPERTIES, 0,  propertyFile, wptr);
}

void ConfigurationFile::saveScalaConfig(struct ScalaScaleConfig *scalaScaleConfig) {
    int wptr = 0;

    // Save scala information
	wptr += fsu->copy_string((char*)propertyFile + wptr, "\n# Scala Scales\n");

	wptr += fsu->copy_string((char*)propertyFile + wptr, SCALA_ENABLED);
	propertyFile[wptr++] = '=';
	propertyFile[wptr++] = scalaScaleConfig->scalaEnabled ? '1' : '0';
	propertyFile[wptr++] = '\n';

	if (scalaScaleConfig->scalaFile->fileType != FILE_EMPTY) {
		wptr += fsu->copy_string((char*)propertyFile + wptr, SCALA_FILENAME);
		propertyFile[wptr++] = '=';
		wptr += fsu->copy_string((char*)propertyFile + wptr, scalaScaleConfig->scalaFile->name);
		propertyFile[wptr++] = '\n';
	}

	wptr += fsu->copy_string((char*)propertyFile + wptr, SCALA_FREQUENCY);
	propertyFile[wptr++] = '=';
	wptr += fsu->printFloat((char*)propertyFile + wptr, scalaScaleConfig->scalaFreq);
	propertyFile[wptr++] = '\n';

	wptr += fsu->copy_string((char*)propertyFile + wptr, SCALA_KEYBOARD);
	propertyFile[wptr++] = '=';
	propertyFile[wptr++] = scalaScaleConfig->keyboardMapping ? '1' : '0';
	propertyFile[wptr++] = '\n';


	// delete it so that we're sure the new one has the right size...
    remove(SCALA_CONFIG);
    save(SCALA_CONFIG, 0,  propertyFile, wptr);
}


void ConfigurationFile::fillMidiConfig(char* midiConfigBytes, char* line) {
	char key[21];
	char value[21];

	int equalPos = fsu->getPositionOfEqual(line);
	if (equalPos == -1) {
		return;
	}
	fsu->getKey(line, key);

	for (int k=0; k < MIDICONFIG_SIZE; k++) {
		if (fsu->str_cmp(key, midiConfig[k].nameInFile) == 0) {
			fsu->getValue(line + equalPos+1, value);
			midiConfigBytes[k] = fsu->toInt(value);
			return;
		}
	}
}


void ConfigurationFile::fillScalaConfig(struct ScalaScaleConfig* scalaScaleConfig, char* line) {
	char key[21];
	char value[21];

	int equalPos = fsu->getPositionOfEqual(line);
	if (equalPos == -1) {
		return;
	}
	fsu->getKey(line, key);

	if (fsu->str_cmp(key, SCALA_ENABLED) == 0) {
		fsu->getValue(line + equalPos+1, value);
		scalaScaleConfig->scalaEnabled = (fsu->toInt(value) == 1);
		return;
	}
	if (fsu->str_cmp(key, SCALA_FILENAME) == 0) {
		fsu->getTextValue(line + equalPos+1, value);
		int i = scalaFile->getFileIndex(value);
		if (i >= 0) {
			scalaScaleConfig->scalaFile = scalaFile->getFile(i);
		}
		return;
	}
	if (fsu->str_cmp(key, SCALA_FREQUENCY) == 0) {
		fsu->getFloatValue(line + equalPos+1, value);
		scalaScaleConfig->scalaFreq = fsu->toFloat(value);
		return;
	}
	if (fsu->str_cmp(key, SCALA_KEYBOARD) == 0) {
		fsu->getFloatValue(line + equalPos+1, value);
		scalaScaleConfig->keyboardMapping = (fsu->toInt(value) == 1);
		return;
	}
}

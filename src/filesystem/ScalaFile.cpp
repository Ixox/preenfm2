/*
 * ScalaFile.cpp
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#include <Math.h>
#include "ScalaFile.h"

#include "LiquidCrystal.h"
extern LiquidCrystal      lcd;


float scalaFrequency[128];

extern char lineBuffer[128];
extern float *frequencyToUse;
extern float frequency[];


ScalaFile::ScalaFile() {
	numberOfFilesMax = NUMBEROFSCALASCALEFILES;
	myFiles = scalaScaleFile;
}

ScalaFile::~ScalaFile() {
	// TODO Auto-generated destructor stub
}

const char* ScalaFile::getFolderName() {
	return SCALA_DIR;
}



void ScalaFile::loadScalaScale(const struct ScalaScaleConfig* config) {
	if (!config->scalaEnabled) {
		return;
	}

    const char* fullScalaFileName = getFullName(config->scalaFile->name);
    char *line = lineBuffer;
    // propertyFiles is a 2048 char array
    char* scalaBuffer = propertyFile;

    int size = checkSize(fullScalaFileName);
    if (size >= PROPERTY_FILE_SIZE || size == -1) {
    	// ERROR but should not happen
    	return;
    }
    scalaBuffer[size] = 0;

    int result = load(fullScalaFileName, 0,  (void*)scalaBuffer, size);
    int loop = 0;
    char *readProperties = scalaBuffer;
    int state = 0;

    numberOfDegrees = 0;
    for (int i=0; i< 24; i++) {
    	interval[i] = 0.0f;
    }
    while (loop !=-1 && (readProperties - scalaBuffer) < size) {
    	loop = fsu->getLine(readProperties, line);
    	if (line[0] != '!') {
            switch (state) {
                case 0:
                    // line contains short description
                    break;
                case 1:
                    // line contains number of degrees
                	numberOfDegrees = fsu->toInt(line);
                	if (numberOfDegrees > 36) {
                		numberOfDegrees = 36;
                	}
                    break;
                default:
                    // contains the frequencey (state-2)
                	if ((state -2) < 36) {
                		interval[state -2] = getScalaIntervale(line);
                	}
                    break;
            }
            state ++;
    	}
    	readProperties += loop;
    }
    applyScalaScale(config);
}

void ScalaFile::applyScalaScale(const struct ScalaScaleConfig* config) {
	if (!config->scalaEnabled) {
		return;
	}


	float octaveRatio = interval[numberOfDegrees-1];
	int octaveDegree = numberOfDegrees;
	if (config->keyboardMapping) {
		octaveDegree = ((numberOfDegrees + 11) / 12) * 12;
	}

	// Fill middle C
	scalaFrequency[60] = config->scalaFreq;
	// Fill all C
	int firstC = 0;
	int lastNote = 127;
	for (int n = 60 - octaveDegree; n >=0; n = n - octaveDegree) {
		scalaFrequency[n] = scalaFrequency[n + octaveDegree]  / octaveRatio;
		firstC = n;
	}
	for (int n = 60 + octaveDegree; n <=127; n = n + octaveDegree) {
		scalaFrequency[n] = scalaFrequency[n - octaveDegree]  * octaveRatio;
	}

	// init unusable last 8 notes
	for (int n = 0; n < firstC; n++) {
		scalaFrequency[n] = config->scalaFreq;
	}

	// Fill all notes
	for (int octave = firstC ; octave <= 127; octave += octaveDegree) {
		for (int n = 1; n < numberOfDegrees; n++) {
			if (octave + n <= 127) {
				scalaFrequency[octave + n] = scalaFrequency[octave] * interval[n-1];
				lastNote = octave + n;
			}
		}
		// Same scalaFrequency for all remaining notes
		for (int nn = numberOfDegrees ; nn < octaveDegree; nn++) {
			if (octave + nn <= 127) {
				scalaFrequency[octave + nn] = scalaFrequency[octave] * octaveRatio;
				lastNote = octave + nn;
			}
		}
	}
	for (int n = lastNote; n < 128; n++) {
		scalaFrequency[n] = config->scalaFreq;
	}

    frequencyToUse = scalaFrequency;
}


void ScalaFile::clearScalaScale() {
	frequencyToUse = frequency;
}

float ScalaFile::getScalaIntervale(const char* line) {
    int slashPos = fsu->getPositionOfSlash(line);
    if (slashPos != -1) {
        float num = fsu->toFloat(line);
        float den = fsu->toFloat(line + slashPos + 1);
        return num/den;
    } else {
        return pow(2.0f, fsu->toFloat(line) / 1200.0f);
    }
}


bool ScalaFile::isCorrectFile(char *name, int size)  {
	// scalaScale Dump sysex size is 4104
	if (size >= 2048) {
		return false;
	}

	int pointPos = -1;
    for (int k=1; k<9 && pointPos == -1; k++) {
        if (name[k] == '.') {
            pointPos = k;
        }
    }
    if (pointPos == -1) return false;
    if (name[pointPos+1] != 's' && name[pointPos+1] != 'S') return false;
    if (name[pointPos+2] != 'c' && name[pointPos+2] != 'C') return false;
    if (name[pointPos+3] != 'l' && name[pointPos+3] != 'L') return false;

    return true;
}


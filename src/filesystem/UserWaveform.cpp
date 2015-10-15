/*
 * UserWaveforms.cpp
 *
 *  Created on: Oct 3, 2015
 *      Author: xhosxe
 */



/*

 #include "LiquidCrystal.h"
extern LiquidCrystal      lcd;
*/
// User waveforms
extern float userWaveform[6][1024];

extern char lineBuffer[512];
#define LINE_BUFFER_SIZE 512

extern char *oscShapeNames[];
extern struct WaveTable waveTables[];

#include "UserWaveform.h"

UserWaveform::UserWaveform() {
    for (int k=0; k<6; k++) {
        userWaveFormNames[k][4] = 0;
    }
}

UserWaveform::~UserWaveform() {
}

const char* UserWaveform::getFolderName() {
    return USERWAVEFORM_DIR;
}


void UserWaveform::loadUserWaveforms() {
    char fileName[30];


    for (int f=0; f<6; f++) {
        // Check if bin exists

        fsu->copy_string(fileName, USERWAVEFORM_FILENAME_BIN);
        fileName[20] = (char)('1' + f);
        int sizeBin = checkSize(fileName);


        if (sizeBin != -1) {
            loadUserWaveformFromBin(f, fileName);
        } else {
            fsu->copy_string(fileName, USERWAVEFORM_FILENAME_TXT);
            fileName[20] = (char)('1' + f);

            int sizeTxt = checkSize(fileName);
            if (sizeTxt == -1) {
                // Does not exist. Neither Bin nor txt
                for (int s=0; s<1024; s++) {
                    userWaveform[f][s] = 0.0f;
                }
            } else {
                numberOfSample = -1;
                userWaveFormNames[f][0] = 0;
                loadUserWaveformFromTxt(f, fileName, sizeTxt);

                if (numberOfSample > 0) {
                    if (numberOfSample > 512 && numberOfSample < 1024) {
                        interpolate(userWaveform[f], numberOfSample, 1024);
                    } else if (numberOfSample > 256 && numberOfSample < 512) {
                        interpolate(userWaveform[f], numberOfSample, 512);
                    } else if (numberOfSample > 128 && numberOfSample < 256) {
                        interpolate(userWaveform[f], numberOfSample, 256);
                    } else if (numberOfSample > 64 && numberOfSample < 128) {
                        interpolate(userWaveform[f], numberOfSample, 128);
                    } else if (numberOfSample > 32 && numberOfSample < 64) {
                        interpolate(userWaveform[f], numberOfSample, 64);
                    }

                    normalize(userWaveform[f], numberOfSample);

                    fsu->copy_string(fileName, USERWAVEFORM_FILENAME_BIN);
                    fileName[20] = (char)('1' + f);
                    saveUserWaveformToBin(f, fileName);
                    // Reload from Bin
                    loadUserWaveformFromBin(f, fileName);
                }
            }
        }
    }
}

void UserWaveform::loadUserWaveformFromTxt(int f, const char* fileName, int size) {
    int readIndex = 0;
    floatRead = 0;


    while (floatRead != numberOfSample) {
        int toRead = (size - readIndex > LINE_BUFFER_SIZE) ? LINE_BUFFER_SIZE : size - readIndex;
        load(fileName, readIndex,  (void*)&lineBuffer, toRead);

        int used = fillUserWaveFormFromTxt(f, lineBuffer, toRead, (readIndex+toRead) >= size);
        if (used < 0) {
            return;
        }
        readIndex += used;
    }
}


int UserWaveform::fillUserWaveFormFromTxt(int f, char* buffer, int filled, bool last) {
    int index = 0;
    bool bStop = false;
    int cpt=0;
    while (!bStop) {
        int floatSize = 0;

        // Init name
        if (userWaveFormNames[f][0] == 0) {

            while (fsu->isSeparator(buffer[index])) {
                index++;
            }
            int b = 0;
            while (b<4 && !fsu->isSeparator(buffer[index])) {
                userWaveFormNames[f][b++] = buffer[index++];
            }
            // complete with space
            while (b<4) {
                userWaveFormNames[f][b++] = ' ';
            }
        }
        // init number of sample
        if (numberOfSample <= 0) {
            while (fsu->isSeparator(buffer[index])) {
                index++;
            }
            numberOfSample = (int)(fsu->stof(&buffer[index], floatSize) + .5f);

            if (numberOfSample < 32 || numberOfSample > 1024) {
                return numberOfSampleError(f);
            }
            waveTables[f + 8].max = (numberOfSample  -1);
            index += floatSize;
        }

        userWaveform[f][floatRead++] = fsu->stof(&buffer[index], floatSize);
        index += floatSize;

        // Stop if index > (filled - 30) Or if last && floatRead == 1024
        bStop = (!last && index > (filled - 40)) || floatRead == numberOfSample;
    }
    return index;
}

int UserWaveform::numberOfSampleError(int f) {
    numberOfSample = 0;
    userWaveFormNames[f][0] = '#';
    oscShapeNames[8 + f] = userWaveFormNames[f];
    return -1;
}


void UserWaveform::loadUserWaveformFromBin(int f, const char* fileName) {
    load(fileName, 0, userWaveFormNames[f], 4);
    oscShapeNames[8 + f] = userWaveFormNames[f];

    load(fileName, 4, &numberOfSample, 4);
    waveTables[f + 8].max = (numberOfSample  -1);
    waveTables[f + 8].precomputedValue = (waveTables[f + 8].max + 1) * waveTables[f + 8].useFreq * PREENFM_FREQUENCY_INVERSED;

    load(fileName, 8, userWaveform[f], numberOfSample * 4);
}

void UserWaveform::saveUserWaveformToBin(int f, const char* fileName) {
    save(fileName, 0, userWaveFormNames[f], 4);
    save(fileName, 4, &numberOfSample, 4);
    save(fileName, 8, userWaveform[f], numberOfSample * 4);
}

void UserWaveform::normalize(float* buffer, int numberOfSamples) {
    float min = 0;
    float max = 0;
    float average = 0;
    for (int i=0; i < numberOfSamples; i++) {
        average += buffer[i];
        if (buffer[i] < min) {
            min = buffer[i];
        }
        if (buffer[i] > max) {
            max = buffer[i];
        }
    }
    average /= numberOfSamples;
    min -= average;
    max -= average;
    float m1 = min != 0 ? - 1 / min : 1;
    float m2 = max != 0 ? 1 / max : 1;
    float m = m1 > m2 ? m2 : m1;

    for (int i=0; i < numberOfSamples; i++) {
        buffer[i] -= average;
        buffer[i] *= m;
    }
}

void UserWaveform::interpolate(float* buffer, int sourceNumberOfSamples, int targetNumberOfSamples) {
    for (int i = targetNumberOfSamples-1; i>=0; i--) {
        float pos = (float)i * (float)sourceNumberOfSamples /  (float)targetNumberOfSamples;
        int iPos = pos;
        float decimal = pos - iPos;
        buffer[i] = buffer[iPos] * (1-decimal) + buffer[iPos+1] * decimal;
    }
    numberOfSample = targetNumberOfSamples;
}


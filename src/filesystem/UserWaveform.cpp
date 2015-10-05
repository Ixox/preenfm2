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
/*
    lcd.clear();
    lcd.setRealTimeAction(true);
    */
    for (int f=0; f<6; f++) {
        numberOfSample = 0;
        userWaveFormNames[f][0] = 0;

        fsu->copy_string(fileName, USERWAVEFORM_FILENAME);
        fileName[20] = (char)('1' + f);

        int size = checkSize(fileName);
/*
        if (f<4) {
            lcd.setCursor(0, 0);
            lcd.print(f);
            lcd.print(' ');
            lcd.print(size);
            lcd.print(' ');
            lcd.print((char*)&fileName[16]);
        }
*/
        if (size == -1) {
            // Does not exist
            for (int s=0; s<1024; s++) {
                userWaveform[f][s] = 0.0f;
            }
        } else {
            loadUserWaveform(f, fileName, size);
        }
    }
}

void UserWaveform::loadUserWaveform(int f, const char* fileName, int size) {
    int readIndex = 0;
    int floatRead = 0;


    while (size - readIndex > 0) {
        int toRead = (size - readIndex > LINE_BUFFER_SIZE) ? LINE_BUFFER_SIZE : size - readIndex;
        load(fileName, readIndex,  (void*)&lineBuffer, toRead);

        int used = fillUserWaveForm(f, lineBuffer, toRead, floatRead, (readIndex+toRead) >= size);
        readIndex += used;
    }
}


int UserWaveform::fillUserWaveForm(int f, char* buffer, int filled, int &floatRead, bool last) {
    int index = 0;
    bool bStop = false;
    int cpt=0;
    while (!bStop) {
        int floatSize = 0;

        if (userWaveFormNames[f][0] == 0) {

            while (fsu->isSeparator(buffer[index])) {
                index++;
            }
            int b = 0;
            while (b<4 && !fsu->isSeparator(buffer[index])) {
                userWaveFormNames[f][b++] = buffer[index++];
            }
            oscShapeNames[8 + f] = userWaveFormNames[f];
        }

        if (numberOfSample == 0) {
            while (fsu->isSeparator(buffer[index])) {
                index++;
            }
            numberOfSample = (int)(fsu->stof(&buffer[index], floatSize) + .5f);
            waveTables[f + 8].max = (numberOfSample  -1);
            index += floatSize;
        }

        userWaveform[f][floatRead++] = fsu->stof(&buffer[index], floatSize);
        index += floatSize;

        // Stop if index > (filled - 30) Or if last && floatRead != 1024
        bStop = (last || index > (filled - 40)) && floatRead != 1024;
    }
    return index;
}


/*
 * UserWaveforms.h
 *
 *  Created on: Oct 3, 2015
 *      Author: xhosxe
 */

#ifndef USERWAVEFORMS_H_
#define USERWAVEFORMS_H_



#include "PreenFMFileType.h"

class UserWaveform: public PreenFMFileType {
public:
    UserWaveform();
    virtual ~UserWaveform();
    void loadUserWaveforms();

protected:
    const char* getFolderName();
    bool isCorrectFile(char *name, int size) { return true; }

private:
    void loadUserWaveformFromTxt(int f, const char* fileName, int size);
    int fillUserWaveFormFromTxt(int f, char* buffer, int filled, bool last);
    void loadUserWaveformFromBin(int f, const char* fileName);
    void saveUserWaveformToBin(int f, const char* fileName);
    int numberOfSample;
    char userWaveFormNames[6][5];
    int floatRead;
};

#endif /* USERWAVEFORMS_H_ */

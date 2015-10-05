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
    void loadUserWaveform(int f, const char* fileName, int size);
    int fillUserWaveForm(int f, char* buffer, int filled, int &floatRead, bool last);
    int numberOfSample;
    char userWaveFormNames[6][5];
};

#endif /* USERWAVEFORMS_H_ */

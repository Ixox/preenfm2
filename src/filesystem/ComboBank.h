/*
 * ComboBank.h
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#ifndef COMBOBANK_H_
#define COMBOBANK_H_

#include "PreenFMFileType.h"

class ComboBank : public PreenFMFileType {
public:
	ComboBank();
	virtual ~ComboBank();
    void init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4);


    void saveDefaultCombo();
    bool loadDefaultCombo();
    void removeDefaultCombo();
    void createComboBank(const char* name);

    void loadPreenFMCombo(const struct PFM2File* combo, int comboNumber);
    const char* loadPreenFMComboName(const struct PFM2File* combo, int comboNumber);
    void savePreenFMCombo(const struct PFM2File* combo, int comboNumber, char* comboName);
    const struct PFM2File* addEmptyCombo(const char* newComboName);


protected:
    const char* getFolderName();
	bool isCorrectFile(char *name, int size);
    struct OneSynthParams* timbre[4];
private:
    char presetName[13];
    struct PFM2File preenFMCombo[NUMBEROFPREENFMCOMBOS];

};

#endif /* COMBOBANK_H_ */

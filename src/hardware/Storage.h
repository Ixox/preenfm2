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



#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "Common.h"

enum FILE_ENUM {
    DEFAULT_COMBO = 0,
    PROPERTIES,
    FIRMWARE
};



struct BankFile {
	char name[13];
	FileType fileType;
};


#define PFM_PATCH_SIZE sizeof(struct OneSynthParams)
#define ALIGNED_PATCH_SIZE 1024
#define ALIGNED_PATCH_ZERO ALIGNED_PATCH_SIZE-PFM_PATCH_SIZE
#define ALIGNED_COMBO_SIZE ALIGNED_PATCH_SIZE*4+13

#define DX7_PACKED_PATCH_SIZED 128
#define DX7_UNPACKED_PATCH_SIZED 155

struct FlashSynthParams {
    struct Engine1Params engine1;
    struct FlashEngineIm1 flashEngineIm1;
    struct FlashEngineIm2 flashEngineIm2;
    struct EngineMix1 engineMix1;
    struct EngineMix2 engineMix2;
    struct EngineMix3 engineMix3;
    struct OscillatorParams osc1;
    struct OscillatorParams osc2;
    struct OscillatorParams osc3;
    struct OscillatorParams osc4;
    struct OscillatorParams osc5;
    struct OscillatorParams osc6;
    struct EnvelopeParamsA env1a;
    struct EnvelopeParamsB env1b;
    struct EnvelopeParamsA env2a;
    struct EnvelopeParamsB env2b;
    struct EnvelopeParamsA env3a;
    struct EnvelopeParamsB env3b;
    struct EnvelopeParamsA env4a;
    struct EnvelopeParamsB env4b;
    struct EnvelopeParamsA env5a;
    struct EnvelopeParamsB env5b;
    struct EnvelopeParamsA env6a;
    struct EnvelopeParamsB env6b;
    struct MatrixRowParams matrixRowState1;
    struct MatrixRowParams matrixRowState2;
    struct MatrixRowParams matrixRowState3;
    struct MatrixRowParams matrixRowState4;
    struct MatrixRowParams matrixRowState5;
    struct MatrixRowParams matrixRowState6;
    struct MatrixRowParams matrixRowState7;
    struct MatrixRowParams matrixRowState8;
    struct MatrixRowParams matrixRowState9;
    struct MatrixRowParams matrixRowState10;
    struct MatrixRowParams matrixRowState11;
    struct MatrixRowParams matrixRowState12;
    struct LfoParams lfoOsc1;
    struct LfoParams lfoOsc2;
    struct LfoParams lfoOsc3;
    struct EnvelopeParams lfoEnv1;
    struct Envelope2Params lfoEnv2;
    struct StepSequencerParams lfoSeq1;
    struct StepSequencerParams lfoSeq2;
    struct StepSequencerSteps lfoSteps1;
    struct StepSequencerSteps lfoSteps2;
    char presetName[13];
    struct EngineArp1 engineApr1;
    struct EngineArp2 engineApr2;
    struct FlashEngineVeloIm1 flashEngineVeloIm1;
    struct FlashEngineVeloIm2 flashEngineVeloIm2;
};


class Storage {
public:
    virtual ~Storage() {}
    virtual void init(struct OneSynthParams*timbre1, struct OneSynthParams*timbre2, struct OneSynthParams*timbre3, struct OneSynthParams*timbre4);
    void setArpeggiatorPartOfThePreset(char *pointer) { arpeggiatorPartOfThePreset = pointer; }


    void saveDefaultCombo();
    bool loadDefaultCombo();
    void removeDefaultCombo();
    void createPatchBank(const char* name);
    void createComboBank(const char* name);
    void loadConfig(char* midiConfig);
    void saveConfig(const char* midiConfig);

    void saveBank(const char* newBankName, const uint8_t* sysexTmpMem);
    bool bankNameExist(const char* bankName);
    int bankBaseLength(const char* bankName);
    bool comboNameExist(const char* comboName);

    virtual const struct BankFile* getDx7Bank(int bankNumber) = 0;
    uint8_t* dx7LoadPatch(const struct BankFile* bank, int patchNumber);

    virtual const struct BankFile* getPreenFMBank(int bankNumber) = 0;
    void loadPreenFMPatch(const struct BankFile* bank, int patchNumber, struct OneSynthParams *params);
    const char* loadPreenFMPatchName(const struct BankFile* bank, int patchNumber);
    void savePreenFMPatch(const struct BankFile* bank, int patchNumber, const struct OneSynthParams *params);

    virtual const struct BankFile* getPreenFMCombo(int comboNumber) = 0;
    void loadPreenFMCombo(const struct BankFile* combo, int comboNumber);
    const char* loadPreenFMComboName(const struct BankFile* combo, int comboNumber);
    void savePreenFMCombo(const struct BankFile* combo, int comboNumber, char* comboName);

    virtual int renameBank(const struct BankFile* bank, const char* newName) = 0;
    virtual int renameCombo(const struct BankFile* bank, const char* newName) = 0;

#ifdef DEBUG
    void testMemoryPreset();
#endif

private:
    // Pure Virtual
    virtual const struct BankFile*  addEmptyBank(const char* newBankName) = 0;
    virtual const struct BankFile*  addEmptyCombo(const char* newComboName) = 0;
    virtual const char* getDX7BankFullName(const char* bankName) = 0;
    virtual const char* getPreenFMFullName(const char* bankName) = 0;
    void dx7patchUnpack(uint8_t *packed_patch, uint8_t *unpacked_patch);
    virtual int save(FILE_ENUM file,int seek, void* bytes, int size) = 0;
    virtual int save(const char* fileName, int seek, void* bytes, int size) = 0;
    virtual int load(FILE_ENUM file, int seek, void* bytes, int size) = 0;
    virtual int load(const char* fileName, int seek, void* bytes, int size) = 0;
    virtual int remove(FILE_ENUM file) = 0;
    // checkSize must return -1 if file does not exist
    virtual int checkSize(FILE_ENUM file) = 0;
    void addNumber(char* name, int offset, int number);
    void copy(char* dest, const char* source, int length);


    void  __attribute__ ((noinline)) copyFloat(float* source, float* dest, int n);
    void  __attribute__ ((noinline)) convertParamsToMemory(const struct OneSynthParams* params, struct FlashSynthParams* memory);
    void  __attribute__ ((noinline)) convertMemoryToParams(const struct FlashSynthParams* memory, struct OneSynthParams* params);

    char *arpeggiatorPartOfThePreset;
    char presetName[13];
    struct OneSynthParams* timbre[4];
};

#endif /*__STORAGE_H__*/



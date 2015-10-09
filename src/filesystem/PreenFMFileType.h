/*
 * PreenFMFileType.h
 *
 *  Created on: 23 juil. 2015
 *      Author: xavier
 */

#ifndef PREENFMFILETYPE_H_
#define PREENFMFILETYPE_H_

#include "Common.h"
#include "FileSystemUtils.h"
#include "usbKey_usr.h"

extern USB_OTG_CORE_HANDLE          usbOTGHost;
extern USBH_HOST                    usbHost;


#define ARRAY_SIZE(x)  ( sizeof(x) / sizeof((x)[0]) )

enum FILE_ENUM {
    DEFAULT_COMBO = 0,
    PROPERTIES,
    SCALA_CONFIG,
    FIRMWARE
};

#define DEFAULT_COMBO_NAME       "0:/pfm2/DfltCmbo.pfm"
#define PROPERTIES_NAME          "0:/pfm2/Settings.txt"
#define SCALA_CONFIG_NAME        "0:/pfm2/ScalaCfg.txt"

#define USERWAVEFORM_FILENAME_TXT            "0:/pfm2/waveform/usr#.txt"
#define USERWAVEFORM_FILENAME_BIN            "0:/pfm2/waveform/usr#.bin"

#define DX7_PACKED_PATCH_SIZED 128
#define DX7_UNPACKED_PATCH_SIZED 155

#define FIRMWARE_DIR             "0:/pfm2"
#define DX7_DIR                  "0:/pfm2/dx7"
#define PREENFM_DIR              "0:/pfm2"
#define SCALA_DIR                "0:/pfm2/scala"
#define USERWAVEFORM_DIR         "0:/pfm2/waveform"


#define SYSEX_NEW_PFM2_BYTE_PATCH 5

#ifndef BOOTLOADER
#define NUMBEROFDX7BANKS 256
#define NUMBEROFPREENFMBANKS 64
#define NUMBEROFPREENFMCOMBOS 8
#define NUMBEROFSCALASCALEFILES 128
#endif

#ifdef BOOTLOADER
#define NUMBEROFDX7BANKS 1
#define NUMBEROFPREENFMBANKS 1
#define NUMBEROFPREENFMCOMBOS 1
#define NUMBEROFSCALASCALEFILES 1
#endif


struct PFM2File {
	char name[13];
	FileType fileType;
};

// Storage maping of the parameters
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
    struct EngineArp1 engineArp1;
    struct EngineArp2 engineArp2;
    struct FlashEngineVeloIm1 flashEngineVeloIm1;
    struct FlashEngineVeloIm2 flashEngineVeloIm2;
    struct EffectRowParams effect;
    struct EngineArpUserPatterns engineArpUserPatterns;
    struct LfoPhaseRowParams lfoPhases;
    struct MidiNoteCurveRowParams midiNote1Curve;
    struct MidiNoteCurveRowParams midiNote2Curve;
};

#define USB_PATCH_SIZE 1024
#define USB_PATCH_ZERO USB_PATCH_SIZE-PFM_PATCH_SIZE


#define PFM_PATCH_SIZE sizeof(struct FlashSynthParams)
#define ALIGNED_PATCH_SIZE 1024
#define ALIGNED_PATCH_ZERO ALIGNED_PATCH_SIZE-PFM_PATCH_SIZE
#define ALIGNED_COMBO_SIZE ALIGNED_PATCH_SIZE*4+13


#define PROPERTY_FILE_SIZE 2048
extern char propertyFile [PROPERTY_FILE_SIZE];

extern struct FlashSynthParams reachableFlashParam;
class Storage;

class PreenFMFileType {
	friend Storage;
public:
	PreenFMFileType();
	virtual ~PreenFMFileType();

	bool exists(const char* name);
	void setFileSystemUtils(FileSystemUtils* fsu) { this->fsu = fsu; }

	const struct PFM2File* getFile(int fileNumber);
	int getFileIndex(const char* name);
	int getFileIndex(const struct PFM2File* file);
	int renameFile(const struct PFM2File* bank, const char* newName);
	bool nameExists(const char* bankName);

protected:
	virtual const char* getFolderName() = 0;
	virtual bool isCorrectFile(char *name, int size) = 0;

	int numberOfFiles;
	int numberOfFilesMax;

   	int remove(FILE_ENUM file);
	const char* getFileName(FILE_ENUM file);
	const char* getFullName(const char* fileName);
	int load(FILE_ENUM file, int seek, void* bytes, int size);
	int load(const char* fileName, int seek, void* bytes, int size);
	int save(FILE_ENUM file, int seek, void* bytes, int size);
	int save(const char* fileName, int seek, void* bytes, int size);
	int checkSize(FILE_ENUM file);
	int checkSize(const char* fileName);
	void usbProcess();

	int initFiles();
	int readNextFile(struct PFM2File* bank);

	void swapFiles(struct PFM2File* bankFiles, int i, int j);
	void sortFiles(struct PFM2File* bankFiles, int numberOfFiles);

	void convertParamsToMemory(const struct OneSynthParams* params, struct FlashSynthParams* memory, bool saveArp);
	void convertMemoryToParams(const struct FlashSynthParams* memory, struct OneSynthParams* params, bool loadArp);


	int bankBaseLength(const char* bankName);

	struct PFM2File * myFiles;
    struct PFM2File errorFile;
	char fullName[40];

	FileSystemUtils* fsu;
	bool isInitialized;

};

#endif /* PREENFMFILETYPE_H_ */

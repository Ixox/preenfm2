/*
 * hexter DSSI software synthesizer
 *
 * Copyright (C) 2004, 2009, 2012 Sean Bolton and others.
 * Adapted for the PreenFM : Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */


#ifndef HEXTER_H_
#define HEXTER_H_

#include <stdint.h>
#include "Common.h"

#define DX7_VOICE_SIZE_PACKED         128
#define DX7_VOICE_SIZE_UNPACKED       155
#define FRIENDLY_PATCH_COUNT  71

struct _dx7_patch_t
{
    uint8_t data[128];  /* dx7_patch_t is packed patch data */
};
typedef struct _dx7_patch_t dx7_patch_t;

extern const dx7_patch_t friendly_patches[FRIENDLY_PATCH_COUNT];



class Hexter {
public:
	Hexter();
    void setArpeggiatorPartOfThePreset(short *pointer) { arpeggiatorPartOfThePreset = pointer; }
	virtual ~Hexter() {}
	void loadHexterPatch(uint8_t* packedPatch, struct OneSynthParams *params);

protected:
	void patchUnpack(uint8_t *packed_patch, uint8_t *unpacked_patch);
	int limit(int x, int min, int max);
	void setIM(struct OneSynthParams *params, int im, uint8_t *patch, int op);
	void setIMWithMax(struct OneSynthParams *params, int im, uint8_t *patch, int op, float max);
	void setMix(struct OneSynthParams *params, int im, uint8_t *patch, int op);
	float getPreenFMIM(int lvl);
	void voiceSetData(struct OneSynthParams *params, uint8_t *voice);
	int  bulkDumpChecksum(uint8_t *data, int length);
	void voiceCopyName(char *name, uint8_t *patch);
	int abs(int value);
	float abs(float value);
	float getRounded(float r);
	float getChangeTime(int outputLevel, int time, int value1, int value2);
	int getActualLevel(int value);
	int getActualOutputLevel(int value);
	float getAttackRatio(int outputLevel, int value1, int value2);

	char patchName[11];
    uint8_t unpackedData[DX7_VOICE_SIZE_UNPACKED];
    short *arpeggiatorPartOfThePreset;
};


#endif

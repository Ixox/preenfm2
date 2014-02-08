/*
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



#include <math.h>
#include "Hexter.h"
#include "SynthState.h"



int outputLevelValues[] = { 0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46 } ;

float dx7_voice_eg_rate_rise_duration[128] = {  /* generated from my f04new */

     39.638000,  37.013000,  34.388000,  31.763000,  27.210500,
     22.658000,  20.408000,  18.158000,  15.908000,  14.557000,
     13.206000,  12.108333,  11.010667,   9.913000,   8.921000,
      7.929000,   7.171333,   6.413667,   5.656000,   5.307000,
      4.958000,   4.405667,   3.853333,   3.301000,   2.889000,
      2.477000,   2.313000,   2.149000,   1.985000,   1.700500,
      1.416000,   1.274333,   1.132667,   0.991000,   0.909000,
      0.827000,   0.758000,   0.689000,   0.620000,   0.558000,
      0.496000,   0.448667,   0.401333,   0.354000,   0.332000,
      0.310000,   0.275667,   0.241333,   0.207000,   0.180950,
      0.154900,   0.144567,   0.134233,   0.123900,   0.106200,
      0.088500,   0.079667,   0.070833,   0.062000,   0.056800,
      0.051600,   0.047300,   0.043000,   0.038700,   0.034800,
      0.030900,   0.028000,   0.025100,   0.022200,   0.020815,
      0.019430,   0.017237,   0.015043,   0.012850,   0.011230,
      0.009610,   0.009077,   0.008543,   0.008010,   0.006960,
      0.005910,   0.005357,   0.004803,   0.004250,   0.003960,
      0.003670,   0.003310,   0.002950,   0.002590,   0.002420,
      0.002250,   0.002000,   0.001749,   0.001499,   0.001443,
      0.001387,   0.001242,   0.001096,   0.000951,   0.000815,
      0.000815,   0.000815,   0.000815,   0.000815,   0.000815,
      0.000815,   0.000815,   0.000815,   0.000815,   0.000815,
      0.000815,   0.000815,   0.000815,   0.000815,   0.000815,
      0.000815,   0.000815,   0.000815,   0.000815,   0.000815,
      0.000815,   0.000815,   0.000815,   0.000815,   0.000815,
      0.000815,   0.000815,   0.000815

};

float dx7_voice_eg_rate_decay_duration[128] = {  /* generated from my f06new */

    317.487000, 285.764500, 254.042000, 229.857000, 205.672000,
    181.487000, 170.154000, 158.821000, 141.150667, 123.480333,
    105.810000,  98.382500,  90.955000,  81.804667,  72.654333,
     63.504000,  58.217000,  52.930000,  48.512333,  44.094667,
     39.677000,  33.089000,  26.501000,  24.283333,  22.065667,
     19.848000,  17.881500,  15.915000,  14.389667,  12.864333,
     11.339000,  10.641000,   9.943000,   8.833333,   7.723667,
      6.614000,   6.149500,   5.685000,   5.112667,   4.540333,
      3.968000,   3.639000,   3.310000,   3.033667,   2.757333,
      2.481000,   2.069500,   1.658000,   1.518667,   1.379333,
      1.240000,   1.116500,   0.993000,   0.898333,   0.803667,
      0.709000,   0.665500,   0.622000,   0.552667,   0.483333,
      0.414000,   0.384500,   0.355000,   0.319333,   0.283667,
      0.248000,   0.228000,   0.208000,   0.190600,   0.173200,
      0.155800,   0.129900,   0.104000,   0.095400,   0.086800,
      0.078200,   0.070350,   0.062500,   0.056600,   0.050700,
      0.044800,   0.042000,   0.039200,   0.034833,   0.030467,
      0.026100,   0.024250,   0.022400,   0.020147,   0.017893,
      0.015640,   0.014305,   0.012970,   0.011973,   0.010977,
      0.009980,   0.008310,   0.006640,   0.006190,   0.005740,
      0.005740,   0.005740,   0.005740,   0.005740,   0.005740,
      0.005740,   0.005740,   0.005740,   0.005740,   0.005740,
      0.005740,   0.005740,   0.005740,   0.005740,   0.005740,
      0.005740,   0.005740,   0.005740,   0.005740,   0.005740,
      0.005740,   0.005740,   0.005740,   0.005740,   0.005740,
      0.005740,   0.005740,   0.005740

};

float dx7_voice_eg_rate_decay_percent[128] = {  /* generated from P/H/Op f07 */

    0.000010, 0.025009, 0.050008, 0.075007, 0.100006,
    0.125005, 0.150004, 0.175003, 0.200002, 0.225001,
    0.250000, 0.260000, 0.270000, 0.280000, 0.290000,
    0.300000, 0.310000, 0.320000, 0.330000, 0.340000,
    0.350000, 0.358000, 0.366000, 0.374000, 0.382000,
    0.390000, 0.398000, 0.406000, 0.414000, 0.422000,
    0.430000, 0.439000, 0.448000, 0.457000, 0.466000,
    0.475000, 0.484000, 0.493000, 0.502000, 0.511000,
    0.520000, 0.527000, 0.534000, 0.541000, 0.548000,
    0.555000, 0.562000, 0.569000, 0.576000, 0.583000,
    0.590000, 0.601000, 0.612000, 0.623000, 0.634000,
    0.645000, 0.656000, 0.667000, 0.678000, 0.689000,
    0.700000, 0.707000, 0.714000, 0.721000, 0.728000,
    0.735000, 0.742000, 0.749000, 0.756000, 0.763000,
    0.770000, 0.777000, 0.784000, 0.791000, 0.798000,
    0.805000, 0.812000, 0.819000, 0.826000, 0.833000,
    0.840000, 0.848000, 0.856000, 0.864000, 0.872000,
    0.880000, 0.888000, 0.896000, 0.904000, 0.912000,
    0.920000, 0.928889, 0.937778, 0.946667, 0.955556,
    0.964444, 0.973333, 0.982222, 0.991111, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000

};



/* This table converts LFO speed to frequency in Hz. It is based on
 * interpolation of Jamie Bullock's measurements. */
float dx7_voice_lfo_frequency[128] = {
     0.062506,  0.124815,  0.311474,  0.435381,  0.619784,
     0.744396,  0.930495,  1.116390,  1.284220,  1.496880,
     1.567830,  1.738994,  1.910158,  2.081322,  2.252486,
     2.423650,  2.580668,  2.737686,  2.894704,  3.051722,
     3.208740,  3.366820,  3.524900,  3.682980,  3.841060,
     3.999140,  4.159420,  4.319700,  4.479980,  4.640260,
     4.800540,  4.953584,  5.106628,  5.259672,  5.412716,
     5.565760,  5.724918,  5.884076,  6.043234,  6.202392,
     6.361550,  6.520044,  6.678538,  6.837032,  6.995526,
     7.154020,  7.300500,  7.446980,  7.593460,  7.739940,
     7.886420,  8.020588,  8.154756,  8.288924,  8.423092,
     8.557260,  8.712624,  8.867988,  9.023352,  9.178716,
     9.334080,  9.669644, 10.005208, 10.340772, 10.676336,
    11.011900, 11.963680, 12.915460, 13.867240, 14.819020,
    15.770800, 16.640240, 17.509680, 18.379120, 19.248560,
    20.118000, 21.040700, 21.963400, 22.886100, 23.808800,
    24.731500, 25.759740, 26.787980, 27.816220, 28.844460,
    29.872700, 31.228200, 32.583700, 33.939200, 35.294700,
    36.650200, 37.812480, 38.974760, 40.137040, 41.299320,
    42.461600, 43.639800, 44.818000, 45.996200, 47.174400,
    47.174400, 47.174400, 47.174400, 47.174400, 47.174400,
    47.174400, 47.174400, 47.174400, 47.174400, 47.174400,
    47.174400, 47.174400, 47.174400, 47.174400, 47.174400,
    47.174400, 47.174400, 47.174400, 47.174400, 47.174400,
    47.174400, 47.174400, 47.174400, 47.174400, 47.174400,
    47.174400, 47.174400, 47.174400
};



float dx7_voice_eg_rate_rise_percent[128] = {  /* checked, matches P/H/Op f05 */

    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.000010, 0.000010, 0.000010,
    0.000010, 0.000010, 0.005007, 0.010005, 0.015003,
    0.020000, 0.028000, 0.036000, 0.044000, 0.052000,
    0.060000, 0.068000, 0.076000, 0.084000, 0.092000,
    0.100000, 0.108000, 0.116000, 0.124000, 0.132000,
    0.140000, 0.150000, 0.160000, 0.170000, 0.180000,
    0.190000, 0.200000, 0.210000, 0.220000, 0.230000,
    0.240000, 0.251000, 0.262000, 0.273000, 0.284000,
    0.295000, 0.306000, 0.317000, 0.328000, 0.339000,
    0.350000, 0.365000, 0.380000, 0.395000, 0.410000,
    0.425000, 0.440000, 0.455000, 0.470000, 0.485000,
    0.500000, 0.520000, 0.540000, 0.560000, 0.580000,
    0.600000, 0.620000, 0.640000, 0.660000, 0.680000,
    0.700000, 0.732000, 0.764000, 0.796000, 0.828000,
    0.860000, 0.895000, 0.930000, 0.965000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000, 1.000000, 1.000000,
    1.000000, 1.000000, 1.000000

};

/* This table converts pitch envelope level parameters into the
 * actual pitch shift in semitones.  For levels [17,85], this is
 * just ((level - 50) / 32 * 12), but at the outer edges the shift
 * is exagerated to 0 = -48 and 99 => 47.624.  This is based on
 * measurements I took from my TX7. */
double dx7_voice_pitch_level_to_shift[128] = {

    -48.000000, -43.497081, -38.995993, -35.626132, -31.873615,
    -28.495880, -25.500672, -22.872620, -20.998167, -19.496961,
    -18.373238, -17.251065, -16.122139, -15.375956, -14.624487,
    -13.876516, -13.126351, -12.375000, -12.000000, -11.625000,
    -11.250000, -10.875000, -10.500000, -10.125000, -9.750000,
    -9.375000, -9.000000, -8.625000, -8.250000, -7.875000,
    -7.500000, -7.125000, -6.750000, -6.375000, -6.000000,
    -5.625000, -5.250000, -4.875000, -4.500000, -4.125000,
    -3.750000, -3.375000, -3.000000, -2.625000, -2.250000,
    -1.875000, -1.500000, -1.125000, -0.750000, -0.375000, 0.000000,
    0.375000, 0.750000, 1.125000, 1.500000, 1.875000, 2.250000,
    2.625000, 3.000000, 3.375000, 3.750000, 4.125000, 4.500000,
    4.875000, 5.250000, 5.625000, 6.000000, 6.375000, 6.750000,
    7.125000, 7.500000, 7.875000, 8.250000, 8.625000, 9.000000,
    9.375000, 9.750000, 10.125000, 10.500000, 10.875000, 11.250000,
    11.625000, 12.000000, 12.375000, 12.750000, 13.125000,
    14.251187, 15.001922, 16.126327, 17.250917, 18.375718,
    19.877643, 21.753528, 24.373913, 27.378021, 30.748956,
    34.499234, 38.627888, 43.122335, 47.624065, 48.0, 48.0, 48.0,
    48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0,
    48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0, 48.0,
    48.0, 48.0, 48.0, 48.0, 48.0

};


/* This table converts pitch modulation sensitivity to semitones at full
 * modulation (assuming a perfectly linear pitch mod depth to pitch
 * relationship).  It is from a simple averaging of Jamie Bullock's
 * TX-data-1/PMD and TX-data-2/ENV data, and ignores the apparent ~0.1
 * semitone positive bias that Jamie observed. [-FIX- smbolton: my
 * inclination would be to call this bias, if it's reproducible, a
 * non-desirable 'bug', and _not_ implement it in hexter. And, at
 * least for my own personal build, I'd change that PMS=7 value to a
 * full octave, since that's one thing that's always bugged me about
 * my TX7.  Thoughts? ] */
float dx7_voice_pms_to_semitones[8] = {
    0.0, 0.450584, 0.900392, 1.474744,
    2.587385, 4.232292, 6.982097, /* 11.722111 */ 12.0
};


/* This table converts amplitude modulation depth to output level
 * reduction at full modulation with an amplitude modulation sensitivity
 * of 3.  It was constructed from regression of a very few data points,
 * using this code:
 *   perl -e 'for ($i = 0; $i <= 99; $i++) { printf " %f,\n", exp($i * 0.0428993 - 0.285189); }' >x.c
 * and is probably rather rough in its accuracy. -FIX- */
float dx7_voice_amd_to_ol_adjustment[100] = {
    0.0, 0.784829, 0.819230, 0.855139, 0.892622, 0.931748,
    0.972589, 1.015221, 1.059721, 1.106171, 1.154658, 1.205270,
    1.258100, 1.313246, 1.370809, 1.430896, 1.493616, 1.559085,
    1.627424, 1.698759, 1.773220, 1.850945, 1.932077, 2.016765,
    2.105166, 2.197441, 2.293761, 2.394303, 2.499252, 2.608801,
    2.723152, 2.842515, 2.967111, 3.097167, 3.232925, 3.374633,
    3.522552, 3.676956, 3.838127, 4.006362, 4.181972, 4.365280,
    4.556622, 4.756352, 4.964836, 5.182458, 5.409620, 5.646738,
    5.894251, 6.152612, 6.422298, 6.703805, 6.997652, 7.304378,
    7.624549, 7.958754, 8.307609, 8.671754, 9.051861, 9.448629,
    9.862789, 10.295103, 10.746365, 11.217408, 11.709099,
    12.222341, 12.758080, 13.317302, 13.901036, 14.510357,
    15.146387, 15.810295, 16.503304, 17.226690, 17.981783,
    18.769975, 19.592715, 20.451518, 21.347965, 22.283705,
    23.260462, 24.280032, 25.344294, 26.455204, 27.614809,
    28.825243, 30.088734, 31.407606, 32.784289, 34.221315,
    35.721330, 37.287095, 38.921492, 40.627529, 42.408347,
    44.267222, 46.207578, 48.232984, 50.347169, 52.75
};

Hexter::Hexter() {

}


void Hexter::loadHexterPatch(uint8_t* packedPatch, struct OneSynthParams *params) {
	patchUnpack(packedPatch, unpackedData);
	voiceSetData(params, unpackedData);
}

void Hexter::patchUnpack(uint8_t *packed_patch, uint8_t *unpacked_patch) {
	uint8_t *up = unpacked_patch,
			*pp = packed_patch;
	int     i, j;

	/* ugly because it used to be 68000 assembly... */
	for(i = 6; i > 0; i--) {
		for(j = 11; j > 0; j--) {
			*up++ = *pp++;
		}                           /* through rd */
		*up++     = (*pp) & 0x03;   /* lc */
		*up++     = (*pp++) >> 2;   /* rc */
		*up++     = (*pp) & 0x07;   /* rs */
		*(up + 6) = (*pp++) >> 3;   /* pd */
		*up++     = (*pp) & 0x03;   /* ams */
		*up++     = (*pp++) >> 2;   /* kvs */
		*up++     = *pp++;          /* ol */
		*up++     = (*pp) & 0x01;   /* m */
		*up++     = (*pp++) >> 1;   /* fc */
		*up       = *pp++;          /* ff */
		up += 2;
	}                               /* operator done */
	for(i = 9; i > 0; i--) {
		*up++ = *pp++;
	}                               /* through algorithm */
	*up++ = (*pp) & 0x07;           /* feedback */
	*up++ = (*pp++) >> 3;           /* oks */
	for(i = 4; i > 0; i--) {
		*up++ = *pp++;
	}                               /* through lamd */
	*up++ = (*pp) & 0x01;           /* lfo ks */
	*up++ = ((*pp) >> 1) & 0x07;    /* lfo wave */
	*up++ = (*pp++) >> 4;           /* lfo pms */
	for(i = 11; i > 0; i--) {
		*up++ = *pp++;
	}
}

int Hexter::limit(int x, int min, int max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void Hexter::setIM(struct OneSynthParams *params, int im, uint8_t *patch, int op) {
	float ratios[]  = { 1.0, .7, .5, .3, .2, .15, .1, .1};
	im--;
	op--;
	uint8_t *eb_op = patch + ((5 - op) * 21);
	float ratio =  ratios[eb_op[15] & 0x07];
	float modulation = getPreenFMIM(limit(eb_op[16], 0, 99));
	((float*)&params->engineIm1.modulationIndex1)[im * 2] =  modulation * ratio;
	((float*)&params->engineIm1.modulationIndexVelo1)[im * 2] = modulation * (1 - ratio);
}

void Hexter::setIMWithMax(struct OneSynthParams *params, int im, uint8_t *patch, int op, float max) {
	setIM(params, im, patch, op);

	if (((float*)&params->engineIm1.modulationIndex1)[im * 2] > max) {
		((float*)&params->engineIm1.modulationIndex1)[im * 2] = max;
	}
	if (((float*)&params->engineIm1.modulationIndexVelo1)[im * 2] > max*.5f) {
		((float*)&params->engineIm1.modulationIndexVelo1)[im * 2] = max*.5f;
	}
}

void Hexter::setMix(struct OneSynthParams *params, int im, uint8_t *patch, int op) {
	im--;
	op--;
	uint8_t *eb_op = patch + ((5 - op) * 21);
	((float*)&params->engineMix1.mixOsc1)[im * 2]  = (float)(limit(eb_op[16], 0, 99) + 1.0) / 100.0f;
}

float Hexter::getPreenFMIM(int lvl) {
	if (lvl>100)  {
		lvl = 100;
	}

	float im = 0;
	if (lvl < 50) {
		im =  .0f + lvl * .006;
	} else if (lvl < 60) {
		im = .3f + (lvl - 50) *  .02;
	} else if (lvl < 70) {
		im =  .5f + (lvl - 60) *  .07;
	} else if (lvl < 80) {
		im =  1.2f + (lvl - 70) *  .1;
	} else if (lvl < 85) {
		im =  2.2f + (lvl - 80) *  .2;
	} else if (lvl < 90) {
		im =  3.2f + (lvl - 85) *  .25;
	} else  {
		im =  3.95f + (lvl - 90) * .35;
	}
	return im;
}


int Hexter::abs(int value) {
	return value < 0 ? -value : value;
}

float Hexter::abs(float value) {
	return value < 0 ? -value : value;
}


float Hexter::getRounded(float r) {
	float t = r*2;

	int ti = t + .5;
	// Round 1.46, 2.02 to close half decimal number
	if (abs(t -ti ) < 0.25f || r > 3) {
		return ((float)ti) / 2.0f;
	}

	return r;
}


/*
 * dx7_voice_set_data
 */
void Hexter::voiceSetData(struct OneSynthParams *params, uint8_t *patch)
{

    for (int k=0; k<sizeof(struct OneSynthParams)/sizeof(float); k++) {
    	if ((*arpeggiatorPartOfThePreset) == 0 && (k >> 2) == ROW_ARPEGGIATOR1 || (k >> 2) == ROW_ARPEGGIATOR2) {
    		// Don't override the arpeggiator
    	} else {
    		((float*)params)[k] = ((float*)&defaultPreset)[k];
    	}
    }


    struct EnvelopeParamsA* envParamsA[] = { &params->env1a, &params->env2a, &params->env3a, &params->env4a, &params->env5a, &params->env6a};
    struct EnvelopeParamsB* envParamsB[] = { &params->env1b, &params->env2b, &params->env3b, &params->env4b, &params->env5b, &params->env6b};
    struct OscillatorParams* oscParams[] = { &params->osc1, &params->osc2, &params->osc3, &params->osc4, &params->osc5, &params->osc6};


    int transpose = limit(patch[144], 0, 48) - 24;
    float transposeMultiply = 1.0f;
    if (transpose < -6) {
        transposeMultiply = .5f;
    } else if (transpose < -18) {
        transposeMultiply = .25f;
    }

	for (int i = 0; i < 6; i++) {
		uint8_t *eb_op = patch + ((5 - i) * 21);
		struct OscillatorParams* oscParam = oscParams[i];
		struct EnvelopeParamsA* envA = envParamsA[i];
		struct EnvelopeParamsB* envB = envParamsB[i];

		oscParam->shape = OSC_SHAPE_SIN;

		// voice->op[i].osc_mode      = eb_op[17] & 0x01;
		oscParam->frequencyType = eb_op[17] & 0x01;

		// voice->op[i].detune        = limit(eb_op[20], 0, 14);
		oscParam->detune = ((float)limit(eb_op[20], 0, 14) - 7.0f) / 100.0f / 4.0f;

		// Keyboard
		if ((eb_op[17] & 0x01) == 0) {
			// voice->op[i].coarse        = eb_op[18] & 0x1f;
			if ((eb_op[18] & 0x1f) == 0) {
				oscParam->frequencyMul = .5;
			} else {
				oscParam->frequencyMul = eb_op[18] & 0x1f;
			}
			// voice->op[i].fine          = limit(eb_op[19], 0, 99);
			oscParam->frequencyMul *= 1.0f + (float)limit(eb_op[19], 0, 99) / 100.0f;
		} else {
			float freq = exp(M_LN10 * ((double)(eb_op[18] & 3) + (double)eb_op[19] / 100.0f));
			oscParam->frequencyMul = freq / 1000.0f;

			// Low frequency does not work at all with PreenFM
			if (oscParam->frequencyMul < 40) {
				oscParam->frequencyType = 0;
				oscParam->frequencyMul = 1.0f;
				oscParam->detune = 0.0;
			}
		}

		// transpose ?
		oscParam->frequencyMul *= transposeMultiply;



//		voice->op[i].level_scaling_bkpoint = limit(eb_op[ 8], 0, 99);
//		voice->op[i].level_scaling_l_depth = limit(eb_op[ 9], 0, 99);
//		voice->op[i].level_scaling_r_depth = limit(eb_op[10], 0, 99);
//		voice->op[i].level_scaling_l_curve = eb_op[11] & 0x03;
//		voice->op[i].level_scaling_r_curve = eb_op[12] & 0x03;
//		voice->op[i].rate_scaling          = eb_op[13] & 0x07;
//		voice->op[i].amp_mod_sens          = (compat059 ? 0 : eb_op[14] & 0x03);
//		voice->op[i].velocity_sens         = eb_op[15] & 0x07;

//		for (int j = 0; j < 4; j++) {
//			voice->op[i].eg.base_rate[j]  = limit(eb_op[j], 0, 99);
//			voice->op[i].eg.base_level[j] = limit(eb_op[4 + j], 0, 99);
		// Use base_rate[0]



		envA->attackTime = getChangeTime(limit(eb_op[16], 0, 99), limit(eb_op[0], 0, 99), 0, limit(eb_op[4], 0, 99));
		envA->decayTime = getChangeTime(limit(eb_op[16], 0, 99), limit(eb_op[1], 0, 99), limit(eb_op[4], 0, 99), limit(eb_op[5], 0, 99));
		envB->sustainTime = getChangeTime(limit(eb_op[16], 0, 99), limit(eb_op[2], 0, 99), limit(eb_op[5], 0, 99), limit(eb_op[6], 0, 99));
		// Take previous value if the release one is 0
		int releaseFromValue = limit(eb_op[6] != 0 ? eb_op[6] : (eb_op[5] != 0 ? eb_op[5] : eb_op[4]), 0, 99);
		envB->releaseTime = getChangeTime(limit(eb_op[16], 0, 99), limit(eb_op[3], 0, 99), releaseFromValue, 0);


//		envA->attackTime = dx7_voice_eg_rate_rise_duration[limit(eb_op[0], 0, 99)] * eb_op[0] / ;
//		if (eb_op[5] >= eb_op[4]) {
//			envA->decayTime = dx7_voice_eg_rate_decay_duration[limit(eb_op[1], 0, 99)]  / 4.0f * abs(eb_op[4] - eb_op[5]) / 99.0f;
//		} else {
//			envA->decayTime = dx7_voice_eg_rate_rise_duration[limit(eb_op[1], 0, 99)]  / 4.0f * abs(eb_op[4] - eb_op[5]) / 99.0f;
//		}
//		envB->sustainTime = dx7_voice_eg_rate_decay_duration[limit(eb_op[2], 0, 99)] / 4.0f * abs(eb_op[6] - eb_op[5]) / 99.0f;
//		envB->releaseTime = dx7_voice_eg_rate_decay_duration[limit(eb_op[3], 0, 99)] / 4.0f;

		if (envA->attackTime > 16.0) {
			envA->attackTime = 16.0;
		}
		if (envA->decayTime > 16.0) {
			envA->decayTime = 16.0;
		}
		if (envB->sustainTime > 16.0) {
			envB->sustainTime = 16.0;
		}
		if (envB->releaseTime > 16.0) {
			envB->releaseTime = 16.0;
		}
		if (envB->releaseTime < 0.04) {
			envB->releaseTime = 0.04;
		}

		envA->attackLevel = dx7_voice_eg_rate_rise_percent[ limit(eb_op[4], 0, 99)];
		envA->decayLevel = dx7_voice_eg_rate_decay_percent[limit(eb_op[5], 0, 99)];
		envB->sustainLevel = dx7_voice_eg_rate_decay_percent[limit(eb_op[6], 0, 99)];
		envB->releaseLevel = dx7_voice_eg_rate_decay_percent[limit(eb_op[7], 0, 99)];



		//		}
	}

//	for (int i = 0; i < 4; i++) {
//		voice->pitch_eg.rate[i]  = limit(edit_buffer[126 + i], 0, 99);
//		voice->pitch_eg.level[i] = limit(edit_buffer[130 + i], 0, 99);
//	}


	// voice->op[i].output_level  = limit(eb_op[16], 0, 99);


	int fb = patch[135] & 0x07;

	int preenAlgo = -1;
	int algo = (patch[134] & 0x1f) + 1;
	switch (algo) {
	case 1:
	case 2:
		preenAlgo = ALG10;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIMWithMax(params, 3, patch, 5, 4);
		setIMWithMax(params, 4, patch, 6, 2);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		if (fb > 4) {
			if (algo == 1) {
				params->osc6.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc2.shape = OSC_SHAPE_SAW;;
			}
		}
		break;
	case 3:
	case 4:
		preenAlgo = ALG11;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 3);
		setIMWithMax(params, 3, patch, 5, 4);
		setIMWithMax(params, 4, patch, 6, 4);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 4);
		if (fb > 4) {
			if (algo == 3) {
				params->osc6.shape = OSC_SHAPE_SAW;;
			}
		}

		break;
	case 5:
	case 6:
		preenAlgo = ALG12;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIM(params, 3, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		setMix(params, 3, patch, 5);
		if (fb > 4) {
			if (algo == 5) {
				params->osc6.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc6.shape = OSC_SHAPE_SAW;;
			}
		}
		break;
	case 7:
	case 8:
	case 9:
		preenAlgo = ALG13;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIM(params, 3, patch, 5);
		setIMWithMax(params, 4, patch, 6, 4);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		if (fb > 4) {
			if (algo == 7) {
				params->osc6.shape = OSC_SHAPE_SAW;;
			} else if (algo == 8) {
				params->osc4.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc2.shape = OSC_SHAPE_SAW;;
			}
		}

		break;
	case 10:
	case 11:
		preenAlgo = ALG14;
		setIM(params, 1, patch, 2);
		setIMWithMax(params, 2, patch, 3, 4);
		setIM(params, 3, patch, 5);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 4);
		if (fb > 4) {
			if (algo == 10) {
				params->osc3.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc6.shape = OSC_SHAPE_SAW;;
			}
		}
		break;

	case 12:
	case 13:
		preenAlgo = ALG15;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIM(params, 3, patch, 5);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		if (fb > 4) {
			if (algo == 12) {
				params->osc2.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc6.shape = OSC_SHAPE_SAW;;
			}
		}
		break;
	case 14:
	case 15:
		preenAlgo = ALG16;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIMWithMax(params, 3, patch, 5, 4);
		setIMWithMax(params, 4, patch, 6, 4);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		if (fb > 4) {
			if (algo == 14) {
				params->osc6.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc2.shape = OSC_SHAPE_SAW;;
			}
		}

		break;
	case 16:
	case 17:
		preenAlgo = ALG17;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 3);
		setIMWithMax(params, 3, patch, 4, 4);
		setIM(params, 4, patch, 5);
		setIMWithMax(params, 5, patch, 6, 4);

		setMix(params, 1, patch, 1);
		if (fb > 4) {
			if (algo == 16) {
				// TOO VIOLENT WITH PREENFM !
				// params->osc6.shape = OSC_SHAPE_SAW;;
			} else {
				params->osc2.shape = OSC_SHAPE_SAW;;
			}
		}

		break;
	case 18:
		preenAlgo = ALG18;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 3);
		setIM(params, 3, patch, 4);
		setIMWithMax(params, 4, patch, 5, 4);
		setIMWithMax(params, 5, patch, 6, 2);

		setMix(params, 1, patch, 1);
		if (fb > 4) {
			params->osc3.shape = OSC_SHAPE_SAW;;
		}

		break;
	case 19:
		preenAlgo = ALG19;
		setIM(params, 1, patch, 2);
		setIMWithMax(params, 2, patch, 3, 4);
		setIM(params, 3, patch, 6);
		setIM(params, 4, patch, 6);

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 4);
		setMix(params, 3, patch, 5);
		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;;
		}

		break;
	case 20:
		preenAlgo = ALG20;
		setIM(params, 1, patch, 3);
		setIM(params, 2, patch, 3);
		setIM(params, 3, patch, 5);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 4);
		if (fb > 4) {
			params->osc3.shape = OSC_SHAPE_SAW;;
		}
		break;
	case 21:
		preenAlgo = ALG21;
		setIM(params, 1, patch, 3);
		setIM(params, 2, patch, 3);
		setIM(params, 3, patch, 6);
		setIM(params, 4, patch, 6);

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 4);
		setMix(params, 4, patch, 5);
		if (fb > 4) {
			params->osc3.shape = OSC_SHAPE_SAW;;
		}
		break;
	case 22:
		preenAlgo = ALG22;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 6);
		setIM(params, 3, patch, 6);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		setMix(params, 3, patch, 4);
		setMix(params, 4, patch, 5);
		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;;
		}
		break;
	case 23:
		preenAlgo = ALG21;
		params->engineIm1.modulationIndex1 = 0;
		setIM(params, 2, patch, 3);
		setIM(params, 3, patch, 6);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 4);
		setMix(params, 4, patch, 5);
		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;;
		}
		break;
	case 24:
	case 25:
		preenAlgo = ALG23;
		setIM(params, 1, patch, 6);
		setIM(params, 2, patch, 6);
		setIM(params, 3, patch, 6);

		if (algo == 25) {
			params->engineIm1.modulationIndex1 = 0;
		}

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 3);
		setMix(params, 4, patch, 4);
		setMix(params, 5, patch, 5);

		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;;
		}
		break;

	case 26:
	case 27:
		preenAlgo = ALG20;
		params->engineIm1.modulationIndex1 = 0;
		setIM(params, 2, patch, 3);
		setIM(params, 3, patch, 5);
		setIM(params, 4, patch, 6);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 4);
		if (fb > 4) {
			if (algo == 26) {
				params->osc6.shape = OSC_SHAPE_SAW;
			} else {
				params->osc3.shape = OSC_SHAPE_SAW;
			}
		}
		break;
	case 28:
		preenAlgo = ALG24;
		setIM(params, 1, patch, 2);
		setIM(params, 2, patch, 4);
		setIMWithMax(params, 3, patch, 5, 4);
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 3);
		setMix(params, 3, patch, 6);
		if (fb > 4) {
			params->osc5.shape = OSC_SHAPE_SAW;
		}
		break;
	case 29:
		preenAlgo = ALG25;
		setIM(params, 1, patch, 4);
		setIM(params, 2, patch, 6);

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 3);
		setMix(params, 4, patch, 5);
		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;
		}
		break;
	case 30:
		preenAlgo = ALG26;
		setIM(params, 1, patch, 4);
		setIMWithMax(params, 2, patch, 5, 2.0);

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 3);
		setMix(params, 4, patch, 6);
		if (fb > 4) {
			params->osc5.shape = OSC_SHAPE_SAW;
		}
		break;

	case 31:
		preenAlgo = ALG28;

		params->engineIm1.modulationIndex1 = 0;
		params->engineIm1.modulationIndex2 = 0;
		setIM(params, 1, patch, 5);

		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 3);
		setMix(params, 4, patch, 4);
		setMix(params, 5, patch, 6);

		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;
		}
		break;

	case 32:
		preenAlgo = ALG27;
		setMix(params, 1, patch, 1);
		setMix(params, 2, patch, 2);
		setMix(params, 3, patch, 3);
		setMix(params, 4, patch, 4);
		setMix(params, 5, patch, 5);
		setMix(params, 6, patch, 6);
		if (fb > 4) {
			params->osc6.shape = OSC_SHAPE_SAW;;
		}
		break;
	}

	if (preenAlgo == -1) {
		params->presetName[0] = '#';
		params->presetName[1] = '!';
		voiceCopyName(params->presetName + 2, patch);
		params->engine1.algo = ALGO7;
	} else {
		params->engine1.algo = preenAlgo ;
		voiceCopyName(params->presetName, patch);
	}

    int algoWithSensibleOp6[] = { ALG10, ALG11, ALG13, ALG16, ALG17, ALG18, ALGO_END } ;
    int algoWithSensibleOp5[] = { ALG10, ALG16, ALG18, ALG24, ALG26, ALGO_END } ;
    int algoWithSensibleOp4[] = { ALG17, ALGO_END } ;
    int algoWithSensibleOp3[] = { ALG11, ALG14 , ALG19, ALGO_END } ;

	while (params->osc1.frequencyMul > 8) {
		params->osc1.frequencyMul /= 2;
	}
	while (params->osc2.frequencyMul > 8) {
		params->osc2.frequencyMul /= 2;
	}
	while (params->osc3.frequencyMul > 8) {
		params->osc3.frequencyMul /= 2;
	}
	while (params->osc4.frequencyMul > 8) {
		params->osc4.frequencyMul /= 2;
	}
	while (params->osc5.frequencyMul > 8) {
		params->osc5.frequencyMul /= 2;
	}
	while (params->osc6.frequencyMul > 8) {
		params->osc6.frequencyMul /= 2;
	}



	for (int k=0; algoWithSensibleOp6[k] != ALGO_END; k++) {
		if (preenAlgo == algoWithSensibleOp6[k]) {
			while (params->osc6.frequencyMul > 2) {
				params->osc6.frequencyMul /= 2;
			}
			// Let's round it 0.5
			params->osc6.frequencyMul = ((int)(params->osc6.frequencyMul * 2 + 1)) / 2.0f;
			params->osc6.detune = 0;
			params->osc6.shape = OSC_SHAPE_SIN;
		}
	}

	for (int k=0; algoWithSensibleOp5[k] != ALGO_END; k++) {
		if (preenAlgo == algoWithSensibleOp5[k]) {
			while (params->osc5.frequencyMul > 2) {
				params->osc5.frequencyMul /= 2;
			}
			// Let's round it 0.5
			params->osc5.frequencyMul = ((int)(params->osc5.frequencyMul * 2 + 1)) / 2.0f;
			params->osc5.detune = 0;
			params->osc5.shape = OSC_SHAPE_SIN;
		}
	}

	for (int k=0; algoWithSensibleOp4[k] != ALGO_END; k++) {
		if (preenAlgo == algoWithSensibleOp4[k]) {
			while (params->osc4.frequencyMul > 2) {
				params->osc4.frequencyMul /= 2;
			}
			// Let's round it 0.5
			params->osc4.frequencyMul = ((int)(params->osc4.frequencyMul * 2 + 1)) / 2.0f;
			params->osc4.detune = 0;
			params->osc4.shape = OSC_SHAPE_SIN;
		}
	}

	for (int k=0; algoWithSensibleOp3[k] != ALGO_END; k++) {
		if (preenAlgo == algoWithSensibleOp3[k]) {
			while (params->osc3.frequencyMul > 2) {
				params->osc3.frequencyMul /= 2;
			}
			// Let's round it 0.5
			params->osc3.frequencyMul = ((int)(params->osc3.frequencyMul * 2 + 1)) / 2.0f;
			params->osc3.detune = 0;
			params->osc3.shape = OSC_SHAPE_SIN;
		}
	}

	params->osc1.frequencyMul = getRounded(params->osc1.frequencyMul);
	params->osc2.frequencyMul = getRounded(params->osc2.frequencyMul);
	params->osc3.frequencyMul = getRounded(params->osc3.frequencyMul);
	params->osc4.frequencyMul = getRounded(params->osc4.frequencyMul);
	params->osc5.frequencyMul = getRounded(params->osc5.frequencyMul);
	params->osc6.frequencyMul = getRounded(params->osc6.frequencyMul);
//

	/* the "99.0" here is because we're also using this multiplier to scale the
	 * eg level from 0-99 to 0-1 */
//	voice->feedback_multiplier = DOUBLE_TO_FP(aux_feedbk / 99.0);
//
//	voice->osc_key_sync = edit_buffer[136] & 0x01;


	//	voice->lfo_speed    = limit(edit_buffer[137], 0, 99);
	//	voice->lfo_delay    = limit(edit_buffer[138], 0, 99);
	//	voice->lfo_wave     = limit(edit_buffer[142], 0, 5);
	float delay = (float)limit(patch[138], 0, 99)/ 15.0f;
	params->lfoOsc1.freq = dx7_voice_lfo_frequency[limit(patch[137], 0, 99)];
	if (params->lfoOsc1.freq > 24.0) {
		params->lfoOsc1.freq = 24.0;
	}
	params->lfoOsc1.keybRamp = delay;
	params->lfoOsc1.shape = LFO_SIN;
	params->lfoOsc1.bias = 0;

	// lfo2 for mix
	params->lfoOsc2.freq = dx7_voice_lfo_frequency[limit(patch[137], 0, 99)];
	if (params->lfoOsc2.freq > 24.0) {
		params->lfoOsc2.freq = 24.0;
	}
	params->lfoOsc2.keybRamp = delay;
	params->lfoOsc2.shape = LFO_SIN;
	// To avoid mix to overload...
	params->lfoOsc2.bias = -1.0f;

//	voice->lfo_pmd      = limit(edit_buffer[139], 0, 99);
//	voice->lfo_amd      = limit(edit_buffer[140], 0, 99);
//	voice->lfo_key_sync = edit_buffer[141] & 0x01;
//	voice->lfo_pms      = (compat059 ? 0 : edit_buffer[143] & 0x07);
	//	voice->lfo_pmd      = limit(edit_buffer[139], 0, 99);
	//	voice->lfo_amd      = limit(edit_buffer[140], 0, 99);


/*
	float *toDelete = &params->matrixRowState1.source;
	for (int k=0; 12 * 4; k++) {
		toDelete[k] = 0.0f;
	}
*/


	// Matrix use LFO 1 (o all frequency
	params->matrixRowState1.source = MATRIX_SOURCE_LFO1;
	params->matrixRowState1.mul =  (float)patch[139] / 120.0f;
	params->matrixRowState1.destination = ALL_OSC_FREQ;

	params->matrixRowState2.source = MATRIX_SOURCE_LFO2;
	params->matrixRowState2.mul = dx7_voice_amd_to_ol_adjustment[(patch[140])] / 100.0f;
	params->matrixRowState2.destination = ALL_MIX;

	params->matrixRowState2.mul = 0.0f;
	params->matrixRowState3.mul = 0.0f;
	params->matrixRowState4.mul = 0.0f;
	params->matrixRowState5.mul = 0.0f;
	params->matrixRowState6.mul = 0.0f;
	params->matrixRowState7.mul = 0.0f;
	params->matrixRowState8.mul = 0.0f;
	params->matrixRowState9.mul = 0.0f;

	params->matrixRowState10.source = MATRIX_SOURCE_AFTERTOUCH;
	params->matrixRowState10.mul = 0.0f;
	params->matrixRowState10.destination = INDEX_MODULATION1;

	params->matrixRowState11.source = MATRIX_SOURCE_MODWHEEL;
	params->matrixRowState11.mul = 3.0f;
	params->matrixRowState11.destination = INDEX_ALL_MODULATION;

	params->matrixRowState12.source = MATRIX_SOURCE_PITCHBEND;
	params->matrixRowState12.mul = 1.0f;
	params->matrixRowState12.destination = ALL_OSC_FREQ;
}



/*
 * dx7_bulk_dump_checksum
 */
int Hexter::bulkDumpChecksum(uint8_t *data, int length)
{
    int sum = 0;
    int i;

    for (i = 0; i < length; sum -= data[i++]);
    return sum & 0x7F;
}



/*
 * dx7_voice_copy_name
 */
void Hexter::voiceCopyName(char *name, uint8_t *patch)
{
    int i;
    unsigned char c;

    for (i = 0; i < 10; i++) {
        c = (unsigned char)patch[i + 145];
        switch (c) {
            case  92:  c = 'Y';  break;  /* yen */
            case 126:  c = '>';  break;  /* >> */
            case 127:  c = '<';  break;  /* << */
            default:
                if (c < 32 || c > 127) c = 32;
                break;
        }
        name[i] = c;
    }
    name[10] = 0;
}

float Hexter::getAttackRatio(int outputLevel, int value1, int value2) {
	if (value1 < 32 && value2 > 32) {
		return 12.0f;
	}
	return 8;
//	int multipliers = 0;
//	int ol = getActualOutputLevel(outputLevel) * 32;
//	for (int k=value1; k< value2 ; k++) {
//		multipliers += 2 + (8096 - (ol + getActualLevel(k) * 64)) / 256;
//	}
//	return multipliers / (value2 - value1);
}


#define FP_DIVIDE_CEIL(n, d)  (((n) + (d) - 1) / (d))

float Hexter::getChangeTime(int outputLevel, int time, int value1, int value2) {
//	float duration;
//	int need_compensation = 0;
//    if (value1 <= 31) {
//        if (value2 > 31) {
//            /* rise quickly to 31, then continue normally */
//            need_compensation = 1;
//            duration = dx7_voice_eg_rate_rise_duration[time] *
//                       (dx7_voice_eg_rate_rise_percent[value2] -
//                        dx7_voice_eg_rate_rise_percent[value1]);
//        } else if (value2 - value1 > 9) {
//            /* these seem to take zero time */
//            need_compensation = 0;
//            duration = 0.0f;
//        } else {
//            /* these are the exploited delays */
//            need_compensation = 0;
//            /* -FIX- this doesn't make WATER GDN work? */
//            duration = dx7_voice_eg_rate_rise_duration[time] *
//                       (float)(value2 - value1) / 100.0f;
//        }
//    } else {
//        need_compensation = 0;
//        duration = dx7_voice_eg_rate_rise_duration[time] *
//                   (dx7_voice_eg_rate_rise_percent[value2] -
//                    dx7_voice_eg_rate_rise_percent[value1]);
//    }
//
//    duration *= PREENFM_FREQUENCY;
//
//    float duration = dx7_voice_eg_rate_rise_duration[99] *
//      (dx7_voice_eg_rate_rise_percent[99] - dx7_voice_eg_rate_rise_percent[0]);
//    float dx7_eg_max_slew = 99.0f / duration * PREENFM_FREQUENCY;
//
//    int32_t precomp_duration = FP_DIVIDE_CEIL(31 - value1, dx7_eg_max_slew);

	float value1InDb = .0235f * getActualLevel(value1) * 64;
	float value2InDb = .0235f * getActualLevel(value2) * 64;
	float diffBetweenBaluesInDb = abs(value1InDb - value2InDb);

	int qtime = time * 41 / 64;
	float dbPerS = .5 * pow(2, (float)qtime * .25f);

	if (value2 > value1) {
		dbPerS *= getAttackRatio(outputLevel, value1, value2);
	}

	return diffBetweenBaluesInDb / dbPerS;

}
int Hexter::getActualLevel(int value) {
	// FROM : https://code.google.com/p/music-synthesizer-for-android/wiki/Dx7Envelope
	//	Level 0..5 -> actual level = 2 * l
	//	Level 5..16 -> actual level = 5 + l
	//	Level 17..20 -> actual level = 4 + l
	//	Level 20..99 -> actual level = 14 + (l >> 1)
	if (value < 5) {
		return 2 * value;
	} else if (value < 16) {
		return 5 + value;
	} else if (value < 20) {
		return 4 + value;
	} else {
		return 14 + (value >> 1);
	}
}

int Hexter::getActualOutputLevel(int value) {
	if (value < 20) {
		return outputLevelValues[value];
	} else {
		return 28 + value;
	}
}



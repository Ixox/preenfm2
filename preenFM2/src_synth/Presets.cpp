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

#include "Common.h"
#include "SynthState.h"


const struct OneSynthParams presets[]  = {
        {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO9, 7, 3, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0,0,1,2.56} ,
                { .5,.5,1,4} ,
                { .1,.5,.6,4} ,
                { .1,.1,.56,2.66} ,
                { 0,.6,0,0.6} ,
                { .1,.5,.6,.3} ,
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX5_MUL, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION1, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFO2, 0, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFO3, 0, PAN_OSC1, 0} ,
                { MATRIX_SOURCE_NONE, 0, DESTINATION_NONE, 0} ,
                // LFOs
                { LFO_SIN, 4.5, 0, 0} ,
                { LFO_SIN, 4.8, 0, 1.0} ,
                { LFO_SIN, 6.0, 0, 4.0} ,
                { .2, 0, 1.0, 1.0} ,
                { 1, .2, .2, 1.0} ,
                { 110, .5,  0, 0}  ,
                { 140, .6, 0, 0},
                {{ 0,7,15,15,15,0,15,15,  0,15,15,15,0,15,15,15}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "Preen mk2"
        } ,
        {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO1, 7, 2, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0,0,1,2.56} ,
                { .5,.5,1,4} ,
                { .1,.5,.6,4} ,
                { .1,.1,.56,2.66} ,
                { 0,.6,0,0.6} ,
                { .1,.5,.6,.3} ,
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX5_MUL, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION1, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFO2, 0, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFO3, 0, PAN_OSC1, 0} ,
                { MATRIX_SOURCE_NONE, 0, DESTINATION_NONE, 0} ,
                // LFOs
                { LFO_SIN, 4.5, 0, 0} ,
                { LFO_SIN, 4.8, 0, 1.0} ,
                { LFO_SIN, 6.0, 0, 4.0} ,
                { .2, 0, 1.0, 1.0} ,
                { 1, .2, .2, 1.0} ,
                { 110, .5,  0, 0}  ,
                { 140, .6, 0, 0},
                {{ 0,7,15,15,15,0,15,15,  0,15,15,15,0,15,15,15}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "Preset 2"
        } ,
        {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO2, 7, 3, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0,0,1,2.56} ,
                { .5,.5,1,4} ,
                { .1,.5,.6,4} ,
                { .1,.1,.56,2.66} ,
                { 0,.6,0,0.6} ,
                { .1,.5,.6,.3} ,
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX5_MUL, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION1, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFO2, 0, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFO3, 0, PAN_OSC1, 0} ,
                { MATRIX_SOURCE_NONE, 0, DESTINATION_NONE, 0} ,
                // LFOs
                { LFO_SIN, 4.5, 0, 0} ,
                { LFO_SIN, 4.8, 0, 1.0} ,
                { LFO_SIN, 6.0, 0, 4.0} ,
                { .2, 0, 1.0, 1.0} ,
                { 1, .2, .2, 1.0} ,
                { 110, .5,  0, 0}  ,
                { 140, .6, 0, 0},
                {{ 0,7,15,15,15,0,15,15,  0,15,15,15,0,15,15,15}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "Preset 3"
        } ,
        {
                // patch name : 'Preen 2.0'
                // Engine
                { ALGO3, 7, 1, 4} ,
                { 1.5,1.9,1.8,0.7} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                { 1, 0, 1, 0} ,
                // Oscillator
                { OSC_SHAPE_SAW,  OSC_FT_KEYBOARD , 1.0, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , .5, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 2, 0} ,
                { OSC_SHAPE_SQUARE, OSC_FT_KEYBOARD , 4, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 6, 0} ,
                { OSC_SHAPE_RAND, OSC_FT_KEYBOARD , 2.5, 0} ,
                // Enveloppe
                { 0,0,1,2.56} ,
                { .5,.5,1,4} ,
                { .1,.5,.6,4} ,
                { .1,.1,.56,2.66} ,
                { 0,.6,0,0.6} ,
                { .1,.5,.6,.3} ,
                // Modulation matrix
                { MATRIX_SOURCE_LFO1, 1, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFOSEQ1, 0, MAIN_GATE, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFOENV1, 0, LFO1_FREQ, 0} ,
                { MATRIX_SOURCE_LFO1, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_VELOCITY, 0, INDEX_MODULATION4, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, MTX5_MUL, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION1, 0} ,
                { MATRIX_SOURCE_MODWHEEL, 0, INDEX_MODULATION2, 0} ,
                { MATRIX_SOURCE_LFO2, 0, PAN_OSC2, 0} ,
                { MATRIX_SOURCE_LFO3, 0, PAN_OSC1, 0} ,
                { MATRIX_SOURCE_NONE, 0, DESTINATION_NONE, 0} ,
                // LFOs
                { LFO_SIN, 4.5, 0, 0} ,
                { LFO_SIN, 4.8, 0, 1.0} ,
                { LFO_SIN, 6.0, 0, 4.0} ,
                { .2, 0, 1.0, 1.0} ,
                { 1, .2, .2, 1.0} ,
                { 110, .5,  0, 0}  ,
                { 140, .6, 0, 0},
                {{ 0,7,15,15,15,0,15,15,  0,15,15,15,0,15,15,15}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "Preset 4"
        } ,
        {
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,

                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,

                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,
                { 0,0,0,0} ,

                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,

                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                { 0, 0, 0, 0} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15 ,0}} ,
                {{ 15, 4, 2, 0, 15, 2, 0, 8, 15, 0, 12, 0, 8, 0, 15, 0}} ,
                "*"
        }
};

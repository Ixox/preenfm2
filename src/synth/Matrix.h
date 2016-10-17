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


#ifndef MATRIX_H_
#define MATRIX_H_

#define MATRIX_SIZE 12

#include "Common.h"


#ifdef CVIN
extern uint16_t ADCBuffer[];
#endif

class Matrix  {
	friend class Timbre;
public:
    Matrix();
    ~Matrix();

	void init(struct MatrixRowParams* matrixRows);

	void resetSources() {
        for (int k=0; k< MATRIX_SOURCE_MAX; k++) {
        	setSource((SourceEnum)k, 0);
        }
	}

    void resetAllDestination() {
        for (int k=0; k< DESTINATION_MAX; k++) {
            destinations[k] = 0;
        }
    }

    void resetDestination(int k) {
        destinations[k] = 0;
    }


    void computeAllDestintations() {


        float mul;

        float mul1 = destinations[MTX1_MUL];
        float mul2 = destinations[MTX2_MUL];
        float mul3 = destinations[MTX3_MUL];
        float mul4 = destinations[MTX4_MUL];
        //
        int k = 1;
        // No need to erase first row
        //destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;
        destinations[(int)rows[k++].destination] = 0;


        mul = rows[0].mul + mul1;
        if (likely(mul != 0.0f)) {
            destinations[(int)rows[0].destination] = sources[(int)rows[0].source] * mul;
        }
        mul = rows[1].mul + mul2;
        if (likely(mul != 0.0f)) {
            destinations[(int)rows[1].destination] += sources[(int)rows[1].source] * mul;
        }
        mul = rows[2].mul + mul3;
        if (likely(mul != 0.0f)) {
            destinations[(int)rows[2].destination] += sources[(int)rows[2].source] * mul;
        }
        mul = rows[3].mul + mul4;
        if (likely(mul != 0.0f)) {
            destinations[(int)rows[3].destination] += sources[(int)rows[3].source] * mul;
        }

        if (likely(rows[4].mul != 0.0f)) {
            destinations[(int)rows[4].destination] += sources[(int)rows[4].source] * rows[4].mul;
        }
        if (likely(rows[5].mul != 0.0f)) {
            destinations[(int)rows[5].destination] += sources[(int)rows[5].source] * rows[5].mul;
        }
        if (likely(rows[6].mul != 0.0f)) {
            destinations[(int)rows[6].destination] += sources[(int)rows[6].source] * rows[6].mul;
        }
        if (likely(rows[7].mul != 0.0f)) {
            destinations[(int)rows[7].destination] += sources[(int)rows[7].source] * rows[7].mul;
        }
        if (likely(rows[8].mul != 0.0f)) {
            destinations[(int)rows[8].destination] += sources[(int)rows[8].source] * rows[8].mul;
        }
        if (likely(rows[9].mul != 0.0f)) {
            destinations[(int)rows[9].destination] += sources[(int)rows[9].source] * rows[9].mul;
        }
        if (likely(rows[10].mul != 0.0f)) {
            destinations[(int)rows[10].destination] += sources[(int)rows[10].source] * rows[10].mul;
        }
        if (likely(rows[11].mul != 0.0f)) {
            destinations[(int)rows[11].destination] += sources[(int)rows[11].source] * rows[11].mul;
        }
    }

    void setSource(SourceEnum source, float value) __attribute__((always_inline)) {
        this->sources[source] = value;
    }

    float getDestination(DestinationEnum destination)   __attribute__((always_inline))  {
        return this->destinations[destination];
    }


private:
    float sources[MATRIX_SOURCE_MAX];
    float destinations[DESTINATION_MAX];
    struct MatrixRowParams* rows;
};

#endif /* MATRIX_H_ */

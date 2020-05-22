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



class Matrix  {
	friend class Timbre;
public:
    Matrix();
    ~Matrix();

	void init(struct MatrixRowParams* matrixRows);

	void resetSources() {
        for (int k = 0; k <  MATRIX_SOURCE_MAX; k++) {
        	setSource((SourceEnum)k, 0);
        }
	}

    void resetAllDestination() {
        for (int k = 0; k < DESTINATION_MAX; k++) {
            destinations[k] = 0;
        }
    }

    void resetDestination(int k) {
        destinations[k] = 0;
    }


    void computeAllDestinations() {
        // Store values before erasing destinations
        float mul1 = destinations[MTX1_MUL];
        float mul2 = destinations[MTX2_MUL];
        float mul3 = destinations[MTX3_MUL];
        float mul4 = destinations[MTX4_MUL];

        // Let's erase destinations first
        int k = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;
        destinations[(int)rows[k].dest1] = 0;
        destinations[(int)rows[k++].dest2] = 0;

        // For first row can be modified by destination
        if (likely(rows[0].source != MATRIX_SOURCE_NONE)) {
            float sourceTimesMul = sources[(int) rows[0].source] * (rows[0].mul + mul1);
            destinations[(int) rows[0].dest1] += sourceTimesMul;
            destinations[(int) rows[0].dest2] += sourceTimesMul;
        }
        if (likely(rows[1].source != MATRIX_SOURCE_NONE)) {
            float sourceTimesMul = sources[(int) rows[1].source] * (rows[1].mul + mul2);
            destinations[(int) rows[1].dest1] += sourceTimesMul;
            destinations[(int) rows[1].dest2] += sourceTimesMul;
        }
        if (likely(rows[2].source != MATRIX_SOURCE_NONE)) {
            float sourceTimesMul = sources[(int) rows[2].source] * (rows[2].mul + mul3);
            destinations[(int) rows[2].dest1] += sourceTimesMul;
            destinations[(int) rows[2].dest2] += sourceTimesMul;
        }
        if (likely(rows[3].source != MATRIX_SOURCE_NONE)) {
            float sourceTimesMul = sources[(int) rows[3].source] * (rows[3].mul + mul4);
            destinations[(int) rows[3].dest1] += sourceTimesMul;
            destinations[(int) rows[3].dest2] += sourceTimesMul;
        }

        // and compute other rows
        for (int r = 4; r < MATRIX_SIZE; r++) {
            if (likely(rows[r].source != MATRIX_SOURCE_NONE && rows[r].mul != 0.0f)) {
                float sourceTimesMul = sources[(int) rows[r].source] * rows[r].mul;
                destinations[(int) rows[r].dest1] += sourceTimesMul;
                destinations[(int) rows[r].dest2] += sourceTimesMul;
            }
        }
    }

    void setSource(SourceEnum source, float value) __attribute__((always_inline)) {
        this->sources[source] = value;
    }

    float getSource(SourceEnum source) __attribute__((always_inline)) {
        return this->sources[source];
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

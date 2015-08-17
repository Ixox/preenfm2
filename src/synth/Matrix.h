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
        for (int k=0; k< MATRIX_SOURCE_MAX; k++) {
        	setSource((SourceEnum)k, 0);
        }
	}

    void resetAllDestination() {
        for (int k=0; k< DESTINATION_MAX; k++) {
            currentDestinations[k] = 0;
            futurDestinations[k] = 0;
        }
    }

    void resetDestination(int k) {
    	currentDestinations[k] = 0;
		futurDestinations[k] = 0;
    }


    void resetUsedFuturDestination() {
        int k=0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
        futurDestinations[(int)rows[k++].destination] = 0;
    }

    void computeAllFutureDestintationAndSwitch() {
        float mul;

        resetUsedFuturDestination();
		mul = rows[0].mul + currentDestinations[MTX1_MUL];
		futurDestinations[(int)rows[0].destination] += sources[(int)rows[0].source] * mul;
		mul = rows[1].mul + currentDestinations[MTX2_MUL];
        futurDestinations[(int)rows[1].destination] += sources[(int)rows[1].source] * mul;
        mul = rows[2].mul + currentDestinations[MTX3_MUL];
        futurDestinations[(int)rows[2].destination] += sources[(int)rows[2].source] * mul;
        mul = rows[3].mul + currentDestinations[MTX4_MUL];
        futurDestinations[(int)rows[3].destination] += sources[(int)rows[3].source] * mul;
        futurDestinations[(int)rows[4].destination] += sources[(int)rows[4].source] * rows[4].mul;
        futurDestinations[(int)rows[5].destination] += sources[(int)rows[5].source] * rows[5].mul;
        futurDestinations[(int)rows[6].destination] += sources[(int)rows[6].source] * rows[6].mul;
        futurDestinations[(int)rows[7].destination] += sources[(int)rows[7].source] * rows[7].mul;
        futurDestinations[(int)rows[8].destination] += sources[(int)rows[8].source] * rows[8].mul;
        futurDestinations[(int)rows[9].destination] += sources[(int)rows[9].source] * rows[9].mul;
        futurDestinations[(int)rows[10].destination] += sources[(int)rows[10].source] * rows[10].mul;
        futurDestinations[(int)rows[11].destination] += sources[(int)rows[11].source] * rows[11].mul;

        useNewValues();
    }

    void setSource(SourceEnum source, float value) __attribute__((always_inline)) {
        this->sources[source] = value;
    }

    float getDestination(DestinationEnum destination)   __attribute__((always_inline))  {
        return this->currentDestinations[destination];
    }

    void useNewValues() {
        if (currentDestinations == destinations1) {
            currentDestinations = destinations2;
            futurDestinations = destinations1;
        } else {
            currentDestinations = destinations1;
            futurDestinations = destinations2;
        }
    }

private:
    float sources[MATRIX_SOURCE_MAX];
    float destinations1[DESTINATION_MAX];
    float destinations2[DESTINATION_MAX];
    float *currentDestinations;
    float *futurDestinations;
    struct MatrixRowParams* rows;
};

#endif /* MATRIX_H_ */

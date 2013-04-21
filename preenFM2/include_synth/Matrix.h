/*
 * Copyright 2011 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier.hosxe@gmail.com)
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
        for (int k=0; k< MATRIX_SIZE; ) {
        	futurDestinations[(int)rows[k++].destination] = 0;
            futurDestinations[(int)rows[k++].destination] = 0;
            futurDestinations[(int)rows[k++].destination] = 0;
            futurDestinations[(int)rows[k++].destination] = 0;
        }
    }

    void computeAllFutureDestintationAndSwitch() {
        float mul;
    	resetUsedFuturDestination();
        for (int k=0; k< MATRIX_SIZE;) {
			mul = rows[k].mul + currentDestinations[MTX1_MUL + k];
			futurDestinations[(int)rows[k].destination] += sources[(int)rows[k].source] * mul;
			k++;

            mul = rows[k].mul + currentDestinations[MTX1_MUL + k];
            futurDestinations[(int)rows[k].destination] += sources[(int)rows[k].source] * mul;
            k++;

            mul = rows[k].mul + currentDestinations[MTX1_MUL + k];
            futurDestinations[(int)rows[k].destination] += sources[(int)rows[k].source] * mul;
            k++;

            mul = rows[k].mul + currentDestinations[MTX1_MUL + k];
            futurDestinations[(int)rows[k].destination] += sources[(int)rows[k].source] * mul;
            k++;
        }
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

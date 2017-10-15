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

struct AlgoInformation  algoInformation[] = {
        { 3, 3, 1}, // ALGO1
        { 3, 2, 2}, // ALGO2
        { 4, 4, 1}, // ALGO3
        { 4, 4, 2}, // ALGO4
        { 4, 4, 1}, // ALGO5
        { 4, 3, 3}, // ALGO6
        { 6, 4, 3},  // ALGO7
        { 6, 4, 2},  // ALGO8
        { 6, 4, 2},  // ALGO9
        { 6, 4, 2},   // ALG10
        { 6, 4, 2},   // ALG11
        { 6, 3, 3},   // ALG12
        { 6, 4, 2},   // ALG13
        { 6, 4, 2},   // ALG14
        { 6, 4, 2},   // ALG15
        { 6, 4, 2},   // ALG16
        { 6, 5, 1},   // ALG17
        { 6, 5, 1},   // ALG18
        { 6, 4, 3},   // ALG19
        { 6, 4, 3},   // ALG20
        { 6, 4, 4},   // ALG21
        { 6, 4, 4},   // ALG22
        { 6, 3, 5},   // ALG23
        { 6, 3, 3},   // ALG24
        { 6, 2, 4},   // ALG25
        { 6, 2, 4},   // ALG26
        { 6, 0, 6},   // ALG27
        { 6, 1, 5},   // ALG28
};

int algoOpInformation[][NUMBER_OF_OPERATORS] = {
        {1,2,2,0,0,0}, // ALGO1
        {1,1,2,0,0,0}, // ALGO2
        {1,2,2,2,0,0}, // ALGO3
        {1,1,2,2,0,0}, // ALGO4
        {1,2,2,2,0,0}, // ALGO5
        {1,1,1,2,0,0}, // ALGO6
        {1,2,1,2,1,2}, // ALGO7
        {1,2,2,2,1,2}, // ALGO8
        {1,2,2,1,2,2}, // ALGO9
        {1,2,1,2,2,2}, // ALGO10
        {1,2,2,1,2,2}, // ALGO11
        {1,2,1,2,1,2}, // ALGO12
        {1,2,1,2,2,2}, // ALGO13
        {1,2,2,1,2,2}, // ALGO14
        {1,2,1,2,2,2}, // ALGO15
        {1,2,1,2,2,2}, // ALGO16
        {1,2,2,2,2,2}, // ALGO17
        {1,2,2,2,2,2}, // ALGO18
        {1,2,2,1,1,2}, // ALGO19
        {1,1,2,1,2,2}, // ALGO20
        {1,1,2,1,1,2}, // ALGO21
        {1,2,1,1,1,2}, // ALGO22
        {1,1,1,1,1,2}, // ALGO23
        {1,2,1,2,2,1}, // ALGO24
        {1,1,1,2,1,2}, // ALGO25
        {1,1,1,2,2,1}, // ALGO26
        {1,1,1,1,1,1}, // ALGO27
        {1,1,1,1,1,2}, // ALGO28
};


int strcmp(const char *s1, const char *s2) {
	while (*s1==*s2)
	{
		if(*s1=='\0')
			return(0);
		s1++;
		s2++;
	}
	char rs1 = *s1;
	char rs2 = *s2;
	if (rs1 == '_') {
		rs1 = 0;
	}
	if (rs2 == '_') {
		rs2 = 0;
	}
	return(rs1-rs2);
}

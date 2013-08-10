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
        { 6, 4, 4},   // ALG10
        { 6, 4, 2},   // ALG11
        { 6, 4, 2},   // ALG12
        { 6, 5, 1},   // ALG13
        { 6, 0, 6},  // ALG14
        { 6, 4, 2},   // ALG15
        { 6, 4, 4},   // ALG16
        { 6, 4, 2},   // ALG17
        { 6, 4, 2},   // ALG18
        { 6, 4, 2},   // ALG19
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

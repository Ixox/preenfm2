/*
 * FileSystemUtils.cpp
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#include "FileSystemUtils.h"

FileSystemUtils::FileSystemUtils() {
	// TODO Auto-generated constructor stub

}

FileSystemUtils::~FileSystemUtils() {
	// TODO Auto-generated destructor stub
}



void FileSystemUtils::copy(char* dest, const char* source, int length) {
    for (int k=0; k<length; k++) {
        dest[k] = source[k];
    }
}

void FileSystemUtils::addNumber(char* name, int offset, int number) {
    int div = 100;
    while (div > 0) {
        int digit = number / div;
        name[offset++] = '0' + digit;
        number -= digit * div;
        div /= 10;
    }
    name[offset] = '\0';
}




int FileSystemUtils::strlen(const char *string) {
    int k;
    for (k=0; k<1000 && string[k] != 0; k++);
    return k;
}

int FileSystemUtils::copy_string(char *target, const char *source)
{
	int written = 0;
	while(*source)
	{
		*target = *source;
		written ++;
		source++;
		target++;
	}
	*target = '\0';
	return written;
}

int FileSystemUtils::printFloat(char *target, float f) {
	int integerPart = (int)f;
	f -= integerPart;
	int wptr = printInt(target, integerPart);
	target[wptr++] = '.';
	target[wptr++] = '0' + (int)(f * 10.0f);
	return wptr;
}

int FileSystemUtils::printInt(char *target, int i) {
	int wptr = 0;
	if (i < 10) {
		target[wptr++] = '0' + i;
	} else if (i < 100) {
		target[wptr++] = '0' + (i / 10);
		target[wptr++] = '0' + (i % 10);
	} else if (i < 1000) {
		target[wptr++] = '0' + (i / 100);
		target[wptr++] = '0' + ((i % 100) / 10);
		target[wptr++] = '0' + (i % 10);
	} else {
		target[wptr++] = '0' + (i / 1000);
		target[wptr++] = '0' + ((i % 1000) / 100);
		target[wptr++] = '0' + ((i % 100) / 10);
		target[wptr++] = '0' + (i % 10);
	}
	return wptr;
}

int FileSystemUtils::getPositionOfEqual(const char *line) {
	for (int k=0; k < 20; k++) {
		if (line[k] == '=') {
			return k;
		}
	}
	return -1;
}

void FileSystemUtils::getKey(const char * line, char* key) {
	int k;
	for (k=0; k < 20 && line[k] != ' ' && line[k] != '='; k++) {
		key[k] = line[k];
	}
	key[k] = 0;
}

int FileSystemUtils::toInt(const char *str) {
	int           result;

	result = 0;
	while ((*str >= '0') && (*str <= '9'))
	{
		result = (result * 10) + ((*str) - '0');
		str++;
	}
	return result;
}

float FileSystemUtils::toFloat(const char *str) {
	float           result;
    float           result2;
	float           puiss = 1.0f;

	result = 0.0f;
    result2 = 0.0f;
	while ((*str >= '0') && (*str <= '9'))
	{
		result = (result * 10.0f) + ((*str) - '0');
		str++;
	}
    if (*str == '.') {
        str++;
        while ((*str >= '0') && (*str <= '9'))
    	{
    		result2 = (result2 * 10.0f) + ((*str) - '0');
            puiss *= 10.0f;
    		str++;
    	}
        result += result2 / puiss;
    }

	return result;
}


void FileSystemUtils::getValue(char * line, char *value) {
	int kk;
	for (int kk=0; kk < 20 && (line[kk] == ' ' || line[kk] == '\t'); kk++);
	int k;
	for (k=kk; k < 20 && line[k] >= '0' && line[k] <= '9'; k++) {
		value[k-kk] = line[k];
	}
	value[k-kk] = 0;
}

void FileSystemUtils::getFloatValue(char * line, char *value) {
	int kk;
	for (int kk=0; kk < 20 && (line[kk] == ' ' || line[kk] == '\t'); kk++);
	int k;
	for (k=kk; k < 20 && ((line[k] >= '0' && line[k] <= '9') || line[k] == '.'); k++) {
		value[k-kk] = line[k];
	}
	value[k-kk] = 0;
}


void FileSystemUtils::getTextValue(char * line, char *value) {
	int kk;
	for (int kk=0; kk < 20 && (line[kk] == ' ' || line[kk] == '\t'); kk++);
	int k;
	for (k=kk; k < 20 && line[k] >= 43 && line[k] <= 126; k++) {
		value[k-kk] = line[k];
	}
	value[k-kk] = 0;
}


int FileSystemUtils::str_cmp(const char*s1, const char*s2) {
	int ret = 0;
	while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2)
		++s1, ++s2;

	if (ret < 0) {
		ret = -1;
	} else if (ret > 0) {
		ret = 1 ;
	}

	return ret;
}

int FileSystemUtils::getLine(char* file, char* line) {
	int k;
	for (k=0; k <128 && file[k] != '\n' && file[k] != '\r'; k++) {
		line[k] = file[k];
	}
	line[k] = 0;
	while (file[k]=='\n' || file[k]=='\r' || file[k]=='\t' || file[k]==' ') {
		k++;
	}
	// Let's continue...
	if (file[k] == 0) {
		return -1;
	}
	return k;
}

void  FileSystemUtils::copyFloat(float* source, float* dest, int n) {
	for (int k=0; k<n; k++) {
		dest[k] = source[k];
	}
}

int FileSystemUtils::getPositionOfSlash(const char *line) {
  for (int k=0; k < 20 && line[k] != 0; k++) {
		if (line[k] == '/') {
			return k;
		}
	}
	return -1;

}
int FileSystemUtils::getPositionOfPeriod(const char *line) {
  for (int k=0; k < 20 && line[k] != 0; k++) {
		if (line[k] == '.') {
			return k;
		}
	}
	return -1;

}

float FileSystemUtils::stof(const char* s, int &charRead) {
    float rez = 0, fact = 1;
    const char* sPointer = s;
    while (!isNumber(*s)) {
        s++;
    }
    if (*s == '-'){
        s++;
        fact = -1;
    };
    for (int point_seen = 0; isNumber(*s); s++) {
        if (*s == '.') {
            point_seen = 1;
            continue;
        };
        int d = *s - '0';
        if (d >= 0 && d <= 9) {
            if (point_seen) fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        };
    };
    charRead = (s - sPointer);
    return rez * fact;
};



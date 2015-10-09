/*
 * FileSystemUtils.h
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#ifndef FILESYSTEMUTILS_H_
#define FILESYSTEMUTILS_H_


class FileSystemUtils {
public:
	FileSystemUtils();
	virtual ~FileSystemUtils();

	void copy(char* dest, const char* source, int length);
	void addNumber(char* name, int offset, int number);
	int strlen(const char *string);
	int copy_string(char *target, const char *source);
	int printFloat(char *target, float f);
	int printInt(char *target, int i);
	int getPositionOfEqual(const char *line);
	void getKey(const char * line, char* key);
	int toInt(const char *str);
	float toFloat(const char *str);
	void getValue(char * line, char *value);
	void getFloatValue(char * line, char *value);
	void getTextValue(char * line, char *value);
	int str_cmp(const char*s1, const char*s2);
	int getLine(char* file, char* line);
	void  copyFloat(float* source, float* dest, int n);
	int getPositionOfSlash(const char *line);
	int getPositionOfPeriod(const char *line);
	float stof(const char* s, int &charRead);
	bool isNumber(char c) {
	    return (c >= '0' && c <= '9') || c == '.' || c == '-';
	}

	bool isSeparator(char c) {
	    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ;
	}


protected:
    char fullName[40];

};

#endif /* FILESYSTEMUTILS_H_ */

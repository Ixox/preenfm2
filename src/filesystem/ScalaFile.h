/*
 * ScalaFile.h
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#ifndef SCALAFILE_H_
#define SCALAFILE_H_

#include "PreenFMFileType.h"

class ScalaFile: public PreenFMFileType {
public:
	ScalaFile();
	virtual ~ScalaFile();
	void loadScalaScale(const struct ScalaScaleConfig* config);
	void applyScalaScale(const struct ScalaScaleConfig* config);
	void clearScalaScale();

protected:
	const char* getFolderName();
	bool isCorrectFile(char *name, int size);


private:
	float getScalaIntervale(const char* line);
	struct PFM2File scalaScaleFile[NUMBEROFSCALASCALEFILES];
    float interval[36];
    int numberOfDegrees;

};

#endif /* SCALAFILE_H_ */

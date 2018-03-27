/*
 * ConfigurationFile.h
 *
 *  Created on: 24 juil. 2015
 *      Author: xavier
 */

#ifndef CONFIGURATIONFILE_H_
#define CONFIGURATIONFILE_H_

#include "PreenFMFileType.h"
#include "ScalaFile.h"

class ConfigurationFile: public PreenFMFileType {
public:
	ConfigurationFile();
	virtual ~ConfigurationFile();


	void loadConfig(short* midiConfigBytes);
	void saveConfig(const short* midiConfigBytes);

	void loadScalaConfig(struct ScalaScaleConfig *scalaScaleConfig);
	void saveScalaConfig(struct ScalaScaleConfig *scalaScaleConfig);

	void setScalaFile(ScalaFile* scalaFile) { this->scalaFile = scalaFile; }

protected:
	ScalaFile* scalaFile;
	const char* getFolderName();
	bool isCorrectFile(char *name, int size) { return true; }

	void fillMidiConfig(short* midiConfigBytes, char* line);
	void fillScalaConfig(struct ScalaScaleConfig* scalaScaleConfig, char* line);
};

#endif /* CONFIGURATIONFILE_H_ */

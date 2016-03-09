#ifndef CPSETTINGS_H
#define CPSETTINGS_H

#include <QString>
#include <QSettings>
#include <QFileInfo>

class ControllerPlacementSettings
{
public:
	ControllerPlacementSettings();

	void saveSettings(QSettings* const settings);	///сохранение пользовательских настроек
	void loadSettings(QSettings* const settings);

	QString inFileName;
	QString outFileName;
	QString pathToGraphviz;
	QString pathToImgFolder;
	int algorithm;
	int Lmax;
	int CPerfomance;
	int SPerfomance;
	int CCost;
	int SCCost;
	int SCTF_a;
	int SCTF_b;
	int syncTime;
	bool FixedCP;
	bool FixedSP;
	bool FixedCC;
	bool FixedSCC;
	bool constST;
	bool UsingHops;
	bool useGraphviz;
	int algoTime;
	int maxTopoSize;
	int iterations;
};

class DefaultCPSettings
{
public:

	DefaultCPSettings();

	QString inFileName;
	QString outFileName;
	QString pathToGraphviz;
	QString pathToImgFolder;
	int algorithm;
	int Lmax;
	int CPerfomance;
	int SPerfomance;
	int CCost;
	int SCCost;
	int SCTF_a;
	int SCTF_b;
	int syncTime;
	bool FixedCP;
	bool FixedSP;
	bool FixedCC;
	bool FixedSCC;
	bool constST;
	bool UsingHops;
	bool useGraphviz;
	int algoTime;
	int maxTopoSize;
	int iterations;
};

#endif // CPSETTINGS_H

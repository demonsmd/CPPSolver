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
	double Lmax;
	int CPerfomance;
	int SPerfomance;
	int CCost;
	int SCCost;
	int SCTF_a;
	int SCTF_b;
	int syncTime;
	bool FixedCP;
	bool LmaxMultiplier;
	bool FixedSP;
	bool FixedCC;
	bool FixedSCC;
	bool HopsDepSCC;
	bool LatDepSCC;
	bool constST;
	bool useGraphviz;

	int algoTime;
	int maxTopoSize;
	int FixedConNum;
	int IncrementalConNum;
	int PercentageConNumFrom;
	int PercentageConNumTo;
	bool FixedCon;
	bool IncrementalCon;
	bool PercentageCon;

	int totalLmax;
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
	double Lmax;
	int CPerfomance;
	int SPerfomance;
	int CCost;
	int SCCost;
	int SCTF_a;
	int SCTF_b;
	int syncTime;
	bool FixedCP;
	bool LmaxMultiplier;
	bool FixedSP;
	bool FixedCC;
	bool FixedSCC;
	bool HopsDepSCC;
	bool LatDepSCC;
	bool constST;
	bool useGraphviz;

	int algoTime;
	int maxTopoSize;
	int FixedConNum;
	int IncrementalConNum;
	int PercentageConNumFrom;
	int PercentageConNumTo;
	bool FixedCon;
	bool IncrementalCon;
	bool PercentageCon;
};

#endif // CPSETTINGS_H

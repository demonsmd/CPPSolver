#include "CPSettings.h"

ControllerPlacementSettings::ControllerPlacementSettings()
{}

DefaultCPSettings::DefaultCPSettings()
{
	inFileName = QFileInfo("../").absoluteFilePath();
	outFileName = QFileInfo("../").absoluteFilePath();
	pathToGraphviz = QFileInfo("../").absoluteFilePath();
	pathToImgFolder = QFileInfo("../").absoluteFilePath();
	algorithm = 0;
	Lmax = 1000;
	CPerfomance = 100000;
	SPerfomance = 5000;
	CCost = 0;
	SCCost = 0;
	SCTF_a = 0;
	SCTF_b = 0;
	syncTime = 0;
	FixedCP = true;
	FixedSP = true;
	FixedCC = true;
	FixedSCC = true;
	UsingHops = true;
	constST = false;
	useGraphviz = true;
	algoTime = 0;
	maxTopoSize = 0;
	iterations = 0;
}

void ControllerPlacementSettings::loadSettings(QSettings* const settings)
{
	DefaultCPSettings def;
	inFileName = settings->value("inFileName", def.inFileName).toString();
	outFileName = settings->value("outFileName", def.outFileName).toString();
	pathToGraphviz = settings->value("pathToGraphviz", def.pathToGraphviz).toString();
	pathToImgFolder = settings->value("pathToImgFolder", def.pathToImgFolder).toString();
	algorithm = settings->value("algorithm", def.algorithm).toInt();
	Lmax = settings->value("Lmax", def.Lmax).toInt();
	CPerfomance = settings->value("CPerfomance", def.CPerfomance).toInt();
	SPerfomance = settings->value("SPerfomance", def.SPerfomance).toInt();
	CCost = settings->value("CCost", def.CCost).toInt();
	SCCost = settings->value("SCCost", def.SCCost).toInt();
	SCTF_a = settings->value("SCTF_a", def.SCTF_a).toInt();
	SCTF_b = settings->value("SCTF_b", def.SCTF_b).toInt();
	syncTime = settings->value("syncTime", def.syncTime).toInt();
	algoTime = settings->value("algoTime", def.algoTime).toInt();
	maxTopoSize = settings->value("maxTopoSize", def.maxTopoSize).toInt();
	iterations = settings->value("iterations", def.iterations).toInt();
	FixedCP = settings->value("FixedCP", def.FixedCP).toBool();
	FixedSP = settings->value("FixedSP", def.FixedSP).toBool();
	FixedCC = settings->value("FixedCC", def.FixedCC).toBool();
	FixedSCC = settings->value("FixedSCC", def.FixedSCC).toBool();
	UsingHops = settings->value("UsingHops", def.UsingHops).toBool();
	constST = settings->value("constST", def.constST).toBool();
	useGraphviz = settings->value("useGraphviz", def.useGraphviz).toBool();
}

void ControllerPlacementSettings::saveSettings(QSettings* const settings)
{
	settings->setValue("algorithm", algorithm);
	settings->setValue("inFileName", inFileName);
	settings->setValue("outFileName", outFileName);
	settings->setValue("pathToGraphviz", pathToGraphviz);
	settings->setValue("pathToImgFolder", pathToImgFolder);
	settings->setValue("Lmax", Lmax);
	settings->setValue("CPerfomance", CPerfomance);
	settings->setValue("SPerfomance", SPerfomance);
	settings->setValue("CCost", CCost);
	settings->setValue("SCCost", SCCost);
	settings->setValue("SCTF_a", SCTF_a);
	settings->setValue("SCTF_b", SCTF_b);
	settings->setValue("syncTime", syncTime);
	settings->setValue("algoTime", algoTime);
	settings->setValue("maxTopoSize", maxTopoSize);
	settings->setValue("iterations", iterations);
	settings->setValue("FixedCP", FixedCP);
	settings->setValue("FixedSP", FixedSP);
	settings->setValue("FixedCC", FixedCC);
	settings->setValue("FixedSCC", FixedSCC);
	settings->setValue("UsingHops", UsingHops);
	settings->setValue("constST", constST);
	settings->setValue("useGraphviz", useGraphviz);
}

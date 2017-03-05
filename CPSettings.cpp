#include "CPSettings.h"
#include "topoelements.h"

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
    AVGSPerfomance = 2000;
    CCost = 0;
    SCCost = 0;
    SCTF_a = 0;
    SCTF_b = 0;
    syncTime = 0;
    FixedCP = true;
    FixedSP = true;
    FixedAVGSP = true;
    FixedCC = true;
    FixedSCC = false;
    LmaxMultiplier = true;
    HopsDepSCC = false;
    LatDepSCC = true;
    constST = false;
    useGraphviz = true;

    algoTime = 180;
    maxTopoSize = 20;
    FixedConNum = 5;
    IncrementalConNum = 2;
    PercentageConNumFrom = 2;
    PercentageConNumTo = 10;
    IncrementalCon = true;
    FixedCon = false;
    PercentageCon = false;
    mutationPosibility = 20;
    selectionPosibility = 20;
    geneticIterations = 10000;
    geneticPopSize = 100;
    WTotalCost = 1;
    WAvgLat = 1;
    WDisballance = 1;
    WWCLatency = 1;
    WOverload = 1;
}

void ControllerPlacementSettings::loadSettings(QSettings* const settings)
{
    DefaultCPSettings def;
    inFileName = settings->value("inFileName", def.inFileName).toString();
    outFileName = settings->value("outFileName", def.outFileName).toString();
    pathToGraphviz = settings->value("pathToGraphviz", def.pathToGraphviz).toString();
    pathToImgFolder = settings->value("pathToImgFolder", def.pathToImgFolder).toString();
    algorithm = settings->value("algorithm", def.algorithm).toInt();
    useGraphviz = settings->value("useGraphviz", def.useGraphviz).toBool();

    Lmax = settings->value("Lmax", def.Lmax).toInt();
    CPerfomance = settings->value("CPerfomance", def.CPerfomance).toInt();
    SPerfomance = settings->value("SPerfomance", def.SPerfomance).toInt();
    AVGSPerfomance = settings->value("AVGSPerfomance", def.AVGSPerfomance).toInt();
    CCost = settings->value("CCost", def.CCost).toInt();
    SCCost = settings->value("SCCost", def.SCCost).toInt();
    SCTF_a = settings->value("SCTF_a", def.SCTF_a).toInt();
    SCTF_b = settings->value("SCTF_b", def.SCTF_b).toInt();
    syncTime = settings->value("syncTime", def.syncTime).toInt();
    FixedCP = settings->value("FixedCP", def.FixedCP).toBool();
    LmaxMultiplier = settings->value("LmaxMultiplier",def.LmaxMultiplier).toBool();
    FixedSP = settings->value("FixedSP", def.FixedSP).toBool();
    FixedAVGSP = settings->value("FixedAVGSP", def.FixedAVGSP).toBool();
    FixedCC = settings->value("FixedCC", def.FixedCC).toBool();
    FixedSCC = settings->value("FixedSCC", def.FixedSCC).toBool();
    HopsDepSCC = settings->value("HopsDepSCC", def.HopsDepSCC).toBool();
    LatDepSCC = settings->value("LatDepSCC", def.LatDepSCC).toBool();
    constST = settings->value("constST", def.constST).toBool();

    FixedConNum = settings->value("FixedConNum", def.FixedConNum).toInt();
    IncrementalConNum = settings->value("IncrementalConNum", def.IncrementalConNum).toInt();
    PercentageConNumFrom = settings->value("PercentageConNumFrom", def.PercentageConNumFrom).toInt();
    PercentageConNumTo = settings->value("PercentageConNumTo", def.PercentageConNumTo).toInt();
    maxTopoSize = settings->value("maxTopoSize", def.maxTopoSize).toInt();
    algoTime = settings->value("algoTime", def.algoTime).toInt();
    FixedCon = settings->value("FixedCon", def.FixedCon).toBool();
    IncrementalCon = settings->value("IncrementalCon", def.IncrementalCon).toBool();
    PercentageCon = settings->value("PercentageCon", def.PercentageCon).toBool();
    selectionPosibility = settings->value("selectionPosibility", def.selectionPosibility).toInt();
    mutationPosibility = settings->value("mutationPosibility", def.mutationPosibility).toInt();
    geneticIterations = settings->value("geneticIterations", def.geneticIterations).toInt();
    geneticPopSize = settings->value("geneticPopSize", def.geneticPopSize).toInt();
    WTotalCost = settings->value("WTotalCost", def.WTotalCost).toInt();
    WAvgLat = settings->value("WAvgLat", def.WAvgLat).toInt();
    WDisballance = settings->value("WDisballance", def.WDisballance).toInt();
    WWCLatency = settings->value("WWCLatency", def.WWCLatency).toInt();
    WOverload = settings->value("WOverload", def.WOverload).toInt();
}

void ControllerPlacementSettings::saveSettings(QSettings* const settings)
{
    settings->setValue("algorithm", algorithm);
    settings->setValue("inFileName", inFileName);
    settings->setValue("outFileName", outFileName);
    settings->setValue("pathToGraphviz", pathToGraphviz);
    settings->setValue("pathToImgFolder", pathToImgFolder);
    settings->setValue("useGraphviz", useGraphviz);

    settings->setValue("Lmax", Lmax);
    settings->setValue("CPerfomance", CPerfomance);
    settings->setValue("SPerfomance", SPerfomance);
    settings->setValue("AVGSPerfomance", AVGSPerfomance);
    settings->setValue("CCost", CCost);
    settings->setValue("SCCost", SCCost);
    settings->setValue("SCTF_a", SCTF_a);
    settings->setValue("SCTF_b", SCTF_b);
    settings->setValue("syncTime", syncTime);
    settings->setValue("FixedCP", FixedCP);
    settings->setValue("LmaxMultiplier", LmaxMultiplier);
    settings->setValue("FixedSP", FixedSP);
    settings->setValue("FixedAVGSP", FixedAVGSP);
    settings->setValue("FixedCC", FixedCC);
    settings->setValue("FixedSCC", FixedSCC);
    settings->setValue("constST", constST);
    settings->setValue("HopsDepSCC", HopsDepSCC);

    settings->setValue("maxTopoSize", maxTopoSize);
    settings->setValue("algoTime", algoTime);
    settings->setValue("PercentageCon", PercentageCon);
    settings->setValue("IncrementalCon", IncrementalCon);
    settings->setValue("FixedCon", FixedCon);
    settings->setValue("PercentageConNumTo", PercentageConNumTo);
    settings->setValue("PercentageConNumFrom", PercentageConNumFrom);
    settings->setValue("IncrementalConNum", IncrementalConNum);
    settings->setValue("FixedConNum", FixedConNum);
    settings->setValue("selectionPosibility", selectionPosibility);
    settings->setValue("mutationPosibility", mutationPosibility);
    settings->setValue("geneticIterations", geneticIterations);
    settings->setValue("geneticPopSize", geneticPopSize);
    settings->setValue("WTotalCost", WTotalCost);
    settings->setValue("WAvgLat", WAvgLat);
    settings->setValue("WDisballance", WDisballance);
    settings->setValue("WWCLatency", WWCLatency);
    settings->setValue("WOverload", WOverload);
}

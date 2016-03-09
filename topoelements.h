#ifndef TOPOELEMENTS
#define TOPOELEMENTS

#include <QtMath>
#include <QVector>
#include <iostream>

using namespace std;

struct NODE
{
	NODE():id(-1), longitude(-1000), latitude(-1000), SwitchLoad(-1), ControllerCost(-1){}
	int id;
	float longitude;
	float latitude;
	int SwitchLoad;
	int ControllerLoad;
	int ControllerCost;
};

struct EDGE
{
	EDGE(): HopCost(-1), srcId(-1), dstId(-1){}
	int HopCost;
	int srcId;
	int dstId;
	int latency;
};

inline int geoLatency(float srcLat, float srcLong, float dstLat, float dstLong)
{
	return round(6371*qAcos(qSin(qDegreesToRadians(srcLat))*qSin(qDegreesToRadians(dstLat)) + qCos(qDegreesToRadians(dstLat))*qCos(qDegreesToRadians(srcLat))*qCos(qDegreesToRadians(srcLong)-qDegreesToRadians(dstLong))));
}

struct CPPSolution
{
	QVector<int> controllerPlacement;
	QVector<int> masterControllersDistribution;
//	QVector<int> reserveControllersDistribution;
	int controllerNumber;
	int totalCost;
	int avgLayency;
};

struct SolvedTopo
{
	QVector<NODE> nodes;
	QVector<EDGE> edges;
	CPPSolution solution;
};


#endif // TOPOELEMENTS

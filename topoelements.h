#ifndef TOPOELEMENTS
#define TOPOELEMENTS

#include <QtMath>
#include <QVector>
#include <iostream>
#include <exceptions.h>

#define INF 65536
#define MAX_ITERATIONS 4294967295
enum programStatus {RUNNING, NOTRUNNING};

using namespace std;

struct NODE
{
	int id;
	float longitude;
	float latitude;
	int SwitchLoad;
	int ControllerLoad;
	int ControllerCost;

	NODE():id(-1), longitude(-1000), latitude(-1000), SwitchLoad(-1), ControllerLoad(-1), ControllerCost(-1){}
};

struct EDGE
{
	int HopCost;
	int srcId;
	int dstId;
	int latency;

	EDGE(): HopCost(-1), srcId(-1), dstId(-1){}
};

inline int geoLatency(float srcLat, float srcLong, float dstLat, float dstLong)
{
	return round(6371*qAcos(qSin(qDegreesToRadians(srcLat))*qSin(qDegreesToRadians(dstLat)) + qCos(qDegreesToRadians(dstLat))*qCos(qDegreesToRadians(srcLat))*qCos(qDegreesToRadians(srcLong)-qDegreesToRadians(dstLong))));
}
inline unsigned long int CisNpoK(int n, int k)
{
	ensureExp(n>=k && n>0 && k>0, "CisNpoK error!");
	double a=1;
	for (int i=1;i<=k;i++)
		a=a*(n-k+i)/i;
	return round(a);
}

struct CPPSolution
{
	QVector<int> controllerPlacement;
	QVector<int> masterControllersDistribution;
//	QVector<int> reserveControllersDistribution;
	int totalCost;
	int avgLayency;

	//statistics
	int WCLatency;
	int WorkTime;
	unsigned long int totalNumberOfIterations;
	unsigned long int FoundIteration;
};

struct SolvedTopo
{
	QVector<NODE> nodes;
	QVector<EDGE> edges;
	CPPSolution solution;
};


#endif // TOPOELEMENTS

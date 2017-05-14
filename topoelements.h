#ifndef TOPOELEMENTS
#define TOPOELEMENTS

#include <exceptions.h>
#include <QtMath>
#include <QVector>
#include <iostream>

#define INF 10000000
#define LAUNCH_NUM 1
#define MAX_ITERATIONS 4294967295
#define EASY_OUTPUT false

#define BOOL int
#define TRUE 1
#define FALSE 0
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
    int AverageSwitchLoad;

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

inline float geoLatency(float srcLat, float srcLong, float dstLat, float dstLong)
{
    return round(6371*qAcos((float)(qSin(qDegreesToRadians(srcLat))*qSin(qDegreesToRadians(dstLat)) +
                            qCos(qDegreesToRadians(dstLat))*qCos(qDegreesToRadians(srcLat))*
                            qCos(qDegreesToRadians(srcLong)-qDegreesToRadians(dstLong)))));
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
    CPPSolution():
        totalCost(-1), avgLayency(-1), disballance(-1), WCLatency(-1), overload(-1) {}
    QVector<int> controllerPlacement;
    QVector<int> masterControllersDistribution;
    int totalCost;
    int avgLayency;
    int disballance;

    //statistics
    int WCLatency;
    int overload;
    int WorkTime;
    unsigned long int totalNumberOfIterations;
    unsigned long int FoundIteration;
    int mainMetric;
    int cnstraintMetric;
    BOOL isSolution;
};

struct SolvedTopo
{
    QVector<NODE> nodes;
    QVector<EDGE> edges;
    CPPSolution solution;
};


#endif // TOPOELEMENTS

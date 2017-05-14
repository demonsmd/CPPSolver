#ifndef CONTROLLERPLACEMENTALGOROTHM_H
#define CONTROLLERPLACEMENTALGOROTHM_H

#include "exceptions.h"
#include "topoelements.h"
#include "network.h"
#include "CPSettings.h"
#include <QSet>
#include <QTimer>

class ControllerPlacementAlgorothm: public QObject
{
    Q_OBJECT
public:
    ControllerPlacementAlgorothm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus);

    virtual CPPSolution solveCPP() = 0;
    virtual ~ControllerPlacementAlgorothm(){

    }

protected:
    const NetworkWithAlgorithms* network;
    const ControllerPlacementSettings* settings;
    CPPSolution bestSolution;	///лучшее найденное размещение
    CPPSolution curSolution;	///текущее размещение
    int nodesNumber;
    bool toLongToWait;
    const programStatus *pStatus;
    int minConNum;
    int maxConNum;

    //statistics
    unsigned long int totalNumberOfIterations;
    unsigned long int iteration;

    virtual bool existDistributionForCur(int failController, int failSwitch);
    bool checkSolutionCorrectness(CPPSolution& sol);
    bool curConstraintsVerification(int failSwitch, int failController, const QVector<QVector<int> >& newShortestMatrix, int syncTime=0);
    virtual void checkIfCurIsBest();
    int computeSynTimeForCur(int failController, const QVector<QVector<int> >* newShortestMatrix);
    int computeWCLTime(int failController, int failSwitch, const QVector<QVector<int> >* newShortestMatrix, int syncTime);
    void computeAVGTimeForCur();
    void computeCostAndLatForCur();

protected slots:
    virtual void timeOut();

signals:
    void curTopoProcess(int done, int from, int conNumber);
};
//====================================

class EnumerationAlgorithm: public ControllerPlacementAlgorothm
{
    Q_OBJECT
public:
    EnumerationAlgorithm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
        :ControllerPlacementAlgorothm(net, set, pStatus), analyseTime(30)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    }

    CPPSolution solveCPP();

private slots:
    virtual void timeOut();

private:
    bool nextPlacement();
    QTimer timer;
    int analyseTime;
};


//=================== GREEDY =============================

class GreedyAlgorithm: public ControllerPlacementAlgorothm
{
    Q_OBJECT
public:
    GreedyAlgorithm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
        :ControllerPlacementAlgorothm(net, set, pStatus)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(timeOut()));
    }

    virtual CPPSolution solveCPP();
    virtual ~GreedyAlgorithm(){}

protected:
    QTimer timer;
    QSet<QSet<int> >seenPlacements;
    int maxLmax;

    //statistics
    int curPlacementNumber;


    virtual void checkChildSolution(CPPSolution* parentSol);	///recursively calls itself if curent solution is better than parent
    virtual bool parentComparation(CPPSolution* parentSol);	///compare cur solution with parent. return true if cur is not worse than parent
    virtual void solveNeighbors();
    void placeInTopoCenter(int conNum);
    void initialDistribution(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);
    void controllerLoadCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);
    void connectionLatencyCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);
};


//=================== GENETIC =============================

class GeneticAlgorithm: public ControllerPlacementAlgorothm
{
    Q_OBJECT
public:
    GeneticAlgorithm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
        :ControllerPlacementAlgorothm(net, set, pStatus)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(timeOut()));
        computeWeights();
    }

    virtual CPPSolution solveCPP();
    virtual ~GeneticAlgorithm(){

    }

protected:
    CPPSolution generateRandomPlacement(int conNum);
    void makeParetoPopulation();    //из curPopulation удаляет все элементы не с границы парето
    void makeCrossing();
    void makeMutation();
    void reduceToPopSize();
    void computeCurPopulation();
    void computeWeights();
    bool existDistributionForCur(int failController, int failSwitch);
    bool findCorrectDistributionForCur(int failController, int failSwitch, const QVector<QVector<int> >& newShortestMatrix);
    QVector<CPPSolution> curPopulation;
    QVector<CPPSolution> bestPopulation;

    float W_totalCost;
    float W_avgLayency;
    float W_disballance;
    float W_WCLatency;
    float W_overload;

    QTimer timer;
    QSet<QSet<int> >seenPlacements;
    int maxLmax;
    int curPopNum;

    //statistics
    int curPlacementNumber;
};


//=================== G & G =============================
class GreedyAndGenetic: virtual public GreedyAlgorithm
{
    Q_OBJECT
public:
    GreedyAndGenetic(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
        :GreedyAlgorithm(net, set, pStatus)
    {
        computeWeights();
        bestSolution.mainMetric = bestSolution.cnstraintMetric = INF;
    }

    virtual CPPSolution solveCPP() override;

private:
    virtual void checkChildSolution(CPPSolution* parentSol);
    virtual void computeWeights();
    void findDistAndCompute();
    int findCorrectDistributionForCurWithMetric(int failController, int failSwitch, const QVector<QVector<int> >& newShortestMatrix);
    void findMainMetricForCur();
    virtual bool parentComparation(CPPSolution* parentSol);
    virtual void solveNeighbors() override;

    float W_totalCost;
    float W_avgLayency;
    float W_disballance;
    float W_WCLatency;
    float W_overload;
};

#endif // CONTROLLERPLACEMENTALGOROTHM_H

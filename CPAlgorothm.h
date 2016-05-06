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
	virtual ~ControllerPlacementAlgorothm(){}

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


	bool curConstraintsVerification(int failSwitch, int failController, const QVector<QVector<int> >& newShortestMatrix, int syncTime=0);
	virtual void checkIfCurIsBest();
	int computeSynTimeForCur(int failController, const QVector<QVector<int> >* newShortestMatrix);
	int computeWCLTime(int failController, int failSwitch, const QVector<QVector<int> >* newShortestMatrix, int syncTime);
	void computeCostAndLatForCur();

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

private:
	bool existDistributionForCur(int failController, int failSwitch);
	bool nextPlacement();
	QTimer timer;
	int analyseTime;

private slots:
	void timeOut();
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

	CPPSolution solveCPP();

private:
	QTimer timer;
	QSet<QSet<int> >seenPlacements;
	int maxLmax;

	//statistics
	int curPlacementNumber;


	void checkChildSolution(CPPSolution* parentSol);	///recursively calls itself if curent solution is better than parent
	bool parentComparation(CPPSolution* parentSol);	///compare cur solution with parent. return true if cur is not worse than parent
	void solveNeighbors();
	void placeInTopoCenter(int conNum);
	void initialDistribution(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);
	void controllerLoadCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);
	void connectionLatencyCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix);

private slots:
	void timeOut();
};

#endif // CONTROLLERPLACEMENTALGOROTHM_H

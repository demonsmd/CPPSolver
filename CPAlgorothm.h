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
	ControllerPlacementAlgorothm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
		:network(net), settings(set), nodesNumber(net->getTopoSize()), toLongToWait(false), pStatus(pStatus){}

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
	bool curConstraintsVerification(int failSwitch, int failController, const QVector<QVector<int> >& newShortestMatrix, int syncTime= -1);
	virtual bool existDistributionForCur(int failController, int failSwitch);
	virtual void checkIfCurIsBest();
	int computeSynTimeForCur(int failController, const QVector<QVector<int> >* newShortestMatrix);

signals:
	void curTopoProcess(int done, int from, int conNumber);
};

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
	unsigned long int totalNumberOfIterations;
	unsigned long int iteration;
	bool nextPlacement();
	QTimer timer;
	int analyseTime;

private slots:
	void timeOut();
};

//class GreedyAlgorithm: public ControllerPlacementAlgorothm
//{
//	Q_OBJECT
//public:
//	GreedyAlgorithm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
//		:ControllerPlacementAlgorothm(net, set, pStatus), analyseTime(30)
//	{
//		connect(&timer, SIGNAL(timeout()), this, SLOT(timeOut()));
//	}

//	CPPSolution solveCPP();

//private:
//	unsigned long int totalNumberOfIterations;
//	unsigned long int iteration;
//	bool nextPlacement();
//	QTimer timer;
//	int analyseTime;

//private slots:
//	void timeOut();
//};

#endif // CONTROLLERPLACEMENTALGOROTHM_H

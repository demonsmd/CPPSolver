#ifndef CONTROLLERPLACEMENTALGOROTHM_H
#define CONTROLLERPLACEMENTALGOROTHM_H

#include "exceptions.h"
#include "topoelements.h"
#include <QSet>

class ControllerPlacementAlgorothm
{
public:
	ControllerPlacementAlgorothm(const QVector<QVector<int> >* conMatr,
				     const QVector<QVector<int> >* shPMatr,
				     const QVector<NODE>* nodes,
				     const QVector<EDGE>* edges)
		:connectivityMatrix(conMatr),
		  shortestPathMatrix(shPMatr),
		  nodes(nodes),
		  edges(edges),
		  nodesNumber(nodes->size()){}

	virtual CPPSolution solveCPP() = 0;
	virtual ~ControllerPlacementAlgorothm(){}

protected:
	const QVector<QVector<int> >* connectivityMatrix;
	const QVector<QVector<int> >* shortestPathMatrix;
	const QVector<NODE>* nodes;
	const QVector<EDGE>* edges;
	CPPSolution bestSolution;	///лучшее найденное размещение
	CPPSolution curSolution;	///текущее размещение
	int nodesNumber;

	virtual bool curConstraintsVerification();
	virtual bool existDistributionForCur()=0;
	virtual void checkIfCurIsBest();
	virtual bool nextDistribution()=0;
};

class EnumerationAlgorithm: public ControllerPlacementAlgorothm
{
public:
	EnumerationAlgorithm(const QVector<QVector<int> >* conMatr,
			     const QVector<QVector<int> >* shPMatr,
			     const QVector<NODE>* nodes,
			     const QVector<EDGE>* edges)
		:ControllerPlacementAlgorothm(conMatr, shPMatr, nodes, edges){}

	CPPSolution solveCPP();

private:
	bool nextPlacement();
	bool existDistributionForCur();
	bool nextDistribution();
};

#endif // CONTROLLERPLACEMENTALGOROTHM_H

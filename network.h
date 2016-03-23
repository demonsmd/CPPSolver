#include "exceptions.h"
#include "topoelements.h"
#include "CPSettings.h"
#include <QVector>
#include <QSet>

#ifndef NETWORK_H
#define NETWORK_H

class Network
{
public:
	Network(const QVector<NODE>* nodes, const QVector<EDGE>* edges);
	const QVector<NODE>* getNodes() const {return nodes;}
	const QVector<EDGE>* getEdges() const {return edges;}
	const QVector< QVector<int> >* getConnectivityMatrix() const {return &connectivityMatrix;}
	const QVector< QVector<int> >* getShortestMatrix() const {return &shortestMatrix;}
	const QVector<QVector< QVector<int> > >* getPathMatrix() const {return &pathMatrix;}

	void makeConMatr(QVector< QVector<int> > &matr, const QVector<NODE>* nodes, const QVector<EDGE>* edges, int nodesNumber);
	void makeShortestMatr(QVector< QVector<int> > &matr, const QVector< QVector<int> > &connectivityMatrix, int nodesNumber) const;
	int getTopoSize() const {return nodesNumber;}

protected:
	const QVector<NODE>* nodes;
	const QVector<EDGE>* edges;
	int nodesNumber;

	void makeShortestPathMatr();

	QVector< QVector<int> > connectivityMatrix;	///матрица смежности
	QVector< QVector<int> > shortestMatrix;		///матрица кратчайших расстояний
	QVector<QVector<QVector<int> > > pathMatrix;	///матрица кратчайших путей
};

class NetworkWithAlgorithms: public Network
{
public:
	NetworkWithAlgorithms(const QVector<NODE>* nodes, const QVector<EDGE>* edges, \
			      const ControllerPlacementSettings* set, int FixedConnectionCost);
private:
	const ControllerPlacementSettings* settings;
	QVector<QVector<int> > connectionCostMatrix;

public:
	const QVector<QVector<int> >* getConCostMatr() const {return &connectionCostMatrix;}
	bool CheckBiconnectivity();
	void biConSearchDFS(int curNode, int parent, int &time, QVector <int> &seen, QVector <int> &low);
};

#endif // NETWORK_H

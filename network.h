#include "exceptions.h"
#include "topoelements.h"
#include <QVector>
#include <QSet>

#ifndef NETWORK_H
#define NETWORK_H

class Network
{
public:
	Network(const QVector<NODE>* nodes, const QVector<EDGE>* edges);
	const QVector<NODE>* getNodes(){return nodes;}
	const QVector<EDGE>* getEdges(){return edges;}
	const QVector< QVector<int> >* getConnectivityMatrix(){return &connectivityMatrix;}
	const QVector< QVector<int> >* getShortestPathMatrix(){return &shortestPathMatrix;}

	void makeConMatr(QVector< QVector<int> > &matr, const QVector<NODE>* nodes, const QVector<EDGE>* edges, int nodesNumber);
	void makeShortestPathMatr(QVector< QVector<int> > &matr, QVector< QVector<int> > connectivityMatrix, int nodesNumber);
	int getTopoSize(){return nodesNumber;}

protected:
	const QVector<NODE>* nodes;
	const QVector<EDGE>* edges;
	int nodesNumber;

	QVector< QVector<int> > connectivityMatrix;	///матрица смежности
	QVector< QVector<int> > shortestPathMatrix;	///матрица кратчайших расстояний
};

class NetworkWithAlgorithms: public Network
{
public:
	NetworkWithAlgorithms(const QVector<NODE>* nodes, const QVector<EDGE>* edges):Network(nodes, edges){}

	bool CheckBiconnectivity();
	void biConSearchDFS(int curNode, int parent, int &time, QVector <int> &seen, QVector <int> &low);
};

#endif // NETWORK_H

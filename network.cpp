#include "network.h"
#include "CPService.h"

Network::Network(const QVector<NODE>* eNodes, const QVector<EDGE>* eEdges)
{
	ensureExp(eNodes&&eEdges,"попытка инициализации пустой сети");
	nodes=eNodes;
	edges=eEdges;
	nodesNumber=eNodes->size();
	makeConMatr(connectivityMatrix, nodes, edges, nodesNumber);
	makeShortestPathMatr(shortestPathMatrix, connectivityMatrix, nodesNumber);
}

void Network::makeConMatr(QVector< QVector<int> > &connectivityMatrix, const QVector<NODE>* nodes, const QVector<EDGE>* edges, int nodesNumber)
{
	connectivityMatrix.resize(nodesNumber);
	for (int i=0; i<nodesNumber;i++)
	{
		connectivityMatrix[i].resize(nodesNumber);
		for (int j=0;j<nodesNumber;j++)
			if (i==j)
				connectivityMatrix[i][i]=0;
			else
				connectivityMatrix[i][j]=-1;
	}

	for (int i=0; i<edges->size();i++)
	{
		NODE src = (*nodes)[(*edges)[i].srcId];
		NODE dst = (*nodes)[(*edges)[i].dstId];
		connectivityMatrix[src.id][dst.id] =
			connectivityMatrix[dst.id][src.id] =
			(*edges)[i].latency;
	}
}

void Network::makeShortestPathMatr(QVector< QVector<int> > &shortestPathMatrix, QVector< QVector<int> > connectivityMatrix, int nodesNumber)
{
	shortestPathMatrix = connectivityMatrix;
	for (int k=0;k<nodesNumber;k++)
	    for (int i=0;i<nodesNumber;i++)
		for (int j=0;j<nodesNumber;j++)
		    shortestPathMatrix[i][j]=std::min(shortestPathMatrix[i][j],
							shortestPathMatrix[i][k]+shortestPathMatrix[k][j]);
}

void NetworkWithAlgorithms::biConSearchDFS(int curNode, int parent, int &time, QVector <int> &seen, QVector <int> &low)
{
	seen[curNode] = low[curNode] = ++time;
	bool rootHasOneChild=false;

	for(int child=0; child<nodesNumber; child++)
	{
		if(child != parent && child != curNode && (connectivityMatrix[curNode][child] >= 0))
		{
			if( seen[child] == 0) //не были в этой вершине
			{
				ensureExp(curNode!=0 || !rootHasOneChild, "is not biconnective");
				biConSearchDFS(child, curNode, time, seen, low);
				ensureExp((curNode!=0 && low[child]<seen[curNode])
					|| (curNode==0 && low[child]<=seen[curNode]), "is not biconnective");
				low[curNode] = qMin(low[curNode], low[child]);
			}
			else
				low[curNode] = qMin(low[curNode], seen[child]);
			if (curNode==0)
				rootHasOneChild=true;
		}
	}
}

bool NetworkWithAlgorithms::CheckBiconnectivity()
{
	ensureExp(nodesNumber>0, "Попытка проверки двусвязности пустой сети");
	int time = 0;
	QVector <int> seen(nodesNumber, 0);
	QVector <int> low(nodesNumber, 0);
	try
	{
		biConSearchDFS(0, -1, time, seen, low);
		return true;
	}
	catch(Exceptions x)
	{
		return false;
	}
}

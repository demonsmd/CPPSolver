#include "network.h"
#include "CPService.h"

Network::Network(const QVector<NODE>* eNodes, const QVector<EDGE>* eEdges)
{
    ensureExp(eNodes&&eEdges,"попытка инициализации пустой сети");
    nodes=eNodes;
    edges=eEdges;
    nodesNumber=eNodes->size();
    makeConMatr(connectivityMatrix, nodes, edges, nodesNumber);
    makeShortestPathMatr();
}

//void Network::makeHopsPathMatr(QVector< QVector<int> > &hopsMatrix, const QVector<NODE>* nodes, const QVector<EDGE>* edges, int nodesNumber)
//{
//	QVector<QVector<int> > hopsConMatr;
//	hopsConMatr.resize(nodesNumber);
//	for (int i=0; i<nodesNumber;i++)
//	{
//		hopsConMatr[i].resize(nodesNumber);
//		for (int j=0;j<nodesNumber;j++)
//			if (i==j)
//				hopsConMatr[i][i]=0;
//			else
//				hopsConMatr[i][j]=INF;
//	}

//	for (int i=0; i<edges->size();i++)
//	{
//		NODE src = (*nodes)[(*edges)[i].srcId];
//		NODE dst = (*nodes)[(*edges)[i].dstId];
//		hopsConMatr[src.id][dst.id] = hopsConMatr[dst.id][src.id] =1;
//	}
//	makeShortestPathMatr(hopsMatrix, hopsConMatr, nodesNumber);
//}

void Network::makeConMatr(QVector< QVector<int> > &connectivityMatrix, const QVector<NODE>* nodes, const QVector<EDGE>* edges, int nodesNumber)
{
    connectivityMatrix.resize(nodesNumber);
    pathMatrix.resize(nodesNumber);
    for (int i=0; i<nodesNumber;i++)
    {
        connectivityMatrix[i].resize(nodesNumber);
        pathMatrix[i].resize(nodesNumber);
        for (int j=0;j<nodesNumber;j++)
            if (i==j)
                connectivityMatrix[i][i]=0;
            else
                connectivityMatrix[i][j]=INF;
    }

    for (int i=0; i<edges->size();i++)
    {
        NODE src = (*nodes)[(*edges)[i].srcId];
        NODE dst = (*nodes)[(*edges)[i].dstId];
        connectivityMatrix[src.id][dst.id] =
            connectivityMatrix[dst.id][src.id] =
            (*edges)[i].latency;

        pathMatrix[src.id][dst.id].push_back(src.id);
        pathMatrix[src.id][dst.id].push_back(dst.id);
        pathMatrix[dst.id][src.id].push_back(dst.id);
        pathMatrix[dst.id][src.id].push_back(src.id);
    }
}

void Network::makeShortestPathMatr()
{
    shortestMatrix = connectivityMatrix;
    for (int k=0;k<nodesNumber;k++)
        for (int i=0;i<nodesNumber;i++)
        for (int j=0;j<nodesNumber;j++)
        {
            if (shortestMatrix[i][k]+shortestMatrix[k][j] < shortestMatrix[i][j])
            {
                shortestMatrix[i][j]= shortestMatrix[i][k]+shortestMatrix[k][j];

                pathMatrix[i][j].clear();
                for (int l=0;l<pathMatrix[i][k].size()-1;l++)
                    pathMatrix[i][j].push_back(pathMatrix[i][k][l]);
                for (int l=0;l<pathMatrix[k][j].size();l++)
                    pathMatrix[i][j].push_back(pathMatrix[k][j][l]);
            }
        }
}

void Network::makeShortestMatr(QVector< QVector<int> > &shortestMatrix, const QVector< QVector<int> > &connectivityMatrix, int nodesNumber) const
{
    shortestMatrix = connectivityMatrix;
    for (int k=0;k<nodesNumber;k++)
        for (int i=0;i<nodesNumber;i++)
            for (int j=0;j<nodesNumber;j++)
            {
                if (shortestMatrix[i][k]+shortestMatrix[k][j] < shortestMatrix[i][j])
                {
                    shortestMatrix[i][j]= shortestMatrix[i][k]+shortestMatrix[k][j];
                }
            }
}

void NetworkWithAlgorithms::biConSearchDFS(int curNode, int parent, int &time, QVector <int> &seen, QVector <int> &low)
{
    seen[curNode] = low[curNode] = ++time;
    bool rootHasOneChild=false;

    for(int child=0; child<nodesNumber; child++)
    {
        if(child != parent && child != curNode && (connectivityMatrix[curNode][child] != INF))
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

NetworkWithAlgorithms::NetworkWithAlgorithms(const QVector<NODE>* nodes, const QVector<EDGE>* edges, const ControllerPlacementSettings* set, int FixedConnectionCost)
    :Network(nodes, edges), settings(set)
{
    ensureExp(nodesNumber<=set->maxTopoSize, "Сеть слишком большая");

    //создание connectionCostMatrix
    connectionCostMatrix.resize(nodesNumber);
    for (int i=0;i<nodesNumber;i++)
        connectionCostMatrix[i].resize(nodesNumber);
    for (int i=0;i<nodesNumber;i++)
        for (int j=0;j<nodesNumber;j++)
        {
            if (i==j)
            {
                connectionCostMatrix[i][i]=0;
                continue;
            }
            int cost=0;
            int last = 0;
            if (settings->HopsDepSCC)
            {
                for (int k=1;k<pathMatrix[i][j].size();k++)
                {
                    for (int l=0;l<edges->size();l++)
                    {
                        if (((*edges)[l].dstId==pathMatrix[i][j][k]
                           &&(*edges)[l].srcId==pathMatrix[i][j][last])
                           ||
                            ((*edges)[l].srcId==pathMatrix[i][j][k]
                           &&(*edges)[l].dstId==pathMatrix[i][j][last]))
                        {
                            cost+=(*edges)[l].HopCost;
                            break;
                        }
                    }
                    last++;
                }
            }
            else if (settings->FixedSCC)
            {
                cost=FixedConnectionCost;
            }
            else
            {
                if (settings->SCCost==0)
                    cost=FixedConnectionCost;
                cost=shortestMatrix[i][j]/settings->SCCost;
            }
            connectionCostMatrix[i][j]=cost;
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

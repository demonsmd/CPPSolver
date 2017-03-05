#ifndef GRAPHMLREADER_H
#define GRAPHMLREADER_H

#include "exceptions.h"
#include "topoelements.h"
#include "CPSettings.h"
#include <iostream>
#include <QVector>
#include <QString>

class GRAPHMLReader
{
public:
    GRAPHMLReader(QString topo, const ControllerPlacementSettings* settings);

    const QVector<NODE>* getNodes() const {return nodes;}
    const QVector<EDGE>* getEdges() const {return edges;}
    int getFixedConnectionCost(){return FixedConnectionCost;}
    QString topoName;

private:
    int FixedConnectionCost;
    QVector<NODE>* nodes;
    QVector<EDGE>* edges;
    const ControllerPlacementSettings* CPSettings;
    void latencyCalculate();

    void parce(QString fileName);	///извлечение из файла списка вершин, рёбер, а также их атрибутов
    void validate();				///проверка корректности полученных данных
    void normalizeNumeration();	///нормальизация нумерации вершин (i-ая вершина имеет i-ый id)

};

#endif // GRAPHMLREADER_H

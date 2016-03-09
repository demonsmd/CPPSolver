#ifndef GRAPHMLREADER_H
#define GRAPHMLREADER_H

#include "exceptions.h"
#include "topoelements.h"
#include "CPSettings.h"
#include <QVector>
#include <iostream>

class GRAPHMLReader
{
public:
	GRAPHMLReader(QString topo, const ControllerPlacementSettings* settings);

	QVector<NODE>* getNodes(){return nodes;}
	QVector<EDGE>* getEdges(){return edges;}

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

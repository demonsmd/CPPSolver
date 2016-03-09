#ifndef GRAPHVIZ_H
#define GRAPHVIZ_H
#include "exceptions.h"
#include "topoelements.h"

#include <QProcess>
#include <QFileInfo>
#include <QStringList>

class GRAPHVIZ: public QObject
{
	Q_OBJECT
public:
	GRAPHVIZ(QString dot, QString img, QString format);
	void visualize(QString name, const SolvedTopo& solution);
private:
	QString pathToDot;
	QString pathToImg;
	QString format;
	void calculateScaleK(const SolvedTopo& solution, int& minX, int& minY, float& scale);
};

#endif // GRAPHVIZ_H

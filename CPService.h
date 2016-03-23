#ifndef CONTROLLERPLACEMENTSERVICE_H
#define CONTROLLERPLACEMENTSERVICE_H

#include "network.h"
#include "CPAlgorothm.h"
#include "exceptions.h"
#include "graphmlReader.h"
#include "CPSettings.h"
#include "graphviz.h"

#include <QDir>
#include <QCoreApplication>
#include <QUrl>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QObject>

using namespace std;

class ControllerPlacementService: public QObject
{
	Q_OBJECT
public:
	ControllerPlacementService();
	~ControllerPlacementService();

	void saveSettings(QSettings* const settings);	///сохранение пользовательских настроек
	ControllerPlacementSettings* loadSettings(QSettings* const settings);	///загрузка пользовательских настроек
	QStringList ALGORITHMS;

private:
	NetworkWithAlgorithms* network;				///внутреннее представление сети
	ControllerPlacementAlgorothm* algorithm;	///алгоритм, используемый для решения задачи размещения контроллеров
	ControllerPlacementSettings* CPSettings;	///настройки запуска алгоритма размещения
	DefaultCPSettings* defSettings;
	programStatus PStatus;				///текущее состояние программы
	GRAPHMLReader* xmlReader;			///парсер xml файла
	QStringList inFilesList;			///список входных файлов
	CPPSolution solution;
	QVector<SolvedTopo> solvedTopoList;		///список рёбер, который используется для построения графа
	GRAPHVIZ* visualizator;
	int curTopoNum;

private:
	void getInFiles();
	void saveNetworkSolution(int i);

public slots:
	void startButtonPressed();
	void stopButtonPressed();
	void networkNamePressed(QUrl url);
	void curTopoProcess(int done, int from, int conNumber);

signals:
	void toLog(const QString& text);
	void programFinnished();
	void processingTopo(int min, int max, int val, QString topoName);
};

#endif // CONTROLLERPLACEMENTSERVICE_H

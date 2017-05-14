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
#include <iostream>
#include <memory>

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
    std::shared_ptr<NetworkWithAlgorithms> network;			///внутреннее представление сети
    std::shared_ptr<ControllerPlacementAlgorothm> algorithm;	///алгоритм, используемый для решения задачи размещения контроллеров
    ControllerPlacementSettings* CPSettings;	///настройки запуска алгоритма размещения
    DefaultCPSettings* defSettings;
    programStatus PStatus;				///текущее состояние программы
    std::shared_ptr<GRAPHMLReader> xmlReader;			///парсер xml файла
    QStringList inFilesList;			///список входных файлов
    CPPSolution solution;
    QVector<SolvedTopo> solvedTopoList;
    GRAPHVIZ* visualizator;
    int curTopoNum;

private:
    void getInFiles();
    void saveNetworkSolution(int i);
    void computeLmax();				///нужно для определения действительной Lmax
    QString resOutFile;

public slots:
    void startButtonPressed();
    void stopButtonPressed();
    void networkNamePressed(QUrl url);
    void curTopoProcess(int done, int from, int conNumber);

signals:
    void toLog(const QString& text);
    void clearLog();
    void programFinnished();
    void processingTopo(int min, int max, int val, QString topoName);
};

#endif // CONTROLLERPLACEMENTSERVICE_H

#include "CPService.h"
#include <QTextStream>
#include <QTime>

ControllerPlacementService::ControllerPlacementService()
{
    CPSettings = new ControllerPlacementSettings();
    defSettings = new DefaultCPSettings();
    visualizator=NULL;
    PStatus = NOTRUNNING;
    ALGORITHMS = QStringList()<<"Переборный"<<"Жадный"<<"Генетический"<<"Жадный+Генетический";
}

ControllerPlacementService::~ControllerPlacementService()
{
    delete CPSettings;
    delete defSettings;
    if (visualizator) delete visualizator;
}

void ControllerPlacementService::saveSettings(QSettings* const settings)
{
    settings->beginGroup("CPSettings/");
    CPSettings->saveSettings(settings);
    settings->endGroup();
}

ControllerPlacementSettings* ControllerPlacementService::loadSettings(QSettings* const settings)
{
    settings->beginGroup("CPSettings/");
    CPSettings->loadSettings(settings);
    settings->endGroup();
    return CPSettings;
}

void ControllerPlacementService::saveNetworkSolution(int i)
{
    solvedTopoList[i].edges=(*network->getEdges());
    solvedTopoList[i].nodes=(*network->getNodes());
    solvedTopoList[i].solution.avgLayency=solution.avgLayency;
    solvedTopoList[i].solution.controllerPlacement=solution.controllerPlacement;
    solvedTopoList[i].solution.masterControllersDistribution=solution.masterControllersDistribution;
    solvedTopoList[i].solution.totalCost=solution.totalCost;

    QFile logFile(resOutFile);
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        if (!EASY_OUTPUT)
            toLog("<font color=\"red\">/nНевозможно открыть файл для записи/n</font>!");
        return;
    }
    QTextStream stream(&logFile);
    stream	<< inFilesList[i].right(inFilesList[i].size() - inFilesList[i].lastIndexOf("/") - 1)<<";"
        << network->getTopoSize()<<";"
        << network->getEdges()->size()<<";"
        << solution.controllerPlacement.size()<<";"
        << ";"
        << ALGORITHMS[CPSettings->algorithm]<<";"
        << CPSettings->totalLmax<<";";

    if (CPSettings->FixedSCC)
        stream<<"fixed ("<<CPSettings->SCCost<<");";
    else if (CPSettings->HopsDepSCC)
        stream<<"hopsDep ("<<CPSettings->SCCost<<");";
    else if (CPSettings->LatDepSCC)
        stream<<"LatDep ("<<CPSettings->SCCost<<");";

    if (CPSettings->constST)
        stream<<"const syn time ("<<CPSettings->syncTime<<");";
    else
        stream<<"Linear ("<<CPSettings->SCTF_a<<"*x+"<<CPSettings->SCTF_b<<");";
    stream<<solution.WCLatency<<";"<<solution.avgLayency<<";"<<solution.totalCost<<";";
    stream<<solution.WorkTime<<";"<<solution.FoundIteration<<";"<<solution.totalNumberOfIterations<<";";
    stream<<solution.mainMetric<<";"<<endl;


    for (int i=0;i<solution.controllerPlacement.size();i++)
    {
        stream	<<";;;"<< solution.controllerPlacement[i]<<";";
        bool first = true;
        for (int j=0;j<solution.masterControllersDistribution.size();j++)
        {
            if (solution.masterControllersDistribution[j]==i)
            {
                if (first)
                {
                    first=false;
                    stream	<<j;
                }
                else
                {
                    stream	<<"  "<<j;
                }
            }
        }
        stream<<";;;;"<<endl;
    }
    logFile.close();

}

//=====================================================================
//========================== SLOTS ====================================
//=====================================================================

void ControllerPlacementService::startButtonPressed()
{
    PStatus = RUNNING;
    try
    {
        if(CPSettings->useGraphviz)
        {
            if (visualizator){
                delete visualizator;
            }
            visualizator = new GRAPHVIZ(CPSettings->pathToGraphviz,
                            CPSettings->pathToImgFolder, "png");
        }
        for (int i=0; i<LAUNCH_NUM; i++){
            cout<<i<<endl;
            if (EASY_OUTPUT)
                toLog(QString::number(i));
            //clearLog();
            int doneTopos = 0;
            QString topoTag;

            if (LAUNCH_NUM>1)
                resOutFile = QString("%1_%2.csv").
                    arg(CPSettings->outFileName.left(CPSettings->outFileName.size()-4)).
                    arg(QString::number(i));
            else
                resOutFile =CPSettings->outFileName;

            QFile logFile(resOutFile);
            ensureExp(logFile.open(QIODevice::WriteOnly | QIODevice::Text), "Невозможно открыть лог файл");
            QTextStream stream(&logFile);
            stream<<"Topo name;Nodes number;Edges number;Controller placement;Switch distribution;Algorithm;Lmax;Cost metric;Latency metric;WCL;AVG_LAT;COST;Time(msec);Found_On_Iteration;Iterations;Metric"<<endl;
            logFile.close();

            getInFiles();
            if (!EASY_OUTPUT)
                toLog("Найдено входных файлов: " + QString::number(inFilesList.size()));
            solvedTopoList.resize(inFilesList.size());

            for (curTopoNum=0 ; curTopoNum<inFilesList.size(); curTopoNum++)
            {
                try
                {
                    if (CPSettings->useGraphviz)
                        topoTag = QString("<a href=\"graphviz:%1\">%2</a>").arg(QString::number(curTopoNum)).arg(inFilesList.at(curTopoNum));
                    else
                        topoTag = QString("<font color=\"blue\">%1</font>").arg(inFilesList.at(curTopoNum));

    //				emit processingTopo(0,inFilesList.size()-1, curTopoNum, QString("Обрабатывается топология %1").arg(inFilesList.at(curTopoNum)));
                    xmlReader = std::shared_ptr<GRAPHMLReader>(new GRAPHMLReader(inFilesList.at(curTopoNum), CPSettings));
                    network = std::shared_ptr<NetworkWithAlgorithms>(new NetworkWithAlgorithms(xmlReader.get()->getNodes(),xmlReader.get()->getEdges()
                            ,CPSettings ,xmlReader.get()->getFixedConnectionCost()));
                    ensureExp(network.get()->CheckBiconnectivity(), "Сеть не является двусвязной");
                    computeLmax();
                    if (CPSettings->algorithm==0)
                        algorithm = std::shared_ptr<EnumerationAlgorithm>(new EnumerationAlgorithm(network.get(), CPSettings, &PStatus));
                    else if (CPSettings->algorithm==1)
                        algorithm = std::shared_ptr<GreedyAlgorithm>(new GreedyAlgorithm(network.get(), CPSettings, &PStatus));
                    else if (CPSettings->algorithm==2)
                        algorithm = std::shared_ptr<GeneticAlgorithm>(new GeneticAlgorithm(network.get(), CPSettings, &PStatus));
                    else if (CPSettings->algorithm==3)
                        algorithm = std::shared_ptr<GreedyAndGenetic>(new GreedyAndGenetic(network.get(), CPSettings, &PStatus));
                    else
                        throw Exceptions("Неизвестный алгоритм");
                    connect(algorithm.get(), SIGNAL(curTopoProcess(int, int, int)),
                        this, SLOT(curTopoProcess(int, int, int)));

                    QTime timer;
                    timer.start();
                    solution = algorithm.get()->solveCPP();
                    solution.WorkTime = timer.elapsed();

                    ensureExp(solution.controllerPlacement.size()>0,"Не найдено решения для данных параметров");


                    saveNetworkSolution(curTopoNum);

                    doneTopos++;
                    if (!EASY_OUTPUT)
                        toLog(QString("В топологии %1[%2] была <font color=\"green\"><b><u>успешно</u></b></font> обработана!").arg(topoTag).arg(QString::number(network.get()->getTopoSize())));
                }
                catch (Exceptions ex)
                {
                    if (!EASY_OUTPUT)
                        toLog(QString("В топологии <font color=\"blue\">%1</font> была найдена <font color=\"red\"><b><u>ошибка</u></b>: %2</font>. Эта топология будет пропущена!").arg(inFilesList.at(curTopoNum)).arg(ex.getText()));
                }
            }
            emit processingTopo(0,0,0,QString("Готово! %1 из %2 файлов успешно обработаны").arg(QString::number(doneTopos)).arg(QString::number(inFilesList.size())));
        }
        PStatus = NOTRUNNING;
        programFinnished();
    }
    catch(StopProgram ex)
    {
        if (!EASY_OUTPUT)
            toLog(ex.getText());
        emit processingTopo(0,0,0,QString("Прервано пользователем."));
        programFinnished();
    }
    catch(Exceptions ex)
    {
        if (!EASY_OUTPUT)
            toLog(ex.getText());
        programFinnished();
    }
    catch(...)
    {
        toLog("lol cats!");
        programFinnished();
    }
}

void ControllerPlacementService::curTopoProcess(int done, int from, int conNumber)
{
    emit processingTopo(0,from, done, QString("Обрабатывается топология [%1/%2] %3: %4 контроллеров").arg(curTopoNum+1).arg(inFilesList.size()).arg(inFilesList.at(curTopoNum)).arg(conNumber));
}

void ControllerPlacementService::stopButtonPressed()
{
    PStatus = NOTRUNNING;
}

void ControllerPlacementService::networkNamePressed(QUrl url)
{
    QString surl = url.toString();
    if (surl.startsWith("graphviz:"))
    {
        try
        {
            ensureExp(CPSettings->useGraphviz, "визуализаор не активен");
            bool ok;
            int curTopoNum = surl.remove(0,9).toInt(&ok);
            ensureExp(ok,"невозможно определить номер топологии");

            QFileInfo finfo(inFilesList[curTopoNum]);
            visualizator->visualize(finfo.baseName(), solvedTopoList[curTopoNum]);
        }
        catch(Exceptions ex)
        {
            QMessageBox EMB(0);
            EMB.setWindowTitle("Ошибка визуализации сети");
            EMB.setText("При визуализиции графа произошла ошибка: " + ex.getText());
            EMB.exec();
        }
    }
}

void ControllerPlacementService::getInFiles()
{
    QFileInfo finfo(CPSettings->inFileName);
    if (finfo.isFile() && finfo.suffix()=="graphml")
    {
        inFilesList.clear();
        inFilesList<<finfo.absoluteFilePath();
    }
    else if (finfo.isDir())
    {
        inFilesList.clear();
        inFilesList<<QDir(finfo.absoluteFilePath()).entryList(QStringList("*.graphml"), QDir::Files);
        for(int i=0;i<inFilesList.size();i++)
            inFilesList[i]=inFilesList[i].prepend(finfo.absoluteFilePath() + "/");
    }
    else
        throw(Exceptions("входной файл " + CPSettings->inFileName + " не является .graphml файлом или директорией"));
}

void ControllerPlacementService::computeLmax()
{
    if (CPSettings->LmaxMultiplier)
    {
        const QVector< QVector<int> >* matr = network->getShortestMatrix();
        int max = 0;
        for (int i=0;i<matr->size();i++)
            for (int j=i; j<matr->size(); j++)
                if ((*matr)[i][j]>max)
                    max = (*matr)[i][j];
        CPSettings->totalLmax=max*CPSettings->Lmax;
        if (max == INF)
            cout<<max<<endl;
    }
    else
        CPSettings->totalLmax=CPSettings->Lmax;
}


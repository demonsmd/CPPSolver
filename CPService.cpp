#include "CPService.h"

ControllerPlacementService::ControllerPlacementService()
{
	CPSettings = new ControllerPlacementSettings();
	defSettings = new DefaultCPSettings();
	visualizator=NULL;
	PStatus = NOTRUNNING;
	ALGORITHMS = QStringList()<<"Переборный"<<"Жадный";
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
	solvedTopoList[i].solution.controllerNumber=solution.controllerNumber;
	solvedTopoList[i].solution.controllerPlacement=solution.controllerPlacement;
	solvedTopoList[i].solution.masterControllersDistribution=solution.masterControllersDistribution;
//	solvedTopoList[i].solution.reserveControllersDistribution=solution.reserveControllersDistribution;
	solvedTopoList[i].solution.totalCost=solution.totalCost;
}

//=====================================================================
//========================== SLOTS ====================================
//=====================================================================

void ControllerPlacementService::startButtonPressed()
{
	PStatus = RUNNING;
	try
	{
		int doneTopos = 0;
		QString topoTag;

		if(CPSettings->useGraphviz)
		{
			if (visualizator)
				delete visualizator;
			visualizator = new GRAPHVIZ(CPSettings->pathToGraphviz,
						    CPSettings->pathToImgFolder, "png");
		}
		getInFiles();
		toLog("Найдено входных файлов: " + QString::number(inFilesList.size()));
		solvedTopoList.resize(inFilesList.size());

		for (int topoNum=0 ; topoNum<inFilesList.size(); topoNum++)
		{
			try
			{
				if (CPSettings->useGraphviz)
					topoTag = QString("<a href=\"graphviz:%1\">%2</a>").arg(QString::number(topoNum)).arg(inFilesList.at(topoNum));
				else
					topoTag = QString("<font color=\"blue\">%1</font>").arg(inFilesList.at(topoNum));

				emit processingTopo(0,inFilesList.size(), topoNum, QString("Обрабатывается топология %1").arg(inFilesList.at(topoNum)));
				xmlReader = new GRAPHMLReader(inFilesList.at(topoNum), CPSettings);
				network = new NetworkWithAlgorithms(xmlReader->getNodes(),xmlReader->getEdges());
				ensureExp(network->CheckBiconnectivity(), "Сеть не является двусвязной");

				if (CPSettings->algorithm==0)
					algorithm = new EnumerationAlgorithm(network->getConnectivityMatrix(), network->getShortestPathMatrix(), network->getNodes(), network->getEdges());
				else if (CPSettings->algorithm==1)
					ensureExp(false, "Жадный алгоритм ещё не написан");
				solution = algorithm->solveCPP();
				saveNetworkSolution(topoNum);

				doneTopos++;
				QCoreApplication::processEvents();
				if (PStatus == NOTRUNNING)
					throw Exceptions("Работа программы прервана пользователем");
				toLog(QString("В топологии %1[%2] была <font color=\"green\"><b><u>успешно</u></b></font> обработана!").arg(topoTag).arg(QString::number(network->getTopoSize())));
				delete algorithm;
				delete network;
				delete xmlReader;
			}
			catch (Exceptions ex)
			{
				if (PStatus == NOTRUNNING)
					throw Exceptions("Работа программы прервана пользователем");
				toLog(QString("В топологии <font color=\"blue\">%1</font> была найдена <font color=\"red\"><b><u>ошибка</u></b>: %2</font>. Эта топология будет пропущена!").arg(inFilesList.at(topoNum)).arg(ex.getText()));
			}
		}
		emit processingTopo(0,0,0,QString("Готово! %1 из %2 файлов успешно обработаны").arg(QString::number(doneTopos)).arg(QString::number(inFilesList.size())));
		PStatus = NOTRUNNING;
		programFinnished();
	}
	catch(Exceptions ex)
	{
		toLog(ex.getText());
		if (PStatus == NOTRUNNING)
			emit processingTopo(0,0,0,QString("Прервано пользователем."));
		else
			PStatus = NOTRUNNING;
		programFinnished();
	}
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
			int topoNum = surl.remove(0,9).toInt(&ok);
			ensureExp(ok,"невозможно определить номер топологии");

			QFileInfo finfo(inFilesList[topoNum]);
			visualizator->visualize(finfo.baseName(), solvedTopoList[topoNum]);
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

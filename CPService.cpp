#include "CPService.h"
#include <QTextStream>

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
	solvedTopoList[i].solution.controllerPlacement=solution.controllerPlacement;
	solvedTopoList[i].solution.masterControllersDistribution=solution.masterControllersDistribution;
	solvedTopoList[i].solution.totalCost=solution.totalCost;

	QFile logFile(CPSettings->outFileName);
	if (!logFile.open(QIODevice::Append | QIODevice::Text))
	{
		toLog("<font color=\"red\">/nНевозможно открыть файл для записи/n</font>!");
		return;
	}
	QTextStream stream(&logFile);
	stream	<< inFilesList[i].right(inFilesList[i].size() - inFilesList[i].lastIndexOf("/") - 1)<<";"
		<< network->getTopoSize()<<";"
		<< network->getEdges()->size()<<";"
		<< ";;"
		<< ALGORITHMS[CPSettings->algorithm]<<";"
		<< CPSettings->Lmax<<";";

	if (CPSettings->FixedSCC)
		stream<<"fixed ("<<CPSettings->SCCost<<");";
	else if (CPSettings->HopsDepSCC)
		stream<<"hopsDep ("<<CPSettings->SCCost<<");";
	else if (CPSettings->LatDepSCC)
		stream<<"LatDep ("<<CPSettings->SCCost<<");";

	if (CPSettings->constST)
		stream<<"const syn time ("<<CPSettings->syncTime<<");";
	else
		stream<<"Linear ("<<CPSettings->SCTF_a<<"*x+"<<CPSettings->SCTF_b<<");"<<endl;


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
		int doneTopos = 0;
		QString topoTag;

		QFile logFile(CPSettings->outFileName);
		ensureExp(logFile.open(QIODevice::WriteOnly | QIODevice::Text), "Невозможно открыть лог файл");
		QTextStream stream(&logFile);
		stream<<"Topo name;Nodes number;Edges number;Controller placement;Switch distribution;Algorithm;Lmax;Cost metric;Latency metric"<<endl;
		logFile.close();


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

		for (curTopoNum=0 ; curTopoNum<inFilesList.size(); curTopoNum++)
		{
			try
			{
				if (CPSettings->useGraphviz)
					topoTag = QString("<a href=\"graphviz:%1\">%2</a>").arg(QString::number(curTopoNum)).arg(inFilesList.at(curTopoNum));
				else
					topoTag = QString("<font color=\"blue\">%1</font>").arg(inFilesList.at(curTopoNum));

//				emit processingTopo(0,inFilesList.size()-1, curTopoNum, QString("Обрабатывается топология %1").arg(inFilesList.at(curTopoNum)));
				xmlReader = new GRAPHMLReader(inFilesList.at(curTopoNum), CPSettings);

				network = new NetworkWithAlgorithms(xmlReader->getNodes(),xmlReader->getEdges()
						,CPSettings ,xmlReader->getFixedConnectionCost());
				ensureExp(network->CheckBiconnectivity(), "Сеть не является двусвязной");

				if (CPSettings->algorithm==0)
					algorithm = new EnumerationAlgorithm(network, CPSettings, &PStatus);
				else if (CPSettings->algorithm==1)
					algorithm = new GreedyAlgorithm(network, CPSettings, &PStatus);
				connect(algorithm, SIGNAL(curTopoProcess(int, int, int)),
					this, SLOT(curTopoProcess(int, int, int)));
				solution = algorithm->solveCPP();

				saveNetworkSolution(curTopoNum);

				doneTopos++;
				toLog(QString("В топологии %1[%2] была <font color=\"green\"><b><u>успешно</u></b></font> обработана!").arg(topoTag).arg(QString::number(network->getTopoSize())));
				delete algorithm;
				delete network;
				delete xmlReader;
			}
			catch (Exceptions ex)
			{
				toLog(QString("В топологии <font color=\"blue\">%1</font> была найдена <font color=\"red\"><b><u>ошибка</u></b>: %2</font>. Эта топология будет пропущена!").arg(inFilesList.at(curTopoNum)).arg(ex.getText()));
			}
		}
		emit processingTopo(0,0,0,QString("Готово! %1 из %2 файлов успешно обработаны").arg(QString::number(doneTopos)).arg(QString::number(inFilesList.size())));
		PStatus = NOTRUNNING;
		programFinnished();
	}
	catch(StopProgram ex)
	{
		toLog(ex.getText());
		emit processingTopo(0,0,0,QString("Прервано пользователем."));
		programFinnished();
	}
	catch(Exceptions ex)
	{
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

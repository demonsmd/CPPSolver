#include "CPAlgorothm.h"
#include <QCoreApplication>
#include <QMessageBox>

CPPSolution EnumerationAlgorithm::solveCPP()
{
	for (int conNum = 2; conNum<=nodesNumber; conNum++)
	//итерация по всевозможным количествам контроллеров
	{
		totalNumberOfIterations = CisNpoK(nodesNumber, conNum);
		iteration = 0;
		emit curTopoProcess(0, totalNumberOfIterations, conNum);

		//инициализация начального размещения [0][1][2][3]
		curSolution.controllerPlacement.resize(conNum);
		for (int i=0; i<conNum; i++)
			curSolution.controllerPlacement[i]=i;

		timer.setSingleShot(true);
		timer.start(analyseTime*1000);
		while (true)
		{
			try //проверяем текущее размещение
			{
				for (int conFaultNumber=conNum-1; conFaultNumber>=-1;conFaultNumber--)
				{
					//итерация по всевозможным отказам контроллера (-1 - нет отказов контроллера)
					for (int swFaultNumber=nodesNumber-1; swFaultNumber>=-1; swFaultNumber--)
					{
						//итерация по всевозможным отказам комутаторов
						ensureExp(!toLongToWait, "топология пропущена пользователем");
						if (*pStatus==NOTRUNNING)
							throw StopProgram();

						ensureExp(existDistributionForCur(conFaultNumber, swFaultNumber),
							QString("Нет размещения для отказа ") +
							QString::number(conFaultNumber) + " контроллера и " +
							QString::number(swFaultNumber) + " коммутатора");
						QCoreApplication::processEvents();
					}
				}
			}
			catch(Exceptions ex)
			{
				ensureExp(!toLongToWait, "топология пропущена пользователем");
				if (nextPlacement())
					continue;
				else
					break;
			}
			ensureExp(!toLongToWait, "топология пропущена пользователем");
			if (nextPlacement())
				continue;
			else
				break;
		}
		if (bestSolution.controllerPlacement.size()>1)
			return bestSolution;

	}//конец итерации по всем количествам контроллеров

	throw Exceptions("Для данной топологии не найдено подходящих размещений контроллеров");
}

bool ControllerPlacementAlgorothm::curConstraintsVerification(int failSwitch, int failController, const QVector<QVector<int> >& newShortestMatrix, int syncTime)
//failController не используется так как он никем не управляет
{
	int conNum = curSolution.controllerPlacement.size();
	int conPlaceInFailSwitch = -1;	//для случая если отказал коммутатор, где есть кнтроллер
	for (int i=0;i<conNum;i++)
		if (curSolution.controllerPlacement[i]==failSwitch)
			conPlaceInFailSwitch=i;


	//проверка загруженности контроллеров:
	{
		QVector<int> conLoad(conNum,0);
		for (int i=0;i<nodesNumber;i++)
		{
			if (i==failSwitch)
				continue;
			conLoad[curSolution.masterControllersDistribution[i]]+=(*network->getNodes())[i].SwitchLoad;
		}
		for (int i=0;i<conNum;i++)
		{
			if (conLoad[i]>(*network->getNodes())[curSolution.controllerPlacement[i]].ControllerLoad)
				return false;
		}
	}
	//вычисление междоменной и  внутридоменной задержки и проверка задержки
	{
		int max1 = -1, max2 = -1;//2 наибольшие задержки в доменах
		int inDL = 0;		//максимальная задержка внутри домена
		for (int c=0;c<conNum;c++)
		{
			if (c==failController)
				continue;
			int cmax=-1, cmax2=-1;//две наибольшие задержки внутри одного домена
			for (int n=0;n<nodesNumber;n++)
			{
				if (curSolution.masterControllersDistribution[n]!=c || n==failSwitch)
					continue;
				if (conPlaceInFailSwitch>=0)
				{
					if (cmax<(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]])
					{
						cmax2=cmax;
						cmax=(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]];
					}
					else if (cmax2<(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]])
						cmax2=(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]];
				}
				else
				{
					if (cmax<newShortestMatrix[n][curSolution.controllerPlacement[c]])
					{
						cmax2=cmax;
						cmax=newShortestMatrix[n][curSolution.controllerPlacement[c]];
					}
					else if (cmax2<newShortestMatrix[n][curSolution.controllerPlacement[c]])
						cmax2=newShortestMatrix[n][curSolution.controllerPlacement[c]];
				}
			}
			if (cmax>max1)
			{
				max2=max1;
				max1=cmax;
			}
			else if (cmax>max2)
				max2=cmax;

			if (inDL<cmax+cmax2)//тут задержка в домене максимальна
				inDL=cmax+cmax2;
		}

		int MDSync;

		if (max2==-1) //всего один контроллер
			MDSync=0;
		else
			MDSync=max1+max2+syncTime;

		if (max(inDL,MDSync)>settings->Lmax)
			return false;
	}
	return true;
}

int ControllerPlacementAlgorothm::computeSynTimeForCur(int failController, const QVector<QVector<int> >* newShortestMatrix)
{
	if (settings->constST)
		return settings->syncTime;
	else
	{
		int max = 0;
		int conNum = curSolution.controllerPlacement.size();
		for (int i=0;i<conNum;i++)
		{
			if (i==failController) continue;
			for (int j=0; j<conNum; j++)
			{
				if (j==failController) continue;
				if ((*newShortestMatrix)
				    [curSolution.controllerPlacement[i]]
				    [curSolution.controllerPlacement[j]]  >max)
					max = (*newShortestMatrix)
					      [curSolution.controllerPlacement[i]]
					      [curSolution.controllerPlacement[j]];
			}
		}
		return settings->SCTF_a*max+settings->SCTF_b;
	}
}

bool ControllerPlacementAlgorothm::existDistributionForCur(int failController, int failSwitch)
{
	//топология фиксирована. контроллеры размещены. отказы произошли.
	//нужно проверить есть ли тут распределение коммутаторов по контроллерам.
	QVector<QVector<int> > newConnectivityMatrix = (*network->getConnectivityMatrix());
	QVector<QVector<int> > newShortestMatrix;
	int conNum = curSolution.controllerPlacement.size();
	QSet<int> conPlaces;
	int firstCon = failController==0?1:0;	//номер первого исправного контроллера
	int lastCon = failController==conNum-1?conNum-2:conNum-1;	//номер последнего исправного контроллера
	curSolution.masterControllersDistribution = QVector<int>(nodesNumber, firstCon);
	bool EDFC = false;
	for (int i=0;i<nodesNumber;i++) //создание новой матрицы связности
		for (int j=0; j<nodesNumber;j++)
		{
			if (i==failSwitch || j==failSwitch)
			{
				newConnectivityMatrix[i][j] = INF;
			}
		}
	if (failController==-1 && failSwitch==-1)
	{
		conPlaces.clear();
	}

	network->makeShortestMatr(newShortestMatrix,newConnectivityMatrix,nodesNumber);

	//начальное распределение
	for (int i=0;i<nodesNumber;i++)
	{
		if (i==failSwitch)
			continue;
		for (int con=0;con<conNum;con++)
		{
			if (curSolution.controllerPlacement[con]==i
			    && con!=failController)
			{
				curSolution.masterControllersDistribution[i]=con;
				conPlaces.insert(i);
				break;
			}
		}
	}

	//определение времени синхронизации
	int syncTime;
	if (conPlaces.contains(failSwitch))
		syncTime = computeSynTimeForCur(failController, network->getShortestMatrix());
	else
		syncTime = computeSynTimeForCur(failController, &newShortestMatrix);
	while (true)//перебираем всевозможные распределения контроллеров
	{
		QCoreApplication::processEvents();
		if (toLongToWait)
			return false;

		if (curConstraintsVerification(failSwitch, failController, newShortestMatrix, syncTime))//следующее размещение
		{
			EDFC=true;
			if (failController==-1&&failSwitch==-1)
				checkIfCurIsBest();
			else
				return true;
		}

		int shiftPoint = -1;
		//поиск точки сдвига
		for( int i=nodesNumber-1; i>=0 ;i--)
		{
			if (i==failSwitch || conPlaces.contains(i))
				continue;
			if (curSolution.masterControllersDistribution[i]!=lastCon)
			{
				shiftPoint=i;
				curSolution.masterControllersDistribution[i]++;
				if (curSolution.masterControllersDistribution[i]==failController)
					curSolution.masterControllersDistribution[i]++;
				break;
			}
		}

		if (shiftPoint<0)
		{
			//нет больше размещений
			return EDFC;
		}
		for (int i=shiftPoint+1;i<nodesNumber;i++)
		{
			if (i==failSwitch || conPlaces.contains(i))
				continue;
			curSolution.masterControllersDistribution[i]=firstCon;
		}
	}
	return false;
}

void ControllerPlacementAlgorothm::checkIfCurIsBest()
{
	int totalCost = 0;
	int conNum = curSolution.controllerPlacement.size();

	//вычисление стоимости контроллеров
	for (int i=0;i<conNum;i++)
		totalCost+=(*network->getNodes())[curSolution.controllerPlacement[i]].ControllerCost;

	//вычисление стоимости соединений
	for (int i=0;i<nodesNumber;i++)
	{
		totalCost+=(*network->getConCostMatr())[i]
			   [curSolution.controllerPlacement[curSolution.masterControllersDistribution[i]]];
	}
	curSolution.totalCost=totalCost;

	//вычисление средней задержки
	int avgLat = 0, nFlows =0;
	int synTime = computeSynTimeForCur(-1, network->getShortestMatrix());
	for (int i=0;i<nodesNumber;i++)
	{
		ensureExp(!toLongToWait,"топология пропущена пользователем");
		for (int j=i;j<nodesNumber;j++)
		{
			nFlows++;
			if (curSolution.masterControllersDistribution[i]!=
			    curSolution.masterControllersDistribution[j])
			{
				avgLat+=synTime;
			}
			int cPlace = curSolution.controllerPlacement [curSolution.masterControllersDistribution[i]];
			avgLat+=(*network->getShortestMatrix())[i][cPlace];
			int max=0;
			for (int k=1;k<(*network->getPathMatrix())[i][j].size();k++)
			{
				cPlace = curSolution.controllerPlacement [curSolution.masterControllersDistribution[(*network->getPathMatrix())[i][j][k]]];
				if ((*network->getShortestMatrix())[cPlace][k]>max)
					max = (*network->getShortestMatrix())[cPlace][k];
			}
			avgLat+=max;
		}
		QCoreApplication::processEvents();
	}
	curSolution.avgLayency=avgLat/nFlows;


	if (curSolution.controllerPlacement.size()==3)
		if(curSolution.controllerPlacement[0]==2&&curSolution.controllerPlacement[1]==4&&curSolution.controllerPlacement[2]==7
		   &&curSolution.masterControllersDistribution[1]==2&&curSolution.masterControllersDistribution[3]==1
		   &&curSolution.masterControllersDistribution[4]==1&&curSolution.masterControllersDistribution[5]==1)
	{
		cout<<1<<endl;
	}

	if (bestSolution.totalCost<0 || bestSolution.totalCost>curSolution.totalCost || (bestSolution.totalCost==curSolution.totalCost && bestSolution.avgLayency>curSolution.avgLayency))
	{
		bestSolution.controllerPlacement=curSolution.controllerPlacement;
		bestSolution.avgLayency=curSolution.avgLayency;
		bestSolution.masterControllersDistribution=curSolution.masterControllersDistribution;
		bestSolution.totalCost=curSolution.totalCost;
	}
}

void EnumerationAlgorithm::timeOut()
{
	if(*pStatus==NOTRUNNING)
		return;
	if (iteration==0)
	{
		toLongToWait=true;
		return;
	}
	int a = totalNumberOfIterations/iteration*analyseTime/60;
	if (a>90)
	{
//		QMessageBox mb;
//		mb.setText(QString("предполагаемое время работы программы %1 минут").arg(a));
//		mb.setInformativeText("Желаете всё равно продолжить?");
//		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
//		int ret = mb.exec();
//		if(ret==QMessageBox::Cancel)
			toLongToWait=true;
	}
}

bool EnumerationAlgorithm::nextPlacement()
{
	int conNum = curSolution.controllerPlacement.size();

	iteration++;
	emit curTopoProcess(iteration, totalNumberOfIterations, conNum);

	//определяем корень сдвига
	int shiftRoot = -1;
	for (int i=conNum-1;i>=0;i--)
	{
		if (curSolution.controllerPlacement[i]!=nodesNumber-conNum+i)
		{
			shiftRoot=i;
			break;
		}
	}

	if (shiftRoot==-1)
		return false;

	curSolution.controllerPlacement[shiftRoot]++;	//смещаем вершину
	for (int i=shiftRoot+1;i<conNum;i++)		//смещаем всё остальное
		curSolution.controllerPlacement[i]=curSolution.controllerPlacement[shiftRoot]+i-shiftRoot;

	return true;
}

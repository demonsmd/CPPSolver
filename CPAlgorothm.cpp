#include "CPAlgorothm.h"
#include <QCoreApplication>
#include <QMessageBox>

ControllerPlacementAlgorothm::ControllerPlacementAlgorothm(const NetworkWithAlgorithms* net, const ControllerPlacementSettings* set, const programStatus *pStatus)
	:network(net), settings(set), nodesNumber(net->getTopoSize()), toLongToWait(false), pStatus(pStatus)
{
	//определение количества контроллеров
	if (settings->FixedCon)
	{
		if (settings->FixedConNum > nodesNumber)
			minConNum=maxConNum=nodesNumber;
		else
			minConNum=maxConNum=settings->FixedConNum;
	}
	else if(settings->IncrementalCon)
	{
		minConNum=settings->IncrementalConNum;
		maxConNum = nodesNumber;
	}
	else
	{
		minConNum=nodesNumber*settings->PercentageConNumFrom/100;
		maxConNum=nodesNumber*settings->PercentageConNumTo/100;
	}
}

int ControllerPlacementAlgorothm::computeWCLTime(int failController, int failSwitch, const QVector<QVector<int> >* newShortestMatrix, int syncTime)
{
	int conPlaceInFailSwitch = -1;	//для случая если отказал коммутатор, где есть кнтроллер
	int conNum = curSolution.controllerPlacement.size();
	for (int i=0;i<conNum;i++)
		if (curSolution.controllerPlacement[i]==failSwitch)
			conPlaceInFailSwitch=i;
	int max1 = -1, max2 = -1;//2 наибольшие задержки в доменах
	int inDL = 0;		//максимальная задержка внутри домена
	for (int c=0;c<conNum;c++)
	{
		if (c==failController)
			continue;
		int cmax=-1;//наибольшая задержка внутри одного домена
		for (int n=0;n<nodesNumber;n++)
		{
			if (curSolution.masterControllersDistribution[n]!=c || n==failSwitch)
				continue;

			if (conPlaceInFailSwitch>=0)
			{
				if (cmax<(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]])
					cmax=(*network->getShortestMatrix())[n][curSolution.controllerPlacement[c]];
			}
			else
			{
				if (cmax<(*newShortestMatrix)[n][curSolution.controllerPlacement[c]])
					cmax=(*newShortestMatrix)[n][curSolution.controllerPlacement[c]];
			}
		}
		if (cmax>max1)
		{
			max2=max1;
			max1=cmax;
		}
		else if (cmax>max2)
			max2=cmax;

		if (inDL<cmax*2)//тут задержка в домене максимальна
			inDL=cmax*2;
	}

	int MDSync;

	if (max2==-1) //всего один контроллер
		MDSync=0;
	else
		MDSync=max1+max2+syncTime;

	return max(inDL,MDSync);
}

CPPSolution EnumerationAlgorithm::solveCPP()
{
	for (int conNum = minConNum; conNum<=maxConNum; conNum++)
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
						ensureExp(!toLongToWait, "время работы топологии слишком большое");
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
				ensureExp(!toLongToWait, "время работы топологии слишком большое");
				if (nextPlacement())
					continue;
				else
					break;
			}
			ensureExp(!toLongToWait, "время работы топологии слишком большое");
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
	if (computeWCLTime(failController, failSwitch, &newShortestMatrix,syncTime)>settings->Lmax)
		return false;
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

bool EnumerationAlgorithm::existDistributionForCur(int failController, int failSwitch)
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
//	if (failController==-1 && failSwitch==-1)
//	{
//		conPlaces.clear();
//	}

	network->makeShortestMatr(newShortestMatrix,newConnectivityMatrix,nodesNumber);

	//начальное распределение
	for (int i=0;i<nodesNumber;i++)
	{
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
			{
				computeCostAndLatForCur();
				checkIfCurIsBest();
			}
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

void ControllerPlacementAlgorothm::computeCostAndLatForCur()
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
		ensureExp(!toLongToWait,"время работы топологии слишком большое");
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
}

void ControllerPlacementAlgorothm::checkIfCurIsBest()
{
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
//		toLongToWait=true;
		return;
	}
	int a = totalNumberOfIterations/iteration*analyseTime/60;
	if (settings->algoTime==0)
	{
		QMessageBox mb;
		mb.setText(QString("предполагаемое время работы программы %1 минут").arg(a));
		mb.setInformativeText("Желаете всё равно продолжить?");
		mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		int ret = mb.exec();
		if(ret==QMessageBox::Cancel)
			toLongToWait=true;
	}
	else if (a>settings->algoTime)
	{
//		toLongToWait=true;
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

CPPSolution GreedyAlgorithm::solveCPP()
{
	for (int conNum = minConNum; conNum<=maxConNum; conNum++)
	//итерация по всевозможным количествам контроллеров
	{
		try
		{
			timer.setSingleShot(true);
			timer.start(settings->algoTime*60*1000);
			seenPlacements.clear();
			bestSolution.totalCost=-1;

			placeInTopoCenter(conNum);
			checkChildSolution(NULL);
			if (bestSolution.totalCost>=0)
				return bestSolution;
		}
		catch(Exceptions ex)
		{
			ensureExp(!toLongToWait, "время работы топологии слишком большое");
		}
	}
	return bestSolution;
}

void GreedyAlgorithm::placeInTopoCenter(int conNum)
{
	QMap<int,int> latencys;
	for (int node=0;node<nodesNumber;node++)
	{
		int lat=0;
		for (int i=0;i<nodesNumber;i++)
			lat+=(*network->getShortestMatrix())[node][i];
		latencys.insertMulti(lat,node);
	}
	//qmap is sorted by key so:
	curSolution.controllerPlacement.resize(conNum);
	QMap<int, int>::const_iterator iter = latencys.constBegin();
	QSet<int> placement;
	for (int i=0;i<conNum;i++)
	{
		curSolution.controllerPlacement[i]=(iter+i).value();
		placement.insert((iter+i).value());
	}
	seenPlacements.insert(placement);
	curSolution.WCLatency = 0;
	curSolution.totalCost=-1;
}

void GreedyAlgorithm::checkChildSolution(CPPSolution* parentSol)
{
	int conNum = curSolution.controllerPlacement.size();

	for (int failController=conNum-1; failController>=-1;failController--)
	{
		//итерация по всевозможным отказам контроллера (-1 - нет отказов контроллера)
		for (int failSwitch=nodesNumber-1; failSwitch>=-1; failSwitch--)
		{
			//==================== сценарий отказа зафиксирован ====================
			//======= изменение внутреннего представления топологии после отказа =======
			ensureExp(!toLongToWait, "время работы топологии слишком большое");
			if (*pStatus==NOTRUNNING)
				throw StopProgram();
			QVector<QVector<int> > newConnectivityMatrix = (*network->getConnectivityMatrix());
			QVector<QVector<int> > newShortestMatrix;
			for (int i=0;i<nodesNumber;i++) //создание новой матрицы связности
				for (int j=0; j<nodesNumber;j++)
				{
					if (i==failSwitch || j==failSwitch)
					{
						newConnectivityMatrix[i][j] = INF;
					}
				}
			network->makeShortestMatr(newShortestMatrix,newConnectivityMatrix,nodesNumber);

			//=======================================================================
			initialDistribution(failSwitch, failController, newShortestMatrix);
			controllerLoadCheck(failSwitch, failController, newShortestMatrix);
			connectionLatencyCheck(failSwitch, failController, newShortestMatrix);
		}
	}
	//просмотрели все сценарии отказа
	if (parentComparation(parentSol))
	{
		if (curSolution.WCLatency<=settings->Lmax)
			checkIfCurIsBest();
		solveNeighbors();
	}
	else
		return;
}

void GreedyAlgorithm::initialDistribution(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix)
{
	curSolution.masterControllersDistribution.resize(nodesNumber);
	for (int node = 0; node<nodesNumber;node++)
	{
		if (failSwich==node)
			continue;

		int minLat = INF;
		int minCon = 0;
		for (int i=0;i<curSolution.controllerPlacement.size();i++)
		{
			if (i==failCon)
				continue;
			if (curSolution.controllerPlacement[i]==failSwich)
			{
				if ((*network->getShortestMatrix())[node][curSolution.controllerPlacement[i]]<minLat)
				{
					minLat=newShortestMatrix[node][curSolution.controllerPlacement[i]];
					minCon=i;
				}
			}
			else
			{
				if (newShortestMatrix[node][curSolution.controllerPlacement[i]]<minLat)
				{
					minLat=newShortestMatrix[node][curSolution.controllerPlacement[i]];
					minCon=i;
				}
			}
		}
		curSolution.masterControllersDistribution[node]=minCon;
	}
}

void GreedyAlgorithm::controllerLoadCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix)
{
	bool error = false;
	bool repeat = true;
	int conNum = curSolution.controllerPlacement.size();
	while (repeat&&!error)
	{
		QVector<int> loads = QVector<int>(conNum,0);
		for (int i=0;i<nodesNumber;i++)
		{
			if (i==failSwich)
				continue;
			loads[curSolution.masterControllersDistribution[i] ]+= (*network->getNodes())[i].SwitchLoad;
		}

		for (int i=0;i<conNum;i++)
		{
			if (i==failCon)
				continue;
			loads[i] -= (*network->getNodes())[curSolution.controllerPlacement[i]].ControllerLoad;
		}

		repeat = false;
		for (int i=0;i<conNum;i++)
		{
			if (i==failCon)
				continue;

			if (loads[i]>0)
			{
				//перегружен i-ый контроллер

				int newSw=-1;
				int newLat=INF;
				int newCon = -1;

				//для каждого коммутатора из домена вычисляем расстояние до другого неперегруженного контроллера
				for (int s=0;s<nodesNumber;s++)
				{
					if (s==failSwich)
						continue;

					if (i!=curSolution.masterControllersDistribution[s])
						continue;

					int con = -1; //ближайший контроллер
					int lat = INF;//задержка до блишайшего контроллера

					for(int c=0;c<conNum;c++)
					{
						if (con==failCon)
							continue;
						if (newShortestMatrix[s][curSolution.controllerPlacement[c]]<lat &&
						    (loads[c]+(*network->getNodes())[s].SwitchLoad)<=0)
						{
							con=c;
							lat = newShortestMatrix[s][curSolution.controllerPlacement[c]];
						}
					}

					if (lat<newLat)
					{
						newLat=lat;
						newSw = s;
						newCon = con;
					}
				}

				if (newSw==-1)
				{
					//не удалось перераспределить контроллеры
					error=true;
				}
				else
				{
					curSolution.masterControllersDistribution[newSw]=newCon;
					loads[newCon]+=(*network->getNodes())[newSw].SwitchLoad;
					loads[i]-=(*network->getNodes())[newSw].SwitchLoad;
					repeat = true;
				}

			}
		}
		ensureExp(!error,"не удалось перераспределить контроллеры");
		if (!repeat)
			return;
	}
}

void GreedyAlgorithm::connectionLatencyCheck(int failSwich, int failCon, const QVector<QVector<int> > &newShortestMatrix)
{
	int syncTime;

	QSet<int> conPlaces;
	for (int i=0;i<curSolution.controllerPlacement.size();i++)
		conPlaces.insert(curSolution.controllerPlacement[i]);

	if (conPlaces.contains(failSwich))
		syncTime = computeSynTimeForCur(failCon, network->getShortestMatrix());
	else
		syncTime = computeSynTimeForCur(failCon, &newShortestMatrix);
	int WCL = computeWCLTime(failCon, failSwich, &newShortestMatrix, syncTime);
	if (WCL>curSolution.WCLatency)
		curSolution.WCLatency=WCL;
}


bool GreedyAlgorithm::parentComparation(CPPSolution* parentSol)
{
	if (curSolution.WCLatency<=settings->Lmax)
	{
		//получили решение!
		computeCostAndLatForCur();
		if (parentSol==NULL)
			return true;
		if (parentSol->totalCost<0)
			return true;
		if (parentSol->totalCost>=curSolution.totalCost)
			return true;
		return false;
	}
	else
	{
		curSolution.totalCost=-1;
		if (parentSol==NULL)
			return true;
		if (parentSol->totalCost>=0)
			return false;
		if (parentSol->WCLatency<curSolution.WCLatency)
			return false;
		return true;
	}
	return false;
}

void GreedyAlgorithm::solveNeighbors()
{
	//curSol - предок надо рекурсивно вызвать всех соседей
	CPPSolution parentSol = curSolution;

	int conNum = curSolution.controllerPlacement.size();

	QSet<int> conPlaces;
	for (int i=0;i<conNum;i++)
		conPlaces.insert(curSolution.controllerPlacement[i]);

	for (int con = 0;con<conNum;con++)
	{
		for (int n=0;n<nodesNumber;n++)
		{
			if (n==curSolution.controllerPlacement[con] || conPlaces.contains(n))
				continue;
			if((*network->getConnectivityMatrix())[curSolution.controllerPlacement[con]][n]!=INF)
				//есть линк к узлу n
			{
				curSolution.controllerPlacement[con]=n;

				QSet<int> set;
				for (int i=0;i<conNum;i++)
					set.insert(curSolution.controllerPlacement[i]);

				if (!seenPlacements.contains(set))
				{
					//такого размещения ещё не было
					seenPlacements.insert(set);
					curSolution.totalCost=-1;
					curSolution.WCLatency=0;
					checkChildSolution(&parentSol);
				}
				curSolution.controllerPlacement=parentSol.controllerPlacement;
			}
		}
	}
}

void GreedyAlgorithm::timeOut()
{
	toLongToWait=true;
}

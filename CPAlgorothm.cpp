#include "CPAlgorothm.h"

CPPSolution EnumerationAlgorithm::solveCPP()
{
	bestSolution.totalCost=-1;	//индикатор существования наилучшего размещения
	for (int conNum = 2; conNum<nodesNumber; conNum++)
	//итерация по всевозможным количествам контроллеров
	{
		//инициализация начального размещения
		curSolution.controllerPlacement.resize(conNum);
		for (int i=0; i<conNum; i++)
			curSolution.controllerPlacement[i]=i;

		while (true)
		{
			if(existDistributionForCur())	//проверка размещения
				checkIfCurIsBest();
			if (!nextPlacement())
				break;	//перебрали все размещения

		}

		if (bestSolution.totalCost>=0)
			return bestSolution;


	}//конец итерации по всем количествам контроллеров

//	throw Exceptions("Для данной топологии не найдено подходящих размещений контроллеров");
	return bestSolution;
}

bool ControllerPlacementAlgorothm::curConstraintsVerification()
{

	return false;
}

bool EnumerationAlgorithm::nextDistribution()
{
	return false;
}

bool EnumerationAlgorithm::existDistributionForCur()
{
	return false;
}

void ControllerPlacementAlgorothm::checkIfCurIsBest()
{
	return;
}


bool EnumerationAlgorithm::nextPlacement()
{
	int conNum = curSolution.controllerPlacement.size();
	if (curSolution.controllerPlacement[conNum-1]==nodesNumber-1 &&
	    curSolution.controllerPlacement[0]==nodesNumber-conNum)
	{
		//перебрали всевозможные размещения при данном количестве контроллеров
		return false;
	}
	return false;
}

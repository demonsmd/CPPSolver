#include "graphmlReader.h"
#include <QXmlStreamReader>
#include <QFile>
#include <QXmlStreamAttributes>
#include <QSet>
#include <QMap>

GRAPHMLReader::GRAPHMLReader(QString fileName, const ControllerPlacementSettings* settings)
{
	FixedConnectionCost=-1;
	nodes = new QVector<NODE>;
	edges = new QVector<EDGE>;
	CPSettings = settings;
	parce(fileName);
	normalizeNumeration();
	latencyCalculate();
	validate();
}

void GRAPHMLReader::latencyCalculate()
{
	for (int i=0;i<edges->size();i++)
	{
		(*edges)[i].latency=geoLatency((*nodes)[(*edges)[i].srcId].latitude,
			(*nodes)[(*edges)[i].srcId].longitude,
			(*nodes)[(*edges)[i].dstId].latitude,
			(*nodes)[(*edges)[i].dstId].longitude);
		//sfg
	}
}

void GRAPHMLReader::normalizeNumeration()
{
	QMap<int, int> swap;
	int curNum=0;
	for (int i=0;i<nodes->size();i++)
	{
		swap.insert((*nodes)[i].id, curNum);
		(*nodes)[i].id = curNum++;
	}
	for (int i=0;i<edges->size();i++)
	{
		(*edges)[i].dstId=swap[(*edges)[i].dstId];
		(*edges)[i].srcId=swap[(*edges)[i].srcId];
	}
}

void GRAPHMLReader::validate()
{
	if (CPSettings->FixedSCC && FixedConnectionCost<0)
		FixedConnectionCost=CPSettings->SCCost;
	QSet<int> nset;
	for (int i=0;i<nodes->size();i++)
	{
		ensureExp((*nodes)[i].latitude!=-1000,QString("не заданы координаты узла %1").arg(QString::number((*nodes)[i].id)));
		ensureExp((*nodes)[i].longitude!=-1000,QString("не заданы координаты узла %1").arg(QString::number((*nodes)[i].id)));
		ensureExp((*nodes)[i].latitude>-90 && (*nodes)[i].latitude<90,QString("широта не может принимать значение %1!").arg(QString::number((*nodes)[i].latitude)));
		ensureExp((*nodes)[i].longitude>-180 && (*nodes)[i].longitude<180,QString("долгота не может принимать значение %1!").arg(QString::number((*nodes)[i].longitude)));

		if (CPSettings->FixedCC || (*nodes)[i].ControllerCost<0)
			(*nodes)[i].ControllerCost=CPSettings->CCost;
		if (CPSettings->FixedCP || (*nodes)[i].ControllerLoad<0)
			(*nodes)[i].ControllerLoad=CPSettings->CPerfomance;
		if (CPSettings->FixedSP || (*nodes)[i].SwitchLoad<0)
			(*nodes)[i].SwitchLoad=CPSettings->SPerfomance;

		ensureExp((*nodes)[i].id>=0, QString("id не может принимать значение %1").arg(QString::number((*nodes)[i].id)));
		ensureExp(!nset.contains((*nodes)[i].id), QString("найдены 2 вершины с id =  %1").arg(QString::number((*nodes)[i].id)));
		nset.insert((*nodes)[i].id);
	}
	for (int i=0;i<edges->size();i++)
	{
		if ((*edges)[i].HopCost<0)
		{
			if (CPSettings->HopsDepSCC)
				(*edges)[i].HopCost=CPSettings->SCCost;
			else if (CPSettings->LatDepSCC)
				(*edges)[i].HopCost=(*edges)[i].latency / CPSettings->SCCost;
		}
		ensureExp(nset.contains((*edges)[i].srcId), QString("ошибка! не найдена вершина %1 для ребра").arg(QString::number((*edges)[i].srcId)));
		ensureExp(nset.contains((*edges)[i].dstId), QString("ошибка! не найдена вершина %1 для ребра").arg(QString::number((*edges)[i].dstId)));
	}
}

void GRAPHMLReader::parce(QString fileName)
{
	QFile* file = new QFile(fileName);
	ensureExp(file->open(QIODevice::ReadOnly | QIODevice::Text),QString("Не получилось открыть файл %1").arg(fileName));
	QXmlStreamReader xmlstream(file);

	QString idTag;
	QString longitude;
	QString latitude;
	QString SLoadTag;
	QString CLoadTag;
	QString CCostTag;
	QString HopCostTag;
	QString GlobalSCCTag;
	QXmlStreamReader::TokenType token;
	bool ok;

	while(!xmlstream.atEnd()&&!xmlstream.hasError())
	{
		token = xmlstream.readNext();
		if (token == QXmlStreamReader::StartDocument)
			continue;
SAMETOKEN:	if (token == QXmlStreamReader::StartElement)
		{
			if (xmlstream.name() == "key") //описание атрибутов
			{
				QXmlStreamAttributes attr = xmlstream.attributes();
				if (attr.value("attr.name")=="id" && attr.value("for")=="node")
				{
					idTag = xmlstream.attributes().value("id").toString();
					continue;
				}
				if (attr.value("attr.name")=="Longitude")
				{
					longitude = xmlstream.attributes().value("id").toString();
					continue;
				}
				if (attr.value("attr.name")=="Latitude")
				{
					latitude = xmlstream.attributes().value("id").toString();
					continue;
				}
				if (attr.value("attr.name")=="SwitchLoad")
				{
					SLoadTag = xmlstream.attributes().value("id").toString();
					continue;
				}
				if (attr.value("attr.name")=="ControllerLoad")
				{
					CLoadTag = xmlstream.attributes().value("id").toString();
					continue;
				}
				if (attr.value("attr.name")=="cost")
				{
					if (attr.value("for")=="edge")
					{
						HopCostTag = xmlstream.attributes().value("id").toString();
						continue;
					}
					if (attr.value("for")=="node")
					{
						CCostTag = xmlstream.attributes().value("id").toString();
						continue;
					}
				}
				if (attr.value("attr.name")=="FixedConnectionCost")
				{
					GlobalSCCTag = xmlstream.attributes().value("id").toString();
					continue;
				}
				continue;
			}//end if element = key
			if (xmlstream.name()=="data")
			{
				while (xmlstream.name() == "data")
				{
					QXmlStreamAttributes attr = xmlstream.attributes();
					if (attr.value("key")==GlobalSCCTag)
					{
						QString str=xmlstream.readElementText();
						FixedConnectionCost=str.toInt(&ok);
						ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута FixedConnectionCost к целочисленному значению").arg(str));
					}
					token = xmlstream.readNext();
				}
				goto SAMETOKEN;
			}
			if (xmlstream.name()=="node")
			{
				xmlstream.readNext();
				NODE node;
				while (!(xmlstream.tokenType() == QXmlStreamReader::EndElement && xmlstream.name() == "node") && !xmlstream.atEnd())
				{
					QXmlStreamAttributes attr = xmlstream.attributes();
					if (xmlstream.tokenType() == QXmlStreamReader::StartElement && xmlstream.name()=="data")
					{
						if(attr.value("key")==idTag)
						{
							QString str=xmlstream.readElementText();
							node.id=str.toInt(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута idTag к целочисленному значению").arg(str));
						}
						if(attr.value("key")==longitude)
						{
							QString str=xmlstream.readElementText();
							node.longitude=str.toFloat(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута longitude к целочисленному значению").arg(str));
						}
						if(attr.value("key")==latitude)
						{
							QString str=xmlstream.readElementText();
							node.latitude=str.toFloat(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута latitude к целочисленному значению").arg(str));
						}
						if(attr.value("key")==SLoadTag)
						{
							QString str=xmlstream.readElementText();
							node.SwitchLoad=str.toInt(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута SLoadTag к целочисленному значению").arg(str));
						}
						if (attr.value("key")==CLoadTag)
						{
							QString str=xmlstream.readElementText();
							node.ControllerLoad=str.toInt(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута CLoadTag к целочисленному значению").arg(str));
						}
						if(attr.value("key")==CCostTag)
						{
							QString str=xmlstream.readElementText();
							node.ControllerCost=str.toInt(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута CCostTag к целочисленному значению").arg(str));
						}
					}
					xmlstream.readNext();
				}
				nodes->push_back(node);
			}
			if (xmlstream.name()=="edge")
			{
				EDGE edge;
				QXmlStreamAttributes attr = xmlstream.attributes();
				if (attr.hasAttribute("source"))
				{
					QString str=attr.value("source").toString();
					edge.srcId=str.toInt(&ok);
					ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута source к целочисленному значению").arg(str));
				}
				if (attr.hasAttribute("target"))
				{
					QString str=attr.value("target").toString();
					edge.dstId=str.toInt(&ok);
					ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута target к целочисленному значению").arg(str));
				}
				xmlstream.readNext();
				while (!(xmlstream.tokenType() == QXmlStreamReader::EndElement && xmlstream.name() == "edge"))
				{
					QXmlStreamAttributes attr = xmlstream.attributes();
					if (xmlstream.tokenType() == QXmlStreamReader::StartElement && xmlstream.name()=="data")
					{
						if(attr.value("key")==HopCostTag)
						{
							QString str=xmlstream.readElementText();
							edge.HopCost=str.toInt(&ok);
							ensureExp(ok, QString("Ошибка приведения значения \"%1\" атрибута HopCostTag к целочисленному значению").arg(str));
						}
					}
					xmlstream.readNext();
				}
				edges->push_back(edge);
			}

		}//end if token == startElement
	}
}


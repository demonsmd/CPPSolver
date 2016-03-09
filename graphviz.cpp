#include "graphviz.h"
#include <QSet>
#include <QApplication>
#include <QLabel>
#include "mainwindow.h"
#include <QDesktopWidget>

GRAPHVIZ::GRAPHVIZ(QString dot, QString img, QString format)
	:pathToDot(dot), pathToImg(img), format(format)
{
	QFileInfo finfo(pathToDot);
	ensureExp(finfo.isFile(),"не найден dot.exe");
	finfo.setFile(pathToImg);
	ensureExp(finfo.isDir(), "не найден путь к папке с изабражениями");
	ensureExp(format=="png", "неверный формат выходного файла");
}

void GRAPHVIZ::visualize(QString name, const SolvedTopo& solution)
{
	QProcess* process = new QProcess(this);
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), process, SLOT(deleteLater()));
	QStringList arg;
	arg<<QString("-T%1").arg(format)
		<<"-Kfdp"
		<<QString("-o%1/%2.%3").arg(pathToImg).arg(name).arg(format);

	QSet<int> conSet;
	for (int i=0;i<solution.solution.controllerPlacement.size();i++)
		conSet.insert(solution.solution.controllerPlacement[i]);

	process->start(pathToDot,arg);
	process->write("strict graph topo {");
//	process->write("graph [ size =\"15,9\"];");
	process->write("graph [ dpi = 300 ];");
	for (int i=0;i<solution.nodes.size();i++)
	{
		int minX, minY;
		float scale;
		calculateScaleK(solution, minX, minY, scale);
		process->write((QString::number(i) + "[").toStdString().c_str());
		if (conSet.contains(i))
			process->write("shape = box");
		process->write(QString("pos = \"%1,%2!\"]")
			       .arg(QString::number((solution.nodes[i].longitude-minY)/scale))
			       .arg(QString::number((solution.nodes[i].latitude-minX)/scale))
			       .toStdString().c_str());
	}
	for (int i=0;i<solution.edges.size();i++)
	{
		process->write(QString("%1 -- %2 [label=\"%3\"")
			       .arg(QString::number(solution.edges[i].srcId))
			       .arg(QString::number(solution.edges[i].dstId))
			       .arg(QString::number(solution.edges[i].latency))
			       .toStdString().c_str());
		process->write("fontsize=7];");
	}
	process->write("}");
	process->closeWriteChannel();
	process->waitForFinished(2000);

	QRect rec = QApplication::desktop()->screenGeometry();
	QLabel* label = new QLabel;
	QPixmap pm(pathToImg+"/"+name+"."+format);
	label->setAttribute(Qt::WA_DeleteOnClose);
	label->setGeometry(rec.width()*0.1,rec.height()*0.1,rec.width()*0.8,rec.height()*0.8);
	label->setPixmap(pm);
	label->setScaledContents(true);
	label->setWindowTitle(name + "." + format);
	label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
	label->show();
}
void GRAPHVIZ::calculateScaleK(const SolvedTopo& solution, int& minX, int& minY, float& scale)
{
	int maxX=0;
	int maxY=0;
	minX=180;
	minY=90;
	for (int i=0;i<solution.nodes.size();i++)
	{
		if (solution.nodes[i].latitude<minX)
			minX=solution.nodes[i].latitude;
		if (solution.nodes[i].latitude>maxX)
			maxX=solution.nodes[i].latitude;
		if (solution.nodes[i].longitude<minY)
			minY=solution.nodes[i].longitude;
		if (solution.nodes[i].longitude>maxY)
			maxY=solution.nodes[i].longitude;
	}
	scale=(maxX-minX)*10;
}

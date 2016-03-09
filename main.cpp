#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("CPPSolver");
	MainWindow w;
	w.show();

	return a.exec();
}
//TODO кнопка параметры алгоритма

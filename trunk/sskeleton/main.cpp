#include "ssmainwind.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SSMainWind w;
	w.show();
	return a.exec();
}

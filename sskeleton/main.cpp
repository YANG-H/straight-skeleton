#include "ssmainwind.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QApplication::setApplicationName(QObject::tr("Straight Skeleton"));
	QApplication::setOrganizationName(QObject::tr("Institute of CG & CAD, Tsinghua University"));
	a.setFont(QFont("Microsoft Yahei", 9));

	SSMainWind w;
	w.show();
	return a.exec();
}

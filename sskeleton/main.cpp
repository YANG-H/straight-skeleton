#include "ssmainwind.h"
#include <QtGui/QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
	QApplication::addLibraryPath("./plugins");
	QTextCodec *codec = QTextCodec::codecForName("GB2312");	
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);

	QApplication a(argc, argv);

	QApplication::setApplicationName(QObject::tr("Straight Skeleton"));
	QApplication::setOrganizationName(QObject::tr("Institute of CG & CAD, Tsinghua University"));
	a.setFont(QFont("Microsoft Yahei", 9));

	SSMainWind w;
	w.show();
	return a.exec();
}

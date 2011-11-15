#ifndef SSMAINWIND_H
#define SSMAINWIND_H

#include <QtGui/QMainWindow>
#include "ui_ssmainwind.h"

class SSMainWind : public QMainWindow
{
	Q_OBJECT

public:
	SSMainWind(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SSMainWind();

private:
	Ui::SSMainWindClass ui;
};

#endif // SSMAINWIND_H

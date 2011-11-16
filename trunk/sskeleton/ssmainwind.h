#ifndef SSMAINWIND_H
#define SSMAINWIND_H

#include <QtGui/QMainWindow>
#include <QMdiArea>
#include "ui_ssmainwind.h"

class SSWidget;

class SSMainWind : public QMainWindow
{
	Q_OBJECT

public:
	SSMainWind(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SSMainWind();

private:
	Ui::SSMainWindClass ui;

	QMdiArea* m_mdiArea;
	SSWidget* m_sswidget;
};

#endif // SSMAINWIND_H

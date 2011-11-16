#include "ssmainwind.h"

#include <QMdiSubWindow>
#include "sswidget.h"

SSMainWind::SSMainWind(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	m_mdiArea = new QMdiArea;
	setCentralWidget(m_mdiArea);

	//QMdiSubWindow* swin = new QMdiSubWindow;
	//swin->setWidget(m_sswidget = new SSWidget);
	m_mdiArea->addSubWindow(m_sswidget = new SSWidget)->showMaximized();
}

SSMainWind::~SSMainWind()
{

}

#ifndef SSMAINWIND_H
#define SSMAINWIND_H

#include <QtGui/QMainWindow>
#include <QFileInfo>
#include <QStateMachine>
#include <QSettings>
#include "ui_ssmainwind.h"

class SSWidget;
class SS2DWidget;
class SS3DWidget;
class SSTask;
class SSTipDlg;

class SSMainWind : public QMainWindow
{
	Q_OBJECT

public:
	SSMainWind(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SSMainWind();

	void initEditors();
	void initUI();
	void updateTitle();

public slots:
	void on_actionRun_triggered();
	void on_actionSuspend_triggered();
	void on_actionStop_triggered();
	
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void on_actionSave_triggered();
	void on_actionSaveAs_triggered();
	void on_actionInfo_triggered();
	void on_actionTips_triggered();

	void on_actionRemoveVertex_triggered();
	void on_actionInsertPolygon_triggered();
	void on_actionRubberBand_triggered();
	void on_actionScrollHand_triggered();

	void on_actionTopView_triggered();
	void on_actionResetCamera_triggered();

	void updateSettings();
	void popMessage();

private:
	Ui::SSMainWindClass ui;

	//QMdiArea* m_mdiArea;
	//SSWidget* m_sswidget;//
	SS2DWidget* m_ss2dwidget;
	SS3DWidget* m_ss3dwidget;
	SSTask* m_task;

	SSTipDlg* m_tipDlg;
	QSettings* m_settings;
	//QStateMachine* m_machine;
};

#endif // SSMAINWIND_H

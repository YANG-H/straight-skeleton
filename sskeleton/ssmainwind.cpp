#include "ssmainwind.h"

#include <QMdiSubWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QFileInfo>
#include <QActionGroup>
#include <QSizePolicy>
#include <QItemEditorFactory>
#include <QTimeEdit>
#include <QDateEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QColor>
#include <QSize>
#include <QtDebug>

#include "sstask.h"
#include "sswidget.h"
#include "ss2dwidget.h"
#include "ss3dwidget.h"
#include "ssconfigdlg.h"
#include "sstipdlg.h"
#include "sswidget.h"
#include "sspropmodel.h"
#include "sstip.h"

#define FILTER_STR tr("Polygon Files (*.plgx);;Old Polygon Files (*.plg);;Text Format Polygon Files (*.poly);;All Files (*.*)")
#define OUT_FILTER_STR tr("Polygon Files (*.plgx)")

namespace Private{
	class IntEditor : public QSpinBox {
	public:
		IntEditor(QWidget* parent = 0):QSpinBox(parent){setRange(0, 5000);}
	};
	class DoubleEditor : public QDoubleSpinBox {
	public:
		DoubleEditor(QWidget* parent = 0):QDoubleSpinBox(parent){
			setRange(0, 5000); setDecimals(8); setSingleStep(0.1);
		}
	};
	class ColorListEditor : public QComboBox
	{
		Q_OBJECT
		Q_PROPERTY(QColor color READ getColor WRITE setColor USER true)

	public:
		ColorListEditor(QWidget *parent): QComboBox(parent){populateList();}
		~ColorListEditor(){}
		inline QColor getColor() const{
			return qVariantValue<QColor>(itemData(currentIndex(), Qt::DecorationRole));
		}
		inline void setColor(QColor c){
			setCurrentIndex(findData(c, int(Qt::DecorationRole)));
		}
	private:
		inline void populateList(){
			QStringList colorNames = QColor::colorNames();
			for (int i = 0; i < colorNames.size(); ++i) {
				QColor color(colorNames[i]);
				insertItem(i, colorNames[i]);
				setItemData(i, color, Qt::DecorationRole);
			}
		}
	};
}

#include "ssmainwind.moc"

SSMainWind::SSMainWind(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), 
	m_task(0), m_ss2dwidget(0), m_ss3dwidget(0), m_tipDlg(0), m_settings(0)
{
	// ui things
	ui.setupUi(this);
	
	initUI();
	initEditors();
}

SSMainWind::~SSMainWind()
{

}

void SSMainWind::updateTitle()
{
	setWindowTitle(m_ss2dwidget->fileInfo().fileName() + " - " + tr("Straight Skeleton"));
}

void SSMainWind::initEditors()
{
	// init editors
	QItemEditorFactory *factory = new QItemEditorFactory;		
	factory->registerEditor(QVariant::Color, new QStandardItemEditorCreator<Private::ColorListEditor>());
	factory->registerEditor(QVariant::Int, new QStandardItemEditorCreator<Private::IntEditor>());
	factory->registerEditor(QVariant::Double, new QStandardItemEditorCreator<Private::DoubleEditor>());
	//factory->registerEditor(QVariant::Float, new QStandardItemEditorCreator<QDoubleSpinBox>());
	factory->registerEditor(QVariant::String, new QStandardItemEditorCreator<QLineEdit>());
	factory->registerEditor(QVariant::Time, new QStandardItemEditorCreator<QTimeEdit>());
	factory->registerEditor(QVariant::Date, new QStandardItemEditorCreator<QDateEdit>());
	factory->registerEditor(QVariant::DateTime, new QStandardItemEditorCreator<QDateTimeEdit>());
	//factory->registerEditor(QVariant::StringList, new QStandardItemEditorCreator<StringListEditButton>());
	factory->registerEditor(QVariant::Bool, new QStandardItemEditorCreator<QCheckBox>());
	//factory->registerEditor(QVariant::Vector3D, new QStandardItemEditorCreator<Vector3DEdit>());
	QItemEditorFactory::setDefaultFactory(factory);
}

void SSMainWind::initUI()
{
	m_settings = new QSettings(tr("config.ini"), QSettings::IniFormat, this);
	if(!m_settings->contains(tr("showTip")))
		m_settings->setValue(tr("showTip"), true);
	if(!m_settings->contains(tr("windowSize")))
		m_settings->setValue(tr("windowSize"), QSize(1200, 900));

	// tipdlg things
	bool showTip = m_settings->value(tr("showTip")).toBool();
	QString tipContent = tr(sstip);

	m_tipDlg = new SSTipDlg(tipContent, showTip, this);
	connect(m_tipDlg, SIGNAL(settingsUpdated()), this, SLOT(updateSettings()));
	if(showTip)
		m_tipDlg->show();
	
	// statusbar things
	QProgressBar* progBar = new QProgressBar(this);
	progBar->setRange(0, 0);
	statusBar()->addPermanentWidget(progBar, 0);
	progBar->setFixedSize(200, 15);
	//progBar->hide();
	
	QLabel* statusLb = new QLabel(this);
	statusBar()->addPermanentWidget(statusLb);
	QLabel* toolLb = new QLabel(this);
	statusBar()->addPermanentWidget(toolLb);

	// center widget things
	QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
	splitter->addWidget(m_ss2dwidget = new SS2DWidget);
	splitter->addWidget(m_ss3dwidget = new SS3DWidget);
	splitter->setSizes(QList<int>() << 50 << 80);
	setCentralWidget(splitter);

	m_task = new SSTask(this);
	connect(m_task, SIGNAL(finished()), this, SLOT(popMessage()));

	m_ss2dwidget->bind(m_task);
	m_ss3dwidget->bind(m_task);
	connect(m_ss2dwidget, SIGNAL(canvasSizeChanged(const QSizeF&)), m_ss3dwidget, SLOT(resizeCanvas(const QSizeF&)));

	connect(m_task, SIGNAL(notify()), m_ss2dwidget, SLOT(updateCanvas()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(notify()), m_ss3dwidget, SLOT(update()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(started()), m_ss2dwidget, SLOT(updateCanvas()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(started()), m_ss3dwidget, SLOT(update()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(terminated()), m_ss2dwidget, SLOT(updateCanvas()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(terminated()), m_ss3dwidget, SLOT(update()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(stopped()), m_ss2dwidget, SLOT(updateCanvas()), Qt::QueuedConnection);
	connect(m_task, SIGNAL(stopped()), m_ss3dwidget, SLOT(update()), Qt::QueuedConnection);

	// dock widget things
	SSPropDock* taskPD = new SSPropDock(m_task, this);
	taskPD->setWindowTitle(tr("Algorithm Configuration"));
	addDockWidget(Qt::LeftDockWidgetArea, taskPD);
	SSPropDock* widget2DPD = new SSPropDock(m_ss2dwidget, this);
	widget2DPD->setWindowTitle(tr("2D Widget Configuration"));
	addDockWidget(Qt::LeftDockWidgetArea, widget2DPD);
	SSPropDock* widget3DPD = new SSPropDock(m_ss3dwidget, this);
	widget3DPD->setWindowTitle(tr("3D Widget Configuration"));
	addDockWidget(Qt::LeftDockWidgetArea, widget3DPD);

	taskPD->hide();
	widget2DPD->hide();
	widget3DPD->hide();


	QStateMachine* taskMachine = new QStateMachine(this);

	QState* taskStopped = new QState;
	taskStopped->assignProperty(progBar, "visible", false);
	taskStopped->assignProperty(statusLb, "text", tr("Idle... "));

	QState* taskRunning = new QState;
	taskRunning->assignProperty(progBar, "visible", true);
	taskRunning->assignProperty(statusLb, "text", tr("Computing... "));

	QState* taskSuspended = new QState;
	taskSuspended->assignProperty(statusLb, "text", tr("Suspended... "));

	taskStopped->addTransition(m_task, SIGNAL(started()), taskRunning);
	taskRunning->addTransition(m_task, SIGNAL(finished()), taskStopped);
	taskRunning->addTransition(m_task, SIGNAL(terminated()), taskStopped);
	taskRunning->addTransition(m_task, SIGNAL(suspended()), taskSuspended);
	taskSuspended->addTransition(m_task, SIGNAL(resumed()), taskRunning);
	taskSuspended->addTransition(m_task, SIGNAL(finished()), taskStopped);		
	taskSuspended->addTransition(m_task, SIGNAL(terminated()), taskStopped);

	taskMachine->addState(taskStopped);
	taskMachine->addState(taskRunning);
	taskMachine->addState(taskSuspended);

	taskMachine->setInitialState(taskStopped);
	taskMachine->start();

	QStateMachine* widgetMachine = new QStateMachine(this);

	QState* scrollHandDrag = new QState; // drag mode
	scrollHandDrag->assignProperty(toolLb, "text", tr("Scroll hand drag mode"));
	scrollHandDrag->assignProperty(m_ss2dwidget, "dragMode", QGraphicsView::ScrollHandDrag);
	scrollHandDrag->assignProperty(m_ss2dwidget, "insertState", SS2DWidget::NoInsert);
	scrollHandDrag->assignProperty(m_ss2dwidget, "mouseTracking", false);

	QState* rubberBandDrag = new QState;
	rubberBandDrag->assignProperty(toolLb, "text", tr("Rubber band drag mode"));
	rubberBandDrag->assignProperty(m_ss2dwidget, "dragMode", QGraphicsView::RubberBandDrag);
	rubberBandDrag->assignProperty(m_ss2dwidget, "insertState", SS2DWidget::NoInsert);
	rubberBandDrag->assignProperty(m_ss2dwidget, "mouseTracking", false);

	QState* insertingFirst = new QState;		
	insertingFirst->assignProperty(toolLb, "text", tr("Please input first vertex for the new polygon"));
	insertingFirst->assignProperty(m_ss2dwidget, "dragMode", QGraphicsView::NoDrag);
	insertingFirst->assignProperty(m_ss2dwidget, "insertState", SS2DWidget::InsertFirst);
	insertingFirst->assignProperty(m_ss2dwidget, "mouseTracking", true);

	QState* insertMore = new QState;
	insertMore->assignProperty(toolLb, "text", tr("Insert more vertexes to current polygon, right/middle click to finish construction"));
	insertMore->assignProperty(m_ss2dwidget, "dragMode", QGraphicsView::NoDrag);
	insertMore->assignProperty(m_ss2dwidget, "insertState", SS2DWidget::InsertMore);
	insertMore->assignProperty(m_ss2dwidget, "mouseTracking", true);

	rubberBandDrag->addTransition(ui.actionScrollHand, SIGNAL(triggered()), scrollHandDrag);
	insertingFirst->addTransition(ui.actionScrollHand, SIGNAL(triggered()), scrollHandDrag);
	insertMore->addTransition(ui.actionScrollHand, SIGNAL(triggered()), scrollHandDrag);	
	insertMore->addTransition(m_task, SIGNAL(finished()), scrollHandDrag);
	insertingFirst->addTransition(m_task, SIGNAL(finished()), scrollHandDrag);

	scrollHandDrag->addTransition(ui.actionRubberBand, SIGNAL(triggered()), rubberBandDrag);
	insertingFirst->addTransition(ui.actionRubberBand, SIGNAL(triggered()), rubberBandDrag);
	insertMore->addTransition(ui.actionRubberBand, SIGNAL(triggered()), rubberBandDrag);

	rubberBandDrag->addTransition(ui.actionInsertPolygon, SIGNAL(triggered()), insertingFirst);
	scrollHandDrag->addTransition(ui.actionInsertPolygon, SIGNAL(triggered()), insertingFirst);
	insertMore->addTransition(ui.actionInsertPolygon, SIGNAL(triggered()), insertingFirst);
	insertMore->addTransition(m_ss2dwidget, SIGNAL(polygonClosed()), insertingFirst);
	insertMore->addTransition(m_ss2dwidget, SIGNAL(polygonUnclosed()), insertingFirst);

	insertingFirst->addTransition(m_ss2dwidget, SIGNAL(firstInserted()), insertMore);

	widgetMachine->addState(scrollHandDrag);
	widgetMachine->addState(rubberBandDrag);
	widgetMachine->addState(insertingFirst);
	widgetMachine->addState(insertMore);

	widgetMachine->setInitialState(insertingFirst);
	widgetMachine->start();

	updateTitle();
}

void SSMainWind::on_actionRun_triggered()
{
	if(m_task->resume())
		return;
	QList<QPolygonF> polygons = m_ss2dwidget->toPolygons();
	if(polygons.empty() || m_task->isRunning())
		return;
	m_task->startTask(polygons);
}

void SSMainWind::on_actionSuspend_triggered()
{
	m_task->suspend();
}

void SSMainWind::on_actionStop_triggered()
{
	m_task->terminate();
}

void SSMainWind::on_actionNew_triggered()
{
	SSConfigDlg dlg(QSize(450, 650), this);
	if(dlg.exec() == QDialog::Accepted){
		m_ss2dwidget->create(dlg.resultSize());
		m_task->terminate();
		m_task->cleanSkeleton();
		m_ss2dwidget->updateCanvas();
		updateTitle();
	}
}

void SSMainWind::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File..."), QString(), 
		FILTER_STR);
	qDebug() << fileName;
	if(!fileName.isEmpty()){
		m_ss2dwidget->setFileInfo(QFileInfo(fileName));
		if(!m_ss2dwidget->load())
			QMessageBox::critical(this, tr("File Open Error..."), 
				tr(";( Sorry but we do not support this format."));
		m_task->terminate();
		m_task->cleanSkeleton();
		m_ss2dwidget->updateCanvas();
		updateTitle();
	}
}

void SSMainWind::on_actionSave_triggered()
{
	if(!m_ss2dwidget->fileExists()){
		on_actionSaveAs_triggered();
	}else{
		if(!m_ss2dwidget->save()){
			QMessageBox::critical(this, tr("File Save Error..."), 
				tr(";( Sorry but we do not support saving as this file format."));
			on_actionSaveAs_triggered();
		}
	}
}

void SSMainWind::on_actionSaveAs_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Set File Name..."), 
		m_ss2dwidget->fileInfo().fileName(), OUT_FILTER_STR);
	if(!fileName.isEmpty()){
		m_ss2dwidget->setFileInfo(QFileInfo(fileName));
		if(!m_ss2dwidget->save()){
			QMessageBox::critical(this, tr("File Save Error..."), 
				tr(";( Sorry but we can't save this file here."));
		}
		updateTitle();
	}
}

void SSMainWind::on_actionInfo_triggered()
{
	QMessageBox::about(this, tr("Infomation"), 
		tr("This application displays a Straight Skeleton generating algorithm. "
		"developers: Cao Yixi, Wu Min and Yang Hao."));
}

void SSMainWind::on_actionInsertPolygon_triggered()
{

}

void SSMainWind::on_actionRemoveVertex_triggered()
{

}

void SSMainWind::on_actionResetCamera_triggered()
{

}

void SSMainWind::on_actionTopView_triggered()
{
	m_ss3dwidget->topView();
	m_ss2dwidget->resetMatrix();
}

void SSMainWind::on_actionRubberBand_triggered()
{

}

void SSMainWind::on_actionScrollHand_triggered()
{

}

void SSMainWind::updateSettings()
{
	m_settings->setValue(tr("showTip"), m_tipDlg->alwaysShow());
}

void SSMainWind::on_actionTips_triggered()
{
	m_tipDlg->show();
}

void SSMainWind::popMessage()
{
	if(m_task->property("-Use Time Counting").toBool())
		QMessageBox::information(this, tr("Duration"), tr("Duration =  %1 seconds").arg(m_task->duration));
}
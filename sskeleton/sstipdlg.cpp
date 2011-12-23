#include "sstipdlg.h"

SSTipDlg::SSTipDlg(const QString& content, bool alwaysShow, QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* mainLy = new QVBoxLayout;

	m_tipTE = new QTextEdit;
	m_tipTE->setReadOnly(true);
	m_tipTE->setText(content);
	m_tipTE->setWhatsThis(tr("Tips to use this program"));
	mainLy->addWidget(m_tipTE);

	m_asCB = new QCheckBox(tr("Always show this tip when program starts"));
	m_asCB->setChecked(alwaysShow);
	m_asCB->setWhatsThis(tr("Check me to show tips when you start this program"));
	mainLy->addWidget(m_asCB);

	setLayout(mainLy);
	setWindowTitle(tr("Tips"));
}

SSTipDlg::~SSTipDlg()
{

}
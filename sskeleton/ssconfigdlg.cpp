#include "ssconfigdlg.h"

#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QFileDialog>

SSConfigDlg::SSConfigDlg(const QSize& init, QWidget *parent)
	: QDialog(parent)
{
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	// groupbox for widget size
	QGroupBox* sizeGB = new QGroupBox(tr("Canvas size"));
	QFormLayout* sizeLayout = new QFormLayout;
	sizeLayout->addRow(tr("width:"), m_widthSB = new QSpinBox);
	sizeLayout->addRow(tr("height:"), m_heightSB = new QSpinBox);
	sizeGB->setLayout(sizeLayout);

	// groupbox for image
	QGroupBox* imgGB = new QGroupBox(tr("Background image"));
	QVBoxLayout* imgLayout = new QVBoxLayout;
	QPushButton* imgBt = new QPushButton(tr("Click to choose an image..."));
	connect(imgBt, SIGNAL(clicked()), this, SLOT(queryBackground()));
	imgLayout->addWidget(imgBt);
	imgLayout->addWidget(m_imageLB = new QLabel);
	imgGB->setLayout(imgLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(sizeGB);
	//mainLayout->addWidget(imgGB);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);

	buttonBox->setWhatsThis(tr("Make your decision here"));
	
	sizeGB->setWhatsThis(tr("Set size of the new canvas here"));
	m_heightSB->setWhatsThis(tr("Set height of the new canvas here"));
	m_widthSB->setWhatsThis(tr("Set width of the new canvas here"));
	imgBt->setWhatsThis(tr("Click to choose an image"));
	imgGB->setWhatsThis(tr("Set background image here"));
	m_imageLB->setWhatsThis(tr("Image used as background"));

	m_heightSB->setRange(0, 5000);
	m_widthSB->setRange(0, 5000);
	m_heightSB->setValue(init.height());
	m_widthSB->setValue(init.width());

	//m_imageLB->resize(QSize());
	m_imageLB->hide();

	setWindowTitle(tr("Set attributes..."));
}

SSConfigDlg::~SSConfigDlg()
{}

QSize SSConfigDlg::resultSize() const {return QSize(m_widthSB->value(), m_heightSB->value());}
QPixmap SSConfigDlg::resultImage() const {return m_originImage;}

void SSConfigDlg::queryBackground()
{
	QString imageFile = QFileDialog::getOpenFileName(this, tr("Select an image..."), 
		QString(), tr("Image files (*.png *.jpg *.jpeg)"));
	if(!imageFile.isEmpty()){
		m_originImage.load(imageFile);
		QPixmap smallImage = m_originImage.scaledToWidth(200);
		m_imageLB->setPixmap(smallImage);
		//m_imageLB->resize(smallImage.size());
		m_imageLB->show();
	}
}

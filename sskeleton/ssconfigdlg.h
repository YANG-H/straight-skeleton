#ifndef SSCONFIGDLG_H
#define SSCONFIGDLG_H

#include <QDialog>
#include <QSpinBox>
#include <QPixmap>
#include <QLabel>

class SSConfigDlg : public QDialog
{
	Q_OBJECT

public:
	SSConfigDlg(const QSize& init = QSize(), QWidget *parent = 0);
	~SSConfigDlg();

	QSize resultSize() const;
	QPixmap resultImage() const;

private slots:
	void queryBackground();

private:
	QSpinBox* m_widthSB;
	QSpinBox* m_heightSB;
	QLabel* m_imageLB;
	QPixmap m_originImage;
};

#endif // SSCONFIGDLG_H

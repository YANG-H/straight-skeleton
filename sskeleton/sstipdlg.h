#ifndef SSTIPDLG_H
#define SSTIPDLG_H

#include <QDialog>
#include <QTextEdit>
#include <QBoxLayout>
#include <QCheckBox>

class SSTipDlg : public QDialog
{
	Q_OBJECT

public:
	SSTipDlg(const QString& content, bool alwaysShow, QWidget *parent);
	~SSTipDlg();

	inline bool alwaysShow() const {return m_asCB->isChecked();}

signals:
	void settingsUpdated();

protected:
	void closeEvent(QCloseEvent * e){
		emit settingsUpdated(); 
		QDialog::closeEvent(e);
	}

private:
	QCheckBox* m_asCB;
	QTextEdit* m_tipTE;
};

#endif // SSTIPDLG_H

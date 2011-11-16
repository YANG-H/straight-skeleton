#ifndef SSWIDGET_H
#define SSWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QVector2D>
#include <QElapsedTimer>

class SSMotor
{
public:
	inline SSMotor(const QPointF& p, const QVector2D& d = QVector2D(0, 0)) 
		: point(p), direction(d), startTime(-1){}	
	QPointF point;
	QVector2D direction;
	qint64 startTime;
};

class SSWidget : public QWidget
{
	Q_OBJECT

public:
	SSWidget(QWidget *parent = 0);
	~SSWidget();

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void advance();

private:
	double m_stepPerMS;
	QList<SSMotor> m_motors;
	QTimer* m_timer;
	QElapsedTimer m_elapsed;
};

#endif // SSWIDGET_H

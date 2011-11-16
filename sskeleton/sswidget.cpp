#include "sswidget.h"

#include <QPainter>
#include <QTimer>
#include <QElapsedTimer>
#include <QMouseEvent>

SSWidget::SSWidget(QWidget *parent)
	: QWidget(parent)
{
	m_stepPerMS = 0.01;
	m_timer = new QTimer(this);
	m_timer->setInterval(200);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(repaint()));

	m_timer->start(100);

	m_elapsed.start();
}

SSWidget::~SSWidget()
{

}

void SSWidget::paintEvent(QPaintEvent * e)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setBackground(Qt::white);
	p.eraseRect(rect());

	for (int i = 0; i < m_motors.size(); i++){
		SSMotor& m = m_motors[i];
		if(m.startTime < 0) {// inactive
			p.drawEllipse(m.point, 3, 3);
			p.setPen(Qt::DashLine);
			p.drawLine(m.point, m.point + (m.direction * width()).toPointF());
		}else{
			p.drawEllipse(m.point, 2, 2);
			p.drawLine(m.point, m.point + 
				(m.direction * (m_elapsed.elapsed() - m.startTime)  * m_stepPerMS).toPointF());	
		}
	}
}

void SSWidget::mousePressEvent(QMouseEvent * e)
{
	if(e->buttons() & Qt::LeftButton){
		m_motors.append(SSMotor(e->posF()));
		update();
	}
	QWidget::mousePressEvent(e);
}

void SSWidget::mouseMoveEvent(QMouseEvent * e)
{
	if(!m_motors.empty() && (e->buttons() | Qt::LeftButton)){
		SSMotor& m = m_motors.last();
		m.direction = QVector2D(e->posF() - m.point);
		m.direction /= m.direction.length();
		update();
	}
	QWidget::mouseMoveEvent(e);
}

void SSWidget::mouseReleaseEvent(QMouseEvent * e)
{
	if(!m_motors.empty() && m_motors.last().startTime < 0){
		if(m_motors.last().direction == QVector2D())
			m_motors.removeLast();
		else
			m_motors.last().startTime = m_elapsed.elapsed();
		update();
	}
	QWidget::mouseReleaseEvent(e);
}
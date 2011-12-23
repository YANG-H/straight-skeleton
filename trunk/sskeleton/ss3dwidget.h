#ifndef SS3DWIDGET_H
#define SS3DWIDGET_H

#include <QGLWidget>
#include <QMatrix4x4>

class SSTask;
class SSWidget;

class SS3DWidget : public QGLWidget
{
	Q_OBJECT
public:
	SS3DWidget(QWidget *parent = 0);
	~SS3DWidget();

	inline void bind(SSTask* t){m_task = t;}

public slots:
	void resizeCanvas(const QSizeF& mapSize);
	void construct();
	void topView();

protected:
	void initializeGL();
	void paintGL();
	void paintTerrain();
	void resizeGL(int w, int h);

	void translate(const QVector3D& t)
		{QMatrix4x4 m; m.translate(t); m_modelmat = m * m_modelmat;}
	inline void rotate(double angle, const QVector3D& v)
		{QMatrix4x4 m; m.rotate(angle, v); m_modelmat = m * m_modelmat;}
	
	void camMoveAroundCenter(const QVector3D& trans);
	void camMoveCenter(const QVector3D& trans);
	inline void camZoom(double d){camMoveAroundCenter(QVector3D(0, 0, d));}

	void mousePressEvent(QMouseEvent * e);
	void mouseMoveEvent(QMouseEvent * e);
	void mouseReleaseEvent(QMouseEvent * e);
	void wheelEvent(QWheelEvent * e);

private:
	SSTask* m_task;

	QMatrix4x4 m_modelmat;
	QMatrix4x4 m_cammat;
	QMatrix4x4 m_projmat;
	QVector3D m_cam_center, m_cam_eye, m_cam_up;

	QSizeF m_mapSize;
	QPixmap m_mapTex;
	QPointF m_mouseLastPos;
};

#endif // SS3DWIDGET_H

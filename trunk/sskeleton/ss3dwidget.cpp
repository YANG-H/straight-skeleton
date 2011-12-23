#include "ss3dwidget.h"

#include <QMouseEvent>
#include <QVector3D>
#include <GL/glu.h>

#include "sstask.h"

inline void _FillArray(const QColor& c, float* arr)
{
	arr[0] = c.redF();
	arr[1] = c.greenF();
	arr[2] = c.blueF();
	arr[3] = c.alphaF();
}

SS3DWidget::SS3DWidget(QWidget *parent)
	: QGLWidget(parent), m_task(0), m_mapSize(450, 650)
{
	m_cam_eye = QVector3D(0, 0, 980);
	m_cam_center = QVector3D(0, 0, 0);
	m_cam_up = QVector3D(0, -1, 0);

	m_projmat.perspective(45, 1.0, 0.01, 5000);

	//setProperty("-Background Color", QColor(Qt::gray));

	//setProperty("-Base Material Diffuse", QColor(200, 200, 200, 255));
	//setProperty("-Base Material Ambient", QColor(180, 180, 180, 200));
	//setProperty("-Base Material Specular", QColor(180, 180, 180, 200));
	//setProperty("-Base Material Shininess", 0.7);

	setProperty("-Terrain Material Diffuse", QColor(100, 100, 100, 255));
	setProperty("-Terrain Material Ambient", QColor(80, 80, 80, 200));
	setProperty("-Terrain Material Specular", QColor(80, 80, 80, 200));
	setProperty("-Terrain Material Shininess", 2.0);

	//setProperty("-Terrain Material Position", 2.0);
	setProperty("-Light Material Diffuse", QColor(100, 100, 100, 255));
	setProperty("-Light Material Ambient", QColor(80, 80, 80, 200));
	setProperty("-Light Material Specular", QColor(80, 80, 80, 20));

}

SS3DWidget::~SS3DWidget()
{

}

void SS3DWidget::resizeCanvas(const QSizeF& mapSize)
{
	m_mapSize = mapSize;
	update();
}

void SS3DWidget::construct()
{

}

void SS3DWidget::topView()
{
	m_cam_eye = QVector3D(0, 0, 980);
	m_cam_center = QVector3D(0, 0, 0);
	m_cam_up = QVector3D(0, -1, 0);
	update();
}

void SS3DWidget::initializeGL()
{
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
 
	glEnable(GL_BLEND); 

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

void SS3DWidget::paintGL()
{
	static const GLfloat bgmat_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	static const GLfloat bgmat_ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	static const GLfloat bgmat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	static const GLfloat bgmat_shininess = 1.0f;

	//_FillArray(property("-Base Material Diffuse").value<QColor>(), bgmat_diffuse);
	//_FillArray(property("-Base Material Ambient").value<QColor>(), bgmat_ambient);
	//_FillArray(property("-Base Material Specular").value<QColor>(), bgmat_specular);
	//bgmat_shininess = property("-Base Material Shininess").toDouble();
		
	static GLfloat mat_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
	static GLfloat mat_ambient[] = {0.4f, 0.4f, 0.4f, 0.8f};
	static GLfloat mat_specular[] = {0.2f, 0.2f, 0.2f, 0.8f};
	static GLfloat mat_shininess = 1.0f;

	_FillArray(property("-Terrain Material Diffuse").value<QColor>(), mat_diffuse);
	_FillArray(property("-Terrain Material Ambient").value<QColor>(), mat_ambient);
	_FillArray(property("-Terrain Material Specular").value<QColor>(), mat_specular);
	mat_shininess = property("-Terrain Material Shininess").toDouble();

	// π‚’’
	static GLfloat light_position[] = {-1.5f, 1.5f, 1.5f, 0.0f};
	static GLfloat light_ambient[] = {.5f, .5f, .5f, 1.0f};
	static GLfloat light_diffuse[] = {.5f, .5f, .5f, 1.0f};
	static GLfloat light_specular[] = {0.5f, 0.5f, 0.5f, 0.2f};

	_FillArray(property("-Light Material Diffuse").value<QColor>(), light_diffuse);
	_FillArray(property("-Light Material Ambient").value<QColor>(), light_ambient);
	_FillArray(property("-Light Material Specular").value<QColor>(), light_specular);


	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	qglClearColor(Qt::gray);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();

	m_cammat.setToIdentity();
	m_cammat.lookAt(m_cam_eye, m_cam_center, m_cam_up);
	glMultMatrixd(m_cammat.constData());
	glMultMatrixd(m_modelmat.constData());


	glMaterialfv(GL_FRONT, GL_AMBIENT, bgmat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bgmat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, bgmat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, bgmat_shininess);

	glBegin(GL_LINES);
	glNormal3d(0, 0, 1);
	glVertex3d(- m_mapSize.width()/2.0, - m_mapSize.height()/2.0, -10);
	glVertex3d(- m_mapSize.width()/2.0, m_mapSize.height()/2.0, -10);
	glVertex3d(- m_mapSize.width()/2.0, m_mapSize.height()/2.0, -10);
	glVertex3d(m_mapSize.width()/2.0, m_mapSize.height()/2.0, -10);
	glVertex3d(m_mapSize.width()/2.0, m_mapSize.height()/2.0, -10);
	glVertex3d(m_mapSize.width()/2.0, - m_mapSize.height()/2.0, -10);
	glVertex3d(m_mapSize.width()/2.0, - m_mapSize.height()/2.0, -10);
	glVertex3d(- m_mapSize.width()/2.0, - m_mapSize.height()/2.0, -10);
	glEnd();

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	paintTerrain();

	glPopMatrix();
}

void SS3DWidget::paintTerrain()
{
	//qglColor(Qt::white);
	//qglColor(Qt::darkCyan);
	glScaled(-1, 1, 1);
	//glTranslated(- m_mapSize.width()/2.0, - m_mapSize.height()/2.0, 0);

	//m_task->drawSkeletons3D();
	m_task->drawMeshes3D(m_mapSize);

	//glTranslated(m_mapSize.width()/2.0, m_mapSize.height()/2.0, 0);
	glScaled(-1, 1, 1);
}

void SS3DWidget::resizeGL(int w, int h)
{
	int side = qMax(w, h);
	glViewport((w - side) / 2, (h - side) / 2, side, side);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	glMultMatrixd(m_projmat.constData());
	glMatrixMode(GL_MODELVIEW);
}

void SS3DWidget::camMoveAroundCenter(const QVector3D& tt)
{
	QVector3D xv = QVector3D::crossProduct((m_cam_center - m_cam_eye), m_cam_up).normalized();
	QVector3D yv = QVector3D::crossProduct(xv, (m_cam_center - m_cam_eye)).normalized();
	QVector3D xyTrans = xv * tt.x() + yv * tt.y();
	double r = (m_cam_eye - m_cam_center).length() * (1 - 0.1 * tt.z()) / 
		(m_cam_eye + xyTrans - m_cam_center).length();
	QVector3D new_cam_eye = (m_cam_eye + xyTrans - m_cam_center) * r + m_cam_center;
	m_cam_up = yv.normalized();

	if(!qFuzzyCompare((new_cam_eye - m_cam_center).normalized(), m_cam_up.normalized()))
		m_cam_eye = new_cam_eye;
}

void SS3DWidget::camMoveCenter(const QVector3D& tt)
{
	QVector3D xv = QVector3D::crossProduct((m_cam_center - m_cam_eye), m_cam_up).normalized();
	QVector3D yv = QVector3D::crossProduct(xv, (m_cam_center - m_cam_eye)).normalized();
	QVector3D zv = (m_cam_center  - m_cam_eye).normalized();
	QVector3D trans = xv * tt.x() + yv * tt.y() + zv * tt.z();
	m_cam_eye += trans;
	m_cam_center += trans;
}

void SS3DWidget::mousePressEvent(QMouseEvent * e)
{
	m_mouseLastPos = e->posF();
	if(e->buttons() & Qt::LeftButton)
		setCursor(Qt::OpenHandCursor);
	else if(e->buttons() & Qt::MidButton)
		setCursor(Qt::SizeAllCursor);
	else
		setCursor(Qt::ArrowCursor);
}

void SS3DWidget::mouseMoveEvent(QMouseEvent * e)
{
	QVector3D t(e->posF() - m_mouseLastPos);
	t.setX( - t.x());
	if(e->buttons() & Qt::LeftButton){
		camMoveAroundCenter(t);
		setCursor(Qt::ClosedHandCursor);
		update();
	}else if(e->buttons() & Qt::MidButton){
		camMoveCenter(t);
		update();
	}
	m_mouseLastPos = e->posF();
}

void SS3DWidget::mouseReleaseEvent(QMouseEvent * e)
{
	setCursor(Qt::ArrowCursor);
}

void SS3DWidget::wheelEvent(QWheelEvent * e)
{
	camZoom(e->delta() / 1000.0);
	update();
}
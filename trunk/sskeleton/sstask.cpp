#include "sstask.h"

#include <QPainter>
#include <QtOpenGL/QtOpenGL>

#include "pslgsskeleton.h"
#include "pslggraph.h"

SSTask::SSTask(QObject *parent)
	: QThread(parent), m_skeleton(0), m_timeIntervalIsZero(false), 
	mutex(QReadWriteLock::Recursive), m_isWaiting(0), m_destroyed(false)
{
	setProperty("-Motor Only", false);
	setProperty("-Sleep Interval (ms)", 5);
	setProperty("-Update Step Length", 1.0);
}

SSTask::~SSTask()
{
	m_destroyed = true;
}

bool SSTask::suspend()
{
	if(isRunning()){
		m_isWaiting = true;
		emit suspended();
		return true;
	}
	return false;
}

bool SSTask::resume()
{
	if(!m_isWaiting)
		return false;
	m_isWaiting = false;
	waitCondition.wakeOne();
	if(isRunning())
		emit resumed();
	return true;
}

void SSTask::updateData()
{
	if(m_destroyed)
		return;

	if(m_isWaiting){
		waitMutex.lock();
		waitCondition.wait(& waitMutex);
		waitMutex.unlock();
	}

	mutex.lockForWrite(); // thread safe !
	if(m_skeleton){
		m_skeletons_cache = m_skeleton->skeleton;
		m_skeletons_cache.detach();
		m_wavefronts_cache << m_skeleton->wavefront;
		m_wavefronts_cache.last().detach();
		m_meshes_cache = m_skeleton->meshes;
		m_meshes_cache.detach();
	}else{
		m_skeletons_cache.clear();
		m_wavefronts_cache.clear();
	}
	mutex.unlock();
}

void SSTask::drawSkeletons(QPainter* painter) const
{
	mutex.lockForRead(); // thread safe !
	foreach(QLineF line, m_skeletons_cache)
		if(line.length() >= 2)
			painter->drawLine(line);
	mutex.unlock();
}

void SSTask::drawSkeletons3D() const
{
	double ti = property("-Update Step Length").toDouble();
	glBegin(GL_QUADS);
	mutex.lockForRead();
	for(int i = 0; i < m_wavefronts_cache.size(); i++){
		QList<QLineF> s = m_wavefronts_cache.at(i);
		for(int j = 0; j < s.size(); j++){
			QPointF& a = s.at(j).p1();
			QPointF& b = s.at(j).p2();
			QVector3D v(b - a);
			v.normalize();
			v = QVector3D::crossProduct(v, QVector3D(0, 0, -1));
			v.setZ(1);
			glNormal3d(v.x(), v.y(), v.z());
			glVertex3d(a.x(), a.y(), ti * i);
			glVertex3d(b.x(), b.y(), ti * i);
			glVertex3d(b.x(), b.y(), ti * i + ti);
			glVertex3d(a.x(), a.y(), ti * i + ti);
		}
	}
	mutex.unlock();
	glEnd();
}

void SSTask::drawMeshes3D( const QSizeF& mapSize ) const
{
	QRectF boundingRect(QPointF( -mapSize.width()/2.0, -mapSize.height()/2.0), mapSize);
	glBegin(GL_TRIANGLES);
	mutex.lockForRead();
	for(int i = 0; i < m_meshes_cache.size(); i++){
		Triangle3D t = m_meshes_cache.at(i);
		if(!boundingRect.contains(t.v1.toPointF()) 
			|| !boundingRect.contains(t.v2.toPointF()) 
			|| !boundingRect.contains(t.v3.toPointF()))
			continue;

		QVector3D normal = QVector3D::crossProduct(t.v2 - t.v1, t.v3 - t.v1);
		if(normal.z() < 0){
			t = Triangle3D(t.v3, t.v2, t.v1);
			normal.setZ( - normal.z());
		}
		glNormal3d(normal.x(), normal.y(), normal.z());
		glVertex3d(t.v1.x(), t.v1.y(), t.v1.z());
		glVertex3d(t.v2.x(), t.v2.y(), t.v2.z());
		glVertex3d(t.v3.x(), t.v3.y(), t.v3.z());
	}
	mutex.unlock();
	glEnd();
}

void SSTask::drawWaveFronts(QPainter* painter) const
{
	mutex.lockForRead();
	if(!m_wavefronts_cache.empty())
	foreach(QLineF line, m_wavefronts_cache.last())
		if(line.length() >= 2)
			painter->drawLine(line);
	mutex.unlock(); // thread safe !
}

void SSTask::startTask(const QList<QPolygonF>& polygons)
{
	cleanSkeleton();
	resume();
	PSLGGraph* graph = new PSLGGraph();
	for(int i = 0; i < polygons.size(); i++){
		if(polygons[i].size() <= 1 || polygons[i].boundingRect().isEmpty())
			continue;
		graph->addPolygon(polygons[i], false );
	}
	m_timeIntervalIsZero = property("-Sleep Interval (ms)").toInt() <= 0;
	m_skeleton = new PSLGSSkeleton(graph);
	start();	
}

void SSTask::run()
{
	if(!m_skeleton)
		return;
	
	m_skeleton->extractSkeletion(this);
}
#ifndef SSTASK_H
#define SSTASK_H

#include <QThread>
#include <QPolygonF>
#include <QProgressBar>
#include <QVariant>
#include <QWidget>
#include <QMutex>
#include <QReadWriteLock>
#include <QWaitCondition>

#include "pslgsskeleton.h"

class SSTask : public QThread
{
	Q_OBJECT
public:
	SSTask(QObject *parent);
	~SSTask();

	// called within another thread
	inline void notifyWidget(){updateData(); emit notify();}
	inline void ssmsleep(ulong t){if(!m_timeIntervalIsZero) msleep(t);}
	void updateData();

	// called within main thread
	void startTask(const QList<QPolygonF>& polygons);

	inline QList<QLineF> skeletons() const 
	{return m_skeletons_cache;}
	inline QList<QLineF> wavefronts() const 
	{return m_wavefronts_cache.empty() ? QList<QLineF>() : m_wavefronts_cache.last();}
	inline QList<Triangle3D> meshes() const
	{return m_meshes_cache;}
	
	void drawSkeletons(QPainter* p) const;
	void drawSkeletons3D() const;
	void drawWaveFronts(QPainter* p) const;
	void drawMeshes3D(const QSizeF& mapSize) const;

	inline bool isWaiting() const {return m_isWaiting;}

signals:
	void notify();
	void suspended();
	void resumed();

public slots:
	inline void cleanSkeleton(){
		delete m_skeleton; m_skeleton = 0; 
		m_skeletons_cache.clear(); 
		m_wavefronts_cache.clear();
		m_meshes_cache.clear();
	}

	bool suspend();
	bool resume();

protected:
	void run();

private:
	PSLGSSkeleton* m_skeleton;
	QList<QLineF> m_skeletons_cache;
	QList<QList<QLineF>> m_wavefronts_cache;
	QList<Triangle3D> m_meshes_cache;
	QWidget* m_widget;
	bool m_timeIntervalIsZero;

	mutable QReadWriteLock mutex;
	mutable QMutex waitMutex;
	QWaitCondition waitCondition;
	bool m_isWaiting;

	bool m_destroyed;
};

#endif // SSTASK_H

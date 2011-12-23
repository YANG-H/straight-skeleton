/**
 * 提取平面直线图的直骨架
 * @author wumabert, 20111129
 */

#ifndef PSLGSSKELETION_H
#define PSLGSSKELETION_H

#include <QLine>
#include "pslggraph.h"
#include "priorityqueue.h"
#include "Triangle3D.h"

class SSTask;

enum PSLGEventType {
	EDGE_EVENT,
	SPLIT_EVENT,
	START_EVENT,
	SWITCH_EVENT,
	MULTI_START_EVENT,
	MULTI_SPLIT_EVENT,
	OTHER_EVENT
};

class PSLGSSkeleton{
private:
	PSLGGraph* ssgraph;
	//所有边构成的事件队列
    PriorityQueue* eventQueue;

public:
	QList<QLineF> skeleton; //直骨架
    QList<QLineF> wavefront; //波前
	QList<Triangle3D> meshes; //三角剖分
	static PSLGEventType eventTypes[6][6];

public:
	PSLGSSkeleton();
	PSLGSSkeleton( PSLGGraph* g);
	~PSLGSSkeleton();
	//事件模拟
	void extractSkeletion();
	void extractSkeletion(SSTask* t);
	void showMotorGraph( bool twoside );

private:
	void handleEvent(PSLGEdge* event);
	void handleEdgeEvent(PSLGEdge* event);
	void handleSplitEvent(PSLGEdge* event);
	void handleStartEvent(PSLGEdge* event);
	void handleSwitchEvent(PSLGEdge* event);
	void handleMultiStartEvent(PSLGEdge* event);
	void handleMultiSplitEvent(PSLGEdge* event);
	void handleRemainingEvent( PSLGEdge* event );
	void addTriangleMesh( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 );
	void addTriangleMesh( PSLGVertex* v1, PSLGVertex* v2, const QPointF& p3, double t3 );
	void addTriangleMesh( const QPointF& p1, double t1, const QPointF& p2, double t2, const QPointF& p3, double t3 );
};

#endif //PSLGSSKELETION_H
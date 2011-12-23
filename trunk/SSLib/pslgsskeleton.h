/**
 * ��ȡƽ��ֱ��ͼ��ֱ�Ǽ�
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
	//���б߹��ɵ��¼�����
    PriorityQueue* eventQueue;

public:
	QList<QLineF> skeleton; //ֱ�Ǽ�
    QList<QLineF> wavefront; //��ǰ
	QList<Triangle3D> meshes; //�����ʷ�
	static PSLGEventType eventTypes[6][6];

public:
	PSLGSSkeleton();
	PSLGSSkeleton( PSLGGraph* g);
	~PSLGSSkeleton();
	//�¼�ģ��
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
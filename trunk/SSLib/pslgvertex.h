/**
 * Notations amd data structures of planar straight line graph (PSLG)
 * Vertex in PSLG
 * @author wumabert, 20111127
 */

#ifndef PSLGVERTEX_H
#define PSLGVERTEX_H

#include <vector>
#include <QPointF>
#include <QVector2D>

#include "sslib_global.h"

class PSLGEdge;

enum PSLGVertexType {
	REFLEX_VERTEX,
	CONVEX_VERTEX,
	MOVING_STEINER_VERTEX,
	MULTI_STEINER_VERTEX,
	RESTING_STEINER_VERTEX,
	OTHER_VERTEX
};

// ����ڵ�
class SSLIB_EXPORT PSLGVertex {
public:
	bool mark; //����ı�Ƿ���
	QPointF oriPosition; //�������ʼλ��
    QVector2D speed; //������ٶ�����
	double startTime; //�������ʼʱ��
	
	PSLGEdge* firstin;  //��ʱ�����õ�ıߣ��뻡
	PSLGEdge* firstout; //��ʱ��Ӹõ�����ıߣ�����
	PSLGVertexType type; //���������

public:
	PSLGVertex();
	PSLGVertex(const QPointF& p);
	PSLGVertex(const QPointF&p, double time);
	PSLGVertex(const QPointF& p, const QVector2D& v, double time);
	PSLGVertex(const PSLGVertex& vex);
	~PSLGVertex();

	QPointF movedPosition(double curtime);
	//�õ������������ߡ�����
	void getVertexInEdges( std::vector< PSLGEdge* > &ins );
	void getVertexOutEdges( std::vector< PSLGEdge* > &outs );
	void getVertexIncidentEdges( std::vector< PSLGEdge* > &es );
	void getVertexAdjacentVertexes( std::vector< PSLGVertex* > &vs );
	static bool equalPosition(const QPointF& p1, const QPointF& p2);
};
#endif //PSLGVERTEX_H
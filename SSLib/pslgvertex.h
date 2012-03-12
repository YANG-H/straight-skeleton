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

// 顶点节点
class SSLIB_EXPORT PSLGVertex {
public:
	bool mark; //顶点的标记访问
	QPointF oriPosition; //顶点的起始位置
    QVector2D speed; //顶点的速度向量
	double startTime; //顶点的起始时间
	
	PSLGEdge* firstin;  //逆时针进入该点的边，入弧
	PSLGEdge* firstout; //逆时针从该点出发的边，出弧
	PSLGVertexType type; //顶点的类型

public:
	PSLGVertex();
	PSLGVertex(const QPointF& p);
	PSLGVertex(const QPointF&p, double time);
	PSLGVertex(const QPointF& p, const QVector2D& v, double time);
	PSLGVertex(const PSLGVertex& vex);
	~PSLGVertex();

	QPointF movedPosition(double curtime);
	//得到顶点的所有入边、出边
	void getVertexInEdges( std::vector< PSLGEdge* > &ins );
	void getVertexOutEdges( std::vector< PSLGEdge* > &outs );
	void getVertexIncidentEdges( std::vector< PSLGEdge* > &es );
	void getVertexAdjacentVertexes( std::vector< PSLGVertex* > &vs );
	static bool equalPosition(const QPointF& p1, const QPointF& p2);
};
#endif //PSLGVERTEX_H
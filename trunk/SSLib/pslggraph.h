/**
 * Notations amd data structures of planar straight line graph (PSLG)
 * @author wumabert, 20111127
 */

#ifndef PSLGGRAPH_H
#define PSLGGRAPH_H

#include <QPointF>
#include <QVector2D>
#include <QPolygonF>
#include <vector>

#include "pslgvertex.h"
#include "pslgedge.h"
#include "priorityqueue.h"

class PSLGGraph {
public:
	std::vector<PSLGEdge*> edgeList;

public:
	PSLGGraph();
	~PSLGGraph();

public:
	//计算p1、p2、p3三个点构成矩阵的行列式，根据这个判断中间p2点的凹凸性
	static double determinant(const QPointF& p1, const QPointF& p2, const QPointF& p3);
	static double determinant(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3);
	static double determinant(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3, double t);
	//根据相邻点p1p3计算点p2的速度向量
	static QVector2D calcVertexSpeed(const QPointF& p1, const QPointF& p2, const QPointF& p3);
	static void calcConvexVertexSpeed( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 );
	static void calcConvexVertexSpeed( PSLGVertex* v );
	//计算度数为1的顶点的速度向量
	static void calcTerminalSpeed(const QPointF& p1, const QPointF& p2, QVector2D& v1, QVector2D& v2);
	//计算顶点的属性，包括凹凸性和速度向量
	static void calcVertexProperty(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3);
	static void calcVertexProperty(PSLGVertex* v);
	//计算边的消失时间
	static double calcEdgeVanishTime(PSLGEdge* e);
	//得到一个顶点在多边形上的入边和出边
	static void getVertexPolygonalInAndOutEdge( PSLGVertex* v, PSLGEdge* &in, PSLGEdge* &out );
	//计算一个MovingSteinerVertex的速度
	//mpv, moving steriner vertex; mv，mpv对应的摩托图边的另一个端点; pv, mpv对应的多边形边的另一个端点
	static void calcMovingSteinerSpeed( PSLGVertex* mpv, PSLGVertex* mv, PSLGVertex* pv );

public:
	//添加一个多边形，根据多边形创建邻接十字链表
	void addPolygon(const QPolygonF& poly);
	void addPolygon( const QPolygonF& poly, bool twoside );
	void addPolygon(const QPolygonF& poly, std::vector< PSLGEdge* > &edges);
	//计算摩托图并把摩托图中的边加到结构中
	void addMotorcycle();
	void addMotorcycle( bool twoside );
	void getMotorGraph(  std::vector< QLineF > &motorLines  );
	void getMotorGraph(  std::vector< QLineF > &motorLines, bool twoside  );
};

#endif //PSLGGRAPH_H
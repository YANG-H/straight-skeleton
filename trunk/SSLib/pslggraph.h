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
	//����p1��p2��p3�����㹹�ɾ��������ʽ����������ж��м�p2��İ�͹��
	static double determinant(const QPointF& p1, const QPointF& p2, const QPointF& p3);
	static double determinant(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3);
	static double determinant(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3, double t);
	//�������ڵ�p1p3�����p2���ٶ�����
	static QVector2D calcVertexSpeed(const QPointF& p1, const QPointF& p2, const QPointF& p3);
	static void calcConvexVertexSpeed( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 );
	static void calcConvexVertexSpeed( PSLGVertex* v );
	//�������Ϊ1�Ķ�����ٶ�����
	static void calcTerminalSpeed(const QPointF& p1, const QPointF& p2, QVector2D& v1, QVector2D& v2);
	//���㶥������ԣ�������͹�Ժ��ٶ�����
	static void calcVertexProperty(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3);
	static void calcVertexProperty(PSLGVertex* v);
	//����ߵ���ʧʱ��
	static double calcEdgeVanishTime(PSLGEdge* e);
	//�õ�һ�������ڶ�����ϵ���ߺͳ���
	static void getVertexPolygonalInAndOutEdge( PSLGVertex* v, PSLGEdge* &in, PSLGEdge* &out );
	//����һ��MovingSteinerVertex���ٶ�
	//mpv, moving steriner vertex; mv��mpv��Ӧ��Ħ��ͼ�ߵ���һ���˵�; pv, mpv��Ӧ�Ķ���αߵ���һ���˵�
	static void calcMovingSteinerSpeed( PSLGVertex* mpv, PSLGVertex* mv, PSLGVertex* pv );

public:
	//���һ������Σ����ݶ���δ����ڽ�ʮ������
	void addPolygon(const QPolygonF& poly);
	void addPolygon( const QPolygonF& poly, bool twoside );
	void addPolygon(const QPolygonF& poly, std::vector< PSLGEdge* > &edges);
	//����Ħ��ͼ����Ħ��ͼ�еı߼ӵ��ṹ��
	void addMotorcycle();
	void addMotorcycle( bool twoside );
	void getMotorGraph(  std::vector< QLineF > &motorLines  );
	void getMotorGraph(  std::vector< QLineF > &motorLines, bool twoside  );
};

#endif //PSLGGRAPH_H
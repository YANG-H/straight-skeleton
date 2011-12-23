/**
 * Notations amd data structures of planar straight line graph (PSLG)
 * Edge in PSLG
 * @author wumabert, 20111127
 */

#ifndef PSLGEDGE_H
#define PSLGEDGE_H

class PSLGVertex;

enum PSLGEdgeType {
	POLYGON_EDGE,
	MOTORCYCLE_EDGE
};
// �߽ڵ㣬ÿһ���߶��з��򣬴ӻ�β����ͷΪ��ʱ�뷽��
class PSLGEdge {
public:
	bool mark; //�Ƿ�ɾ�ı�־
	PSLGVertex* tailvex; //��β�����
	PSLGVertex* headvex; //��ͷ���յ�
	PSLGEdge* tedge; //����β�ı�
	PSLGEdge* hedge; //����ͷ�ı�
	double vanishTime; //����ʧ��ʱ��
	PSLGEdgeType type; //�ߵ�����
	int heapIndex; //������С���ȶ����е�����ֵ

public:
	PSLGEdge();
	PSLGEdge( PSLGVertex* tv, PSLGVertex* hv );
	PSLGEdge(PSLGVertex* tv, PSLGVertex* hv, PSLGEdge* te,  PSLGEdge* he);
	~PSLGEdge();

	static bool isParallel ( PSLGEdge* e1, PSLGEdge* e2 );
	static bool isVertexOnEdge( PSLGVertex* v, PSLGEdge* e );
};
#endif  //PSLGEDGE_H
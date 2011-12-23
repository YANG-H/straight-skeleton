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
// 边节点，每一条边都有方向，从弧尾到虎头为逆时针方向
class PSLGEdge {
public:
	bool mark; //是否被删的标志
	PSLGVertex* tailvex; //弧尾，起点
	PSLGVertex* headvex; //弧头，终点
	PSLGEdge* tedge; //共弧尾的边
	PSLGEdge* hedge; //共弧头的边
	double vanishTime; //边消失的时间
	PSLGEdgeType type; //边的类型
	int heapIndex; //边在最小优先队列中的索引值

public:
	PSLGEdge();
	PSLGEdge( PSLGVertex* tv, PSLGVertex* hv );
	PSLGEdge(PSLGVertex* tv, PSLGVertex* hv, PSLGEdge* te,  PSLGEdge* he);
	~PSLGEdge();

	static bool isParallel ( PSLGEdge* e1, PSLGEdge* e2 );
	static bool isVertexOnEdge( PSLGVertex* v, PSLGEdge* e );
};
#endif  //PSLGEDGE_H
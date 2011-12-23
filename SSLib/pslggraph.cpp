#include "pslggraph.h"
#include "Motorcycle.h"
#include "CrashPoint.h"
#include "MAlgorithm.h"

#define eps 1.0e-6

PSLGGraph::PSLGGraph() : edgeList(NULL) {

}

PSLGGraph::~PSLGGraph() {
	edgeList.clear();
}

void PSLGGraph::addPolygon(const QPolygonF& poly) {
	//多边形没有顶点时，将不作任何处理
	if (poly.empty()) return;

	bool closed = poly.isClosed();
	int vexNum = poly.size();
	int edgeNum = poly.size()-1;
	if (closed) {
		vexNum = vexNum - 1;
	}
	//多边形只有一个顶点时，将不作任何处理
	if (vexNum == 1) return;

	PSLGVertex** vertices = new PSLGVertex*[vexNum];
	PSLGEdge** edges = new PSLGEdge*[edgeNum];
	//多边形只有两个顶点时
	if (vexNum == 2) {
		vertices[0] = new PSLGVertex();
		vertices[0]->oriPosition = poly.at(0);
		vertices[1] = new PSLGVertex();
		vertices[1]->oriPosition = poly.at(1);
		edges[0] = new PSLGEdge();
		edges[0]->tailvex = vertices[0];
		edges[0]->headvex = vertices[1];
		edges[0]->type = PSLGEdgeType::POLYGON_EDGE;
		vertices[0]->firstout = edges[0];
		vertices[1]->firstin = edges[0];

		vertices[0]->type = PSLGVertexType::REFLEX_VERTEX;
		vertices[1]->type = PSLGVertexType::REFLEX_VERTEX;
		calcTerminalSpeed(vertices[0]->oriPosition, vertices[1]->oriPosition, vertices[0]->speed, vertices[1]->speed);
	} else {
		//为多边形的顶点建立对应的PSLGVertex顶点
		for (int i = 0; i < vexNum; i++) {
			vertices[i] = new PSLGVertex();
			vertices[i]->oriPosition = poly.at(i);
		}
		//为多边形的边建立对应的PSLGEdge边
		for (int i = 0; i < edgeNum; i++) {
			edges[i] = new PSLGEdge();
			edges[i]->tailvex = vertices[i];
			edges[i]->headvex = vertices[(i+1)%vexNum];
			edges[i]->type = PSLGEdgeType::POLYGON_EDGE;
			vertices[i]->firstout = edges[i];
			vertices[(i+1)%vexNum]->firstin = edges[i];
		}
		//计算顶点的速度和类型，端点处需单独处理
		for (int i = 1; i < vexNum-1; i++) {
			calcVertexProperty(vertices[i-1], vertices[i], vertices[i+1]);
		}
		if (closed) {
			calcVertexProperty(vertices[vexNum-1], vertices[0], vertices[1]);
			calcVertexProperty(vertices[vexNum-2], vertices[vexNum-1], vertices[0]);
		} else {
			vertices[0]->type = PSLGVertexType::REFLEX_VERTEX;
			QVector2D t;
			calcTerminalSpeed(vertices[0]->oriPosition, vertices[1]->oriPosition, vertices[0]->speed, t);
			vertices[vexNum-1]->type = PSLGVertexType::REFLEX_VERTEX;
			calcTerminalSpeed(vertices[vexNum-2]->oriPosition, vertices[vexNum-1]->oriPosition, t, vertices[vexNum-1]->speed);
		}
	}

	//计算每条边的消失时间
	for (int i = 0; i < edgeNum; i++) {
		edges[i]->vanishTime = calcEdgeVanishTime(edges[i]);
		edgeList.push_back(edges[i]);
	}
	delete [] vertices;
	delete [] edges;

	//addMotorcycle();
}

void PSLGGraph::addPolygon(const QPolygonF& poly, std::vector< PSLGEdge* > &elist ) {
	//多边形没有顶点时，将不作任何处理
	if (poly.empty()) return;

	bool closed = poly.isClosed();
	int vexNum = poly.size();
	int edgeNum = poly.size()-1;
	if (closed) {
		vexNum = vexNum - 1;
	}
	//多边形只有一个顶点时，将不作任何处理
	if (vexNum == 1) return;

	PSLGVertex** vertices = new PSLGVertex*[vexNum];
	PSLGEdge** edges = new PSLGEdge*[edgeNum];
	//多边形只有两个顶点时
	if (vexNum == 2) {
		vertices[0] = new PSLGVertex();
		vertices[0]->oriPosition = poly.at(0);
		vertices[1] = new PSLGVertex();
		vertices[1]->oriPosition = poly.at(1);
		edges[0] = new PSLGEdge();
		edges[0]->tailvex = vertices[0];
		edges[0]->headvex = vertices[1];
		edges[0]->type = PSLGEdgeType::POLYGON_EDGE;
		vertices[0]->firstout = edges[0];
		vertices[1]->firstin = edges[0];

		vertices[0]->type = PSLGVertexType::REFLEX_VERTEX;
		vertices[1]->type = PSLGVertexType::REFLEX_VERTEX;
		calcTerminalSpeed(vertices[0]->oriPosition, vertices[1]->oriPosition, vertices[0]->speed, vertices[1]->speed);
	} else {
		//为多边形的顶点建立对应的PSLGVertex顶点
		for (int i = 0; i < vexNum; i++) {
			vertices[i] = new PSLGVertex();
			vertices[i]->oriPosition = poly.at(i);
		}
		//为多边形的边建立对应的PSLGEdge边
		for (int i = 0; i < edgeNum; i++) {
			edges[i] = new PSLGEdge();
			edges[i]->tailvex = vertices[i];
			edges[i]->headvex = vertices[(i+1)%vexNum];
			edges[i]->type = PSLGEdgeType::POLYGON_EDGE;
			vertices[i]->firstout = edges[i];
			vertices[(i+1)%vexNum]->firstin = edges[i];
		}
		//计算顶点的速度和类型，端点处需单独处理
		for (int i = 1; i < vexNum-1; i++) {
			calcVertexProperty(vertices[i-1], vertices[i], vertices[i+1]);
		}
		if (closed) {
			calcVertexProperty(vertices[vexNum-1], vertices[0], vertices[1]);
			calcVertexProperty(vertices[vexNum-2], vertices[vexNum-1], vertices[0]);
		} else {
			vertices[0]->type = PSLGVertexType::REFLEX_VERTEX;
			QVector2D t;
			calcTerminalSpeed(vertices[0]->oriPosition, vertices[1]->oriPosition, vertices[0]->speed, t);
			vertices[vexNum-1]->type = PSLGVertexType::REFLEX_VERTEX;
			calcTerminalSpeed(vertices[vexNum-2]->oriPosition, vertices[vexNum-1]->oriPosition, t, vertices[vexNum-1]->speed);
		}
	}

	//计算每条边的消失时间
	for (int i = 0; i < edgeNum; i++) {
		edges[i]->vanishTime = calcEdgeVanishTime(edges[i]);
		elist.push_back(edges[i]);
	}
	delete [] vertices;
	delete [] edges;
}
void PSLGGraph::addPolygon( const QPolygonF& poly, bool twoside ) {
	if ( !twoside ) {
		addPolygon( poly );
		return;
	}

	QPolygonF polyAnotherside;
	for (int i = 0; i < poly.size(); i++) {
		polyAnotherside.push_front( poly[i] );
	}

	std::vector < PSLGEdge* > elist;
	addPolygon( poly, elist );
	addPolygon( polyAnotherside, elist );

	int edgeNum = poly.size() - 1;
	if ( !poly.isClosed() ) {
		PSLGEdge* tail = new PSLGEdge();
		PSLGEdge* head = new PSLGEdge();
		if ( elist[0]->tailvex->firstin == NULL ) {
			tail->tailvex = elist[2*edgeNum-1]->headvex;
			tail->headvex = elist[0]->tailvex;
			elist[0]->tailvex->firstin = tail;
			elist[2*edgeNum-1]->headvex->firstout = tail;

			head->tailvex = elist[edgeNum-1]->headvex;
			head->headvex = elist[edgeNum]->tailvex;
			elist[edgeNum-1]->headvex->firstout = head;
			elist[edgeNum]->tailvex->firstin = head;
		} else {
			tail->headvex = elist[2*edgeNum-1]->headvex;
			tail->tailvex = elist[0]->tailvex;
			elist[0]->tailvex->firstout = tail;
			elist[2*edgeNum-1]->headvex->firstin = tail;
			
			head->headvex = elist[edgeNum-1]->headvex;
			head->tailvex = elist[edgeNum]->tailvex;
			elist[edgeNum-1]->headvex->firstin = head;
			elist[edgeNum]->tailvex->firstout = head;
		}
		elist.push_back( tail );
		elist.push_back( head );
	}

	for ( int i = 0; i < elist.size(); i++ ) {
		edgeList.push_back( elist[i] );
	}
}

void PSLGGraph::getMotorGraph( std::vector< QLineF > &motorLines ) {
	//points数组的前n个与输入的motors顺序相同，lines的每个元素是一个int[2] line，其中line[0]是起点(边上的点),line[1]是终点
	//void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	//	/*OUT*/ std::vector<CrashPoint>& points, std::vector<int*>& lines)
	//得到摩托
	std::vector< Motorcycle > motors; //原直线图中的摩托
	std::vector< PSLGVertex* > motorVertexs; //摩托对应的原直线图中的点
	std::vector< QLineF > walls; //将原直线图的边构成的墙
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < edgeList.size(); i++) {
		if ( !edgeList[i]->tailvex->mark && edgeList[i]->tailvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->tailvex->oriPosition, edgeList[i]->tailvex->speed, edgeList[i]->tailvex->startTime );
			m.id = motors.size();
			if ( edgeList[i]->tailvex->firstin == NULL ) {
				m.leftPoint.setX( -1 );
				m.leftPoint.setY( -1 );
			} else {
				m.leftPoint = edgeList[i]->tailvex->firstin->tailvex->oriPosition;
			}
			if ( edgeList[i]->tailvex->firstout == NULL ) {
				m.rightPoint.setX( -1 );
				m.rightPoint.setY( -1 );
			} else {
				m.rightPoint = edgeList[i]->tailvex->firstout->headvex->oriPosition;
			}
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->tailvex );
			edgeList[i]->tailvex->mark = true;
		} 
		if ( !edgeList[i]->headvex->mark && edgeList[i]->headvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->headvex->oriPosition, edgeList[i]->headvex->speed, edgeList[i]->headvex->startTime );
			QPointF pt = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.id = motors.size();
			if ( edgeList[i]->headvex->firstin == NULL ) {
				m.leftPoint.setX( -1 );
				m.leftPoint.setY( -1 );
			} else {
				m.leftPoint = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			}
			if ( edgeList[i]->headvex->firstout == NULL ) {
				m.rightPoint.setX( -1 );
				m.rightPoint.setY( -1 );
			} else {
				m.rightPoint = edgeList[i]->headvex->firstout->headvex->oriPosition;
			}
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->headvex );
			edgeList[i]->headvex->mark = true;
		}
		walls.push_back( QLineF( edgeList[i]->tailvex->oriPosition, edgeList[i]->headvex->oriPosition ) );
	}
	//计算摩托图
	std::vector< CrashPoint > crashPoints; //摩托图中所有的顶点
	std::vector< MotorLines > motorIndexLines; //摩托图中所有的边
	GetMotorcycleGraphBF( motors, walls, crashPoints, motorIndexLines );
	motorLines.clear();

	for ( int i = 0; i < motorIndexLines.size(); i++ ) {
		int start = motorIndexLines[i].start;
		int end = motorIndexLines[i].end;
		motorLines.push_back( QLineF( crashPoints[start].p, crashPoints[end].p ) );
	}
}

void PSLGGraph::getMotorGraph( std::vector< QLineF > &motorLines, bool twoside ) {
	if ( !twoside ) {
		getMotorGraph( motorLines );
		return;
	}
	//points数组的前n个与输入的motors顺序相同，lines的每个元素是一个int[2] line，其中line[0]是起点(边上的点),line[1]是终点
	//void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	//	/*OUT*/ std::vector<CrashPoint>& points, std::vector<int*>& lines)
	//得到摩托
	double inittime = 2;
	std::vector< Motorcycle > motors; //原直线图中的摩托
	std::vector< PSLGVertex* > motorVertexs; //摩托对应的原直线图中的点
	std::vector< QLineF > walls; //将原直线图的边构成的墙
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < edgeList.size(); i++) {
		if ( !edgeList[i]->tailvex->mark && edgeList[i]->tailvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->tailvex->movedPosition( inittime ), edgeList[i]->tailvex->speed, edgeList[i]->tailvex->startTime );
			m.id = motors.size();
			m.leftPoint = edgeList[i]->tailvex->firstin->tailvex->oriPosition;
			m.rightPoint = edgeList[i]->tailvex->firstout->headvex->oriPosition;
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->tailvex );
			edgeList[i]->tailvex->mark = true;
		} 
		if ( !edgeList[i]->headvex->mark && edgeList[i]->headvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->headvex->movedPosition( inittime ), edgeList[i]->headvex->speed, edgeList[i]->headvex->startTime );
			QPointF pt = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.id = motors.size();
			m.leftPoint = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.rightPoint = edgeList[i]->headvex->firstout->headvex->oriPosition;
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->headvex );
			edgeList[i]->headvex->mark = true;
		}
		walls.push_back( QLineF( edgeList[i]->tailvex->movedPosition( inittime ), edgeList[i]->headvex->movedPosition( inittime ) ) );
	}
	//计算摩托图
	std::vector< CrashPoint > crashPoints; //摩托图中所有的顶点
	std::vector< MotorLines > motorIndexLines; //摩托图中所有的边
	GetMotorcycleGraphBF( motors, walls, crashPoints, motorIndexLines );
	motorLines.clear();

	for ( int i = 0; i < motorIndexLines.size(); i++ ) {
		int start = motorIndexLines[i].start;
		int end = motorIndexLines[i].end;
		motorLines.push_back( QLineF( crashPoints[start].p, crashPoints[end].p ) );
	}
}

void PSLGGraph::addMotorcycle() {
	//points数组的前n个与输入的motors顺序相同，lines的每个元素是一个int[2] line，其中line[0]是起点(边上的点),line[1]是终点
	//void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	//	/*OUT*/ std::vector<CrashPoint>& points, std::vector<int*>& lines)
	//得到摩托
	std::vector< Motorcycle > motors; //原直线图中的摩托
	std::vector< PSLGVertex* > motorVertexs; //摩托对应的原直线图中的点
	std::vector< QLineF > walls; //将原直线图的边构成的墙
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < edgeList.size(); i++) {
		if ( !edgeList[i]->tailvex->mark && edgeList[i]->tailvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->tailvex->oriPosition, edgeList[i]->tailvex->speed, edgeList[i]->tailvex->startTime );
			m.id = motors.size();
			if ( edgeList[i]->tailvex->firstin == NULL ) {
				m.leftPoint.setX( -1 );
				m.leftPoint.setY( -1 );
			} else {
				m.leftPoint = edgeList[i]->tailvex->firstin->tailvex->oriPosition;
			}
			if ( edgeList[i]->tailvex->firstout == NULL ) {
				m.rightPoint.setX( -1 );
				m.rightPoint.setY( -1 );
			} else {
				m.rightPoint = edgeList[i]->tailvex->firstout->headvex->oriPosition;
			}
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->tailvex );
			edgeList[i]->tailvex->mark = true;
		} 
		if ( !edgeList[i]->headvex->mark && edgeList[i]->headvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->headvex->oriPosition, edgeList[i]->headvex->speed, edgeList[i]->headvex->startTime );
			QPointF pt = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.id = motors.size();
			if ( edgeList[i]->headvex->firstin == NULL ) {
				m.leftPoint.setX( -1 );
				m.leftPoint.setY( -1 );
			} else {
				m.leftPoint = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			}
			if ( edgeList[i]->headvex->firstout == NULL ) {
				m.rightPoint.setX( -1 );
				m.rightPoint.setY( -1 );
			} else {
				m.rightPoint = edgeList[i]->headvex->firstout->headvex->oriPosition;
			}
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->headvex );
			edgeList[i]->headvex->mark = true;
		}
		walls.push_back( QLineF( edgeList[i]->tailvex->oriPosition, edgeList[i]->headvex->oriPosition ) );
	}
	//计算摩托图
	std::vector< CrashPoint > crashPoints; //摩托图中所有的顶点
	std::vector< MotorLines > motorLines; //摩托图中所有的边
	GetMotorcycleGraphBF( motors, walls, crashPoints, motorLines );
	//为摩托图中的顶点创建对应的PSLGVertex对象
	int motorNumber = motors.size();
	std::vector< PSLGVertex* > crashVertex; //crash point对应的Vertex数组
	for ( std::vector< CrashPoint >::size_type i = motorNumber; i <crashPoints.size(); i++) {
		PSLGVertex* v = new PSLGVertex( crashPoints[i].p );
		switch ( crashPoints[i].type ) {
			case PointType::ClashVertice:
				v->type = PSLGVertexType::MULTI_STEINER_VERTEX;
				break;
			case PointType::MovingVertice:
				v->type = PSLGVertexType::RESTING_STEINER_VERTEX;
				break;
			case PointType::WallVertice:
				v->type = PSLGVertexType::MOVING_STEINER_VERTEX;
				break;
			case PointType::ReflexVertice:
				break;
			case PointType::InfiniteVertice: 
				v->type = PSLGVertexType::OTHER_VERTEX; //无穷远点
				break;
		}
		crashVertex.push_back( v );
	}
	//为摩托图中的边创建对应的PSLGVertex
	std::vector< PSLGEdge* > motorEdges; //摩托图中的边
	std::vector < std:: vector< PSLGEdge* > > splitEdges( edgeList.size() ); //被MOVING_STEINER_VERTEX点分裂的边

	for ( std::vector< int* >::size_type i = 0; i < motorLines.size(); i++ ) {
		PSLGVertex *start, *end; //边的起点和终点分别对应的PSLGVertex对象
		if ( motorLines[i].start < motorNumber ) {
			start = motorVertexs[ motorLines[i].start ];
		} else {
			start = crashVertex[ motorLines[i].start-motorNumber ];
		}

		if ( motorLines[i].end < motorNumber ) {
			end = motorVertexs[ motorLines[i].end  ];
		} else {
			end = crashVertex[ motorLines[i].end-motorNumber ];
		}

		//当顶点为MOVING_STEINER_VERTEX时，需要将原来多边形中的边分裂为两条边
		int index = -1;
		PSLGEdge* ein = NULL;
		PSLGEdge* eout = NULL; 
		PSLGEdge* etemp = NULL;
		if ( start->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			index = crashPoints[ motorLines[i].start ].wallNo;
			if ( splitEdges[index].empty() ) {
				splitEdges[index].push_back( edgeList[index] );
			}
			int j;
			for ( j = 0; j < splitEdges[index].size(); j++ ) {
				if ( PSLGEdge::isVertexOnEdge( start, splitEdges[index][j] ) ) 
					break;
			}
			if ( j >= splitEdges[index].size() ) {
				index = -1;
			} else {
				etemp = splitEdges[index][j];
				splitEdges[index][j] = splitEdges[index][splitEdges[index].size()-1];
				splitEdges[index].pop_back();

				ein = new PSLGEdge( etemp->tailvex, start );
				eout = new PSLGEdge( start, etemp->headvex );
				start->firstin = ein;
				start->firstout = eout;

				if ( PSLGEdge::isVertexOnEdge( start, edgeList[index] ) != 0 ) {
					PSLGGraph::calcMovingSteinerSpeed( start, end, edgeList[index]->tailvex );
				} else {
					PSLGGraph::calcMovingSteinerSpeed( start, end, edgeList[index]->headvex );
				}
			}
		} else if ( end->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			index = crashPoints[ motorLines[i].end ].wallNo;
			if ( splitEdges[index].empty() ) {
				splitEdges[index].push_back( edgeList[index] );
			}
			int j;
			for ( j = 0; j < splitEdges[index].size(); j++ ) {
				if ( PSLGEdge::isVertexOnEdge( end, splitEdges[index][j] ) ) 
					break;
			}
			if ( j >= splitEdges[index].size() ) {
				index = -1;
			} else {
				etemp = splitEdges[index][j];
				splitEdges[index][j] = splitEdges[index][splitEdges[index].size()-1];
				splitEdges[index].pop_back();

				ein = new PSLGEdge( etemp->tailvex, end);
				eout = new PSLGEdge( end, etemp->headvex );
				end->firstin = ein;
				end->firstout = eout;
				if ( PSLGEdge::isVertexOnEdge( end, edgeList[index] ) != 0 ) {
					PSLGGraph::calcMovingSteinerSpeed( end, start, edgeList[index]->tailvex );
				} else {
					PSLGGraph::calcMovingSteinerSpeed( end, start, edgeList[index]->headvex );
				}
			}
		}
		if ( index != -1 ) {
			ein->tedge = etemp->tedge;
			eout->hedge = etemp->hedge;
			etemp->tailvex->firstout = ein;
			etemp->headvex->firstin = eout;
			splitEdges[index].push_back( ein );
			splitEdges[index].push_back( eout );
			if ( etemp != edgeList[index] ) {
				delete etemp;
			}
			edgeList[index]->mark = true;
		} 
		//为这条边建一个对应的PSLGEdge
		PSLGEdge* e = new PSLGEdge( start, end ); 
		e->type = PSLGEdgeType::MOTORCYCLE_EDGE;
		if ( start->firstout != NULL ) {
			PSLGEdge* et;
			for ( et = start->firstout; et ->tedge != NULL; et=et->tedge ) ;
			et->tedge = e;
		} else {
			start->firstout = e;
		}
		if ( end->firstin != NULL ) {
			PSLGEdge* et;
			for ( et = end->firstin; et ->hedge != NULL; et=et->hedge ) ;
			et->hedge = e;
		} else {
			end->firstin = e;
		}
		motorEdges.push_back( e );
		//无穷远点的速度
		if ( start->type == PSLGVertexType::OTHER_VERTEX ) {
			start->speed = end->speed;
		} else if ( end->type == PSLGVertexType::OTHER_VERTEX ) {
			end->speed = start->speed;
		}
	}
	std::vector< PSLGEdge* > templist( edgeList );
	edgeList.clear();
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < templist.size(); i++ ) {
		if ( templist[i]->mark ) {
			delete templist[i];
			templist[i] = NULL;
		} else {
			edgeList.push_back( templist[i] );
		}
	}
	templist.clear();
	//计算motorEdges中边的消失时间，并把motorEdges加到Graph中
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < motorEdges.size(); i++ ) {
		motorEdges[i]->vanishTime = PSLGGraph::calcEdgeVanishTime( motorEdges[i] );
		edgeList.push_back( motorEdges[i] );
	}
	//无穷远点的速度
	for ( std::vector< PSLGVertex* >::size_type i = 0; i < crashVertex.size(); i++) {
		if ( crashVertex[i]->type == PSLGVertexType::OTHER_VERTEX ) {
			PSLGVertex* vt = NULL;
			if ( crashVertex[i]->firstin != NULL ) {
				vt = crashVertex[i]->firstin->tailvex;
			} else {
				vt = crashVertex[i]->firstout->headvex;
			}
			PSLGVertex* prev = crashVertex[i];
			while ( vt->type == PSLGVertexType::RESTING_STEINER_VERTEX ) {
				std::vector< PSLGVertex* > adjacents;
				vt->getVertexAdjacentVertexes( adjacents );
				std::vector< PSLGVertex* >::size_type j;
				for ( j = 0; j < adjacents.size(); j++ ) {
					if ( adjacents[j] == prev ) continue;;
					if ( std::abs( PSLGGraph::determinant( vt, adjacents[j], crashVertex[i] ) ) < 1 )
						break;
				}
				if ( j < adjacents.size() ) {
					prev = vt;
					vt = adjacents[j];
				} else {
					break;
				}
			} 
			if ( vt->type == PSLGVertexType::REFLEX_VERTEX ) {
				crashVertex[i]->speed = vt->speed;
		    }
		}
	}
	for ( int i = 0; i < splitEdges.size(); i++ ) {
		if ( !splitEdges[i].empty() ) {
			for (int j = 0; j < splitEdges[i].size(); j++) {
				splitEdges[i][j]->vanishTime = PSLGGraph::calcEdgeVanishTime( splitEdges[i][j] );
				edgeList.push_back( splitEdges[i][j] );
			}
			splitEdges[i].clear();
		}
	}
	splitEdges.clear();
}

void PSLGGraph::addMotorcycle( bool twoside ) {
	//points数组的前n个与输入的motors顺序相同，lines的每个元素是一个int[2] line，其中line[0]是起点(边上的点),line[1]是终点
	//void GetMotorcycleGraphBF(const std::vector<Motorcycle>& motors, const std::vector<QLineF>& walls,
	//	/*OUT*/ std::vector<CrashPoint>& points, std::vector<int*>& lines)
	//得到摩托
	if ( !twoside ) {
		addMotorcycle();
		return;
	} 
	std::vector< Motorcycle > motors; //原直线图中的摩托
	std::vector< PSLGVertex* > motorVertexs; //摩托对应的原直线图中的点
	std::vector< QLineF > walls; //将原直线图的边构成的墙
	double inittime = 2;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < edgeList.size(); i++) {
		if ( !edgeList[i]->tailvex->mark && edgeList[i]->tailvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->tailvex->movedPosition( inittime ), edgeList[i]->tailvex->speed, edgeList[i]->tailvex->startTime );
			m.id = motors.size();
			m.leftPoint = edgeList[i]->tailvex->firstin->tailvex->oriPosition;
			m.rightPoint = edgeList[i]->tailvex->firstout->headvex->oriPosition;
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->tailvex );
			edgeList[i]->tailvex->mark = true;
		} 
		if ( !edgeList[i]->headvex->mark && edgeList[i]->headvex->type == REFLEX_VERTEX ) {
			Motorcycle m( edgeList[i]->headvex->movedPosition( inittime ), edgeList[i]->headvex->speed, edgeList[i]->headvex->startTime );
			QPointF pt = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.id = motors.size();
			m.leftPoint = edgeList[i]->headvex->firstin->tailvex->oriPosition;
			m.rightPoint = edgeList[i]->headvex->firstout->headvex->oriPosition;
			motors.push_back( m );
			motorVertexs.push_back( edgeList[i]->headvex );
			edgeList[i]->headvex->mark = true;
		}
		walls.push_back( QLineF( edgeList[i]->tailvex->movedPosition( inittime ), edgeList[i]->headvex->movedPosition( inittime ) ) );
	}
	//计算摩托图
	std::vector< CrashPoint > crashPoints; //摩托图中所有的顶点
	std::vector< MotorLines > motorLines; //摩托图中所有的边
	GetMotorcycleGraphBF( motors, walls, crashPoints, motorLines );
	//为摩托图中的顶点创建对应的PSLGVertex对象
	int motorNumber = motors.size();
	std::vector< PSLGVertex* > crashVertex; //crash point对应的Vertex数组
	for ( std::vector< CrashPoint >::size_type i = motorNumber; i <crashPoints.size(); i++) {
		PSLGVertex* v = new PSLGVertex( crashPoints[i].p );
		switch ( crashPoints[i].type ) {
		case PointType::ClashVertice:
			v->type = PSLGVertexType::MULTI_STEINER_VERTEX;
			break;
		case PointType::MovingVertice:
			v->type = PSLGVertexType::RESTING_STEINER_VERTEX;
			break;
		case PointType::WallVertice:
			v->type = PSLGVertexType::MOVING_STEINER_VERTEX;
			v->startTime = inittime;
			break;
		case PointType::ReflexVertice:
			break;
		case PointType::InfiniteVertice: 
			v->type = PSLGVertexType::OTHER_VERTEX; //无穷远点
			break;
		}
		crashVertex.push_back( v );
	}
	//为摩托图中的边创建对应的PSLGVertex
	std::vector< PSLGEdge* > motorEdges; //摩托图中的边
	std::vector < std:: vector< PSLGEdge* > > splitEdges( edgeList.size() ); //被MOVING_STEINER_VERTEX点分裂的边

	for ( std::vector< int* >::size_type i = 0; i < motorLines.size(); i++ ) {
		PSLGVertex *start, *end; //边的起点和终点分别对应的PSLGVertex对象
		if ( motorLines[i].start < motorNumber ) {
			start = motorVertexs[ motorLines[i].start ];
		} else {
			start = crashVertex[ motorLines[i].start-motorNumber ];
		}

		if ( motorLines[i].end < motorNumber ) {
			end = motorVertexs[ motorLines[i].end  ];
		} else {
			end = crashVertex[ motorLines[i].end-motorNumber ];
		}

		//当顶点为MOVING_STEINER_VERTEX时，需要将原来多边形中的边分裂为两条边
		int index = -1;
		PSLGEdge* ein = NULL;
		PSLGEdge* eout = NULL; 
		PSLGEdge* etemp = NULL;
		if ( start->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			index = crashPoints[ motorLines[i].start ].wallNo;
			if ( splitEdges[index].empty() ) {
				splitEdges[index].push_back( edgeList[index] );
			}
			int j;
			for ( j = 0; j < splitEdges[index].size(); j++ ) {
				if ( PSLGEdge::isVertexOnEdge( start, splitEdges[index][j] ) ) 
					break;
			}
			if ( j >= splitEdges[index].size() ) {
				index = -1;
			} else {
				etemp = splitEdges[index][j];
				splitEdges[index][j] = splitEdges[index][splitEdges[index].size()-1];
				splitEdges[index].pop_back();

				ein = new PSLGEdge( etemp->tailvex, start );
				eout = new PSLGEdge( start, etemp->headvex );
				start->firstin = ein;
				start->firstout = eout;

				if ( PSLGEdge::isVertexOnEdge( start, edgeList[index] ) != 0 ) {
					PSLGGraph::calcMovingSteinerSpeed( start, end, edgeList[index]->tailvex );
				} else {
					PSLGGraph::calcMovingSteinerSpeed( start, end, edgeList[index]->headvex );
				}

				start->oriPosition = start->movedPosition( -inittime );
				start->startTime = 0;
			}
		} else if ( end->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			index = crashPoints[ motorLines[i].end ].wallNo;
			if ( splitEdges[index].empty() ) {
				splitEdges[index].push_back( edgeList[index] );
			}
			int j;
			for ( j = 0; j < splitEdges[index].size(); j++ ) {
				if ( PSLGEdge::isVertexOnEdge( end, splitEdges[index][j] ) ) 
					break;
			}
			if ( j >= splitEdges[index].size() ) {
				index = -1;
			} else {
				etemp = splitEdges[index][j];
				splitEdges[index][j] = splitEdges[index][splitEdges[index].size()-1];
				splitEdges[index].pop_back();

				ein = new PSLGEdge( etemp->tailvex, end);
				eout = new PSLGEdge( end, etemp->headvex );
				end->firstin = ein;
				end->firstout = eout;
				if ( PSLGEdge::isVertexOnEdge( end, edgeList[index] ) != 0 ) {
					PSLGGraph::calcMovingSteinerSpeed( end, start, edgeList[index]->tailvex );
				} else {
					PSLGGraph::calcMovingSteinerSpeed( end, start, edgeList[index]->headvex );
				}

				end->oriPosition = end->movedPosition( -inittime );
				end->startTime = 0;
			}
		}
		if ( index != -1 ) {
			ein->tedge = etemp->tedge;
			eout->hedge = etemp->hedge;
			etemp->tailvex->firstout = ein;
			etemp->headvex->firstin = eout;
			splitEdges[index].push_back( ein );
			splitEdges[index].push_back( eout );
			if ( etemp != edgeList[index] ) {
				delete etemp;
			}
			edgeList[index]->mark = true;
		} 
		//为这条边建一个对应的PSLGEdge
		PSLGEdge* e = new PSLGEdge( start, end ); 
		e->type = PSLGEdgeType::MOTORCYCLE_EDGE;
		if ( start->firstout != NULL ) {
			PSLGEdge* et;
			for ( et = start->firstout; et ->tedge != NULL; et=et->tedge ) ;
			et->tedge = e;
		} else {
			start->firstout = e;
		}
		if ( end->firstin != NULL ) {
			PSLGEdge* et;
			for ( et = end->firstin; et ->hedge != NULL; et=et->hedge ) ;
			et->hedge = e;
		} else {
			end->firstin = e;
		}
		motorEdges.push_back( e );
		//无穷远点的速度
		if ( start->type == PSLGVertexType::OTHER_VERTEX ) {
			start->speed = end->speed;
		} else if ( end->type == PSLGVertexType::OTHER_VERTEX ) {
			end->speed = start->speed;
		}
	}
	std::vector< PSLGEdge* > templist( edgeList );
	edgeList.clear();
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < templist.size(); i++ ) {
		if ( templist[i]->mark ) {
			delete templist[i];
			templist[i] = NULL;
		} else {
			edgeList.push_back( templist[i] );
		}
	}
	templist.clear();
	//计算motorEdges中边的消失时间，并把motorEdges加到Graph中
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < motorEdges.size(); i++ ) {
		motorEdges[i]->vanishTime = PSLGGraph::calcEdgeVanishTime( motorEdges[i] );
		edgeList.push_back( motorEdges[i] );
	}
	//无穷远点的速度
	for ( std::vector< PSLGVertex* >::size_type i = 0; i < crashVertex.size(); i++) {
		if ( crashVertex[i]->type == PSLGVertexType::OTHER_VERTEX ) {
			PSLGVertex* vt = NULL;
			if ( crashVertex[i]->firstin != NULL ) {
				vt = crashVertex[i]->firstin->tailvex;
			} else {
				vt = crashVertex[i]->firstout->headvex;
			}
			PSLGVertex* prev = crashVertex[i];
			while ( vt->type == PSLGVertexType::RESTING_STEINER_VERTEX ) {
				std::vector< PSLGVertex* > adjacents;
				vt->getVertexAdjacentVertexes( adjacents );
				std::vector< PSLGVertex* >::size_type j;
				for ( j = 0; j < adjacents.size(); j++ ) {
					if ( adjacents[j] == prev ) continue;;
					if ( std::abs( PSLGGraph::determinant( vt, adjacents[j], crashVertex[i] ) ) < 1 )
						break;
				}
				if ( j < adjacents.size() ) {
					prev = vt;
					vt = adjacents[j];
				} else {
					break;
				}
			} 
			if ( vt->type == PSLGVertexType::REFLEX_VERTEX ) {
				crashVertex[i]->speed = vt->speed;
			}
		}
	}
	for ( int i = 0; i < splitEdges.size(); i++ ) {
		if ( !splitEdges[i].empty() ) {
			for (int j = 0; j < splitEdges[i].size(); j++) {
				splitEdges[i][j]->vanishTime = PSLGGraph::calcEdgeVanishTime( splitEdges[i][j] );
				edgeList.push_back( splitEdges[i][j] );
			}
			splitEdges[i].clear();
		}
	}
	splitEdges.clear();
}

double PSLGGraph::determinant(const QPointF& p1, const QPointF& p2, const QPointF& p3) {
	return p3.x()*(p1.y() - p2.y()) - p3.y()*(p1.x() - p2.x()) + (p1.x()*p2.y() - p2.x()*p1.y());
}

double PSLGGraph::determinant( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 ) {
	// 三个点的起始时间不一致，需要按照最大的起始时间对点坐标进行更新，然后计算速度
	double start = std::max( v1->startTime, v2->startTime );
	start = std::max( start, v3->startTime );
	QPointF p1 = v1->movedPosition( start );
	QPointF p2 = v2->movedPosition( start );
	QPointF p3 = v3->movedPosition( start );

	return determinant(p1, p2, p3);
}

double PSLGGraph::determinant(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3, double t ) {
	QPointF p1 = v1->movedPosition( t );
	QPointF p2 = v2->movedPosition( t );
	QPointF p3 = v3->movedPosition( t );

	return determinant(p1, p2, p3);
}
QVector2D PSLGGraph::calcVertexSpeed(const QPointF& p1, const QPointF& p2, const QPointF& p3) {
	QVector2D e1(p2 - p1);
	QVector2D e2(p3 - p2);
	QVector2D ne1 = e1.normalized();
	QVector2D ne2 = e2.normalized();
	QVector2D bisector = ne2 - ne1;
	//如果三点共线，除数将为 0
	QVector2D speed = bisector / ( ( bisector - (QVector2D::dotProduct(bisector, ne2)) * ne2 ).length() );
	return speed;
}

void PSLGGraph::calcConvexVertexSpeed( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 ) {
	// 三个点的起始时间不一致，需要按照最大的起始时间对点坐标进行更新，然后计算速度
	double start = std::max( v1->startTime, v2->startTime );
	start = std::max( start, v3->startTime );
	QPointF p1 = v1->movedPosition( start );
	QPointF p2 = v2->movedPosition( start );
	QPointF p3 = v3->movedPosition( start );
	v2->startTime = start;
	v2->speed = calcVertexSpeed( p1, p2, p3);

	double d = determinant(p1, p2, p3);
	if ( std::abs(d) < eps  ) {
		v2->speed.setX( 0 );
		v2->speed.setY( 0 );
	} else {
		v2->speed = calcVertexSpeed( p1, p2, p3);
	}
}

void PSLGGraph::calcConvexVertexSpeed( PSLGVertex* v ) {
	PSLGGraph::calcConvexVertexSpeed( v->firstin->tailvex, v, v->firstout->headvex );
}

void PSLGGraph::calcVertexProperty(PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3) {
	// 三个点的起始时间不一致，需要按照最大的起始时间对点坐标进行更新，然后计算速度
	double start = std::max( v1->startTime, v2->startTime );
	start = std::max( start, v3->startTime );
	QPointF p1 = v1->movedPosition( start );
	QPointF p2 = v2->movedPosition( start );
	QPointF p3 = v3->movedPosition( start );

	double d = determinant(p1, p2, p3);
	v2->startTime = start;
	if ( std::abs(d) < eps ) {
		v2->type = PSLGVertexType::CONVEX_VERTEX;
		double d1 = ( p2.x() - p1.x() )*( p3.x() - p2.x() );
		double d2 = ( p2.y() - p1.y() )*( p3.y() - p2.y() );
		if ( d1 < 0 || d2 < 0  ) {
			v2->speed = QVector2D(0, 0);
		} else {
			QVector2D e1(p2 - p1);
			QVector2D e2(-e1.y(), e1.x());
			e2.normalize();
			if ( determinant( p1, QPointF(p2.x()+e2.x(), p2.y()+e2.y()), p3 ) < 0 ) {
				v2->speed = e2;
			} else {
				v2->speed = -e2;
			}
		}
	} else if ( d > 0 ) {
		v2->type = PSLGVertexType::CONVEX_VERTEX;
		v2->speed = calcVertexSpeed( p1, p2, p3);
	} else if ( d < 0 ) {
		v2->type = PSLGVertexType::REFLEX_VERTEX;
		v2->speed = -1 * calcVertexSpeed( p1, p2, p3);
	}
}

void PSLGGraph::calcVertexProperty( PSLGVertex* v ) {
	PSLGGraph::calcVertexProperty( v->firstin->tailvex, v, v->firstout->headvex );
}

//这种情况只能出现在初期建立十字链表时，此时，各个顶点的起始时间相等
void PSLGGraph::calcTerminalSpeed(const QPointF& p1, const QPointF& p2, QVector2D& v1, QVector2D& v2) {
	QVector2D e1(p2 - p1);
	QVector2D e2(-e1.y(), e1.x());
	e1.normalize();
	e2.normalize();
	
    QVector2D speed = e2 - e1;
	if ( determinant(QPointF(p1.x()+speed.x(), p1.y()+speed.y()), p1, p2) > 0 ) {
		v1 = speed;
	} else {
		v1 = -e2 - e1;
	}
	
	speed = e1 + e2;
	if ( determinant(p1, p2, QPointF(p2.x()+speed.x(), p2.y()+speed.y()) ) > 0 ) {
		v2 = speed;
	} else {
		v2 = e1 - e2;
	}
}

//根据边的两个端点计算边消失的时间
double PSLGGraph::calcEdgeVanishTime(PSLGEdge* e) {
	//边的两端点的起始时间不一定相等，需要根据较大的起始时间来更新点的坐标并计算边小时的时间
	double start = std::max( e->tailvex->startTime, e->headvex->startTime );
	QPointF p1 = e->tailvex->movedPosition( start );
	QPointF p2 = e->headvex->movedPosition( start );

	QVector2D e1(p2 - p1);
	QVector2D e2 = e1.normalized();

	double dl = QVector2D::dotProduct(e->tailvex->speed, e2) - QVector2D::dotProduct(e->headvex->speed, e2);
	double time = start;
	if ( dl > 0 ) {
		time = start + e1.length() / dl;
	} else {
		time = std::numeric_limits< double >::max();
	}
	return time;
}

void PSLGGraph::getVertexPolygonalInAndOutEdge( PSLGVertex* v, PSLGEdge* &in, PSLGEdge* &out ) {
	for ( in = v->firstin; in != NULL && in->type != PSLGEdgeType::POLYGON_EDGE; in = in->hedge );
	for ( out = v->firstout; out != NULL && out->type != PSLGEdgeType::POLYGON_EDGE; out = out->tedge );
}

void PSLGGraph::calcMovingSteinerSpeed( PSLGVertex* mpv, PSLGVertex* mv, PSLGVertex* pv ) {
	// 三个点的起始时间不一致，需要按照最大的起始时间对点坐标进行更新，然后计算速度
	double start = std::max( mpv->startTime, mv->startTime );
	start = std::max( start, pv->startTime );
	QPointF p1 = mpv->movedPosition( start );
	QPointF p2 = mv->movedPosition( start );
	QPointF p3 = pv->movedPosition( start );
	QVector2D e1( p2 - p1 );
	QVector2D e2( p3 - p1 );
	e2.normalize();
	mpv->speed = e1 / ( ( e1 - QVector2D::dotProduct( e1, e2 ) * e2 ).length() );
}


//void main() {
//	QPolygonF poly;
//	poly.append(QPointF(0,0));
//	poly.append(QPointF(2, 0));
//	poly.append(QPointF(2, 1));
//	poly.append(QPointF(0, 1));
//	poly.append(QPointF(0, 0));
//
//	PSLGGraph* graph = new PSLGGraph();
//	graph->addPolygon(poly);
//}
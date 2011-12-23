#include "pslgsskeleton.h"

#include "sstask.h"
#define eps 1.0e-6

PSLGEventType PSLGSSkeleton::eventTypes[6][6] = {
	{OTHER_EVENT, SWITCH_EVENT, SPLIT_EVENT, MULTI_SPLIT_EVENT, START_EVENT, OTHER_EVENT}, 
	{SWITCH_EVENT, EDGE_EVENT, SWITCH_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT},
	{SPLIT_EVENT, SWITCH_EVENT, OTHER_EVENT, MULTI_START_EVENT, START_EVENT, OTHER_EVENT },
	{MULTI_SPLIT_EVENT, OTHER_EVENT, MULTI_START_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT },
	{START_EVENT, OTHER_EVENT, START_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT},
	{OTHER_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT, OTHER_EVENT} 
};

PSLGSSkeleton::PSLGSSkeleton() : ssgraph(NULL), eventQueue(NULL){

}

PSLGSSkeleton::PSLGSSkeleton( PSLGGraph* g) : ssgraph(g), eventQueue(NULL) {
	/*eventQueue = new PriorityQueue(ssgraph->edgeList);
	eventQueue->buildMinHeap();*/
}

PSLGSSkeleton::~PSLGSSkeleton() {
	delete ssgraph;
	delete eventQueue;
	ssgraph = NULL;
	eventQueue = NULL;
}

void PSLGSSkeleton::showMotorGraph( bool twoside ) {
	std::vector< QLineF > motorLines;
	ssgraph->getMotorGraph( motorLines, twoside );
	wavefront.clear();

	for ( int i = 0; i < motorLines.size(); i++) {
		wavefront.push_back( motorLines[i] );
	}
}

void PSLGSSkeleton::extractSkeletion() {
	ssgraph->addMotorcycle();
	eventQueue = new PriorityQueue(ssgraph->edgeList);
	eventQueue->buildMinHeap();
	skeleton.clear();
	meshes.clear();
	double curtime = 0;
	while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
		PSLGEdge* e = eventQueue->heapExtractMin();
		curtime = e->vanishTime;
		handleEvent(e);
	}

	//队列中继续增长的边可能是直骨架
	std::vector< PSLGVertex* > vs;
	for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
		eventQueue->edges[i]->tailvex->mark = false;
		eventQueue->edges[i]->headvex->mark = false;
	}
	for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
		if ( !eventQueue->edges[i]->tailvex->mark ) {
			vs.push_back( eventQueue->edges[i]->tailvex );
			eventQueue->edges[i]->tailvex->mark = true;
		}
		if ( !eventQueue->edges[i]->headvex->mark ) {
			vs.push_back( eventQueue->edges[i]->headvex );
			eventQueue->edges[i]->headvex->mark = true;
		}
	}
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
		if ( eventQueue->edges[i]->type == PSLGEdgeType::MOTORCYCLE_EDGE ) {
			skeleton.push_back( QLineF( eventQueue->edges[i]->tailvex->oriPosition, eventQueue->edges[i]->headvex->oriPosition ) );
		} else {
			PSLGVertex* vt;
			if ( eventQueue->edges[i]->tailvex->type == PSLGVertexType::CONVEX_VERTEX ) {
				vt = eventQueue->edges[i]->tailvex;
			} else {
				vt = eventQueue->edges[i]->headvex;
			}
			skeleton.push_back( QLineF( vt->oriPosition, vt->movedPosition( 1000 ) ) );
		}
	}

	for ( std::vector< PSLGVertex* >::size_type i = 0; i < vs.size(); i++ ) {
		delete vs[i];
	}
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
		delete eventQueue->edges[i];
	}
	vs.clear();
	eventQueue->edges.clear();		
}

void PSLGSSkeleton::extractSkeletion(SSTask* task) {
	bool motorOnly = task->property("-Motor Only").toBool();
	int sleepInterval = task->property("-Sleep Interval (ms)").toInt();
	double timeInterval = task->property("-Update Step Length").toDouble();
	double timelimit = 1000;
	bool twoside = false;

	std::vector< PSLGVertex* > vs;
	if (motorOnly) {
		showMotorGraph( twoside );
		task->notifyWidget();

		std::vector< PSLGVertex* > vs;
		for ( std::vector<PSLGEdge* >::size_type i = 0; i < ssgraph->edgeList.size(); i++ ) {
			ssgraph->edgeList[i]->tailvex->mark = false;
			ssgraph->edgeList[i]->headvex->mark = false;
		}
		for ( std::vector<PSLGEdge* >::size_type i = 0; i < ssgraph->edgeList.size(); i++ ) {
			if ( !ssgraph->edgeList[i]->tailvex->mark ) {
				vs.push_back( ssgraph->edgeList[i]->tailvex );
				ssgraph->edgeList[i]->tailvex->mark = true;
			}
			if ( !ssgraph->edgeList[i]->headvex->mark ) {
				vs.push_back( ssgraph->edgeList[i]->headvex );
				ssgraph->edgeList[i]->headvex->mark = true;
			}
		}

		for ( std::vector< PSLGVertex* >::size_type i = 0; i < vs.size(); i++ ) {
			delete vs[i];
		}
		for ( std::vector< PSLGEdge* >::size_type i = 0; i < ssgraph->edgeList.size(); i++ ) {
			delete ssgraph->edgeList[i];
		}
		vs.clear();
		ssgraph->edgeList.clear();

	} else {
		ssgraph->addMotorcycle( twoside );
		eventQueue = new PriorityQueue(ssgraph->edgeList);
		eventQueue->buildMinHeap();
		skeleton.clear();
		meshes.clear();
		double curtimeInterval = timeInterval;
		double curtime = 0;

		//如果timeInterval==0就全速跑
		if ( std::abs( timeInterval ) < eps || std::abs( sleepInterval) < eps) {
			while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
				PSLGEdge* e = eventQueue->heapExtractMin();
				curtime = e->vanishTime;
				handleEvent(e);
			}
		} else {
			while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
				if ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime > timelimit ) {
					//全速跑
					while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
						PSLGEdge* e = eventQueue->heapExtractMin();
						curtime = e->vanishTime;
						handleEvent(e);
					}
				} else { //波前慢慢扩散
					while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime <= curtimeInterval+eps ) {
						PSLGEdge* e = eventQueue->heapExtractMin();
						curtime = e->vanishTime;
						handleEvent(e);

						//当前的波前
						wavefront.clear();
						for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
							//if (eventQueue->edges[i]->type == PSLGEdgeType::POLYGON_EDGE )
							wavefront.push_back( QLineF( eventQueue->edges[i]->tailvex->movedPosition( curtime ), eventQueue->edges[i]->headvex->movedPosition(curtime) ) );
						}
						task->ssmsleep(sleepInterval);
						task->notifyWidget();
					}
					if ( std::abs( curtime - curtimeInterval ) > eps  ) {
						curtime = curtimeInterval;
						//当前的波前
						wavefront.clear();
						for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
							//if (eventQueue->edges[i]->type == PSLGEdgeType::POLYGON_EDGE )
							wavefront.push_back( QLineF( eventQueue->edges[i]->tailvex->movedPosition( curtime ), eventQueue->edges[i]->headvex->movedPosition(curtime) ) );
						}
						task->ssmsleep(sleepInterval);
						task->notifyWidget();
					}
					curtimeInterval += timeInterval;
				}
			}
		}

		//队列中继续增长的边可能是直骨架
		std::vector< PSLGVertex* > vs;
		for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
			eventQueue->edges[i]->tailvex->mark = false;
			eventQueue->edges[i]->headvex->mark = false;
		}
		for ( std::vector<PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
			if ( !eventQueue->edges[i]->tailvex->mark ) {
				vs.push_back( eventQueue->edges[i]->tailvex );
				eventQueue->edges[i]->tailvex->mark = true;
			}
			if ( !eventQueue->edges[i]->headvex->mark ) {
				vs.push_back( eventQueue->edges[i]->headvex );
				eventQueue->edges[i]->headvex->mark = true;
			}
		}
		for ( std::vector< PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
			if ( eventQueue->edges[i]->type == PSLGEdgeType::MOTORCYCLE_EDGE ) {
				skeleton.push_back( QLineF( eventQueue->edges[i]->tailvex->oriPosition, eventQueue->edges[i]->headvex->oriPosition ) );
			} else {
				PSLGVertex* vt;
				if ( eventQueue->edges[i]->tailvex->type == PSLGVertexType::CONVEX_VERTEX ) {
					vt = eventQueue->edges[i]->tailvex;
				} else {
					vt = eventQueue->edges[i]->headvex;
				}
				skeleton.push_back( QLineF( vt->oriPosition, vt->movedPosition( 1000 ) ) );
			}
		}
		wavefront.clear();
		task->notifyWidget();

		for ( std::vector< PSLGVertex* >::size_type i = 0; i < vs.size(); i++ ) {
			delete vs[i];
		}
		for ( std::vector< PSLGEdge* >::size_type i = 0; i < eventQueue->edges.size(); i++ ) {
			delete eventQueue->edges[i];
		}
		vs.clear();
		eventQueue->edges.clear();

	}
}

void PSLGSSkeleton::handleEvent(PSLGEdge* event) {
	if ( event == NULL ) return;

	PSLGVertexType type1 = event->tailvex->type;
	PSLGVertexType type2 = event->headvex->type;
	if ( type1 < PSLGVertexType::REFLEX_VERTEX || type1 >PSLGVertexType::OTHER_VERTEX || 
		 type2 < PSLGVertexType::REFLEX_VERTEX || type2 >PSLGVertexType::OTHER_VERTEX ) {
			 delete event;
			 return;
	}
	switch (PSLGSSkeleton::eventTypes[type1][type2]) {
		case PSLGEventType::EDGE_EVENT:
			handleEdgeEvent(event);
			break;
		case PSLGEventType::SPLIT_EVENT:
			handleSplitEvent(event);
			break;
		case PSLGEventType::START_EVENT:
			handleStartEvent(event);
			break;
		case PSLGEventType::SWITCH_EVENT:
			handleSwitchEvent(event);
			break;
		case PSLGEventType::MULTI_START_EVENT:
			handleMultiStartEvent(event);
			break;
		case PSLGEventType::MULTI_SPLIT_EVENT:
			handleMultiSplitEvent(event);
			break;
		default:
			handleRemainingEvent(event);
			break;
	}
}

// EdgeEvent 两个凸点合并成一个新的凸点
void PSLGSSkeleton::handleEdgeEvent(PSLGEdge* event) {
	//如果该边退化为一个点
	if (event->headvex == event->tailvex) {
		delete event->headvex;
		delete event;
		return;
	}
	//计算新顶点的坐标
	QPointF collisionPoint = event->tailvex->movedPosition( event->vanishTime );
	PSLGVertex* collisionvex = new PSLGVertex(collisionPoint, event->vanishTime); 
	//形成两条直骨架
	if ( !PSLGVertex::equalPosition( event->tailvex->oriPosition, collisionPoint ) ) {
		skeleton.push_back( QLineF( event->tailvex->oriPosition, collisionPoint ) );
	}
	if ( !PSLGVertex::equalPosition( event->headvex->oriPosition, collisionPoint ) ) {
		skeleton.push_back( QLineF( event->headvex->oriPosition, collisionPoint ) );
	}

	//增加三角剖分，方向为逆时针
	addTriangleMesh( event->tailvex, event->headvex, collisionPoint, event->vanishTime );
	addTriangleMesh( event->tailvex->firstin->tailvex, event->tailvex, collisionPoint, event->vanishTime );
	addTriangleMesh( event->headvex, event->headvex->firstout->headvex, collisionPoint, event->vanishTime );

	//建立新顶点与相邻边的链接关系
	PSLGVertex* p1 = event->tailvex;
	PSLGVertex* p2 = event->headvex;
	p1->firstin->headvex = collisionvex;
	p2->firstout->tailvex = collisionvex;
	collisionvex->firstin = p1->firstin;
	collisionvex->firstout = p2->firstout;
	collisionvex->type = PSLGVertexType::CONVEX_VERTEX;
	//如果新的两条邻边平行，直接更新两条邻边消失时间为当前的时间，不用更新点的速度
	if ( std::abs( PSLGGraph::determinant( collisionvex->firstin->tailvex, collisionvex, collisionvex->firstout->headvex ) ) < eps ) {
		//collisionvex->type = PSLGVertexType::CONVEX_VERTEX;
		if ( collisionvex->firstin->vanishTime != event->vanishTime ) {
			eventQueue->heapUpdateKey( collisionvex->firstin->heapIndex, event->vanishTime );
		} 
		if ( collisionvex->firstout->vanishTime != event->vanishTime ) {
			eventQueue->heapUpdateKey( collisionvex->firstout->heapIndex, event->vanishTime );
		}
	} else {
		//计算新顶点的速度
		PSLGGraph::calcConvexVertexSpeed( collisionvex );
		//更新新顶点的邻边的消失时间
		double t = PSLGGraph::calcEdgeVanishTime(collisionvex->firstin);
		eventQueue->heapUpdateKey(collisionvex->firstin->heapIndex, t);
		t = PSLGGraph::calcEdgeVanishTime(collisionvex->firstout);
		eventQueue->heapUpdateKey(collisionvex->firstout->heapIndex, t);
	}
	//释放这条边以及两顶点的空间（边已经在队列中删除）
	delete p1;
	delete p2;
	delete event;
	event = NULL;
}

void PSLGSSkeleton::handleSplitEvent(PSLGEdge* event) {
	if ( event->type == PSLGEdgeType::POLYGON_EDGE ) {
		PSLGVertex *u, *v; //u为凹点，v为Moving点
		if ( event->tailvex->type == PSLGVertexType::REFLEX_VERTEX ) {
			u = event->tailvex;
			v = event->headvex;
		} else {
			u = event->headvex;
			v = event->tailvex;
		}
		PSLGEdge* uedge = NULL; //u的另一条非多边形边
		PSLGVertex* uedgev = NULL;
		uedge = u->firstout->tedge;
		uedgev = uedge->headvex;
		if ( uedge == NULL ) { 
			uedge = u->firstin->hedge;
			uedgev = uedge->tailvex;
		}
		PSLGEdge* vedge = NULL; //v的另一条非多边形边
		vedge = v->firstin->hedge;
		if ( vedge == NULL ) {
			vedge = v->firstout->tedge;
		}
		PSLGEdge* uedge2; 
		std::vector< PSLGEdge* > restedges;
		uedgev->getVertexIncidentEdges( restedges );
		for ( std::vector< PSLGEdge* >::size_type i = 0; i < restedges.size(); i++ ) {
			if ( restedges[i] == vedge || restedges[i] == uedge ) continue;
			uedge2 = restedges[i];
		}
		if ( event->tailvex == v ) {
			addTriangleMesh( v->firstin->tailvex, v, uedgev->oriPosition, event->vanishTime );
			addTriangleMesh( v, u, uedgev->oriPosition, event->vanishTime );
			//保留 u->firstout, v->firstin
			v->firstin->headvex = u;
			u->firstin = v->firstin;
			if ( uedge2->tailvex == uedgev ) {
				uedge2->tailvex = u;
				u->firstout->tedge = uedge2;
				u->firstin->hedge = NULL;
			} else {
				uedge2->headvex = u;
				u->firstin->hedge = uedge2;
				u->firstout->tedge = NULL;
			}
			eventQueue->heapUpdateKey( uedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( uedge2 ) );
			eventQueue->heapUpdateKey( u->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstin ) );
			//删除 vedge, uedge, uedgev, v
			eventQueue->minHeapRemove( vedge->heapIndex );
			eventQueue->minHeapRemove( uedge->heapIndex );
			delete vedge;
			delete uedge;
			delete event;
			delete uedgev;
			delete v;
			return;
		} else {
			addTriangleMesh( u, v, uedgev->oriPosition, event->vanishTime );
			addTriangleMesh( v, v->firstout->headvex, uedgev->oriPosition, event->vanishTime );
			//保留 uin, v->firstout, vedge2
			v->firstout->tailvex = u;
			u->firstout = v->firstout;
			if ( uedge2->tailvex == uedgev ) {
				uedge2->tailvex = u;
				u->firstout->tedge = uedge2;
				u->firstin->hedge = NULL;
			} else {
				uedge2->headvex = u;
				u->firstin->hedge = uedge2;
				u->firstout->tedge = NULL;
			}
			eventQueue->heapUpdateKey( u->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstout ) );
			eventQueue->heapUpdateKey( uedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( uedge2 ) );
			//删除vedge, uedge, v, uedgev
			eventQueue->minHeapRemove( vedge->heapIndex );
			eventQueue->minHeapRemove( uedge->heapIndex );
			delete vedge;
			delete uedge;
			delete event;
			delete v;
			delete uedgev;
			return;
		}
	}
	//计算两顶点相遇的顶点坐标
	QPointF collisionPoint = event->tailvex->movedPosition( event->vanishTime );
	//将凹点对应的边加到直骨架中，并增加三角剖分
	if ( event->tailvex->type == PSLGVertexType::REFLEX_VERTEX ) {
		skeleton.push_back( QLineF( event->tailvex->oriPosition, collisionPoint ) );
		addTriangleMesh( event->tailvex->firstin->tailvex, event->tailvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->tailvex, event->tailvex->firstout->headvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->headvex->firstin->tailvex, event->headvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->headvex, event->headvex->firstout->headvex, collisionPoint, event->vanishTime );
	} else {
		skeleton.push_back( QLineF (event->headvex->oriPosition, collisionPoint ) );
		addTriangleMesh( event->headvex->firstin->tailvex, event->headvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->headvex, event->headvex->firstout->headvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->tailvex->firstin->tailvex, event->tailvex, collisionPoint, event->vanishTime );
		addTriangleMesh( event->tailvex, event->tailvex->firstout->headvex, collisionPoint, event->vanishTime );
	}
	//更新两个端点的坐标和起始时间
	event->tailvex->oriPosition.setX( collisionPoint.x() );
	event->tailvex->oriPosition.setY( collisionPoint.y() );
	event->tailvex->startTime = event->vanishTime;
	event->headvex->oriPosition.setX( collisionPoint.x() );
	event->headvex->oriPosition.setY( collisionPoint.y() );
	event->headvex->startTime = event->vanishTime;
	//分别找到两个顶点对应的多边形上的出边和入边
	//规定顶点的firstin和firstout即为多边形上的入边和出边，在创建图结构的时候维持这一性质
	PSLGEdge *tin, *tout, *hin, *hout;
	tin = event->tailvex->firstin;
	tout = event->tailvex->firstout;
	hin = event->headvex->firstin;
	hout = event->headvex->firstout;
    tout->tedge = NULL;
	hin->hedge = NULL;

	if ( tout->headvex == hin->tailvex ){
		//event、tout、hin构成的三角形将消失
		skeleton.push_back( QLineF( tout->headvex->oriPosition, collisionPoint ) );
		//保留event->tailvertex、tin、hout
		event->tailvex->firstout = hout;
		hout->tailvex = event->tailvex;
		PSLGGraph::calcVertexProperty( event->tailvex );
		eventQueue->heapUpdateKey( tin->heapIndex, PSLGGraph::calcEdgeVanishTime( tin ) );
		eventQueue->heapUpdateKey( hout->heapIndex, PSLGGraph::calcEdgeVanishTime( hout ) );
		//删除 tout、hin、tout->headvex、event、event->headvertex
		delete tout->headvex;
		eventQueue->minHeapRemove( tout->heapIndex );
		delete tout;
		eventQueue->minHeapRemove( hin->heapIndex );
		delete hin;
		delete event->headvex;
		delete event;
		return;
	} else if ( tin->tailvex == hout->headvex ) {
		//event、tin、hout构成的三角形将消失
		skeleton.push_back( QLineF( tin->tailvex->oriPosition, collisionPoint ) );
		//保留event->tailvertex、tout、hin
		event->tailvex->firstin = hin;
		hin->headvex = event->tailvex;
		PSLGGraph::calcVertexProperty( event->tailvex );
		eventQueue->heapUpdateKey( tout->heapIndex, PSLGGraph::calcEdgeVanishTime( tout ) );
		eventQueue->heapUpdateKey( hin->heapIndex, PSLGGraph::calcEdgeVanishTime( hin ) );
		//删除 tin、hout、tin->tailvex、event、event->headvertex
		delete tin->tailvex;
		eventQueue->minHeapRemove( tin->heapIndex );
		delete tin;
		eventQueue->minHeapRemove( hout->heapIndex );
		delete hout;
		delete event->headvex;
		delete event;
		return;
	}
	
    //改变两个端点的边链接关系
	event->tailvex->firstout = hout;
	hout->tailvex = event->tailvex;
	event->headvex->firstout = tout;
	tout->tailvex = event->headvex;
	event->tailvex->type = PSLGVertexType::CONVEX_VERTEX;
	//更新两个顶点的速度及四条邻边的消失时间，如果新顶点的两条边平行，新顶点的速度为0，邻边消失时间为当前时间
	//更新尾结点的速度和邻边消失时间
	if ( std::abs( PSLGGraph::determinant( event->tailvex->firstin->tailvex, event->tailvex, event->tailvex->firstout->headvex ) ) < eps ) {
		//event->tailvex->type = PSLGVertexType::CONVEX_VERTEX;
		event->tailvex->speed.setX( 0 );
		event->tailvex->speed.setY( 0 );
		eventQueue->heapUpdateKey( event->tailvex->firstin->heapIndex, event->vanishTime );
		eventQueue->heapUpdateKey( event->tailvex->firstout->heapIndex, event->vanishTime );
	} else {
		//更新两个顶点的速度
		//PSLGGraph::calcVertexProperty( event->tailvex );
		PSLGGraph::calcConvexVertexSpeed( event->tailvex );
		//更新四条邻边的消失时间
		double t = PSLGGraph::calcEdgeVanishTime( event->tailvex->firstin );
		eventQueue->heapUpdateKey( event->tailvex->firstin->heapIndex, t );
		t = PSLGGraph::calcEdgeVanishTime( event->tailvex->firstout );
		eventQueue->heapUpdateKey( event->tailvex->firstout->heapIndex, t );
	}
	//更新头结点的速度和邻边消失时间
	event->headvex->type = PSLGVertexType::CONVEX_VERTEX;
	if ( std::abs( PSLGGraph::determinant( event->headvex->firstin->tailvex, event->headvex, event->headvex->firstout->headvex ) ) < eps ) {
		//event->headvex->type = PSLGVertexType::CONVEX_VERTEX;
		event->headvex->speed.setX( 0 );
		event->headvex->speed.setY( 0 );
		eventQueue->heapUpdateKey( event->headvex->firstin->heapIndex, event->vanishTime );
		eventQueue->heapUpdateKey( event->headvex->firstout->heapIndex, event->vanishTime );
	} else {
		PSLGGraph::calcConvexVertexSpeed( event->headvex );

		double t = PSLGGraph::calcEdgeVanishTime( event->headvex->firstin );
		eventQueue->heapUpdateKey( event->headvex->firstin->heapIndex, t );
		t = PSLGGraph::calcEdgeVanishTime( event->headvex->firstout );
		eventQueue->heapUpdateKey( event->headvex->firstout->heapIndex, t );
	}

	delete event;
	event = NULL;
}

void PSLGSSkeleton::handleStartEvent(PSLGEdge* event) {
	//这个事件不产生直接的skeleton
	PSLGVertex *u, *v; //event的两个顶点，u为REFLEX_VERTEX, v为RESTING_STEINER_VERTEX
	PSLGEdge *uin, *uout; //u的两条多边形上的邻边
	
	if ( event->tailvex->type != PSLGVertexType::RESTING_STEINER_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	uin = u->firstin;
	uout = u->firstout;
	
	PSLGEdge *vedge1, *vedge2; //v的除event外的两条邻边，其中vedge1与event共线
	vedge1 = vedge2 = NULL;
	std::vector< PSLGEdge* > restes;
	v->getVertexIncidentEdges( restes );
	
	for ( int i = 0; i < restes.size(); i++ ) {
		if ( restes[i] == event ) continue;
		if ( PSLGEdge::isParallel( restes[i], event ) ) {
			vedge1 = restes[i];
		} else {
			vedge2 = restes[i];
		}
	}
	if ( vedge1 == NULL ) {
		//极端情况，v的关联边中没有与event平行的边
		//此时小三角形将消失
		if ( restes[0] == event ) {
			vedge1 = restes[1];
			vedge2 = restes[2];
		} else if ( restes[1] == event ){
			vedge1 = restes[0];
			vedge2 = restes[2];
		} else {
			vedge1 = restes[0];
			vedge2 = restes[1];
		}
		//让vedge1为v的入边，vedge2为v的出边
		if ( vedge1->tailvex == v ) {
			PSLGEdge* temp = vedge1;
			vedge1 = vedge2;
			vedge2 = temp;
		}
		if ( uout->headvex == vedge1->tailvex ) {
			PSLGVertex* vedge1v = vedge1->tailvex;
			addTriangleMesh( uin->tailvex, u, v->oriPosition, event->vanishTime );
			addTriangleMesh( u, vedge1v, v->oriPosition, event->vanishTime );
			//保留vedge1v, vedge1v->firstout, uin, vedge2
			uin->headvex = vedge1v;
			vedge1v->firstin = uin;
			vedge1v->firstout->tedge = vedge2;
			vedge2->tailvex = vedge1v;
			eventQueue->heapUpdateKey( vedge1v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1v->firstin ) );
			eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
			//删除vedge1, uout, u, v
			eventQueue->minHeapRemove( uout->heapIndex );
			eventQueue->minHeapRemove( vedge1->heapIndex );
			delete uout;
			delete vedge1;
			delete event;
			delete u;
			delete v;
			return;
		} else if ( uin->tailvex == vedge1->tailvex ){
			PSLGVertex* vedge1v = vedge1->tailvex;
			addTriangleMesh( vedge1v, u, v->oriPosition, event->vanishTime );
			addTriangleMesh( u, uout->headvex, v->oriPosition, event->vanishTime );
			//保留vedge1v, vedge1v->firstin, uout
			vedge1v->firstout = uout;
			uout->tailvex = vedge1v;
			vedge2->tailvex = vedge1v;
			vedge1v->firstout->tedge = vedge2;
			eventQueue->heapUpdateKey( vedge1v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1v->firstout ) );
			eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
			//删除uin, vedge1, u, v
			eventQueue->minHeapRemove( uin->heapIndex );
			eventQueue->minHeapRemove( vedge1->heapIndex );
			delete uin;
			delete vedge1;
			delete event;
			delete u;
			delete v;
			return;
		}else if ( uout->headvex == vedge2->headvex ) {
			PSLGVertex* vedge2v = vedge2->headvex;
			addTriangleMesh( uin->tailvex, u, v->oriPosition, event->vanishTime );
			addTriangleMesh( u, vedge2v, v->oriPosition, event->vanishTime );
			//保留uin, vedge2v->firstout, vedge1
			vedge2v->firstin = uin;
			uin->headvex = vedge2v;
			vedge1->headvex = vedge2v;
			vedge2v->firstin->hedge = vedge1;
			eventQueue->heapUpdateKey( vedge2v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2v->firstin ) );
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			//删除 uout, vedge2, u, v
			eventQueue->minHeapRemove( uout->heapIndex );
			eventQueue->minHeapRemove( vedge2->heapIndex );
			delete uout;
			delete vedge2;
			delete event;
			delete u;
			delete v;
			return;
		} else if ( uin->tailvex == vedge2->headvex ) {
			PSLGVertex* vedge2v = vedge2->headvex;
			addTriangleMesh( vedge2v, u, v->oriPosition, event->vanishTime );
			addTriangleMesh( u, uout->headvex, v->oriPosition, event->vanishTime );
			//保留 vedge2v->firstin, uout, vedge1
			vedge2v->firstout = uout;
			uout->tailvex = vedge2v;
			vedge1->headvex = vedge2v;
			vedge2v->firstin->hedge = vedge1;
			eventQueue->heapUpdateKey( vedge2v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2v->firstout ) );
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			//删除uin, vedge2, u, v
			eventQueue->minHeapRemove( uin->heapIndex );
			eventQueue->minHeapRemove( vedge2->heapIndex );
			delete uin;
			delete vedge2;
			delete event;
			delete u;
			delete v;
			return;
		}
	}
	//判断vedge2与uout是否位于event的同一侧
	PSLGVertex *vedge2v; //vedge2的不同于v的端点
	if ( vedge2->tailvex == v ) {
		vedge2v = vedge2->headvex;
	} else {
		vedge2v = vedge2->tailvex;
	}
	PSLGVertex* vt;
	for ( vt = uout->headvex; vt->type == PSLGVertexType::MOVING_STEINER_VERTEX; vt = vt->firstout->headvex );
	double d1 = PSLGGraph::determinant( u, v, vt );
	double d2 = PSLGGraph::determinant( u, v, vedge2v);
	if ( d1* d2 > 0 ) {  
		//vedge2与uout位于event的同侧
		//vedge2v与uout->headvex相等，此时uout、vedge2将消失
		if ( vedge2v == uout->headvex ) {
			addTriangleMesh( u, vedge2v, v->oriPosition, event->vanishTime );
			addTriangleMesh( vedge2v, vedge2v->firstout->headvex, v->oriPosition, event->vanishTime );
			u->firstout = vedge2v->firstout;
			vedge2v->firstout->tailvex = u;
			//更新vedge1的链接结构
			if ( vedge1->tailvex == v ) {
				vedge1->tailvex = u;
				u->firstout->tedge = vedge1;
			} else {
				vedge1->headvex = u;
				u->firstin->hedge = vedge1;
			}
			//更新vedge1、u->firstout的时间
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			eventQueue->heapUpdateKey( u->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstout ) );
			//删除uout、vedge2和v、vedge2v
			eventQueue->minHeapRemove( vedge2->heapIndex );
			eventQueue->minHeapRemove( uout->heapIndex );
			delete vedge2;
			delete uout;
			delete event;
			delete v;
			delete vedge2v;
			return;
		}
		//uout将被v点(vedge2)拆成两条边
		addTriangleMesh( u, uout->headvex, v->oriPosition, event->vanishTime );
		//将event的方向改为从u到v
		event->tailvex = u;
		event->headvex = v;
		event->hedge = event->tedge = NULL;
		//将event插入uout中，更新uin，uout的链接结构
		u->firstout = event;
		v->firstin = event;
		v->firstout = uout;
		uout->tailvex = v;
		uin->hedge = NULL;
		uout->tedge = NULL;
		//更新vedge1的链接结构
		if ( vedge1->tailvex == v ) {
			vedge1->tailvex = u;
			event->tedge = vedge1;
		} else {
			vedge1->headvex = u;
			uin->hedge = vedge1;
		}
		//更新vedge2的链接结构
		if ( vedge2->tailvex == v ) {
			uout->tedge = vedge2;
		} else {
			event->hedge = vedge2;
		}
		//更新v点的属性
		v->type = PSLGVertexType::MOVING_STEINER_VERTEX;
		v->startTime = event->vanishTime;
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, uout->headvex );
		//更新uout的消失时间
		eventQueue->heapUpdateKey( uout->heapIndex, PSLGGraph::calcEdgeVanishTime( uout ) );
	} else { 
		//vedge2与uout位于event的异侧
		//vedge2v与uin->tailVex相等，此时uin、vedge2将消失
		if ( vedge2v == uin->tailvex ) {
			addTriangleMesh( vedge2v->firstin->tailvex, vedge2v, v->oriPosition, event->vanishTime );
			addTriangleMesh( vedge2v, u, v->oriPosition, event->vanishTime );
			u->firstin = vedge2v->firstin;
			vedge2v->firstin->headvex = u;
			//更新vedge1的链接结构
			if ( vedge1->tailvex == v ) {
				vedge1->tailvex = u;
				u->firstout->tedge = vedge1;
			} else {
				vedge1->headvex = u;
				u->firstin->hedge = vedge1;
			}
			//更新vedge1、u->firstout的时间
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			eventQueue->heapUpdateKey( u->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstin ) );
			//删除uin、vedge2和v、vedge2v
			eventQueue->minHeapRemove( vedge2->heapIndex );
			eventQueue->minHeapRemove( uin->heapIndex );
			delete vedge2;
			delete uin;
			delete event;
			delete v;
			delete vedge2v;
			return;
		}
		//uin将被v点(vedge2)拆成两条边
		addTriangleMesh( uin->tailvex, u, v->oriPosition, event->vanishTime );
		//将event的方向改为从v到u
		event->tailvex = v;
		event->headvex = u;
		event->hedge = event->tedge = NULL;
		//将event插入uin中，更新uin，uout的链接结构
		u->firstin = event;
		v->firstout = event;
		v->firstin = uin;
		uin->headvex = v;
		uin->hedge = NULL;
		uout->tedge = NULL;
		//更新vedge1的链接结构
		if ( vedge1->tailvex == v ) {
			vedge1->tailvex = u;
			uout->tedge = vedge1;
		} else {
			vedge1->headvex = u;
			event->hedge = vedge1;
		}
		//更新vedge2的链接结构
		if ( vedge2->tailvex == v ) {
			event->tedge = vedge2;
		} else {
			uin->hedge = vedge2;
		}
		//更新v点的属性
		v->type = PSLGVertexType::MOVING_STEINER_VERTEX; //类型
		v->startTime = event->vanishTime; //开始时间
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, uin->tailvex ); //消失时间
		//更新uin的消失时间
		eventQueue->heapUpdateKey( uin->heapIndex, PSLGGraph::calcEdgeVanishTime( uin ) );
	}
	//更新event, vedge1,vedge2的消失时间
	event->vanishTime = std::numeric_limits< double >::max();
	event->type = PSLGEdgeType::POLYGON_EDGE;
	eventQueue->minHeapInsert( event );
	eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
	eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
}

void PSLGSSkeleton::handleSwitchEvent(PSLGEdge* event) {
	PSLGVertex *u, *v;  //event的两个顶点: u指向CONVEX_VERTEX; v指向REFLEX_VERTEX或MOVINGMOVING_STEINER_VERTEX
	
	if ( event->tailvex->type == CONVEX_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	PSLGEdge *uin, *uout; //u的入边与出边
	PSLGEdge *vin, *vout, *vedge;  //v的入边与出边，以及v的另一条不在多边形上的邻边
	uin = u->firstin;
	uout = u->firstout;
	vin = v->firstin;
	vout = v->firstout;
	if( vin->hedge != NULL ) {
		vedge = vin->hedge;
	} else {
		vedge = vout->tedge;
	}
	QPointF collisionPoint( u->movedPosition( event->vanishTime ) ); //两顶点相遇的点
	addTriangleMesh( v->firstin->tailvex, v, collisionPoint, event->vanishTime );
	addTriangleMesh( v, v->firstout->headvex, collisionPoint, event->vanishTime );
	//如果v为REFLEX_VERTEX，增加两条直骨架边；否则不增加直骨架边
	if ( v->type == PSLGVertexType::REFLEX_VERTEX ) { 
		skeleton.push_back( QLineF( u->oriPosition, collisionPoint ) );
		skeleton.push_back( QLineF( v->oriPosition, collisionPoint ) );
	} 
	//找到vedge的另一个端点
	PSLGVertex* vedge2v; //vedge不同于v的顶点
	if ( vedge->tailvex == v ) {
		vedge2v = vedge->headvex;
	} else {
		vedge2v = vedge->tailvex;
	}

	if ( event->tailvex == u && uin->tailvex == vedge2v ) {
		//三个点u,v,vedge2v一起湮灭
		if ( v->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			skeleton.push_back( QLineF( u->oriPosition, collisionPoint ) );
			if ( vedge2v->type == PSLGVertexType::REFLEX_VERTEX ) {
				skeleton.push_back( QLineF( vedge2v->oriPosition, collisionPoint ) );
			}
			addTriangleMesh( vedge2v->firstin->tailvex, vedge2v, collisionPoint, event->vanishTime );
			addTriangleMesh( vedge2v, u, collisionPoint, event->vanishTime );
			addTriangleMesh( vin->tailvex, v, collisionPoint, event->vanishTime );
			addTriangleMesh( v, vout->headvex, collisionPoint, event->vanishTime );
		} else {
			addTriangleMesh( vedge2v, u, collisionPoint, event->vanishTime );
			addTriangleMesh( vedge2v->firstin->tailvex, vedge2v, collisionPoint, event->vanishTime );
		}
		//保留v点，v点的两条邻边为vout, vedge2v->firstin
		v->firstin = vedge2v->firstin;
		vedge2v->firstin->headvex = v;
		v->firstout->tedge = NULL;
		v->firstin->hedge = NULL;
		//计算v点速度
		v->oriPosition.setX( collisionPoint.x() );
		v->oriPosition.setY( collisionPoint.y() );
		v->startTime = event->vanishTime;
		PSLGGraph::calcVertexProperty( v );
		eventQueue->heapUpdateKey( v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstin ) );
		eventQueue->heapUpdateKey( v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstout ) );
		//删除uin,uout,vedge, 删除u,vedge2v
		eventQueue->minHeapRemove( uin->heapIndex );
		eventQueue->minHeapRemove( vedge->heapIndex );
		delete uin;
		delete vedge;
		delete event;
		delete u;
		delete vedge2v;
		return;
	} else if ( event->headvex == u && uout->headvex == vedge2v ){
		if ( v->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			skeleton.push_back( QLineF( u->oriPosition, collisionPoint ) );
			if ( vedge2v->type == PSLGVertexType::REFLEX_VERTEX ) {
				skeleton.push_back( QLineF( vedge2v->oriPosition, collisionPoint ) );
			}
			addTriangleMesh( u, vedge2v, collisionPoint, event->vanishTime );
			addTriangleMesh( vedge2v, vedge2v->firstout->headvex, collisionPoint, event->vanishTime );
			addTriangleMesh( vin->tailvex, v, collisionPoint, event->vanishTime );
			addTriangleMesh( v, vout->headvex, collisionPoint, event->vanishTime );
		} else {
			addTriangleMesh( u, vedge2v, collisionPoint, event->vanishTime );
			addTriangleMesh( vedge2v, vedge2v->firstout->headvex, collisionPoint, event->vanishTime );
		}
		//保留v点，v点的两条邻边为vin vedge2v->firstout, 删除uin,uout,vedge
		v->firstout = vedge2v->firstout;
		vedge2v->firstout->tailvex = v;
		v->firstin->hedge = v->firstout->tedge = NULL;
		//计算v点速度
		v->oriPosition.setX( collisionPoint.x() );
		v->oriPosition.setY( collisionPoint.y() );
		v->startTime = event->vanishTime;
		PSLGGraph::calcVertexProperty( v );
		eventQueue->heapUpdateKey( v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstin ) );
		eventQueue->heapUpdateKey( v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstout ) );
		//删除uin,uout,vedge, 删除u,vedge2v
		eventQueue->minHeapRemove( uout->heapIndex );
		eventQueue->minHeapRemove( vedge->heapIndex );
		delete uout;
		delete vedge;
		delete event;
		delete u;
		delete vedge2v;
		return;
	}

	if ( event->tailvex == u ) {  //event的方向是从u到v
		addTriangleMesh( uin->tailvex, u, collisionPoint, event->vanishTime );
		//将event的方向改为从v到u，并更改event的链接关系
		event->tailvex = v;
		event->headvex = u;
		uin->headvex = v;
		v->firstin = uin;
		v->firstout = event;
		u->firstin = event;
		u->firstout = vout;
		vout->tailvex = u;
		event->tedge = event->hedge = NULL;
		vout->tedge = NULL;
	} else { //event的方向从v到u
		addTriangleMesh( u, uout->headvex, collisionPoint, event->vanishTime );
		//将event的方向设为从u到v
		event->tailvex = u;
		event->headvex = v;
		vin->headvex = u;
		u->firstin = vin;
		u->firstout = event;
		v->firstin = event;
		v->firstout = uout;
		uout->tailvex = v;
		event->tedge = event->hedge = NULL;
		vin->hedge = NULL;
	}
	
	//更新顶点的信息
	if ( v->type == PSLGVertexType::REFLEX_VERTEX ) {
		//u的速度发生变化
		u->oriPosition.setX( collisionPoint.x() );
		u->oriPosition.setY( collisionPoint.y() );
		u->startTime = event->vanishTime;
		if ( u->firstin->tailvex == v ) {
			//PSLGGraph::calcVertexProperty( v->firstin->tailvex, u, u->firstout->headvex );
			PSLGGraph::calcConvexVertexSpeed( v->firstin->tailvex, u, u->firstout->headvex );
			eventQueue->heapUpdateKey( u->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstout ) );
		} else {
			PSLGGraph::calcConvexVertexSpeed( u->firstin->tailvex, u, v->firstout->headvex );
			//PSLGGraph::calcVertexProperty( u->firstin->tailvex, u, v->firstout->headvex );
			eventQueue->heapUpdateKey( u->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstin ) );
		}
	} else {
		if ( u->firstin->tailvex == v ) {
			eventQueue->heapUpdateKey( u->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstout ) );
		} else {
			eventQueue->heapUpdateKey( u->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstin ) );
		}
	}

	//更新vedge的链接结构
	if ( vedge->tailvex == v ) {
		v->firstout->tedge = vedge;
	} else {
		v->firstin->hedge = vedge;
	}
	//v的速度变化
	v->oriPosition.setX( collisionPoint.x() );
	v->oriPosition.setY( collisionPoint.y() );
	v->startTime = event->vanishTime;
	v->type = PSLGVertexType::MOVING_STEINER_VERTEX;

	if ( u->firstin->tailvex == v ) {
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, v->firstin->tailvex );
		eventQueue->heapUpdateKey( v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstin ) );
	} else {
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, v->firstout->headvex );
		eventQueue->heapUpdateKey( v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstout ) );
	}
	//更新event, vedge的消失时间
	event->vanishTime = std::numeric_limits< double >::max();
	eventQueue->minHeapInsert( event );
	eventQueue->heapUpdateKey( vedge->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge ) );
}

void PSLGSSkeleton::handleMultiStartEvent(PSLGEdge* event) {
	//得到event的两个顶点
	PSLGVertex *u, *v; //u为MULTI_STEINER_VERTEX, v为MOVING_STEINER_VERTEX
	if ( event->tailvex->type == PSLGVertexType::MULTI_STEINER_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	//得到u的邻边
	std::vector< PSLGEdge* > uedges; //与u的所有邻边
	u->getVertexIncidentEdges( uedges );
	int index;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++) {
		if ( uedges[i] == event ) {
			index = i;
			continue;
		}
		if ( uedges[i]->tailvex == u ) {
			uedges[i]->tedge = NULL;
		} else {
			uedges[i]->hedge = NULL;
		}
	}
	//在uedges中去掉event边
	uedges[index] = uedges[uedges.size() - 1];
	uedges.pop_back();
	//得到v的邻边
	PSLGEdge *vin, *vout; //v的两条位于多边形上的边
	vin = v->firstin;
	vout = v->firstout;
	//将u的邻边按照与v所在的直线的角度排序，冒泡排序
	QVector2D v1( vout->headvex->movedPosition( event->vanishTime ) - vout->tailvex->movedPosition( event->vanishTime ) );
	v1.normalize();
	std::vector< double > angles;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size() ; i++) {
		QVector2D v2;
		if ( uedges[i]->tailvex == u ) {
			v2 = QVector2D( uedges[i]->headvex->oriPosition - u->oriPosition).normalized();
		} else {
			v2 = QVector2D( uedges[i]->tailvex->oriPosition - u->oriPosition).normalized();
		}
		angles.push_back( QVector2D::dotProduct( v1, v2 ) );
	}
	for ( std::vector< PSLGEdge* >::size_type i = 1; i < uedges.size() ; i++) {
		for ( std::vector< PSLGEdge* >::size_type j = uedges.size()-1; j > i-1; j--) {
			if ( angles[j] > angles[j-1] ) {
				double at = angles[j];
				angles[j] = angles[j-1];
				angles[j-1] = at;

				PSLGEdge* t;
				t = uedges[j];
				uedges[j] = uedges[j-1];
				uedges[j-1] = t;
			}
		}
	}
	//为uedges中的每条边新建一个端点
	std::vector< PSLGVertex* > uvertexs;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size() ; i++) {
		PSLGVertex* vt = new PSLGVertex( u->oriPosition, event->vanishTime );
		vt->type = PSLGVertexType::MOVING_STEINER_VERTEX;
		uvertexs.push_back( vt );
		//计算新顶点的速度，并更新uedges中边的消失时间
		if ( uedges[i]->tailvex == u) {
			uedges[i]->tailvex = vt;
			PSLGGraph::calcMovingSteinerSpeed( vt, uedges[i]->headvex, vout->headvex ) ;
			eventQueue->heapUpdateKey( uedges[i]->heapIndex, PSLGGraph::calcEdgeVanishTime( uedges[i] ) );
		} else {
			uedges[i]->headvex = vt;
			PSLGGraph::calcMovingSteinerSpeed( vt, uedges[i]->tailvex, vout->headvex );
			eventQueue->heapUpdateKey( uedges[i]->heapIndex, PSLGGraph::calcEdgeVanishTime( uedges[i] ) );
		}
	}
	
	//依次连接uvertexs中相邻的顶点，每两点之间形成一条边，插入到事件队列中
	vin->headvex = uvertexs[uvertexs.size()-1];
	uvertexs[uvertexs.size()-1]->firstin = vin;
	eventQueue->heapUpdateKey( vin->heapIndex, PSLGGraph::calcEdgeVanishTime( vin ) );
	for ( std::vector< PSLGVertex* >::size_type i = uvertexs.size()-1; i > 0; i-- ) {
		PSLGEdge* e = new PSLGEdge( uvertexs[i], uvertexs[i-1] );
		uvertexs[i]->firstout = e;
		uvertexs[i-1]->firstin = e;
		e->vanishTime = std::numeric_limits< double >::max();
		e->type = PSLGEdgeType::POLYGON_EDGE;
		eventQueue->minHeapInsert( e );
	}
	vout->tailvex = uvertexs[0];
	uvertexs[0]->firstout = vout;
	eventQueue->heapUpdateKey( vout->heapIndex, PSLGGraph::calcEdgeVanishTime( vout ) );
	//建立uvertexs与uedges间的链接关系
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++) {
		if ( uedges[i]->tailvex == uvertexs[i] ) {
			uvertexs[i]->firstout->tedge = uedges[i];
		} else {
			uvertexs[i]->firstin->hedge = uedges[i];
		}
	}
	//释放顶点u,v和边event的空间
	delete u;
	delete v;
	delete event;
}

void PSLGSSkeleton::handleMultiSplitEvent(PSLGEdge* event) {
	PSLGVertex *u; //为MULTI_STEINER_VERTEX的点
	if ( event->headvex->type == PSLGVertexType::MULTI_STEINER_VERTEX ) {
		u = event->headvex;
	} else {
		u = event->tailvex;
	}
	//得到u点的所有邻边
	std::vector< PSLGEdge* > uedges; 
	u->getVertexIncidentEdges( uedges );
	//将新产生的摩托边（u的邻边，但是拎一个端点的类型不是REFLEX_VERTEX）取出，存放到nonredge中
	int index;
	PSLGEdge* nonredge = NULL;;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++ ) {
		if ( uedges[i]->headvex != u && uedges[i]->headvex->type != PSLGVertexType::REFLEX_VERTEX) {
			index = i;
			break;
		} else if ( uedges[i]->tailvex != u && uedges[i]->tailvex->type !=  PSLGVertexType::REFLEX_VERTEX ) {
			index = i;
			break;
		}
	}
	if ( index < uedges.size() ) {
		nonredge = uedges[index];
		uedges[index] = uedges[uedges.size()-1];
		uedges.pop_back();
	}
	
	//得到与uedges对应的不同于u的顶点
	std::vector< PSLGVertex* > uvertexs;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++ ) {
		if ( uedges[i]->headvex != u ) {
			uvertexs.push_back( uedges[i]->headvex );
		} else {
			uvertexs.push_back( uedges[i]->tailvex );
		}
	}
	//将uedges按逆时针排序，冒泡排序
	for ( std::vector< PSLGVertex* >::size_type i = 1; i < uvertexs.size(); i++ ) {
		for ( std::vector< PSLGVertex* >::size_type j = uvertexs.size() - 1; j > i-1; j-- ) {
			if ( PSLGGraph::determinant( uvertexs[j-1], u, uvertexs[j] ) > 0 ) {
				PSLGVertex* vt = uvertexs[j];
				uvertexs[j] = uvertexs[j-1];
				uvertexs[j-1] = vt;

				PSLGEdge* et = uedges[j];
				uedges[j] = uedges[j-1];
				uedges[j-1] = et;
			}
		}
	}
	//每个凹点对应一条直骨架边，然后更新这个点的属性
	for ( std::vector< PSLGVertex* >::size_type i = 0; i < uvertexs.size(); i++ ) {
		skeleton.push_back( QLineF( uvertexs[i]->oriPosition, u->oriPosition ) );
		uvertexs[i]->oriPosition.setX( u->oriPosition.x() );
		uvertexs[i]->oriPosition.setY( u->oriPosition.y() );
		uvertexs[i]->startTime = event->vanishTime;
		uvertexs[i]->firstin->hedge = NULL;
		uvertexs[i]->firstout->tedge = NULL;
	}
	//更新点的链接关系和点的速度及相邻边的消失时间
	PSLGEdge* out1 = uvertexs[0]->firstout;
	for ( std::vector< PSLGVertex* >::size_type i = 1; i < uvertexs.size(); i++ ) {
		PSLGEdge* out2 = uvertexs[i]->firstout;
		uvertexs[i]->firstout = out1;
		out1->tailvex = uvertexs[i];
		out1 = out2;
		uvertexs[i]->type = PSLGVertexType::CONVEX_VERTEX;
		//PSLGGraph::calcVertexProperty( uvertexs[i] ); //计算新点的速度
		PSLGGraph::calcConvexVertexSpeed( uvertexs[i] ); 
		eventQueue->heapUpdateKey( uvertexs[i]->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( uvertexs[i]->firstin ) ); //更新邻边的消失时间
		eventQueue->heapUpdateKey( uvertexs[i]->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( uvertexs[i]->firstout ) );
	}
	uvertexs[0]->firstout = out1;
	out1->tailvex = uvertexs[0];
	PSLGGraph::calcVertexProperty( uvertexs[0] );
	eventQueue->heapUpdateKey( uvertexs[0]->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( uvertexs[0]->firstin ) );
	eventQueue->heapUpdateKey( uvertexs[0]->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( uvertexs[0]->firstout ) );
	if ( nonredge != NULL && uvertexs[0]->type == PSLGVertexType::REFLEX_VERTEX ) {
		if ( nonredge->headvex == u ) {
			nonredge->headvex = uvertexs[0];
			uvertexs[0]->firstin->hedge = nonredge;
			nonredge->hedge = NULL;
			eventQueue->heapUpdateKey( nonredge->heapIndex, PSLGGraph::calcEdgeVanishTime( nonredge ) );
		} else {
			nonredge->tailvex = uvertexs[0];
			uvertexs[0]->firstout->tedge = nonredge;
			nonredge->tedge = NULL;
			eventQueue->heapUpdateKey( nonredge->heapIndex, PSLGGraph::calcEdgeVanishTime( nonredge ) );
		}
	} else if ( nonredge != NULL ) {
		//删掉nonredge
	}
	//将uedges的边从队列中去掉，其中event已经从队列中剔除了
	for (std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++ ) {
		if (uedges[i] != event) {
			eventQueue->minHeapRemove( uedges[i]->heapIndex );
		}
		delete uedges[i];
	}
	delete u;
}
	
void PSLGSSkeleton::handleRemainingEvent( PSLGEdge* event ) {
	if ( event->tailvex->type == PSLGVertexType::MOVING_STEINER_VERTEX && 
		 event->headvex->type == PSLGVertexType::MOVING_STEINER_VERTEX ) {
			 //删除节点以及对应的边
			 PSLGEdge *tin, *tout, *hin, *hout; //尾结点和头结点的入边和出边
			 tin = event->tailvex->firstin;
			 tout = event->tailvex->firstout;
			 hin = event->headvex->firstin;
			 hout = event->headvex->firstout;
			 if ( tout == hin ) {
				 if ( tin->hedge == hout->tedge ) {
					 eventQueue->minHeapRemove( tin->hedge->heapIndex );
					 delete tin->hedge;
				 }
				 QPointF p = event->tailvex->movedPosition( event->vanishTime );
				 if ( PSLGVertex::equalPosition( p, event->tailvex->oriPosition ) ) {
					 p = event->headvex->movedPosition( event->vanishTime );
				 }
				 addTriangleMesh( tin->tailvex, event->tailvex, p, event->vanishTime );
				 addTriangleMesh( event->tailvex, event->headvex, p, event->vanishTime );
				 addTriangleMesh( event->headvex, hout->headvex, p, event->vanishTime );
				 hout->tailvex = tin->headvex;
				 //tin->headvex->firstin = hout;
				 tin->headvex->firstout = hout;
				 tin->hedge = hout->tedge = NULL;
				 PSLGGraph::calcVertexProperty( tin->headvex );
				 eventQueue->heapUpdateKey( tin->heapIndex, PSLGGraph::calcEdgeVanishTime( tin ) );
				 eventQueue->heapUpdateKey( hout->heapIndex, PSLGGraph::calcEdgeVanishTime( hout ) );
				 delete event->headvex;
				 delete event;
			 } else {
				 QPointF p = event->tailvex->movedPosition( event->vanishTime );
				 addTriangleMesh( tin->tailvex, event->tailvex, p, event->vanishTime );
				 addTriangleMesh( event->tailvex, tout->headvex, p, event->vanishTime );
				 addTriangleMesh( hin->tailvex, event->headvex, p, event->vanishTime );
				 addTriangleMesh( event->headvex, hout->headvex, p, event->vanishTime );
				 //保留tin边，并将tin边指向tout的头结点
				 tin->headvex = tout->headvex;
				 tout->headvex->firstin = tin;
				 tin->hedge = tout->hedge;
				 //保留hin边，并将hin指向hout的头结点
				 hin->headvex = hout->headvex;
				 hout->headvex->firstin = hin;
				 hin->hedge = hout->hedge;
				 eventQueue->minHeapRemove( tout->heapIndex ); //删除tout边
				 eventQueue->heapUpdateKey( tin->heapIndex, PSLGGraph::calcEdgeVanishTime( tin ) );
				 eventQueue->minHeapRemove( hout->heapIndex ); //删除hout边
				 eventQueue->heapUpdateKey( hin->heapIndex, PSLGGraph::calcEdgeVanishTime( hin ) );
				 delete tout;
				 delete hout;
				 delete event->tailvex;
				 delete event->headvex;
				 delete event;
			 }
	}
}

void PSLGSSkeleton::addTriangleMesh( PSLGVertex* v1, PSLGVertex* v2, PSLGVertex* v3 ) {
	double d = PSLGGraph::determinant( v1->oriPosition, v2->oriPosition, v3->oriPosition );
	if ( std::abs(d) < eps ) {
		return;
	} else if ( d > 0 ) {
		meshes.push_back( Triangle3D( QVector3D( v1->oriPosition.x(), v1->oriPosition.y(), v1->startTime ), 
			                                             QVector3D( v2->oriPosition.x(), v2->oriPosition.y(), v2->startTime ), 
			                                             QVector3D( v3->oriPosition.x(), v3->oriPosition.y(), v3->startTime ) ) );
	} else {
		meshes.push_back( Triangle3D( QVector3D( v3->oriPosition.x(), v3->oriPosition.y(), v3->startTime ), 
			                                             QVector3D( v2->oriPosition.x(), v2->oriPosition.y(), v2->startTime ), 
			                                             QVector3D( v1->oriPosition.x(), v1->oriPosition.y(), v1->startTime ) ) );
	}
}

void PSLGSSkeleton::addTriangleMesh( PSLGVertex* v1, PSLGVertex* v2, const QPointF& p3, double t3 ) {
	double d = PSLGGraph::determinant( v1->oriPosition, v2->oriPosition, p3 );
	if ( std::abs(d) < eps ) {
		return;
	} else if ( d > 0 ) {
		meshes.push_back( Triangle3D( QVector3D( v1->oriPosition.x(), v1->oriPosition.y(), v1->startTime ), 
			                                             QVector3D( v2->oriPosition.x(), v2->oriPosition.y(), v2->startTime ),
			                                             QVector3D( p3.x(), p3.y(), t3 ) ) );
	} else {
		meshes.push_back( Triangle3D( QVector3D( p3.x(), p3.y(), t3 ), 
			                                             QVector3D( v2->oriPosition.x(), v2->oriPosition.y(), v2->startTime ),
			                                             QVector3D( v1->oriPosition.x(), v1->oriPosition.y(), v1->startTime ) ) );
	}
}

void PSLGSSkeleton::addTriangleMesh( const QPointF& p1, double t1, const QPointF& p2, double t2, const QPointF& p3, double t3 ) {
	double d = PSLGGraph::determinant( p1, p2, p3 );
	if ( std::abs(d) < eps ) {
		return;
	} else if ( d > 0 ) {
		meshes.push_back( Triangle3D( QVector3D( p1.x(), p1.y(), t1 ), QVector3D( p2.x(), p2.y(), t2 ), QVector3D( p3.x(), p3.y(), t3 ) ) );
	} else {
		meshes.push_back( Triangle3D( QVector3D( p3.x(), p3.y(), t3 ), QVector3D( p2.x(), p2.y(), t2 ), QVector3D( p1.x(), p1.y(), t1 ) ) );
	}
}
//void main() {
//	QPolygonF poly;
//	poly.append(QPointF(0,0));
//	poly.append(QPointF(10, 0));
//	poly.append(QPointF(8, 8));
//	poly.append(QPointF(5, 5));
//	poly.append(QPointF(2, 8));
//	poly.append(QPointF(0, 0));
//
//	PSLGGraph* graph = new PSLGGraph();
//	graph->addPolygon(poly);
//
//	PSLGSSkeleton* ss = new PSLGSSkeleton(graph);
//	ss->extractSkeletion();
//}
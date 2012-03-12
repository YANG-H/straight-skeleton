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

	//�����м��������ı߿�����ֱ�Ǽ�
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

		//���timeInterval==0��ȫ����
		if ( std::abs( timeInterval ) < eps || std::abs( sleepInterval) < eps) {
			while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
				PSLGEdge* e = eventQueue->heapExtractMin();
				curtime = e->vanishTime;
				handleEvent(e);
			}
		} else {
			while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
				if ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime > timelimit ) {
					//ȫ����
					while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime < std::numeric_limits<double>::max() ) {
						PSLGEdge* e = eventQueue->heapExtractMin();
						curtime = e->vanishTime;
						handleEvent(e);
					}
				} else { //��ǰ������ɢ
					while ( !eventQueue->empty() && eventQueue->heapMinimum()->vanishTime <= curtimeInterval+eps ) {
						PSLGEdge* e = eventQueue->heapExtractMin();
						curtime = e->vanishTime;
						handleEvent(e);

						//��ǰ�Ĳ�ǰ
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
						//��ǰ�Ĳ�ǰ
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

		//�����м��������ı߿�����ֱ�Ǽ�
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

// EdgeEvent ����͹��ϲ���һ���µ�͹��
void PSLGSSkeleton::handleEdgeEvent(PSLGEdge* event) {
	//����ñ��˻�Ϊһ����
	if (event->headvex == event->tailvex) {
		delete event->headvex;
		delete event;
		return;
	}
	//�����¶��������
	QPointF collisionPoint = event->tailvex->movedPosition( event->vanishTime );
	PSLGVertex* collisionvex = new PSLGVertex(collisionPoint, event->vanishTime); 
	//�γ�����ֱ�Ǽ�
	if ( !PSLGVertex::equalPosition( event->tailvex->oriPosition, collisionPoint ) ) {
		skeleton.push_back( QLineF( event->tailvex->oriPosition, collisionPoint ) );
	}
	if ( !PSLGVertex::equalPosition( event->headvex->oriPosition, collisionPoint ) ) {
		skeleton.push_back( QLineF( event->headvex->oriPosition, collisionPoint ) );
	}

	//���������ʷ֣�����Ϊ��ʱ��
	addTriangleMesh( event->tailvex, event->headvex, collisionPoint, event->vanishTime );
	addTriangleMesh( event->tailvex->firstin->tailvex, event->tailvex, collisionPoint, event->vanishTime );
	addTriangleMesh( event->headvex, event->headvex->firstout->headvex, collisionPoint, event->vanishTime );

	//�����¶��������ڱߵ����ӹ�ϵ
	PSLGVertex* p1 = event->tailvex;
	PSLGVertex* p2 = event->headvex;
	p1->firstin->headvex = collisionvex;
	p2->firstout->tailvex = collisionvex;
	collisionvex->firstin = p1->firstin;
	collisionvex->firstout = p2->firstout;
	collisionvex->type = PSLGVertexType::CONVEX_VERTEX;
	//����µ������ڱ�ƽ�У�ֱ�Ӹ��������ڱ���ʧʱ��Ϊ��ǰ��ʱ�䣬���ø��µ���ٶ�
	if ( std::abs( PSLGGraph::determinant( collisionvex->firstin->tailvex, collisionvex, collisionvex->firstout->headvex ) ) < eps ) {
		//collisionvex->type = PSLGVertexType::CONVEX_VERTEX;
		if ( collisionvex->firstin->vanishTime != event->vanishTime ) {
			eventQueue->heapUpdateKey( collisionvex->firstin->heapIndex, event->vanishTime );
		} 
		if ( collisionvex->firstout->vanishTime != event->vanishTime ) {
			eventQueue->heapUpdateKey( collisionvex->firstout->heapIndex, event->vanishTime );
		}
	} else {
		//�����¶�����ٶ�
		PSLGGraph::calcConvexVertexSpeed( collisionvex );
		//�����¶�����ڱߵ���ʧʱ��
		double t = PSLGGraph::calcEdgeVanishTime(collisionvex->firstin);
		eventQueue->heapUpdateKey(collisionvex->firstin->heapIndex, t);
		t = PSLGGraph::calcEdgeVanishTime(collisionvex->firstout);
		eventQueue->heapUpdateKey(collisionvex->firstout->heapIndex, t);
	}
	//�ͷ��������Լ�������Ŀռ䣨���Ѿ��ڶ�����ɾ����
	delete p1;
	delete p2;
	delete event;
	event = NULL;
}

void PSLGSSkeleton::handleSplitEvent(PSLGEdge* event) {
	if ( event->type == PSLGEdgeType::POLYGON_EDGE ) {
		PSLGVertex *u, *v; //uΪ���㣬vΪMoving��
		if ( event->tailvex->type == PSLGVertexType::REFLEX_VERTEX ) {
			u = event->tailvex;
			v = event->headvex;
		} else {
			u = event->headvex;
			v = event->tailvex;
		}
		PSLGEdge* uedge = NULL; //u����һ���Ƕ���α�
		PSLGVertex* uedgev = NULL;
		uedge = u->firstout->tedge;
		uedgev = uedge->headvex;
		if ( uedge == NULL ) { 
			uedge = u->firstin->hedge;
			uedgev = uedge->tailvex;
		}
		PSLGEdge* vedge = NULL; //v����һ���Ƕ���α�
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
			//���� u->firstout, v->firstin
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
			//ɾ�� vedge, uedge, uedgev, v
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
			//���� uin, v->firstout, vedge2
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
			//ɾ��vedge, uedge, v, uedgev
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
	//���������������Ķ�������
	QPointF collisionPoint = event->tailvex->movedPosition( event->vanishTime );
	//�������Ӧ�ı߼ӵ�ֱ�Ǽ��У������������ʷ�
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
	//���������˵���������ʼʱ��
	event->tailvex->oriPosition.setX( collisionPoint.x() );
	event->tailvex->oriPosition.setY( collisionPoint.y() );
	event->tailvex->startTime = event->vanishTime;
	event->headvex->oriPosition.setX( collisionPoint.x() );
	event->headvex->oriPosition.setY( collisionPoint.y() );
	event->headvex->startTime = event->vanishTime;
	//�ֱ��ҵ����������Ӧ�Ķ�����ϵĳ��ߺ����
	//�涨�����firstin��firstout��Ϊ������ϵ���ߺͳ��ߣ��ڴ���ͼ�ṹ��ʱ��ά����һ����
	PSLGEdge *tin, *tout, *hin, *hout;
	tin = event->tailvex->firstin;
	tout = event->tailvex->firstout;
	hin = event->headvex->firstin;
	hout = event->headvex->firstout;
    tout->tedge = NULL;
	hin->hedge = NULL;

	if ( tout->headvex == hin->tailvex ){
		//event��tout��hin���ɵ������ν���ʧ
		skeleton.push_back( QLineF( tout->headvex->oriPosition, collisionPoint ) );
		//����event->tailvertex��tin��hout
		event->tailvex->firstout = hout;
		hout->tailvex = event->tailvex;
		PSLGGraph::calcVertexProperty( event->tailvex );
		eventQueue->heapUpdateKey( tin->heapIndex, PSLGGraph::calcEdgeVanishTime( tin ) );
		eventQueue->heapUpdateKey( hout->heapIndex, PSLGGraph::calcEdgeVanishTime( hout ) );
		//ɾ�� tout��hin��tout->headvex��event��event->headvertex
		delete tout->headvex;
		eventQueue->minHeapRemove( tout->heapIndex );
		delete tout;
		eventQueue->minHeapRemove( hin->heapIndex );
		delete hin;
		delete event->headvex;
		delete event;
		return;
	} else if ( tin->tailvex == hout->headvex ) {
		//event��tin��hout���ɵ������ν���ʧ
		skeleton.push_back( QLineF( tin->tailvex->oriPosition, collisionPoint ) );
		//����event->tailvertex��tout��hin
		event->tailvex->firstin = hin;
		hin->headvex = event->tailvex;
		PSLGGraph::calcVertexProperty( event->tailvex );
		eventQueue->heapUpdateKey( tout->heapIndex, PSLGGraph::calcEdgeVanishTime( tout ) );
		eventQueue->heapUpdateKey( hin->heapIndex, PSLGGraph::calcEdgeVanishTime( hin ) );
		//ɾ�� tin��hout��tin->tailvex��event��event->headvertex
		delete tin->tailvex;
		eventQueue->minHeapRemove( tin->heapIndex );
		delete tin;
		eventQueue->minHeapRemove( hout->heapIndex );
		delete hout;
		delete event->headvex;
		delete event;
		return;
	}
	
    //�ı������˵�ı����ӹ�ϵ
	event->tailvex->firstout = hout;
	hout->tailvex = event->tailvex;
	event->headvex->firstout = tout;
	tout->tailvex = event->headvex;
	event->tailvex->type = PSLGVertexType::CONVEX_VERTEX;
	//��������������ٶȼ������ڱߵ���ʧʱ�䣬����¶����������ƽ�У��¶�����ٶ�Ϊ0���ڱ���ʧʱ��Ϊ��ǰʱ��
	//����β�����ٶȺ��ڱ���ʧʱ��
	if ( std::abs( PSLGGraph::determinant( event->tailvex->firstin->tailvex, event->tailvex, event->tailvex->firstout->headvex ) ) < eps ) {
		//event->tailvex->type = PSLGVertexType::CONVEX_VERTEX;
		event->tailvex->speed.setX( 0 );
		event->tailvex->speed.setY( 0 );
		eventQueue->heapUpdateKey( event->tailvex->firstin->heapIndex, event->vanishTime );
		eventQueue->heapUpdateKey( event->tailvex->firstout->heapIndex, event->vanishTime );
	} else {
		//��������������ٶ�
		//PSLGGraph::calcVertexProperty( event->tailvex );
		PSLGGraph::calcConvexVertexSpeed( event->tailvex );
		//���������ڱߵ���ʧʱ��
		double t = PSLGGraph::calcEdgeVanishTime( event->tailvex->firstin );
		eventQueue->heapUpdateKey( event->tailvex->firstin->heapIndex, t );
		t = PSLGGraph::calcEdgeVanishTime( event->tailvex->firstout );
		eventQueue->heapUpdateKey( event->tailvex->firstout->heapIndex, t );
	}
	//����ͷ�����ٶȺ��ڱ���ʧʱ��
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
	//����¼�������ֱ�ӵ�skeleton
	PSLGVertex *u, *v; //event���������㣬uΪREFLEX_VERTEX, vΪRESTING_STEINER_VERTEX
	PSLGEdge *uin, *uout; //u������������ϵ��ڱ�
	
	if ( event->tailvex->type != PSLGVertexType::RESTING_STEINER_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	uin = u->firstin;
	uout = u->firstout;
	
	PSLGEdge *vedge1, *vedge2; //v�ĳ�event��������ڱߣ�����vedge1��event����
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
		//���������v�Ĺ�������û����eventƽ�еı�
		//��ʱС�����ν���ʧ
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
		//��vedge1Ϊv����ߣ�vedge2Ϊv�ĳ���
		if ( vedge1->tailvex == v ) {
			PSLGEdge* temp = vedge1;
			vedge1 = vedge2;
			vedge2 = temp;
		}
		if ( uout->headvex == vedge1->tailvex ) {
			PSLGVertex* vedge1v = vedge1->tailvex;
			addTriangleMesh( uin->tailvex, u, v->oriPosition, event->vanishTime );
			addTriangleMesh( u, vedge1v, v->oriPosition, event->vanishTime );
			//����vedge1v, vedge1v->firstout, uin, vedge2
			uin->headvex = vedge1v;
			vedge1v->firstin = uin;
			vedge1v->firstout->tedge = vedge2;
			vedge2->tailvex = vedge1v;
			eventQueue->heapUpdateKey( vedge1v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1v->firstin ) );
			eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
			//ɾ��vedge1, uout, u, v
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
			//����vedge1v, vedge1v->firstin, uout
			vedge1v->firstout = uout;
			uout->tailvex = vedge1v;
			vedge2->tailvex = vedge1v;
			vedge1v->firstout->tedge = vedge2;
			eventQueue->heapUpdateKey( vedge1v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1v->firstout ) );
			eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
			//ɾ��uin, vedge1, u, v
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
			//����uin, vedge2v->firstout, vedge1
			vedge2v->firstin = uin;
			uin->headvex = vedge2v;
			vedge1->headvex = vedge2v;
			vedge2v->firstin->hedge = vedge1;
			eventQueue->heapUpdateKey( vedge2v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2v->firstin ) );
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			//ɾ�� uout, vedge2, u, v
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
			//���� vedge2v->firstin, uout, vedge1
			vedge2v->firstout = uout;
			uout->tailvex = vedge2v;
			vedge1->headvex = vedge2v;
			vedge2v->firstin->hedge = vedge1;
			eventQueue->heapUpdateKey( vedge2v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2v->firstout ) );
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			//ɾ��uin, vedge2, u, v
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
	//�ж�vedge2��uout�Ƿ�λ��event��ͬһ��
	PSLGVertex *vedge2v; //vedge2�Ĳ�ͬ��v�Ķ˵�
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
		//vedge2��uoutλ��event��ͬ��
		//vedge2v��uout->headvex��ȣ���ʱuout��vedge2����ʧ
		if ( vedge2v == uout->headvex ) {
			addTriangleMesh( u, vedge2v, v->oriPosition, event->vanishTime );
			addTriangleMesh( vedge2v, vedge2v->firstout->headvex, v->oriPosition, event->vanishTime );
			u->firstout = vedge2v->firstout;
			vedge2v->firstout->tailvex = u;
			//����vedge1�����ӽṹ
			if ( vedge1->tailvex == v ) {
				vedge1->tailvex = u;
				u->firstout->tedge = vedge1;
			} else {
				vedge1->headvex = u;
				u->firstin->hedge = vedge1;
			}
			//����vedge1��u->firstout��ʱ��
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			eventQueue->heapUpdateKey( u->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstout ) );
			//ɾ��uout��vedge2��v��vedge2v
			eventQueue->minHeapRemove( vedge2->heapIndex );
			eventQueue->minHeapRemove( uout->heapIndex );
			delete vedge2;
			delete uout;
			delete event;
			delete v;
			delete vedge2v;
			return;
		}
		//uout����v��(vedge2)���������
		addTriangleMesh( u, uout->headvex, v->oriPosition, event->vanishTime );
		//��event�ķ����Ϊ��u��v
		event->tailvex = u;
		event->headvex = v;
		event->hedge = event->tedge = NULL;
		//��event����uout�У�����uin��uout�����ӽṹ
		u->firstout = event;
		v->firstin = event;
		v->firstout = uout;
		uout->tailvex = v;
		uin->hedge = NULL;
		uout->tedge = NULL;
		//����vedge1�����ӽṹ
		if ( vedge1->tailvex == v ) {
			vedge1->tailvex = u;
			event->tedge = vedge1;
		} else {
			vedge1->headvex = u;
			uin->hedge = vedge1;
		}
		//����vedge2�����ӽṹ
		if ( vedge2->tailvex == v ) {
			uout->tedge = vedge2;
		} else {
			event->hedge = vedge2;
		}
		//����v�������
		v->type = PSLGVertexType::MOVING_STEINER_VERTEX;
		v->startTime = event->vanishTime;
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, uout->headvex );
		//����uout����ʧʱ��
		eventQueue->heapUpdateKey( uout->heapIndex, PSLGGraph::calcEdgeVanishTime( uout ) );
	} else { 
		//vedge2��uoutλ��event�����
		//vedge2v��uin->tailVex��ȣ���ʱuin��vedge2����ʧ
		if ( vedge2v == uin->tailvex ) {
			addTriangleMesh( vedge2v->firstin->tailvex, vedge2v, v->oriPosition, event->vanishTime );
			addTriangleMesh( vedge2v, u, v->oriPosition, event->vanishTime );
			u->firstin = vedge2v->firstin;
			vedge2v->firstin->headvex = u;
			//����vedge1�����ӽṹ
			if ( vedge1->tailvex == v ) {
				vedge1->tailvex = u;
				u->firstout->tedge = vedge1;
			} else {
				vedge1->headvex = u;
				u->firstin->hedge = vedge1;
			}
			//����vedge1��u->firstout��ʱ��
			eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
			eventQueue->heapUpdateKey( u->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( u->firstin ) );
			//ɾ��uin��vedge2��v��vedge2v
			eventQueue->minHeapRemove( vedge2->heapIndex );
			eventQueue->minHeapRemove( uin->heapIndex );
			delete vedge2;
			delete uin;
			delete event;
			delete v;
			delete vedge2v;
			return;
		}
		//uin����v��(vedge2)���������
		addTriangleMesh( uin->tailvex, u, v->oriPosition, event->vanishTime );
		//��event�ķ����Ϊ��v��u
		event->tailvex = v;
		event->headvex = u;
		event->hedge = event->tedge = NULL;
		//��event����uin�У�����uin��uout�����ӽṹ
		u->firstin = event;
		v->firstout = event;
		v->firstin = uin;
		uin->headvex = v;
		uin->hedge = NULL;
		uout->tedge = NULL;
		//����vedge1�����ӽṹ
		if ( vedge1->tailvex == v ) {
			vedge1->tailvex = u;
			uout->tedge = vedge1;
		} else {
			vedge1->headvex = u;
			event->hedge = vedge1;
		}
		//����vedge2�����ӽṹ
		if ( vedge2->tailvex == v ) {
			event->tedge = vedge2;
		} else {
			uin->hedge = vedge2;
		}
		//����v�������
		v->type = PSLGVertexType::MOVING_STEINER_VERTEX; //����
		v->startTime = event->vanishTime; //��ʼʱ��
		PSLGGraph::calcMovingSteinerSpeed( v, vedge2v, uin->tailvex ); //��ʧʱ��
		//����uin����ʧʱ��
		eventQueue->heapUpdateKey( uin->heapIndex, PSLGGraph::calcEdgeVanishTime( uin ) );
	}
	//����event, vedge1,vedge2����ʧʱ��
	event->vanishTime = std::numeric_limits< double >::max();
	event->type = PSLGEdgeType::POLYGON_EDGE;
	eventQueue->minHeapInsert( event );
	eventQueue->heapUpdateKey( vedge1->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge1 ) );
	eventQueue->heapUpdateKey( vedge2->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge2 ) );
}

void PSLGSSkeleton::handleSwitchEvent(PSLGEdge* event) {
	PSLGVertex *u, *v;  //event����������: uָ��CONVEX_VERTEX; vָ��REFLEX_VERTEX��MOVINGMOVING_STEINER_VERTEX
	
	if ( event->tailvex->type == CONVEX_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	PSLGEdge *uin, *uout; //u����������
	PSLGEdge *vin, *vout, *vedge;  //v���������ߣ��Լ�v����һ�����ڶ�����ϵ��ڱ�
	uin = u->firstin;
	uout = u->firstout;
	vin = v->firstin;
	vout = v->firstout;
	if( vin->hedge != NULL ) {
		vedge = vin->hedge;
	} else {
		vedge = vout->tedge;
	}
	QPointF collisionPoint( u->movedPosition( event->vanishTime ) ); //�����������ĵ�
	addTriangleMesh( v->firstin->tailvex, v, collisionPoint, event->vanishTime );
	addTriangleMesh( v, v->firstout->headvex, collisionPoint, event->vanishTime );
	//���vΪREFLEX_VERTEX����������ֱ�Ǽܱߣ���������ֱ�Ǽܱ�
	if ( v->type == PSLGVertexType::REFLEX_VERTEX ) { 
		skeleton.push_back( QLineF( u->oriPosition, collisionPoint ) );
		skeleton.push_back( QLineF( v->oriPosition, collisionPoint ) );
	} 
	//�ҵ�vedge����һ���˵�
	PSLGVertex* vedge2v; //vedge��ͬ��v�Ķ���
	if ( vedge->tailvex == v ) {
		vedge2v = vedge->headvex;
	} else {
		vedge2v = vedge->tailvex;
	}

	if ( event->tailvex == u && uin->tailvex == vedge2v ) {
		//������u,v,vedge2vһ������
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
		//����v�㣬v��������ڱ�Ϊvout, vedge2v->firstin
		v->firstin = vedge2v->firstin;
		vedge2v->firstin->headvex = v;
		v->firstout->tedge = NULL;
		v->firstin->hedge = NULL;
		//����v���ٶ�
		v->oriPosition.setX( collisionPoint.x() );
		v->oriPosition.setY( collisionPoint.y() );
		v->startTime = event->vanishTime;
		PSLGGraph::calcVertexProperty( v );
		eventQueue->heapUpdateKey( v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstin ) );
		eventQueue->heapUpdateKey( v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstout ) );
		//ɾ��uin,uout,vedge, ɾ��u,vedge2v
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
		//����v�㣬v��������ڱ�Ϊvin vedge2v->firstout, ɾ��uin,uout,vedge
		v->firstout = vedge2v->firstout;
		vedge2v->firstout->tailvex = v;
		v->firstin->hedge = v->firstout->tedge = NULL;
		//����v���ٶ�
		v->oriPosition.setX( collisionPoint.x() );
		v->oriPosition.setY( collisionPoint.y() );
		v->startTime = event->vanishTime;
		PSLGGraph::calcVertexProperty( v );
		eventQueue->heapUpdateKey( v->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstin ) );
		eventQueue->heapUpdateKey( v->firstout->heapIndex, PSLGGraph::calcEdgeVanishTime( v->firstout ) );
		//ɾ��uin,uout,vedge, ɾ��u,vedge2v
		eventQueue->minHeapRemove( uout->heapIndex );
		eventQueue->minHeapRemove( vedge->heapIndex );
		delete uout;
		delete vedge;
		delete event;
		delete u;
		delete vedge2v;
		return;
	}

	if ( event->tailvex == u ) {  //event�ķ����Ǵ�u��v
		addTriangleMesh( uin->tailvex, u, collisionPoint, event->vanishTime );
		//��event�ķ����Ϊ��v��u��������event�����ӹ�ϵ
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
	} else { //event�ķ����v��u
		addTriangleMesh( u, uout->headvex, collisionPoint, event->vanishTime );
		//��event�ķ�����Ϊ��u��v
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
	
	//���¶������Ϣ
	if ( v->type == PSLGVertexType::REFLEX_VERTEX ) {
		//u���ٶȷ����仯
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

	//����vedge�����ӽṹ
	if ( vedge->tailvex == v ) {
		v->firstout->tedge = vedge;
	} else {
		v->firstin->hedge = vedge;
	}
	//v���ٶȱ仯
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
	//����event, vedge����ʧʱ��
	event->vanishTime = std::numeric_limits< double >::max();
	eventQueue->minHeapInsert( event );
	eventQueue->heapUpdateKey( vedge->heapIndex, PSLGGraph::calcEdgeVanishTime( vedge ) );
}

void PSLGSSkeleton::handleMultiStartEvent(PSLGEdge* event) {
	//�õ�event����������
	PSLGVertex *u, *v; //uΪMULTI_STEINER_VERTEX, vΪMOVING_STEINER_VERTEX
	if ( event->tailvex->type == PSLGVertexType::MULTI_STEINER_VERTEX ) {
		u = event->tailvex;
		v = event->headvex;
	} else {
		u = event->headvex;
		v = event->tailvex;
	}
	//�õ�u���ڱ�
	std::vector< PSLGEdge* > uedges; //��u�������ڱ�
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
	//��uedges��ȥ��event��
	uedges[index] = uedges[uedges.size() - 1];
	uedges.pop_back();
	//�õ�v���ڱ�
	PSLGEdge *vin, *vout; //v������λ�ڶ�����ϵı�
	vin = v->firstin;
	vout = v->firstout;
	//��u���ڱ߰�����v���ڵ�ֱ�ߵĽǶ�����ð������
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
	//Ϊuedges�е�ÿ�����½�һ���˵�
	std::vector< PSLGVertex* > uvertexs;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size() ; i++) {
		PSLGVertex* vt = new PSLGVertex( u->oriPosition, event->vanishTime );
		vt->type = PSLGVertexType::MOVING_STEINER_VERTEX;
		uvertexs.push_back( vt );
		//�����¶�����ٶȣ�������uedges�бߵ���ʧʱ��
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
	
	//��������uvertexs�����ڵĶ��㣬ÿ����֮���γ�һ���ߣ����뵽�¼�������
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
	//����uvertexs��uedges������ӹ�ϵ
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++) {
		if ( uedges[i]->tailvex == uvertexs[i] ) {
			uvertexs[i]->firstout->tedge = uedges[i];
		} else {
			uvertexs[i]->firstin->hedge = uedges[i];
		}
	}
	//�ͷŶ���u,v�ͱ�event�Ŀռ�
	delete u;
	delete v;
	delete event;
}

void PSLGSSkeleton::handleMultiSplitEvent(PSLGEdge* event) {
	PSLGVertex *u; //ΪMULTI_STEINER_VERTEX�ĵ�
	if ( event->headvex->type == PSLGVertexType::MULTI_STEINER_VERTEX ) {
		u = event->headvex;
	} else {
		u = event->tailvex;
	}
	//�õ�u��������ڱ�
	std::vector< PSLGEdge* > uedges; 
	u->getVertexIncidentEdges( uedges );
	//���²�����Ħ�бߣ�u���ڱߣ�������һ���˵�����Ͳ���REFLEX_VERTEX��ȡ������ŵ�nonredge��
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
	
	//�õ���uedges��Ӧ�Ĳ�ͬ��u�Ķ���
	std::vector< PSLGVertex* > uvertexs;
	for ( std::vector< PSLGEdge* >::size_type i = 0; i < uedges.size(); i++ ) {
		if ( uedges[i]->headvex != u ) {
			uvertexs.push_back( uedges[i]->headvex );
		} else {
			uvertexs.push_back( uedges[i]->tailvex );
		}
	}
	//��uedges����ʱ������ð������
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
	//ÿ�������Ӧһ��ֱ�Ǽܱߣ�Ȼ���������������
	for ( std::vector< PSLGVertex* >::size_type i = 0; i < uvertexs.size(); i++ ) {
		skeleton.push_back( QLineF( uvertexs[i]->oriPosition, u->oriPosition ) );
		uvertexs[i]->oriPosition.setX( u->oriPosition.x() );
		uvertexs[i]->oriPosition.setY( u->oriPosition.y() );
		uvertexs[i]->startTime = event->vanishTime;
		uvertexs[i]->firstin->hedge = NULL;
		uvertexs[i]->firstout->tedge = NULL;
	}
	//���µ�����ӹ�ϵ�͵���ٶȼ����ڱߵ���ʧʱ��
	PSLGEdge* out1 = uvertexs[0]->firstout;
	for ( std::vector< PSLGVertex* >::size_type i = 1; i < uvertexs.size(); i++ ) {
		PSLGEdge* out2 = uvertexs[i]->firstout;
		uvertexs[i]->firstout = out1;
		out1->tailvex = uvertexs[i];
		out1 = out2;
		uvertexs[i]->type = PSLGVertexType::CONVEX_VERTEX;
		//PSLGGraph::calcVertexProperty( uvertexs[i] ); //�����µ���ٶ�
		PSLGGraph::calcConvexVertexSpeed( uvertexs[i] ); 
		eventQueue->heapUpdateKey( uvertexs[i]->firstin->heapIndex, PSLGGraph::calcEdgeVanishTime( uvertexs[i]->firstin ) ); //�����ڱߵ���ʧʱ��
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
		//ɾ��nonredge
	}
	//��uedges�ıߴӶ�����ȥ��������event�Ѿ��Ӷ������޳���
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
			 //ɾ���ڵ��Լ���Ӧ�ı�
			 PSLGEdge *tin, *tout, *hin, *hout; //β����ͷ������ߺͳ���
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
				 //����tin�ߣ�����tin��ָ��tout��ͷ���
				 tin->headvex = tout->headvex;
				 tout->headvex->firstin = tin;
				 tin->hedge = tout->hedge;
				 //����hin�ߣ�����hinָ��hout��ͷ���
				 hin->headvex = hout->headvex;
				 hout->headvex->firstin = hin;
				 hin->hedge = hout->hedge;
				 eventQueue->minHeapRemove( tout->heapIndex ); //ɾ��tout��
				 eventQueue->heapUpdateKey( tin->heapIndex, PSLGGraph::calcEdgeVanishTime( tin ) );
				 eventQueue->minHeapRemove( hout->heapIndex ); //ɾ��hout��
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
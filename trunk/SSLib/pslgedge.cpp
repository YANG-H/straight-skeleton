#include "pslgvertex.h"
#include "pslgedge.h"
#include <limits>

#define eps 1.0e-6

PSLGEdge::PSLGEdge() : mark(false), tailvex(NULL), headvex(NULL), tedge(NULL), hedge(NULL),
	vanishTime(std::numeric_limits<double>::max()), type(PSLGEdgeType::POLYGON_EDGE), heapIndex(-1){
}

PSLGEdge::PSLGEdge( PSLGVertex* tv, PSLGVertex* hv ) : mark(false), tailvex(tv), headvex(hv), tedge(NULL), hedge(NULL),
	vanishTime(std::numeric_limits<double>::max()), type(PSLGEdgeType::POLYGON_EDGE), heapIndex(-1){

}

PSLGEdge::PSLGEdge(PSLGVertex* tv, PSLGVertex* hv, PSLGEdge* te,  PSLGEdge* he) : mark(false), tailvex(tv), headvex(hv), 
	tedge(te), hedge(he), vanishTime(std::numeric_limits<double>::max()), type(PSLGEdgeType::POLYGON_EDGE), heapIndex(-1) {
 }

PSLGEdge::~PSLGEdge() {

}

bool PSLGEdge::isParallel( PSLGEdge* e1, PSLGEdge* e2 ) {
	double start = std::max( e1->tailvex->startTime, e1->headvex->startTime );
	QPointF p1 = e1->tailvex->movedPosition( start );
	QPointF p2 = e1->headvex->movedPosition( start );

	start = std::max( e2->tailvex->startTime, e2->headvex->startTime );
	QPointF p3 = e2->tailvex->movedPosition( start );
	QPointF p4 = e2->headvex->movedPosition( start );
	QVector2D v1(p1 - p2);
	QVector2D v2(p3 - p4);
	v1.normalize();
	v2.normalize();
	
	if ( std::abs( v1.x() * v2.y() - v1.y() * v2.x() ) < 1.0e-4 ) {
		return true;
	}
	return false;
}

bool PSLGEdge::isVertexOnEdge( PSLGVertex* v, PSLGEdge* e ) {
	double start;
	start = std::max( e->tailvex->startTime, e->headvex->startTime );
	start = std::max( v->startTime, start );
	QPointF vp = v->movedPosition( start );
	QPointF tp = e->tailvex->movedPosition( start );
	QPointF hp = e->headvex->movedPosition( start );
	
	double d1 = std::abs( hp.x() - tp.x() );
	double d2 = std::abs( hp.y() - tp.y() );
	if ( d1 < eps && d2 < eps ) {
		if ( std::abs( vp.x() - tp.x() ) < eps ) { //三点重合
			return true;
		} else {
			return false;
		}
	} else if ( d1 < d2 ) {
		start = (vp.y() - tp.y()) / (hp.y() - tp.y()); 
	} else {
		start = (vp.x() - tp.x()) / (hp.x() - tp.x()); 
	}

	if ( ( std::abs(start) < eps ) || ( std::abs(start-1) < eps ) || (start > 0 && start < 1) ) {
		return true;
	} else {
		return false;
	}
}

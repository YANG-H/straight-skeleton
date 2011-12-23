#include "pslgvertex.h"
#include "pslgedge.h"

#define  eps 1.0e-6

PSLGVertex::PSLGVertex() : mark(false), oriPosition(0, 0), speed(0, 0), startTime(0.0), 
	firstin(NULL), firstout(NULL), type(PSLGVertexType::OTHER_VERTEX){
}

PSLGVertex::PSLGVertex( const QPointF& p ) : mark(false), oriPosition(p), speed(0, 0), startTime(0),
	firstin(NULL), firstout(NULL), type(PSLGVertexType::OTHER_VERTEX) {

}

PSLGVertex::PSLGVertex(const QPointF&p, double time) : mark(false), oriPosition(p), speed(0, 0), startTime(time), 
	firstin(NULL), firstout(NULL), type(PSLGVertexType::OTHER_VERTEX) {

}

PSLGVertex::PSLGVertex(const QPointF& p, const QVector2D& v, double time) : mark(false), oriPosition(p), 
	speed(v), startTime(time), firstin(NULL), firstout(NULL), type(PSLGVertexType::OTHER_VERTEX) {
}

PSLGVertex::PSLGVertex(const PSLGVertex& vex) : mark(false), oriPosition(vex.oriPosition), 
	speed(vex.speed), startTime(vex.startTime), firstin(vex.firstin), firstout(vex.firstout), type(vex.type) {
}

PSLGVertex::~PSLGVertex() {

}

QPointF PSLGVertex::movedPosition(double curtime) {
	if ( curtime == startTime ) {
		return oriPosition;
	}
	QVector2D moved = speed * (curtime- startTime);
	return QPointF( oriPosition.x() + moved.x(),  oriPosition.y() + moved.y() );
}


void PSLGVertex::getVertexInEdges( std::vector< PSLGEdge* > &ins ) {
	PSLGEdge* in;
	for ( in = firstin; in != NULL; in = in->hedge ) {
		ins.push_back( in );
	}
}

void PSLGVertex::getVertexOutEdges( std::vector< PSLGEdge* > &outs ) {
	PSLGEdge* out;
	for ( out = firstout; out != NULL; out = out->tedge ) {
		outs.push_back( out );
	}
}

void PSLGVertex::getVertexIncidentEdges( std::vector< PSLGEdge* > &es ) {
	PSLGEdge* e;
	for ( e = firstin; e != NULL; e = e->hedge ) {
		es.push_back( e );
	}
	for ( e = firstout; e != NULL; e = e->tedge ) {
		es.push_back( e );
	}
}

void PSLGVertex::getVertexAdjacentVertexes( std::vector< PSLGVertex* > &vs ) {
	PSLGEdge* e;
	for ( e = firstin; e != NULL; e = e->hedge ) {
		vs.push_back( e->tailvex );
	}
	for ( e = firstout; e != NULL; e = e->tedge ) {
	    vs.push_back( e->headvex );
	}
}

bool PSLGVertex::equalPosition( const QPointF& p1, const QPointF& p2 ) {
	return std::abs(p1.x() - p2.x()) < eps && std::abs(p1.y() - p2.y()) < eps;
}
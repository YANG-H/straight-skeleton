#include "priorityqueue.h"

PriorityQueue::PriorityQueue() : edges(NULL), heapsize(0) {

}

PriorityQueue::PriorityQueue(const std::vector<PSLGEdge*>& es) : edges(es), heapsize(0){

}

void PriorityQueue::minHeapify(int i) {
	int l = left(i);
	int r = right(i);
	int least = i;
	if ( l < heapsize && edges[l]->vanishTime < edges[i]->vanishTime ) {
		least = l;
	}
	if ( r < heapsize && edges[r]->vanishTime < edges[least]->vanishTime ) {
		least = r;
	}
	if (least != i) {
		swap(i, least);
		minHeapify(least);
	}
}

void PriorityQueue::buildMinHeap() {
	heapsize = edges.size();
	for (int i = 0; i < heapsize; i++) {
		edges[i]->heapIndex = i;
	}
	for ( int i = (edges.size() - 2)/2; i >= 0; i-- ) {
		minHeapify(i);
	}
}

PSLGEdge* PriorityQueue::heapMinimum() {
	return edges[0];
}

PSLGEdge* PriorityQueue::heapExtractMin() {
	if (heapsize < 1) return NULL;

	PSLGEdge* m = edges[0];
	swap(0, edges.size()-1);
	heapsize = heapsize - 1;
	minHeapify(0);
	//把最小（被换到了最后）的元素移除
	//edges.erase(edges.size()-1);
	edges.pop_back();
	
	return m;
}

void PriorityQueue::heapDecreaseKey(int i, double key) {
	if (key > edges[i]->vanishTime) return;

	edges[i]->vanishTime = key;
	while ( i > 0 && edges[parent(i)]->vanishTime > edges[i]->vanishTime ) {
		swap( i, parent(i) );
		i = parent(i);
	}
}

void PriorityQueue::heapUpdateKey( int i, double key ) {
	if( i >= heapsize ) 
		return;

	if ( key > edges[i]->vanishTime ) {
		edges[i]->vanishTime = key;
		minHeapify(i);
	} else {
		edges[i]->vanishTime = key;
		while ( i > 0 && edges[parent(i)]->vanishTime > edges[i]->vanishTime ) {
			swap( i, parent(i) );
			i = parent(i);
		}
	}
}

void PriorityQueue::minHeapInsert(PSLGEdge* e) {
	heapsize = heapsize + 1;
	edges.push_back(e);
	edges[heapsize-1]->heapIndex = heapsize - 1;
	double key = e->vanishTime;
	e->vanishTime = std::numeric_limits<double>::max();
	heapDecreaseKey(heapsize-1, key);
}

bool PriorityQueue::empty() {
	return edges.empty();
}

void PriorityQueue::minHeapRemove( int i ) {
	if ( i >= heapsize ) return;
	swap(i, edges.size()-1);
	heapsize = heapsize - 1;
	minHeapify( i );
	edges.pop_back();
}
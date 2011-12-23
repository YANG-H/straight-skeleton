/**
 * 基于二叉堆实现优先级队列
 * @author wumabert, 20111129
 */

#ifndef PRIORITYQUEUE_H
#define  PRIORITYQUEUE_H

#include <vector>
#include "pslgedge.h"

class PriorityQueue {
public:
	std::vector<PSLGEdge*> edges;
	int heapsize;

public: 
	PriorityQueue();
	PriorityQueue(const std::vector<PSLGEdge*>& es);

private:
	inline int parent(int i) {
		return (i-1)/2;
	}

	inline int left(int i) {
		return 2*i+1;
	}

	inline int right(int i) {
		return 2*i + 2;
	}

	inline void swap(int i, int j) {
		PSLGEdge* temp = edges[i];
		edges[i] = edges[j];
		edges[j] = temp;

		edges[i]->heapIndex = i;
		edges[j]->heapIndex = j;
	}

public:
	void minHeapify(int i);
	void buildMinHeap();
	PSLGEdge* heapMinimum();
	PSLGEdge* heapExtractMin();
	void heapDecreaseKey(int i, double key);
	void minHeapInsert(PSLGEdge* e);
	void heapUpdateKey( int i, double key );
	bool empty();
	void minHeapRemove( int i ); //删除最小堆中的第i个元素
};

#endif //PRIORITYQUEUE_H
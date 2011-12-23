/*====================================================================*
-  Copyright (C) 2001 Leptonica.  All rights reserved.
-  This software is distributed in the hope that it will be
-  useful, but with NO WARRANTY OF ANY KIND.
-  No author or distributor accepts responsibility to anyone for the
-  consequences of using this software, or for whether it serves any
-  particular purpose or works at all, unless he or she says so in
-  writing.  Everyone is granted permission to copy, modify and
-  redistribute this source code, for commercial or non-commercial
-  purposes, with the following restrictions: (1) the origin of this
-  source code must not be misrepresented; (2) modified versions must
-  be plainly marked as such; and (3) this notice may not be removed
-  or altered from any source or modified source distribution.
*====================================================================*/


/*
*  heap.h
*
*      Expandable priority queue configured as a heap for arbitrary void* data
*
*      The MinHeap is used to implement a priority queue.  The elements
*      in the heap are ordered in either increasing or decreasing key value.
*      The key is a float field 'keyval' that is required to be
*      contained in the elements of the queue.
*
*      The heap is a simple binary tree with the following constraints:
*         - the key of each node is >= the keys of the two children
*         - the tree is complete, meaning that each level (1, 2, 4, ...)
*           is filled and the last level is filled from left to right
*
*      The tree structure is implicit in the queue array, with the
*      array elements numbered as a breadth-first search of the tree
*      from left to right.  It is thus guaranteed that the largest
*      (or smallest) key belongs to the first element in the array.
*
*      Heap sort is used to sort the array.  Once an array has been
*      sorted as a heap, it is convenient to use it as a priority queue,
*      because the min (or max) elements are always at the root of
*      the tree (element 0), and once removed, the heap can be
*      resorted in not more than log[n] steps, where n is the number
*      of elements on the heap.  Likewise, if an arbitrary element is
*      added to the end of the array A, the sorted heap can be restored
*      in not more than log[n] steps.
*
*      A MinHeap differs from a L_Queue in that the elements in the former
*      are sorted by a key.  Internally, the array is maintained
*      as a queue, with a pointer to the end of the array.  The
*      head of the array always remains at array[0].  The array is
*      maintained (sorted) as a heap.  When an item is removed from
*      the head, the last item takes its place (thus reducing the
*      array length by 1), and this is followed by array element
*      swaps to restore the heap property.   When an item is added,
*      it goes at the end of the array, and is swapped up to restore
*      the heap.  If the ptr array is full, adding another item causes
*      the ptr array size to double.
*
*      For further implementation details, see heap.c.
*/
#pragma once
#define L_SORT_INCREASING 0
#define L_SORT_DECREASING 1
#include "CrashEvent.h"
struct HeapEle
{
	int id;
	CrashEvent* e;
	double key;
};
class MinHeap
{
public:
	void print(FILE    *fp);
	int sortStrictOrder();
	int sort();
	int swapDown();
	int swapUp(int  index);
	int getCount();
	struct HeapEle* remove();
	int extendArray();
	int add(HeapEle *item);
	void destroy(int freeflag);
	void create(int  nalloc, int  direction);
public:
	int nalloc;      /* size of allocated ptr array                 */
	int n;           /* number of elements stored in the heap       */
	int direction;   /* L_SORT_INCREASING or L_SORT_DECREASING      */
	struct HeapEle **ptrArray;       /* ptr array                                   */

};

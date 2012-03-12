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
*   heap.c
*
*      Create/Destroy MinHeap
*          MinHeap    *create()
*          void      *destroy()
*
*      Operations to add/remove to/from the heap
*          int    add()
*          int    extendArray()
*          void      *remove()
*
*      Heap operations
*          int    swapUp()
*          int    swapDown()
*          int    sort()
*          int    sortStrictOrder()
*
*      Accessors
*          int    getCount()
*
*      Debug output
*          int    print()
*
*    The MinHeap is useful to implement a priority queue, that is sorted
*    on a key in each element of the heap.  The heap is an array
*    of nearly arbitrary structs, with a float the first field.
*    This field is the key on which the heap is sorted.
*
*    Internally, we keep track of the heap size, n.  The item at the
*    root of the heap is at the head of the array.  Items are removed
*    from the head of the array and added to the end of the array.
*    When an item is removed from the head, the item at the end
*    of the array is moved to the head.  When items are either
*    added or removed, it is usually necesary to swap array items
*    to restore the heap order.  It is guaranteed that the number
*    of swaps does not exceed log(n).
*
*    --------------------------  N.B.  ------------------------------
*    The items on the heap (or, equivalently, in the array) are cast
*    to void*.  Their key is a float, and it is REQUIRED that the
*    key be the first field in the struct.  That allows us to get the
*    key by simply dereferencing the struct.  Alternatively, we could
*    choose (but don't) to pass an application-specific comparison
*    function into the heap operation functions.
*    --------------------------  N.B.  ------------------------------
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MinHeap.h"


static const int  MIN_BUFFER_SIZE = 20;             /* n'importe quoi */
static const int  INITIAL_BUFFER_ARRAYSIZE = 128;   /* n'importe quoi */

#define SWAP_ITEMS(i, j)       { struct HeapEle *tempitem = this->ptrArray[(i)]; \
	this->ptrArray[(i)] = this->ptrArray[(j)]; \
	this->ptrArray[(j)] = tempitem;\
	this->ptrArray[(i)]->id = i;\
	this->ptrArray[(j)]->id = j;}


/*--------------------------------------------------------------------------*
*                          MinHeap create/destroy                           *
*--------------------------------------------------------------------------*/
/*!
*  create()
*
*      Input:  size of ptr array to be alloc'd (0 for default)
*              direction (L_SORT_INCREASING, L_SORT_DECREASING)
*      Return: lheap, or null on error
*/
void MinHeap::create(int  nalloc, int  direction)
{

	if (nalloc < MIN_BUFFER_SIZE)
		nalloc = MIN_BUFFER_SIZE;

	/* Allocate ptr array and initialize counters. */

	this->ptrArray = (struct HeapEle **)malloc(nalloc * sizeof(struct HeapEle *));

	this->nalloc = nalloc;
	this->n = 0;
	this->direction = direction;

}


/*!
*  destroy()
*
*      Input:  &lheap  (<to be nulled>)
*              freeflag (TRUE to free each remaining struct in the array)
*      Return: void
*
*  Notes:
*      (1) Use freeflag == TRUE when the items in the array can be
*          simply destroyed using free.  If those items require their
*          own destroy function, they must be destroyed before
*          calling this function, and then this function is called
*          with freeflag == FALSE.
*      (2) To destroy the lheap, we destroy the ptr array, then
*          the lheap, and then null the contents of the input ptr.
*/
void MinHeap::destroy(int freeflag)
{
	free(this->ptrArray);
	this->n = 0;
	this->nalloc = 0;

}

/*--------------------------------------------------------------------------*
*                                  Accessors                               *
*--------------------------------------------------------------------------*/
/*!
*  add()
*
*      Input:  lheap
*              item to be added to the tail of the heap
*      Return: 0 if OK, 1 on error
*/
int MinHeap::add(struct HeapEle *item)
{


	if (!item)
		return 1;

	/* If necessary, expand the allocated array by a factor of 2 */
	if (this->n >= this->nalloc)
		extendArray();

	/* Add the item */
	item->id = this->n;
	this->ptrArray[this->n] = item;
	this->n++;

	/* Restore the heap */
	return swapUp(this->n - 1);

}


/*!
*  extendArray()
*
*      Input:  lheap
*      Return: 0 if OK, 1 on error
*/
int MinHeap::extendArray()
{

	struct HeapEle** arrayPtr = this->ptrArray;


	if ((this->ptrArray = (struct HeapEle **)malloc(2 * sizeof(struct HeapEle *) * this->nalloc)) == NULL)
		return 1;
	for(int i = 0; i < this->n; i++)
		this->ptrArray[i] = arrayPtr[i];
	this->nalloc = 2 * this->nalloc;
	free(arrayPtr);
	return 0;
}


/*!
*  remove()
*
*      Input:  lheap
*      Return: ptr to item popped from the root of the heap,
*              or null if the heap is empty or on error
*/
struct HeapEle* MinHeap::remove()
{
	struct HeapEle   *item;





	if (this->n == 0)
		return NULL;

	item = this->ptrArray[0];
	this->ptrArray[0] = this->ptrArray[this->n - 1];  /* move last to the head */
	this->ptrArray[0]->id = 0;
	this->ptrArray[this->n - 1] = NULL;  /* set ptr to null */
	this->n--;

	swapDown();  /* restore the heap */
	return item;
}


/*!
*  getCount()
*
*      Input:  lheap
*      Return: count, or 0 on error
*/
int MinHeap::getCount()
{
	return this->n;
}



/*--------------------------------------------------------------------------*
*                               Heap operations                            *
*--------------------------------------------------------------------------*/
/*!
*  swapUp()
*
*      Input:  lh (heap)
*              index (of array corresponding to node to be swapped up)
*      Return: 0 if OK, 1 on error
*
*  Notes:
*      (1) This is called after a new item is put on the heap, at the
*          bottom of a complete tree.
*      (2) To regain the heap order, we let it bubble up,
*          iteratively swapping with its parent, until it either
*          reaches the root of the heap or it finds a parent that
*          is in the correct position already vis-a-vis the child.
*/
int MinHeap::swapUp(int  index)
{
	int    ip;  /* index to heap for parent; 1 larger than array index */
	int    ic;  /* index into heap for child */
	double  valp, valc;



	if (index < 0 || index >= this->n)
		return 1;

	ic = index + 1;  /* index into heap: add 1 to array index */
	if (this->direction == L_SORT_INCREASING) 
	{
		while (1) 
		{
			if (ic == 1)  /* root of heap */
				break;
			ip = ic / 2;
			valc = this->ptrArray[ic - 1]->key;
			valp = this->ptrArray[ip - 1]->key;
			if (valp <= valc)
				break;
			SWAP_ITEMS(ip - 1, ic - 1);
			ic = ip;
		}
	}
	else
	{
		/* lh->direction == L_SORT_DECREASING */
		while (1)
		{
			if (ic == 1)  /* root of heap */
				break;
			ip = ic / 2;
			valc = this->ptrArray[ic - 1]->key;
			valp = this->ptrArray[ip - 1]->key;
			if (valp >= valc)
				break;

			SWAP_ITEMS(ip - 1, ic - 1);
			ic = ip;
		}
	}
	return ic;
}


/*!
*  swapDown()
*
*      Input:  lh (heap)
*      Return: 0 if OK, 1 on error
*
*  Notes:
*      (1) This is called after an item has been popped off the
*          root of the heap, and the last item in the heap has
*          been placed at the root.
*      (2) To regain the heap order, we let it bubble down,
*          iteratively swapping with one of its children.  For a
*          decreasing sort, it swaps with the largest child; for
*          an increasing sort, the smallest.  This continues until
*          it either reaches the lowest level in the heap, or the
*          parent finds that neither child should swap with it
*          (e.g., for a decreasing heap, the parent is larger
*          than or equal to both children).
*/
int MinHeap::swapDown()
{
	int    ip;  /* index to heap for parent; 1 larger than array index */
	int    icr, icl;  /* index into heap for left/right children */
	double  valp, valcl, valcr;

	if (getCount() < 1)
		return 0;

	ip = 1;  /* index into top of heap: corresponds to array[0] */
	if (this->direction == L_SORT_INCREASING) 
	{
		while (1) 
		{
			icl = 2 * ip;
			if (icl > this->n)
				break;
			valp = this->ptrArray[ip - 1]->key;
			valcl = this->ptrArray[icl - 1]->key;
			icr = icl + 1;
			if (icr > this->n)
			{  
				/* only a left child; no iters below */
				if (valp > valcl)
					SWAP_ITEMS(ip - 1, icl - 1);
				break;
			}
			else
			{  
				/* both children present; swap with the smallest if bigger */
				valcr = this->ptrArray[icr - 1]->key;
				if (valp <= valcl && valp <= valcr)  /* smaller than both */
					break;
				if (valcl <= valcr)
				{  
					/* left smaller; swap */
					SWAP_ITEMS(ip - 1, icl - 1);
					ip = icl;
				}
				else
				{ 
					/* right smaller; swap */
					SWAP_ITEMS(ip - 1, icr - 1);
					ip = icr;
				}
			}
		}
	}
	else
	{
		/* lh->direction == L_SORT_DECREASING */
		while (1)
		{
			icl = 2 * ip;
			if (icl > this->n)
				break;
			valp = this->ptrArray[ip - 1]->key;
			valcl = this->ptrArray[icl - 1]->key;
			icr = icl + 1;
			if (icr > this->n)
			{ 
				/* only a left child; no iters below */
				if (valp < valcl)
					SWAP_ITEMS(ip - 1, icl - 1);
				break;
			}
			else
			{
				/* both children present; swap with the biggest if smaller */
				valcr = this->ptrArray[icr - 1]->key;
				if (valp >= valcl && valp >= valcr)  /* bigger than both */
					break;
				if (valcl >= valcr) 
				{
					/* left bigger; swap */
					SWAP_ITEMS(ip - 1, icl - 1);
					ip = icl;
				}
				else
				{
					/* right bigger; swap */
					SWAP_ITEMS(ip - 1, icr - 1);
					ip = icr;
				}
			}
		}
	}
	return 0;
}


/*!
*  sort()
*
*      Input:  lh (heap, with internal array)
*      Return: 0 if OK, 1 on error
*
*  Notes:
*      (1) This sorts an array into heap order.  If the heap is already
*          in heap order for the direction given, this has no effect.
*/
int MinHeap::sort()
{
	int  i;

	for (i = 0; i < this->n; i++)
		swapUp( i);

	return 0;
}


/*!
*  sortStrictOrder()
*
*      Input:  lh (heap, with internal array)
*      Return: 0 if OK, 1 on error
*
*  Notes:
*      (1) This sorts a heap into strict order.
*      (2) For each element, starting at the end of the array and
*          working forward, the element is swapped with the head
*          element and then allowed to swap down onto a heap of
*          size reduced by one.  The result is that the heap is
*          reversed but in strict order.  The array elements are
*          then reversed to put it in the original order.
*/
int MinHeap::sortStrictOrder()
{
	int  i, index, size;

	size = this->n;  /* save the actual size */

	for (i = 0; i < size; i++) 
	{
		index = size - i;
		SWAP_ITEMS(0, index - 1);
		this->n--;  /* reduce the apparent heap size by 1 */
		swapDown();
	}

	this->n = size;  /* restore the size */

	for (i = 0; i < size / 2; i++)  /* reverse */
		SWAP_ITEMS(i, size - i - 1);

	return 0;
}



/*---------------------------------------------------------------------*
*                            Debug output                             *
*---------------------------------------------------------------------*/
/*!
*  print()
*
*      Input:  stream
*              lheap
*      Return: 0 if OK; 1 on error
*/
void MinHeap::print(FILE    *fp)
{
	int  i;

	if (!fp)
		return;    

	fprintf(fp, "\n MinHeap: nalloc = %d, n = %d, array = %p\n",
		this->nalloc, this->n, this->ptrArray);

	for (i = 0; i < this->n; i++)
		fprintf(fp,   "keyval[%d] = %f\n", i, *(float *)this->ptrArray[i]);
}

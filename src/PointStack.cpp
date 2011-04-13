/*
 * $Id: PointStack.cpp,v 1.1 2009/03/18 19:31:50 scott Exp $
 *
 */

#include "PointStack.h"

#define MIN_STACK_DEPTH 256
#define MAX_STACK_DEPTH 65536

/*
  =======================================================================================
  =======================================================================================
*/
PointStack::PointStack()
{
	_list = NULL;
	_head = 0;
	_maxDepth = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
PointStack::~PointStack()
{
	if (_list) {
		delete _list;
		_list = NULL;
	}

	_head = 0;
	_maxDepth = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointStack::push(int x, int y)
{
	if (_head >= _maxDepth) {
		if (!growStack()) {
			return false;
		}
	}

	_list[_head].x = x;
	_list[_head].y = y;
	_head++;

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointStack::pop(POINT &pt)
{
	if (_head > 0) {
		_head--;
		pt = _list[_head];
		return true;
	}

	return false;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointStack::isEmpty()
{
	return (_head > 0) ? false : true;
}

/*
  =======================================================================================
  =======================================================================================
*/
void PointStack::empty()
{
	_head = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointStack::growStack()
{
	POINT *new_list;
	int new_size;

	if (_maxDepth > 0) {
		if (_maxDepth >= MAX_STACK_DEPTH) {
			return false;
		}

		new_size = _maxDepth * 2;
	}
	else {
		new_size = MIN_STACK_DEPTH;
	}

	new_list = new POINT[new_size];

	if (!new_list) {
		return false;
	}

	if (_list) {
		memcpy(new_list, _list, _maxDepth * sizeof(POINT));
		delete [] _list;
	}

	_list = new_list;
	_maxDepth = new_size;

	return true;
}




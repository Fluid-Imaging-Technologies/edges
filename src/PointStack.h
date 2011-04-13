/*
 * $Id: PointStack.h,v 1.1 2009/03/18 19:31:50 scott Exp $
 *
 */

#pragma once
#ifndef POINT_STACK_H
#define POINT_STACK_H

#include "version.h"


class PointStack
{
public:
	PointStack();
	~PointStack();

	bool push(int x, int y);
	bool pop(POINT &pt);
	bool isEmpty();
	void empty();

private:
	bool growStack();

	POINT *_list;
	int _head;
	int _maxDepth;
};

#endif // ifndef POINT_STACK_H

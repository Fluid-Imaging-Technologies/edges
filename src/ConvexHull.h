/*
 * $Id: ConvexHull.h,v 1.3 2009/03/30 17:22:19 scott Exp $
 *
 */

#pragma once
#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include "version.h"
#include "PointChain.h"

class SortPoint
{
public:
	POINT _pt;
	double _angle;
	double _dist;
};

enum eTurnDirection { RightTurn = -1, Straight = 0, LeftTurn = 1 };

class ConvexHull
{
public:
	ConvexHull();
	~ConvexHull();

	bool setPoints(PointChain *pc);
	SortPoint* getConvexHull(int *numPts);
	double getConvexPerimeter();
	void clear();

private:
	static int angleSortCmp(void *p, const void *a, const void *b);

	
	bool findStartPoint(RECT *boundingRect);
	bool sortPoints();
	eTurnDirection findTurnDirection(int start, int a, int b);
	void computeHull();

	SortPoint *_pts;
	int _numPts;
	double _perimeter;
	
	POINT _startPt;
};

#endif // ifndef CONVEX_HULL_H

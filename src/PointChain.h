/*
 * $Id: PointChain.h,v 1.1 2009/03/31 17:18:11 scott Exp $
 *
 */

#pragma once
#ifndef POINT_CHAIN_H
#define POINT_CHAIN_H

#include "version.h"
#include "PixelChain.h"

class PointChain
{
public:
	PointChain();
	PointChain(PointChain &rhs);
	PointChain(PixelChain *pc);
	~PointChain();

	PointChain& operator=(PointChain &rhs);

	void clear();
	bool setChain(PixelChain *pc);

	POINT* getPoints(int *numPoints);
	int getNumPoints();
	bool getBoundingRect(RECT *r);
	bool translate(int dx, int dy);
	bool translateNormalized();

private:
	int _numEdgePts;
	POINT *_edgePts;
	RECT _boundingRect;
};


#endif // ifndef POINT_CHAIN_H

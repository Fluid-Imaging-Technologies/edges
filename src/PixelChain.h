/*
 * $Id: PixelChain.h,v 1.2 2009/03/31 18:07:55 scott Exp $
 *
 */

#pragma once
#ifndef PIXEL_CHAIN_H
#define PIXEL_CHAIN_H

#include "version.h"

class PixelChain
{
public:
	static double _pixelDistance[9];

	PixelChain();
	PixelChain(const PixelChain &rhs);
	~PixelChain();

	PixelChain& operator=(const PixelChain &rhs);

	bool setEdgeChain(int max_points, long *chain_id, long *x, long *y);
	bool loadEdgeChain(const char *str);
	bool saveEdgeChain(HANDLE fh, int id);

	double calculatePerimeter();

	int getId();
	int getNumPoints();
	const unsigned char* getChain();
	bool getStartPoint(POINT *pt);
	bool growList(int size_required);

	char* stripWhitespace(const char *str);

	int _id;
	int _numPoints;
	unsigned char *_chain;
	POINT _startPoint;
};


#endif // ifndef PIXEL_CHAIN_H

/*
 * $Id: PointDist.h,v 1.1 2009/03/27 12:34:56 scott Exp $
 *
 */

#pragma once
#ifndef POINT_DIST_H
#define POINT_DIST_H

#include "version.h"

class PointDist
{
public:
	PointDist() {
		_pt.x = 0;
		_pt.y = 0;
		_dist = 0;
	}

	POINT _pt;
	int _dist;
};

#endif // ifndef POINT_DIST_H

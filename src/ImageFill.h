/*
 * $Id: ImageFill.h,v 1.6 2009/03/27 12:34:41 scott Exp $
 *
 */

#pragma once
#ifndef IMAGE_FILL_H
#define IMAGE_FILL_H

#include "version.h"
#include "PointChain.h"
#include "PointDist.h"


class ImageFill
{
public:
	ImageFill();
	~ImageFill();

	bool setPointChain(PointChain *pc);
	POINT* getFillPoints(int *numPoints);
	int getFillArea();
	int getWaist(POINT *waistCenterPt);
	PointDist* getLocalMaxima(int *numPoints);
	void clear();

private:
	void fillImage();
	void fillImage2();
	POINT findInteriorPoint();
	POINT findInteriorPoint2();
	bool isInteriorPoint(int x, int y);
	bool isInteriorPoint2(int x, int y);
	void calculateSigma();
	void calculateChi();
	void calculateDist();
	void calculateLocalMaxima();
	void markEdges();
	void getNeighborhood(int x, int y);
	int oneThinningPass();
	void skeletonize();

	void freeArrays();
	bool allocArrays(int dimX, int dimY);

private:
	int _dimX;
	int _dimY;
	unsigned char **_image;
	unsigned char **_edge;
	unsigned char **_sigma;
	unsigned char **_chi;
	unsigned short **_scratchPad;
	unsigned short **_dist;
	unsigned char **_localMaxima;
	unsigned char _a[9];
	int _area;
	int _waist;
	POINT _waistCenterPt;
};


#endif // ifndef IMAGE_FILL_H

/*
 * $Id: ImageFill2.cpp,v 1.3 2009/04/01 17:33:21 scott Exp $
 *
 */

#include "ImageFill2.h"
#include "PointStack.h"

#define IMAGE_PADDING 1
#define IMAGE_BORDER 1

#define NORTH_POINT 0x0001
#define SOUTH_POINT 0x0010
#define EAST_POINT	0x0100
#define WEST_POINT	0x1000

/*
  =======================================================================================
  =======================================================================================
*/
ImageFill2::ImageFill2()
{
	_dimX = 0;
	_dimY = 0;
	_image = NULL;
	_edge = NULL;
	_sigma = NULL;
	_chi = NULL;
	_scratchPad = NULL;
	_dist = NULL;
	_localMaxima = NULL;	
	_area = 0;
	_waist = 0;
	_waistCenterPt.x = 0;
	_waistCenterPt.y = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
ImageFill2::~ImageFill2()
{
	freeArrays();
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::clear()
{
	freeArrays();

	_area = 0;
	_waist = 0;
	_waistCenterPt.x = 0;
	_waistCenterPt.y = 0;
}

/*
  =======================================================================================
  Includes the edge
  =======================================================================================
*/
int ImageFill2::getFillArea()
{
	if (_area == 0) {
		for (int x = 0; x < _dimX; x++) {
			for (int y = 0; y < _dimY; y++) {
				if (_image[x][y]) {
					_area++;
				}
			}
		}
	}

	return _area;
}

/*
  =======================================================================================
  =======================================================================================
*/
int ImageFill2::getWaist(POINT *waistCenterPt)
{
	if (_waist == 0) {	
		for (int x = 0; x < _dimX; x++) {
			for (int y = 0; y < _dimY; y++) {
				if (_localMaxima[x][y]) {
					if (_dist[x][y] > _waist) {
						_waist = _dist[x][y];
						_waistCenterPt.x = x;
						_waistCenterPt.y = y;
					}
				}
			}
		}

		if (_waist > 1) {
			_waist *= 2;
			_waist++;
		}
	}

	if (waistCenterPt) {
		waistCenterPt->x = _waistCenterPt.x;
		waistCenterPt->y = _waistCenterPt.y;
	}

	return _waist;
}

/*
  =======================================================================================
  caller owns the list
  =======================================================================================
*/
PointDist* ImageFill2::getLocalMaxima(int *numPoints)
{
	int cnt, x, y, i;
	PointDist *pts;

	cnt = 0;
	pts = NULL;

	for (x = 0; x < _dimX; x++) {
		for (y = 0; y < _dimY; y++) {
			if (_image[x][y] && !_edge[x][y]) {
				if (_localMaxima[x][y]) {
					cnt++;
				}
			}
		}
	}

	if (cnt > 0) {
		pts = new PointDist[cnt];

		if (!pts) {
			cnt = 0;
		}

		i = 0;

		for (x = 0; x < _dimX; x++) {
			for (y = 0; y < _dimY; y++) {
				if (_image[x][y] && !_edge[x][y]) {
					if (_localMaxima[x][y]) {
						pts[i]._dist = _dist[x][y];
						pts[i]._pt.x = x;
						pts[i]._pt.y = y;
						i++;
					}
				}
			}
		}
	}

	if (numPoints) {
		*numPoints = cnt;
	}

	return pts;
}

/*
  =======================================================================================
  =======================================================================================
*/
POINT* ImageFill2::getFillPoints(int *numPoints)
{
	int x, y, i, cnt;
	POINT *pts = NULL;

	cnt = 0;

	for (x = 0; x < _dimX; x++) {
		for (y = 0; y < _dimY; y++) {
			if (_image[x][y] && !_edge[x][y]) {
				cnt++;
			}
		}
	}

	if (cnt > 0) {
		pts = new POINT[cnt];

		if (pts) {
			i = 0;

			for (x = 0; x < _dimX; x++) {
				for (y = 0; y < _dimY; y++) {
					if (_image[x][y] && !_edge[x][y]) {
						pts[i].x = x;
						pts[i].y = y;
						i++;
					}
				}
			}
		}	
		else {
			cnt = 0;
		}
	}

	if (numPoints) {
		*numPoints = cnt;
	}

	return pts;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool ImageFill2::setPointChain(PointChain *pc)
{
	RECT r;
	int numpts, x, y;
	POINT *pts;

	freeArrays();

	if (!pc) {
		return false;
	}

	numpts = 0;

	pts = pc->getPoints(&numpts);

	if (!pts || numpts < 1) {
		return false;
	}

	if (!pc->getBoundingRect(&r)) {
		return false;
	}

	if (!allocArrays(1 + r.right - r.left, 1 + r.bottom - r.top)) {
		return false;
	}

	for (int i = 0; i < numpts; i++) {
		x = pts[i].x;
		y = pts[i].y;

		if (x < _dimX && y < _dimY) {
			_edge[x][y] = 1;
			_dist[x][y] = 1;
		}
	}

	if (_dimX < 3 || _dimY < 3) {
		// there is no area that is not part of the edge
		for (x = 0; x < _dimX; x++) {
			for (y = 0; y < _dimY; y++) {
				if (_edge[x][y]) {
					_image[x][y] = 1;
					_localMaxima[x][y] = 1;
				}
			}
		}
	}
	else {
		if (!fillBlob(pc)) {
			freeArrays();
			return false;
		}
		
		calculateDist();
	}

	return true;
}

/*
  =======================================================================================
  November code from ims
  =======================================================================================
*/
#define NOTSET 0
#define LEFT 1
#define RIGHT 2

#define UNKNOWN 0
#define INSIDE 1
#define OUTSIDE 0
#define BORDER_NOCROSS 3
#define BORDER_CROSS 4

bool ImageFill2::fillBlob(PointChain *pc)
{
	int x, y;
	int nx, ny;
	int enterside;
	int prior;
	int nextx, lastx;
	int nexty, lasty;
	int bordertype;
	int location;
	int numpts;
	POINT *pts;

	numpts = 0;

	pts = pc->getPoints(&numpts);

	if (!pts || numpts < 1) {
		return false;
	}

	nx = _dimX;
	ny = _dimY;

	for (x = 0; x < nx; ++x) {
		for (int y = 0; y < ny; ++y) {
			_image[x][y] = UNKNOWN;
		}
	}

	// go backwards around border to set enterside

	enterside = NOTSET;
	prior = numpts;
	while (--prior > 0) {
		if (pts[0].x > pts[prior].x) {
			enterside = LEFT;
		}
		else if (pts[0].x < pts[prior].x) {
			enterside = RIGHT;
		}

		if (enterside != NOTSET) {
			break;
		}
	}

	if (enterside == NOTSET) {
		return false;
	}

	//  work around the border; determine bordertype = BORDER_CROSS
	//  or BORDER_NOCROSS and set _image accordingly
		
	for (int n = 0; n < numpts; n++) {
		x = pts[n].x;
		y = pts[n].y;

		if (n == 0) {
			lastx = pts[numpts - 1].x;	
			lasty = pts[numpts - 1].y;
		}
		else {
			lastx = pts[n-1].x;			
			lasty = pts[n-1].y;
		}

		if (n == numpts - 1) {
			nextx = pts[0].x;
			nexty = pts[0].y;
		}
		else {
			nextx = pts[n+1].x;
			nexty = pts[n+1].y;
		}
			
		if ((lastx < x && nextx > x) || (lastx > x && nextx < x)) {	// Rule 1
			bordertype = BORDER_CROSS;
		}
		else if ((lastx < x && nextx < x) || (lastx > x && nextx > x)) { // Rule 2
			bordertype = BORDER_NOCROSS;
		}
		else if (lastx != x && nextx == x) { // Rule 3
			bordertype = BORDER_NOCROSS;

			if (lastx < x) {
				enterside = LEFT;
			}
			else {
				enterside = RIGHT;
			}
		}
		else if (lastx == x && nextx != x) { // Rule 6
			if (enterside == LEFT && nextx > x) {
				bordertype = BORDER_CROSS;
			}
			else if (enterside == LEFT && nextx < x) {
				bordertype = BORDER_NOCROSS;
			}
			else if (enterside == RIGHT && nextx < x) {
				bordertype = BORDER_CROSS;
			}
			else if (enterside == RIGHT && nextx > x) {
				bordertype = BORDER_NOCROSS;
			}
		}
		else { // lastx == x && nextx == x   Rules 4 and 5-revised
			bordertype = BORDER_NOCROSS;
		}
			
		// reset bordertype if the current pixel has already been set as a border pixel

		if (_image[x][y] == BORDER_CROSS && bordertype == BORDER_CROSS) {
			bordertype = BORDER_NOCROSS;
		}
		else if (_image[x][y] == BORDER_NOCROSS && bordertype == BORDER_CROSS) {
			bordertype = BORDER_CROSS;
		}
		else if (_image[x][y] == BORDER_CROSS && bordertype == BORDER_NOCROSS) {
			bordertype = BORDER_CROSS;
		}
		else if (_image[x][y] == BORDER_NOCROSS && bordertype == BORDER_NOCROSS) {
			bordertype = BORDER_NOCROSS;
		}
				
		_image[x][y] = bordertype;

	}

	// go through the _image array by columns and set all non-border pixels
	// to OUTSIDE or INSIDE

	for (x = 0; x < nx; ++x) {
		location = OUTSIDE;

		for (y = 0; y < ny; ++y) {
			if (_image[x][y] == BORDER_CROSS) {
				location = (location == OUTSIDE) ? INSIDE : OUTSIDE;
			}
			else if (_image[x][y] == BORDER_NOCROSS) {
				continue;
			}
			else {
				_image[x][y] = location;
			}
		}			
	}

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::calculateSigma()
{
	for (int x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (int y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			getNeighborhood(x, y);
			_sigma[x][y] = _a[1] + _a[2] + _a[3] + _a[4] + _a[5] + _a[6] + _a[7] + _a[8]; 					
		}
	}
}

/*
  =======================================================================================
  4 | 3 | 2
  5 | 0 | 1
  6 | 7 | 8
  =======================================================================================
*/
void ImageFill2::getNeighborhood(int x, int y)
{
	_a[0] = _image[x][y];
	_a[1] = _image[x+1][y];
	_a[2] = _image[x+1][y-1];
	_a[3] = _image[x][y-1];
	_a[4] = _image[x-1][y-1];
	_a[5] = _image[x-1][y];
	_a[6] = _image[x-1][y+1];
	_a[7] = _image[x][y+1];
	_a[8] = _image[x+1][y+1];
}

/*
  =======================================================================================
  Calculates chi and whether points are North, South, East or West points
  =======================================================================================
*/
void ImageFill2::calculateChi()
{
	for (int x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (int y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			getNeighborhood(x, y);

			_chi[x][y] = (_a[1] != _a[3]) + (_a[3] != _a[5]) + (_a[5] != _a[7]) + (_a[7] != _a[1])
					+ 2 * ((!_a[1] && _a[2] && !_a[3]) 
							+ (!_a[3] && _a[4] && !_a[5])
							+ (!_a[5] && _a[6] && !_a[7])
							+ (!_a[7] && _a[8] && !_a[1]));


			_scratchPad[x][y] = 0;

			if (_a[0]) {
				if (_a[7] && !_a[3]) {
					_scratchPad[x][y] = NORTH_POINT;
				}
				else if (!_a[7] && _a[3]) {
					_scratchPad[x][y] = SOUTH_POINT;
				}
				else if (_a[5] && !_a[1]) {
					_scratchPad[x][y] = EAST_POINT;
				}
				else if (!_a[5] && _a[1]) {
					_scratchPad[x][y] = WEST_POINT;
				}
			}
		}
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::markEdges()
{
	calculateSigma();

	for (int x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (int y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			if (_sigma[x][y] == 8) {
				_edge[x][y] = 0;
			}
			else {
				_edge[x][y] = _image[x][y];
			}
		}
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::skeletonize()
{
	while (oneThinningPass() > 0) {}	
}

/*
  =======================================================================================
  =======================================================================================
*/
int ImageFill2::oneThinningPass()
{
	int removals;
	int x, y;

	markEdges();
	calculateChi();

	removals = 0;

	// eliminate North Points
	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			if (!_localMaxima[x][y] && (_scratchPad[x][y] & NORTH_POINT)) {
				if ((_chi[x][y] == 2) && (_sigma[x][y] != 1)) {
					removals++;
					_image[x][y] = 0;
				}
			}
		}
	}

	markEdges();
	calculateChi();

	// eliminate South Points
	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			if (!_localMaxima[x][y] && (_scratchPad[x][y] & SOUTH_POINT)) {
				if ((_chi[x][y] == 2) && (_sigma[x][y] != 1)) {
					removals++;
					_image[x][y] = 0;
				}
			}
		}
	}

	markEdges();
	calculateChi();

	// eliminate East Points
	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			if (!_localMaxima[x][y] && (_scratchPad[x][y] & EAST_POINT)) {
				if ((_chi[x][y] == 2) && (_sigma[x][y] != 1)) {
					removals++;
					_image[x][y] = 0;
				}
			}
		}
	}

	markEdges();
	calculateChi();

	// eliminate West Points
	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			if (!_localMaxima[x][y] && (_scratchPad[x][y] & WEST_POINT)) {
				if ((_chi[x][y] == 2) && (_sigma[x][y] != 1)) {
					removals++;
					_image[x][y] = 0;
				}
			}
		}
	}

	return removals;
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::calculateDist()
{
	int x, y, min;
	int q[9];
	bool done;

	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			_dist[x][y] = 255 * _image[x][y];
		}
	}

	done = false;

	while (!done) {
		done = true;

		// forward raster scan
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
				q[0] = _dist[x][y];
				q[2] = _dist[x+1][y-1];
				q[3] = _dist[x][y-1];
				q[4] = _dist[x-1][y-1];
				q[5] = _dist[x-1][y];

				min = q[0] - 1;

				if (q[2] < min) {
					min = q[2];
				}

				if (q[3] < min) {
					min = q[3];
				}

				if (q[4] < min) {
					min = q[4];
				}

				if (q[5] < min) {
					min = q[5];
				}

				min++;

				if (min < q[0]) {
					done = false;
					_dist[x][y] = min;
				}
			}
		}

		// reverse raster scan
		for (y = _dimY - (IMAGE_PADDING + 1); y >= IMAGE_PADDING; y--) {
			for (x = _dimX - (IMAGE_PADDING + 1); x >= IMAGE_PADDING; x--) {
				q[0] = _dist[x][y];
				q[1] = _dist[x+1][y];
				q[6] = _dist[x-1][y+1];
				q[7] = _dist[x][y+1];
				q[8] = _dist[x+1][y+1];

				min = q[0] - 1;

				if (q[1] < min) {
					min = q[1];
				}

				if (q[6] < min) {
					min = q[6];
				}

				if (q[7] < min) {
					min = q[7];
				}

				if (q[8] < min) {
					min = q[8];
				}

				min++;

				if (min < q[0]) {
					done = false;
					_dist[x][y] = min;
				}
			}
		}
	}

	calculateLocalMaxima();
}

/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::calculateLocalMaxima()
{
	int x, y, max, i;
	int q[9];
	
	for (x = IMAGE_PADDING; x < _dimX - IMAGE_PADDING; x++) {
		for (y = IMAGE_PADDING; y < _dimY - IMAGE_PADDING; y++) {
			q[0] = _dist[x][y];

			if (q[0] > 0) {
				q[1] = _dist[x+1][y];
				q[2] = _dist[x+1][y-1];
				q[3] = _dist[x][y-1];
				q[4] = _dist[x-1][y-1];
				q[5] = _dist[x-1][y];
				q[6] = _dist[x-1][y+1];
				q[7] = _dist[x][y+1];
				q[8] = _dist[x+1][y+1];
				
				max = q[0];

				for (i = 1; i < 9; i++) {
					if (q[i] > max) {
						break;
					}
				}

				if (i == 9) {
					_localMaxima[x][y] = 1;
				}
				else {
					_localMaxima[x][y] = 0;
				}
			}
			else {
				_localMaxima[x][y] = 0;
			}
		}
	}
}



/*
  =======================================================================================
  =======================================================================================
*/
void ImageFill2::freeArrays()
{
	if (_image) {
		for (int i = 0; i < _dimX; i++) {
			if (_image[i]) {
				delete [] _image[i];
			}
		}

		delete [] _image;
		_image = NULL;
	}

	if (_edge) {
		for (int i = 0; i < _dimX; i++) {
			if (_edge[i]) {
				delete [] _edge[i];
			}
		}

		delete [] _edge;
		_edge = NULL;
	}

	if (_sigma) {
		for (int i = 0; i < _dimX; i++) {
			if (_sigma[i]) {
				delete [] _sigma[i];
			}
		}

		delete [] _sigma;
		_sigma = NULL;
	}

	if (_chi) {
		for (int i = 0; i < _dimX; i++) {
			if (_chi[i]) {
				delete [] _chi[i];
			}
		}

		delete [] _chi;
		_chi = NULL;
	}

	if (_scratchPad) {
		for (int i = 0; i < _dimX; i++) {
			if (_scratchPad[i]) {
				delete [] _scratchPad[i];
			}
		}

		delete [] _scratchPad;
		_scratchPad = NULL;
	}

	if (_dist) {
		for (int i = 0; i < _dimX; i++) {
			if (_dist[i]) {
				delete [] _dist[i];
			}
		}

		delete [] _dist;
		_dist = NULL;
	}

	if (_localMaxima) {
		for (int i = 0; i < _dimX; i++) {
			if (_localMaxima[i]) {
				delete [] _localMaxima[i];
			}
		}

		delete [] _localMaxima;
		_localMaxima = NULL;
	}

	_dimX = 0;
	_dimY = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool ImageFill2::allocArrays(int dimX, int dimY)
{
	freeArrays();
	
	if (dimX < 1 || dimY < 1) {
		return false;
	}

	_image = new unsigned char*[dimX];

	if (!_image) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_image[i] = new unsigned char[dimY];

		if (!_image[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_image[i], 0, dimY * sizeof(unsigned char));
		}
	}

	_edge = new unsigned char*[dimX];

	if (!_edge) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_edge[i] = new unsigned char[dimY];

		if (!_edge[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_edge[i], 0, dimY * sizeof(unsigned char));
		}
	}

	_sigma = new unsigned char*[dimX];

	if (!_sigma) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_sigma[i] = new unsigned char[dimY];

		if (!_sigma[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_sigma[i], 0, dimY * sizeof(unsigned char));
		}
	}

	_chi = new unsigned char*[dimX];

	if (!_chi) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_chi[i] = new unsigned char[dimY];

		if (!_chi[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_chi[i], 0, dimY * sizeof(unsigned char));
		}
	}

	_scratchPad = new unsigned short*[dimX];

	if (!_scratchPad) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_scratchPad[i] = new unsigned short[dimY];

		if (!_scratchPad[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_scratchPad[i], 0, dimY * sizeof(unsigned short));
		}
	}

	_dist = new unsigned short*[dimX];

	if (!_dist) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_dist[i] = new unsigned short[dimY];

		if (!_dist[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_dist[i], 0, dimY * sizeof(unsigned short));
		}
	}

	_localMaxima = new unsigned char*[dimX];

	if (!_localMaxima) {
		freeArrays();
		return false;
	}

	for (int i = 0; i < dimX; i++) {
		_localMaxima[i] = new unsigned char[dimY];

		if (!_localMaxima[i]) {
			freeArrays();
			return false;
		}
		else {
			memset(_localMaxima[i], 0, dimY * sizeof(unsigned char));
		}
	}


	_dimX = dimX;
	_dimY = dimY;

	return true;
}



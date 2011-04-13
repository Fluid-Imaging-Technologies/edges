/*
 * $Id: ConvexHull.cpp,v 1.4 2009/04/01 17:33:21 scott Exp $
 *
 */

#include <math.h>

#include "ConvexHull.h"

#define LEFT_TURN 1
#define NO_TURN 0
#define RIGHT_TURN -1

/*
  =======================================================================================
  =======================================================================================
*/
ConvexHull::ConvexHull()
{
	_pts = NULL;
	_numPts = 0;
	_perimeter = 0.0;
}

/*
  =======================================================================================
  =======================================================================================
*/
ConvexHull::~ConvexHull()
{
	clear();
}

/*
  =======================================================================================
  =======================================================================================
*/
SortPoint* ConvexHull::getConvexHull(int *numPts)
{
	if (numPts) {
		*numPts = _numPts;
	}

	return _pts;
}

/*
  =======================================================================================
  =======================================================================================
*/
double ConvexHull::getConvexPerimeter()
{
	double temp;
	POINT prev;
	double dx, dy;

	if (_perimeter == 0.0) {
		if (_numPts > 0) {
			prev = _pts[0]._pt;
			
			for (int i = 1; i < _numPts; i++) {
				dx = prev.x - _pts[i]._pt.x;
				dy = prev.y - _pts[i]._pt.y;

				temp = sqrt((dx * dx) + (dy * dy));
				_perimeter += temp;
				prev = _pts[i]._pt;
			}

			dx = prev.x - _pts[0]._pt.x;
			dy = prev.y - _pts[0]._pt.y;
			temp = sqrt((dx * dx) + (dy * dy));
			_perimeter += temp;
		}
	}

	return _perimeter;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool ConvexHull::setPoints(PointChain *pc)
{
	POINT *temp;
	RECT r;

	clear();

	if (!pc) {
		return false;
	}

	pc->getBoundingRect(&r);

	temp = pc->getPoints(&_numPts);

	if (_numPts < 3) {		
		_perimeter = _numPts;
		_numPts = 0;
		return true;
	}

	if (!temp) {
		_numPts = 0;
		return false;
	}

	// make the list one bigger for the GrahamScan algorithm
	// we are going to repeat the start point at the end later
	// can leave uninitialized for now
	_pts = new SortPoint[_numPts + 1];

	if (!_pts) {
		_numPts = 0;
		return false;
	}

	for (int i = 0; i < _numPts; i++) {
		_pts[i]._pt = temp[i];
	}

	if (!findStartPoint(&r)) {
		clear();
		return false;
	}

	if (!sortPoints()) {
		clear();
		return false;
	}

	if (_numPts > 3) {
		computeHull();
	}

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool ConvexHull::sortPoints()
{
	int i, j;
	double dx, dy;
	double prev;
	int *list;

	if (_numPts < 2) {
		return true;
	}

	for (i = 1; i < _numPts; i++) {
		dx = _pts[i]._pt.x - _startPt.x;
		dy = _pts[i]._pt.y - _startPt.y;
		_pts[i]._angle = atan2(dx, dy);

		if (_pts[i]._angle < 0.0) {
			_pts[i]._angle = (2.0 * PI) + _pts[i]._angle;
		}

		_pts[i]._dist = sqrt((dx * dx) + (dy * dy));
	}

	qsort_s(_pts + 1, _numPts - 1, sizeof(SortPoint), ConvexHull::angleSortCmp, NULL);

	// now remove duplicates
	list = new int[_numPts];

	if (!list) {
		return false;
	}

	
	list[0] = 0;
	list[1] = 1;
	prev = _pts[1]._angle;
	j = 1;

	for (i = 2; i < _numPts; i++) {
		if (_pts[i]._angle != prev) {
			j++;
			list[j] = i;
			prev = _pts[i]._angle;
		}
		// replace the current list[j] entry if the distance is greater
		else if (_pts[i]._dist > _pts[list[j]]._dist) {
			list[j] = i;
		}
	}
	
	// now we just want to keep the list[] entries
	for (i = 0; i < j; i++) {
		if (i != list[i]) {
			_pts[i] = _pts[list[i]];
		}
	}

	_numPts = j;

	delete [] list;

	return true;
}

/*
  =======================================================================================
  Initial condition
  _pts[0] is the known start point
  _pts[1] is current
  _pts[2] is the first candidate
  =======================================================================================
*/
void ConvexHull::computeHull()
{
	SortPoint temp;
	int a, b;
	eTurnDirection turn;

	// we know we have extra space
	_pts[_numPts] = _pts[0];

	_numPts++;

	a = 2;

	for (b = 3; b < _numPts; b++) {
		turn = findTurnDirection(a, a - 1, b);

		while (turn == RightTurn) {
			if (a < 2) {
				break;
			}

			a--;
			turn = findTurnDirection(a, a - 1, b);
		}

		a++;

		if (a != b) {
			temp = _pts[a];
			_pts[a] = _pts[b];
			// don't need to do this but...
			_pts[b] = temp;
		}
	}

	_numPts = a;
}

/*
  =======================================================================================
  =======================================================================================
*/
eTurnDirection ConvexHull::findTurnDirection(int start, int a, int b)
{
	int xa, xb, ya, yb;

	xa = _pts[a]._pt.x - _pts[start]._pt.x;
	ya = _pts[a]._pt.y - _pts[start]._pt.y;

	xb = _pts[b]._pt.x - _pts[start]._pt.x;
	yb = _pts[b]._pt.y - _pts[start]._pt.y;

	if (xa * yb > xb * ya) {
		return LeftTurn;
	}

	if (xa * yb < xb * ya) {
		return RightTurn;
	}

	if ((xa * xb < 0) || (ya * yb) < 0) {
		return RightTurn;
	}

	// already took care of this case
	if ((xa * xa + ya * ya) < (xb * xb + yb * yb)) {
		return LeftTurn;
	}

	return Straight;
}

/*
  =======================================================================================
  static function
  =======================================================================================
*/
int ConvexHull::angleSortCmp(void *p, const void *a, const void *b)
{
	double da, db;

	da = ((SortPoint *)a)->_angle;
	db = ((SortPoint *)b)->_angle;

	if (da < db) {
		return -1;
	}
	else if (da > db) {
		return 1;
	}
	else {
		return 0;
	}
}

/*
  =======================================================================================
  Puts the lowest and in the case of ties leftmost point into _startPt and then removes
  the point from the list, decrementing _numPts. 
  =======================================================================================
*/
bool ConvexHull::findStartPoint(RECT *boundingRect)
{
	int min_x, min_y;
	int n;
	SortPoint temp;

	min_x = boundingRect->right + 1;
	min_y = boundingRect->bottom;
	n = -1;

	for (int i = 0; i < _numPts; i++) {
		if (_pts[i]._pt.y == min_y) {
			if (_pts[i]._pt.x < min_x) {
				min_x = _pts[i]._pt.x;
				n = i;
			}
		}
	}

	if (n == -1) {
		return false;
	}

	_startPt = _pts[n]._pt;

	// _pts[0] is first now
	temp = _pts[n];
	_pts[n] = _pts[0];
	_pts[0] = temp;
	
	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
void ConvexHull::clear()
{
	if (_pts) {
		delete [] _pts;
		_pts = NULL;
	}

	_numPts = 0;
	_perimeter = 0.0;
}

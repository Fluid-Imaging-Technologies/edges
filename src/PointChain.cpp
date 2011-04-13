/*
 * $Id: PointChain.cpp,v 1.2 2009/03/31 18:07:55 scott Exp $
 *
 */

#include "PointChain.h"
#include "PointStack.h"

/*
  =======================================================================================
  =======================================================================================
*/
PointChain::PointChain()
{
	_numEdgePts = 0;
	_edgePts = NULL;
	_boundingRect.left = 0;
	_boundingRect.right = 0;
	_boundingRect.top = 0;
	_boundingRect.bottom = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
PointChain::PointChain(PointChain &rhs)
{
	_edgePts = NULL;
	_boundingRect.left = 0;
	_boundingRect.right = 0;
	_boundingRect.top = 0;
	_boundingRect.bottom = 0;

	_numEdgePts = rhs._numEdgePts;

	if (_numEdgePts > 0) {
		_edgePts = new POINT[_numEdgePts];

		if (!_edgePts) {
			_numEdgePts = 0;
		}
		else {
			for (int i = 0; i < _numEdgePts; i++) {
				_edgePts[i].x = rhs._edgePts[i].x;
				_edgePts[i].y = rhs._edgePts[i].y;
			}

			_boundingRect.left = rhs._boundingRect.left;
			_boundingRect.right = rhs._boundingRect.right;
			_boundingRect.top = rhs._boundingRect.top;
			_boundingRect.bottom = rhs._boundingRect.bottom;
		}
	}
	else {
		_numEdgePts = 0;
	}
}


/*
  =======================================================================================
  =======================================================================================
*/
PointChain::PointChain(PixelChain *pc)
{
	_numEdgePts = 0;
	_edgePts = NULL;
	_boundingRect.left = 0;
	_boundingRect.right = 0;
	_boundingRect.top = 0;
	_boundingRect.bottom = 0;

	setChain(pc);
}

/*
  =======================================================================================
  =======================================================================================
*/
PointChain::~PointChain()
{
	if (_edgePts) {
		delete [] _edgePts;
		_edgePts = NULL;
	}

	_numEdgePts = 0;
	_boundingRect.left = 0;
	_boundingRect.right = 0;
	_boundingRect.top = 0;
	_boundingRect.bottom = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
PointChain& PointChain::operator=(PointChain &rhs)
{
	if (&rhs != this) {
		if (_edgePts) {
			delete [] _edgePts;
			_edgePts = NULL;
		}

		_numEdgePts = rhs._numEdgePts;

		if (_numEdgePts > 0) {
			_edgePts = new POINT[_numEdgePts];

			if (_edgePts) {
				for (int i = 0; i < _numEdgePts; i++) {
					_edgePts[i].x = rhs._edgePts[i].x;
					_edgePts[i].y = rhs._edgePts[i].y;
				}

				_boundingRect.left = rhs._boundingRect.left;
				_boundingRect.right = rhs._boundingRect.right;
				_boundingRect.top = rhs._boundingRect.top;
				_boundingRect.bottom = rhs._boundingRect.bottom;
			}
		}

		if (!_edgePts) {
			_numEdgePts = 0;
			_boundingRect.left = 0;
			_boundingRect.right = 0;
			_boundingRect.top = 0;
			_boundingRect.bottom = 0;
		}
	}

	return *this;
}

/*
  =======================================================================================
  =======================================================================================
*/
void PointChain::clear()
{
	_numEdgePts = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointChain::setChain(PixelChain *pc)
{
	POINT start_pt;
	const unsigned char *chain;
	int i, j;

	if (_edgePts) {
		delete [] _edgePts;
		_edgePts = NULL;
	}

	_numEdgePts = 0;
	_boundingRect.left = 0;
	_boundingRect.right = 0;
	_boundingRect.top = 0;
	_boundingRect.bottom = 0;

	if (!pc) {
		return false;
	}

	_numEdgePts = pc->getNumPoints();

	if (_numEdgePts < 1) {
		return true;
	}

	if (!pc->getStartPoint(&start_pt)) {
		_numEdgePts = 0;
		return false;
	}

	chain = pc->getChain();

	if (!chain) {
		if (_numEdgePts > 1) {
			_numEdgePts = 0;
			return false;
		}
	}

	_edgePts = new POINT[_numEdgePts];

	if (!_edgePts) {
		_numEdgePts = 0;
		return false;
	}

	_edgePts[0].x = start_pt.x;
	_edgePts[0].y = start_pt.y;

	for (j = 1, i = 0; j < _numEdgePts; i++, j++) {
		switch (chain[i])
		{
		case 1:
			_edgePts[j].x = _edgePts[i].x + 1;
			_edgePts[j].y = _edgePts[i].y;
			break;

		case 2:
			_edgePts[j].x = _edgePts[i].x + 1;
			_edgePts[j].y = _edgePts[i].y - 1;
			break;

		case 3:
			_edgePts[j].x = _edgePts[i].x;
			_edgePts[j].y = _edgePts[i].y - 1;
			break;

		case 4:
			_edgePts[j].x = _edgePts[i].x - 1;
			_edgePts[j].y = _edgePts[i].y - 1;
			break;

		case 5:
			_edgePts[j].x = _edgePts[i].x - 1;
			_edgePts[j].y = _edgePts[i].y;
			break;

		case 6:
			_edgePts[j].x = _edgePts[i].x - 1;
			_edgePts[j].y = _edgePts[i].y + 1;
			break;

		case 7:
			_edgePts[j].x = _edgePts[i].x;
			_edgePts[j].y = _edgePts[i].y + 1;
			break;

		case 8:
			_edgePts[j].x = _edgePts[i].x + 1;
			_edgePts[j].y = _edgePts[i].y + 1;
			break;

		default:
			_edgePts[j].x = _edgePts[i].x;
			_edgePts[j].y = _edgePts[i].y;
			break;
		}
	}

	if (_numEdgePts < 1) {
		_boundingRect.left = 0;
		_boundingRect.right = 0;
		_boundingRect.top = 0;
		_boundingRect.bottom = 0;
	}
	else {
		_boundingRect.left = _edgePts[0].x;
		_boundingRect.right = _edgePts[0].x;
		_boundingRect.top = _edgePts[0].y;
		_boundingRect.bottom = _edgePts[0].y;

		for (int i = 1; i < _numEdgePts; i++) {
			if (_edgePts[i].x < _boundingRect.left) {
				_boundingRect.left = _edgePts[i].x;
			}
			else if (_edgePts[i].x > _boundingRect.right) {
				_boundingRect.right = _edgePts[i].x;
			}
			
			if (_edgePts[i].y < _boundingRect.top) {
				_boundingRect.top = _edgePts[i].y;
			}
			else if (_edgePts[i].y > _boundingRect.bottom) {
				_boundingRect.bottom = _edgePts[i].y;
			}
		}
	}

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
POINT* PointChain::getPoints(int *numPoints)
{
	if (numPoints) {
		*numPoints = _numEdgePts;
	}

	return _edgePts;
}

/*
  =======================================================================================
  =======================================================================================
*/
int PointChain::getNumPoints()
{
	return _numEdgePts;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointChain::getBoundingRect(RECT *r)
{
	if (r) {
		r->left = _boundingRect.left;
		r->right = _boundingRect.right;
		r->top = _boundingRect.top;
		r->bottom = _boundingRect.bottom;
		return true;
	}

	return false;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointChain::translate(int dx, int dy)
{
	for (int i = 0; i < _numEdgePts; i++) {
		_edgePts[i].x += dx;
		_edgePts[i].y += dy;
	}

	_boundingRect.left += dx;
	_boundingRect.right += dx;
	_boundingRect.top += dy;
	_boundingRect.bottom += dy;

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PointChain::translateNormalized()
{
	return translate(-1 * _boundingRect.left, -1 * _boundingRect.top);
}
/*
 * $Id: PixelChain.cpp,v 1.2 2009/03/31 18:07:55 scott Exp $
 *
 */

#include "PixelChain.h"

/*
	4 | 3 | 2	
	5 | 0 | 1
 	6 | 7 | 8

	_pixelDistance is the lookup for the distance from 0 to x in the above encoding
*/
double PixelChain::_pixelDistance[9] = { 0.0, 1.0, 1.414, 1.0, 1.414, 1.0, 1.414, 1.0, 1.414 };


/*
  =======================================================================================
  =======================================================================================
*/
PixelChain::PixelChain()
{
	_id = 0;
	_numPoints = 0;
	_startPoint.x = 0;
	_startPoint.y = 0;
	_chain = NULL;
}

/*
  =======================================================================================
  =======================================================================================
*/
PixelChain::PixelChain(const PixelChain &rhs)
{
	_id = rhs._id;

	_numPoints = 0;
	_startPoint.x = 0;
	_startPoint.y = 0;
	_chain = NULL;

	if (rhs._numPoints > 0) {
		_startPoint.x = rhs._startPoint.x;
		_startPoint.y = rhs._startPoint.y;

		if (rhs._numPoints > 1) {
			if (growList(rhs._numPoints)) {
				memcpy(_chain, rhs._chain, rhs._numPoints);
				_numPoints = rhs._numPoints;	
			}
		}
		else {
			_numPoints = 1;		
		}
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
PixelChain::~PixelChain()
{
	if (_chain) {
		delete [] _chain;
		_chain = NULL;
	}

	_numPoints = 0;	
}

/*
  =======================================================================================
  =======================================================================================
*/
PixelChain& PixelChain::operator=(const PixelChain &rhs)
{
	if (this != &rhs) {
		if (_chain) {
			delete [] _chain;
			_chain = NULL;
		}

		_id = rhs._id;

		_numPoints = 0;

		if (rhs._numPoints > 0) {
			_startPoint.x = rhs._startPoint.x;
			_startPoint.y = rhs._startPoint.y;

			if (rhs._numPoints > 1) {
				if (growList(rhs._numPoints)) {
					memcpy(_chain, rhs._chain, rhs._numPoints);
					_numPoints = rhs._numPoints;	
				}
			}
			else {
				_numPoints = 1;
			}
		}
	}

	return *this;
}

/*
  =======================================================================================
  The edge chain is relative to the CaptureX, CaptureY coordinates not the SaveX, SaveY
  coordinates.
  =======================================================================================
*/
bool PixelChain::setEdgeChain(int max_points, long *chain_id, long *x, long *y)
{
	int i, j;

	_numPoints = 0;

	if (_chain) {
		delete [] _chain;
		_chain = NULL;
	}

	if (!chain_id || !x || !y) {
		return false;
	}

	// we only care about the outer chain, chain_id == 1
	for (_numPoints = 0; _numPoints < max_points; _numPoints++) {
		if (chain_id[_numPoints] != 1) {
			break;
		}
	}

	if (_numPoints <= 0) {
		// should never happen
		return false;
	}

	_startPoint.x = x[0];
	_startPoint.y = y[0];

	// we don't need a list
	if (_numPoints == 1) {
		return true;
	}

	if (!growList(_numPoints)) {
		return false;
	}
	
	/*
	4 | 3 | 2	
	5 | 0 | 1
 	6 | 7 | 8
	*/

	for (i = 1, j = 0; i < _numPoints; i++, j++) {
		// column 0
		if (x[i] < x[j]) {
			if (y[i] < y[j]) {
				_chain[j] = 4;
			}
			else if (y[i] == y[j]) {
				_chain[j] = 5;
			}
			else {
				_chain[j] = 6;
			}
		}
		// column 1
		else if (x[i] == x[j]) {
			if (y[i] < y[j]) {
				_chain[j] = 3;
			}
			else if (y[i] == y[j]) {
				_chain[j] = 0;
			}
			else {
				_chain[j] = 7;
			}
		}
		// column 2
		else {
			if (y[i] < y[j]) {
				_chain[j] = 2;
			}
			else if (y[i] == y[j]) {
				_chain[j] = 1;
			}
			else {
				_chain[j] = 8;
			}
		}
	}

	// lastly how to get from _chain[j] to _startPoint
	// j is pointing to _numPoints - 1
	
	if (_startPoint.x < x[j]) {
		if (_startPoint.y < y[j]) {
			_chain[j] = 4;
		}
		else if (_startPoint.y == y[j]) {
			_chain[j] = 5;
		}
		else {
			_chain[j] = 6;
		}
	}
	else if (_startPoint.x == x[j]) {
		if (_startPoint.y < y[j]) {
			_chain[j] = 3;
		}
		else if (_startPoint.y == y[j]) {
			_chain[j] = 0;
		}
		else {
			_chain[j] = 7;
		}
	}
	else {
		if (_startPoint.y < y[j]) {
			_chain[j] = 2;
		}
		else if (_startPoint.y == y[j]) {
			_chain[j] = 1;
		}
		else {
			_chain[j] = 8;
		}
	}

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
int PixelChain::getId()
{
	return _id;
}

/*
  =======================================================================================
  The static public PixelChain::_pixelDistance[] is a simple center to center 
  distance mapping.
  Other perimeter distances are possible using the chain provided by getPerimeterChain().
  =======================================================================================
*/
double PixelChain::calculatePerimeter()
{
	double perim;

	if (!_chain) {
		return 0.0;
	}

	perim = 0.0;

	for (int i = 0; i < _numPoints - 1; i++) {
		perim += _pixelDistance[_chain[i]];
	}

	return perim;
}

/*
  =======================================================================================
  Returns the original setEdgeChain() coordinates of the first point in the chain.
  This is the CaptureX, CaptureY coordinates usually.
  =======================================================================================
*/
int PixelChain::getNumPoints()
{
	return _numPoints;
}

/*
  =======================================================================================
  Return a list of movements using our mapping
  	4 | 3 | 2	
	5 | 0 | 1
 	6 | 7 | 8
  in a array of bytes. 
  
  The caller owns the list.
  The size is returned in num_points which must be provided.

  To use this chain for an overlay, call getFirstPerimeterPoint() to get
  the first point in CaptureX, CaptureY coordinates. 

  For perimeter or filling algorithms, this chain is all that is needed.

  See calculatePerimeter() above for an example on how to use.
  =======================================================================================
*/
const unsigned char* PixelChain::getChain()
{
	return _chain;
}

/*
  =======================================================================================
  Returns the original setEdgeChain() coordinates of the first point in the chain.
  This is the CaptureX, CaptureY coordinates usually.
  =======================================================================================
*/
bool PixelChain::getStartPoint(POINT *pt)
{
	if (!pt || _numPoints < 1) {
		return false;
	}

	pt->x = _startPoint.x;
	pt->y = _startPoint.y;

	return true;
}

/*
  =======================================================================================
  Map coordinates of the edge by first marking the first coordinate, and then using
  the following mapping, all of the remaining coordinates using 1 byte each.

  4 | 3 | 2
  5 | 0 | 1
  6 | 7 | 8
  =======================================================================================
*/
bool PixelChain::saveEdgeChain(HANDLE fh, int id)
{
	char buff[256], temp[32];
	unsigned long bytesWritten;
	int len;
	
	if (!fh) {
		return false;
	}

	_id = id;

	memset(buff, 0, sizeof(buff));
	sprintf_s(buff, sizeof(buff), "%d,%d", id, _numPoints);
	
	if (_numPoints > 0) {
		sprintf_s(temp, sizeof(temp), ",%d,%d", _startPoint.x, _startPoint.y);
		strncat_s(buff, sizeof(buff), temp, _TRUNCATE);

		if (_numPoints > 1) {
			len = strlen(buff);
			memset(buff + len, 0, sizeof(buff) - len);

			buff[len] = ',';
			len++;
		
			for (int i = 0; i < _numPoints; i++) {
				buff[len] = '0' + _chain[i];
				len++;

				if (len > sizeof(buff) - 4) {
					WriteFile(fh, buff, len, &bytesWritten, NULL);
					memset(buff, 0, sizeof(buff));
					len = 0;
				}
			}
		}
	}

	strncat_s(buff, sizeof(buff), "\r\n", _TRUNCATE);

	WriteFile(fh, buff, strlen(buff), &bytesWritten, NULL);

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
char* PixelChain::stripWhitespace(const char *str)
{
	int len;
	char *buff;

	if (!str) {
		return NULL;
	}

	len = strlen(str);

	if (len < 1) {
		return NULL;
	}

	// put some padding on the end so we don't have to keep checking down below
	buff = new char[len + 2];

	if (buff) {
		strncpy_s(buff, len + 2, str, len);

		for (int i = len - 1; i > 0; i--) {
			if (buff[i] == '\r' || buff[i] == '\n') {
				buff[i] = 0;			
			}
			else {
				break;
			}
		}
	}

	return buff;
}

/*
  =======================================================================================
  The format of the string is <id>,<num points>, _pt[0]._x, _pt[0].y, <edge chain>
  where <edge chain> is a packed list of ascii numeric characters 1-8 representing
  	4 | 3 | 2	
	5 | 0 | 1
 	6 | 7 | 8
  this type of movement. See PixelChain::write()

  We transform this list into our _pt array.
  =======================================================================================
*/
bool PixelChain::loadEdgeChain(const char *str)
{
	char *buff, *pBuff, *q;
	int i, j, len, tentative_num_points, x, y;
	bool result = false;

	_numPoints = 0;

	if (_chain) {
		delete [] _chain;
		_chain = NULL;
	}

	if (!str) {
		return false;
	}

	len = strlen(str);

	// think about this...
	if (len < 1) {
		return true;
	}

	buff = stripWhitespace(str);

	if (!buff) {
		return false;
	}

	// read past the id
	q = strchr(buff, ',');

	if (!q) {
		goto END_LOAD_EDGE_CHAIN;
	}

	*q = 0;

	_id = atoi(buff);


	pBuff = q + 1;

	// now read num points
	q = strchr(pBuff, ',');
	
	if (!q) {
		goto END_LOAD_EDGE_CHAIN;
	}

	*q = 0;

	tentative_num_points = atoi(pBuff);

	if (tentative_num_points < 1) {		
		goto END_LOAD_EDGE_CHAIN;
	}

	pBuff = q + 1;
	
	q = strchr(pBuff, ',');

	if (!q) {
		goto END_LOAD_EDGE_CHAIN;
	}

	*q = 0;

	x = atoi(pBuff);

	if (x < 0) {
		goto END_LOAD_EDGE_CHAIN;
	}

	pBuff = q + 1;

	q = strchr(pBuff, ',');

	if (q) {
		*q = 0;
	}
	else if (tentative_num_points != 1) {
		goto END_LOAD_EDGE_CHAIN;
	}

	y = atoi(pBuff);

	if (y < 0) {
		goto END_LOAD_EDGE_CHAIN;
	}

	if (tentative_num_points == 1) {
		_numPoints = 1;
		result = true;
		goto END_LOAD_EDGE_CHAIN;
	}


	pBuff = q + 1;

	len = strlen(pBuff);

	_numPoints = (len > tentative_num_points) ? len : tentative_num_points;
	
	if (_numPoints == 0) {
		goto END_LOAD_EDGE_CHAIN;
	}

	if (!growList(_numPoints)) {
		goto END_LOAD_EDGE_CHAIN;
	}
	
	_startPoint.x = x;
	_startPoint.y = y;

	/*
	4 | 3 | 2	
	5 | 0 | 1
 	6 | 7 | 8
	*/
	for (j = 0, i = 1; i < _numPoints; i++, j++) {
		if (pBuff[j] >= '1' && pBuff[j] <= '8') {
			_chain[j] = pBuff[j] - '0';
		}
		else {
			break;
		}
	}

	_numPoints = j;

	result = true;

END_LOAD_EDGE_CHAIN:

	if (buff) {
		delete [] buff;
	}

	return result;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool PixelChain::growList(int size_required)
{
	unsigned char *new_list;

	if (size_required < 32) {
		size_required = 32;
	}
	else {
		size_required += 31;
		size_required &= 0xffe0;
	}

	new_list = new unsigned char[size_required];

	if (!new_list) {
		return false;
	}

	if (_chain) {
		delete [] _chain;
	}

	_chain = new_list;
	
	return true;
}


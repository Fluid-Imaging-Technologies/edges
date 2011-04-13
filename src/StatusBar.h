/*
 * $Id: StatusBar.h,v 1.7 2009/03/30 17:22:19 scott Exp $
 *
 */

#pragma once
#ifndef STATUS_BAR
#define STATUS_BAR

#include "version.h"


#define STATUS_BAR_HEIGHT 24

class StatusBar
{
public:
	StatusBar(HINSTANCE hInstance, HWND hWndParent);
	~StatusBar();

	static bool registerWinClass(HINSTANCE hInstance);
	static LRESULT CALLBACK StatusBarWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static char StatusBarWinClass[32];
	static bool Registered;

	HWND getWindowHandle();
	int getMinWidth();
	void reSize(int width, int height);
	void setPosition(int x, int y);
	void setStats(int id, int numPoints, double perimeter, double convex_perimeter, int area, int waist);
		
protected:
	void wmPaint();
	void wmCreate(HWND hWnd);
	void wmDestroy();

	void setSortBuff();

private:	
	HINSTANCE _hInstance;
	HWND _hWndParent;	
	HWND _hWnd;
	int _minWidth;
	int _x, _y, _id;
	int _numPoints;
	double _perimeter;
	double _convexPerimeter;
	int _area;
	double _circularity;
	double _smoothness;
	int _waist;
};


#endif // ifndef STATUS_BAR
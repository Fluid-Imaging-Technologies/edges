/*
 * $Id: ToolBar.h,v 1.3 2009/03/19 19:41:44 scott Exp $
 *
 */

#pragma once
#ifndef TOOL_BAR
#define TOOL_BAR

#include "version.h"

#define TOOL_BAR_HEIGHT 36

#define TB_FIRST 0
#define TB_PREV 1
#define TB_NEXT 2
#define TB_LAST 3
#define TB_ZOOM_IN 4
#define TB_ZOOM_OUT 5
#define TB_ZOOM_NORMAL 6
#define TB_RECALCULATE 7
#define NUM_TOOLBAR_BUTTONS 8

class ToolBarButtonStates
{
public:
	ToolBarButtonStates();
	void defaults();
	void disableAll();

	bool _first;
	bool _next;
	bool _prev;
	bool _last;
	bool _zoomIn;
	bool _zoomOut;
	bool _zoomNormal;
	bool _recalculate;
};

class ToolBar
{
public:
	ToolBar(HINSTANCE hInstance, HWND hWndParent);
	~ToolBar();

	static bool registerWinClass(HINSTANCE hInstance);
	static LRESULT CALLBACK ToolBarWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	static char ToolBarWinClass[32];
	static bool Registered;

	HWND getWindowHandle();
	void setButtonStates(ToolBarButtonStates *tbs);
	int getMinWidth();
	void reSize(int width);
	void killFocus(HWND hWnd);

protected:
	void wmPaint();
	void wmCommand(WPARAM wParam, LPARAM lParam);
	void wmCreate(HWND hWnd);
	void wmDestroy();
	void wmNotify(LPNMHDR pnmh);
	void createButtons();
	void drawTBButton(LPDRAWITEMSTRUCT pdis);
	void setButtonStatesInternal();

private:	
	HINSTANCE _hInstance;
	HWND _hWndParent;	
	HWND _hWnd;
	HWND _hWndButtons[NUM_TOOLBAR_BUTTONS];
	int _minWidth;
	ToolBarButtonStates _tbs;
};



#endif // ifndef TOOL_BAR
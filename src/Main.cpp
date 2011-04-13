/*
 * $Id: Main.cpp,v 1.21 2009/04/06 16:59:30 scott Exp $
 *
 */

#define USING_IMAGE_FILL2

#include "main.h"
#include "ToolBar.h"
#include "StatusBar.h"
#include "PixelChain.h"
#include "PointChain.h"

#if defined(USING_IMAGE_FILL2)
#include "ImageFill2.h"
#else
#include "ImageFill.h"
#endif

#include "ConvexHull.h"
#include "LineReader.h"
#include "Utility.h"
#include "resource.h"

#define DEFAULT_GRID_SPACING 10
#define MAX_GRID_SPACING 30
#define MIN_GRID_SPACING 2

#define GRID_ADJUSTMENT 2

#define MAX_PARTICLES 1024

#define BITMAP_MAX_X 4096
#define BITMAP_MAX_Y 4096


char gszMainWindowClass[] = "EdgesMainWindow";
char gszEdgesWindowClass[] = "EdgesWindow";

HINSTANCE ghInstance;
HWND ghWndMain;
HWND ghWndEdge;
bool gMaximized;
ToolBar *gtb;
ToolBarButtonStates tbs;
StatusBar *sb;
int gridSpacing;
PixelChain *gParticleList;
int gNumParticles;
int gCurrentParticle;

POINT gViewOrigin;
SIZE gViewSize;
POINT gStart;
bool gCapturing;

HDC gMemDC;
HBITMAP gBitmap, gOldBitmap;

PointChain gDrawChain;

#if defined(USING_IMAGE_FILL2)
ImageFill2 gImageFill;
#else
ImageFill gImageFill;
#endif

ConvexHull gConvexHull;

/*
  =======================================================================================
  =======================================================================================
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int cmdShow)
{
	MSG msg;

	ghInstance = hInstance;

	if (!InitApplication(hInstance)) {
		return 0;
	}

	if (!InitInstance(hInstance, cmdShow)) {
		return 0;
	}
	
	while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (gParticleList) {
		delete [] gParticleList;
	}

	return msg.wParam;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;
		
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc = MainWndProc;																			
	wc.hInstance = hInstance; 						
	wc.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_EDGES));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);  
	wc.hbrBackground = (HBRUSH) COLOR_BACKGROUND; 
	wc.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN_MENU);			
	wc.lpszClassName = gszMainWindowClass;		

	if (!RegisterClass(&wc)) {
		return false;
	}
	
	wc.lpfnWndProc = EdgesWndProc;
	wc.hbrBackground = 0; //(HBRUSH) COLOR_WINDOW; 
	wc.lpszMenuName = 0;
	wc.lpszClassName = gszEdgesWindowClass;

	if (!RegisterClass(&wc)) {
		return false;
	}

	InitCommonControls();	   

	gMaximized = false;
	gridSpacing = DEFAULT_GRID_SPACING;
	tbs._zoomIn = false;
	tbs._zoomOut = false;
	tbs._zoomNormal = false;

	gParticleList = NULL;
	gNumParticles = 0;
	gCurrentParticle = -1;

	return true;
}

/*
  =======================================================================================
  =======================================================================================
*/
bool InitInstance(HINSTANCE hInstance, int cmdShow)
{
	HWND hWnd;

	hWnd = CreateWindow(gszMainWindowClass,	  
						SOFTWARE_NAME,	  
						WS_OVERLAPPEDWINDOW,						
						CW_USEDEFAULT,	  
						CW_USEDEFAULT,	
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,   
						NULL,   
						hInstance,	  
						NULL);  

	if (!hWnd) {
		MessageBox(NULL, "System error creating main application window!", "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}


	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

	return true;
}

/*
  =======================================================================================  
  =======================================================================================
*/
LRESULT CALLBACK 
MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	/*
	case WM_MOUSEMOVE:
		wmMouseMove(hWnd, wParam, lParam);
		break;
	*/
	case WM_PAINT:
		wmPaint(hWnd);
		break;
	
	case WM_EXITSIZEMOVE:
		wmExitSizeMove(hWnd);
		break;

	case WM_GETMINMAXINFO:
		wmGetMinMaxInfo(hWnd, wParam, lParam);
		break;

	case WM_SIZE:
		wmSize(hWnd, wParam, lParam);
		break;

	case WM_COMMAND:
		wmCommand(hWnd, wParam, lParam);
		break;			  

	case WM_KEYDOWN:
		switch(wParam) 
		{
		case VK_LEFT:
			PostMessage(hWnd, WM_COMMAND, TB_PREV, 0);
			return 0;
		
		case VK_RIGHT:
			PostMessage(hWnd, WM_COMMAND, TB_NEXT, 0);
			return 0;
		
		case VK_UP:
			PostMessage(hWnd, WM_COMMAND, TB_ZOOM_IN, 0);
			return 0;

		case VK_DOWN:
			PostMessage(hWnd, WM_COMMAND, TB_ZOOM_OUT, 0);
			return 0;
		}

		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_CREATE:
		return wmCreate(hWnd);
			
	case WM_DESTROY:	
		if (gtb) {
			delete gtb;
			gtb = NULL;
		}

		if (sb) {
			delete sb;
			sb = NULL;
		}

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}						   

	return 0;
}

/*
  =======================================================================================  
  =======================================================================================
*/
LRESULT CALLBACK 
EdgesWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
	case WM_MOUSEMOVE:
		wmMouseMove(hWnd, wParam, lParam);
		break;

	case WM_LBUTTONDOWN:
		wmLButtonDown(hWnd, wParam, lParam);
		break;

	case WM_LBUTTONUP:
		wmLButtonUp(hWnd, wParam, lParam);
		break;

	case WM_PAINT:
		wmEdgesPaint(hWnd);
		break;
	
	case WM_DESTROY:
		SelectObject(gMemDC, gOldBitmap);
		DeleteObject(gBitmap);
		DeleteDC(gMemDC);
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	POINT old;
	SIZE movement;
	int max;

	if (gCapturing) {
		old = gStart;
		gStart.x = LOWORD(lParam);
		gStart.y = HIWORD(lParam);

		movement.cx = old.x - gStart.x;
		movement.cy = old.y - gStart.y;

		gViewOrigin.x += (old.x - gStart.x);
		
		if (gViewOrigin.x < 0) {
			gViewOrigin.x = 0;
		}

		max = BITMAP_MAX_X - gViewSize.cx;

		if (gViewOrigin.x >= max) {
			gViewOrigin.x = max;
		}

		gViewOrigin.y += (old.y - gStart.y);

		if (gViewOrigin.y < 0) {
			gViewOrigin.y = 0;
		}

		max = BITMAP_MAX_Y - gViewSize.cy;

		if (gViewOrigin.y >= max) {
			gViewOrigin.y = max;
		}
		
		HDC hdc = GetDC(hWnd);
		BitBlt(hdc, 0, 0, gViewSize.cx, gViewSize.cy, gMemDC, gViewOrigin.x, gViewOrigin.y, SRCCOPY); 
		ReleaseDC(hWnd, hdc);
	}

	if (sb) {
		sb->setPosition((gViewOrigin.x + LOWORD(lParam)) / gridSpacing, 
						(gViewOrigin.y + HIWORD(lParam)) / gridSpacing);
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!gCapturing) {
		SetCapture(hWnd);
		gCapturing = true;
		gStart.x = LOWORD(lParam);
		gStart.y = HIWORD(lParam);
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	ReleaseCapture();
	gCapturing = false;
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	
	HDC hdc = BeginPaint(hWnd, &ps);

	EndPaint(hWnd, &ps);
}

/*
  =======================================================================================  
  HDC gMemDC;
  HBITMAP gBitmap, gOldBitmap;
  =======================================================================================
*/
void wmEdgesPaint(HWND hWnd)
{
	PAINTSTRUCT ps;

	HDC hdc = BeginPaint(hWnd, &ps);

	BitBlt(hdc, 0, 0, gViewSize.cx, gViewSize.cy, gMemDC, gViewOrigin.x, gViewOrigin.y, SRCCOPY); 

	EndPaint(hWnd, &ps);
}

/*
  =======================================================================================  
  =======================================================================================
*/
void updateView()
{
	HPEN oldPen;
	HBRUSH oldBrush;
	RECT r;

	if (ghWndEdge) {
		GetClientRect(ghWndEdge, &r);

		gViewSize.cx = r.right - r.left;
		gViewSize.cy = r.bottom - r.top;

		oldPen = (HPEN) SelectObject(gMemDC, GetStockObject(WHITE_PEN));
		oldBrush = (HBRUSH) SelectObject(gMemDC, GetStockObject(WHITE_BRUSH));

		Rectangle(gMemDC, 0, 0, BITMAP_MAX_X, BITMAP_MAX_Y);
		
		SelectObject(gMemDC, oldBrush);
		SelectObject(gMemDC, oldPen);

		drawGrid(ghWndEdge, gMemDC);

		if (gDrawChain.getNumPoints() > 0) {
			drawEdgePoints(ghWndEdge, gMemDC);

			if (gridSpacing > 12) {
				numberCells(ghWndEdge, gMemDC);
			}

			drawFillPoints(ghWndEdge, gMemDC);	
			drawWaist(ghWndEdge, gMemDC);
			drawConvexHull(ghWndEdge, gMemDC);
		}
			
		SelectObject(gMemDC, oldPen);
		SelectObject(gMemDC, oldBrush);
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void numberCells(HWND hWnd, HDC hdc)
{
	HFONT hFont, oldFont;
	unsigned long oldTextColor;
	int oldBkMode;
	int x, y, len, numpts;
	char buff[8];
	SIZE sz;
	POINT *pts;

	pts = gDrawChain.getPoints(&numpts);

	if (!pts || numpts < 1) {
		return;
	}

	if (gridSpacing > 16) {
		hFont = CreateFont(12, 0, 0, 0, 500,								
						   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Tahoma");	
	}
	else {
		hFont = CreateFont(10, 0, 0, 0, 500,								
				   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Tahoma");	
	}

	oldFont = (HFONT) SelectObject(hdc, hFont);
	oldBkMode = SetBkMode(hdc, TRANSPARENT);
	oldTextColor = SetTextColor(hdc, RGB(0,0,0));

	for (int i = 0; i < numpts; i += 10) {
		x = pts[i].x * gridSpacing;
		y = pts[i].y * gridSpacing;

		len = sprintf_s(buff, sizeof(buff), "%d", i / 10);

		GetTextExtentPoint32(hdc, buff, len, &sz);

		TextOut(hdc, x + ((gridSpacing - sz.cx) / 2), y + ((gridSpacing - sz.cy) / 2), buff, len);
	}

	SelectObject(hdc, oldFont);
	SetBkMode(hdc, oldBkMode);
	SetTextColor(hdc, oldTextColor);
	DeleteObject(hFont);
}

/*
  =======================================================================================  
  =======================================================================================
*/
void drawEdgePoints(HWND hWnd, HDC hdc)
{
	HPEN old_pen, pen;
	HBRUSH old_brush, main_brush, start_brush, end_brush, mark_brush;
	int x, y, numpts;
	POINT *pts;

	pts = gDrawChain.getPoints(&numpts);

	if (!pts || numpts < 1) {
		return;
	}

	pen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
	main_brush = CreateSolidBrush(RGB(240,0,0));
	start_brush = CreateSolidBrush(RGB(0,240,0));
	end_brush = CreateSolidBrush(RGB(0,0,240));
	mark_brush = CreateSolidBrush(RGB(240,240,0));

	old_pen = (HPEN) SelectObject(hdc, pen);
	old_brush = (HBRUSH) SelectObject(hdc, start_brush);

	x = pts[0].x * gridSpacing;
	y = pts[0].y * gridSpacing;
	Rectangle(hdc, x, y, x + gridSpacing, y + gridSpacing);

	SelectObject(hdc, main_brush);

	numpts--;

	for (int i = 1; i < numpts; i++) {
		x = pts[i].x * gridSpacing;
		y = pts[i].y * gridSpacing;

		if ((i % 10) == 0) {
			SelectObject(hdc, mark_brush);
		}

		Rectangle(hdc, x, y, x + gridSpacing, y + gridSpacing);

		if ((i % 10) == 0) {
			SelectObject(hdc, main_brush);
		}
	}

	if (numpts > 0) {
		SelectObject(hdc, end_brush);
		x = pts[numpts].x * gridSpacing;
		y = pts[numpts].y * gridSpacing;
		Rectangle(hdc, x, y, x + gridSpacing, y + gridSpacing);
	}

	SelectObject(hdc, old_pen);
	SelectObject(hdc, old_brush);
	DeleteObject(pen);
	DeleteObject(main_brush);
	DeleteObject(start_brush);
	DeleteObject(end_brush);
	DeleteObject(mark_brush);
}

/*
  =======================================================================================  
  =======================================================================================
*/
void drawFillPoints(HWND hWnd, HDC hdc)
{
	HPEN old_pen, pen;
	HBRUSH old_brush, fill_brush;
	int x, y, numpts;
	POINT *pts;

	pts = gImageFill.getFillPoints(&numpts);

	if (!pts || numpts < 1) {
		return;
	}

	pen = CreatePen(PS_SOLID, 1, RGB(128,128,128));
	fill_brush = CreateSolidBrush(RGB(166,180,196));

	old_pen = (HPEN) SelectObject(hdc, pen);
	old_brush = (HBRUSH) SelectObject(hdc, fill_brush);

	for (int i = 0; i < numpts; i++) {
		x = pts[i].x * gridSpacing;
		y = pts[i].y * gridSpacing;
		Rectangle(hdc, x, y, x + gridSpacing, y + gridSpacing);
	}

	SelectObject(hdc, old_pen);
	SelectObject(hdc, old_brush);
	DeleteObject(pen);
	DeleteObject(fill_brush);	
}

/*
  =======================================================================================  
  =======================================================================================
*/
void drawWaist(HWND hWnd, HDC hdc)
{
	HPEN old_pen;
	HBRUSH old_brush;
	HFONT hFont, oldFont;
	unsigned long oldTextColor;
	int oldBkMode;
	PointDist *pts;
	int numpts, x, y, len;
	bool numbering_cells;
	char buff[8];
	SIZE sz;

	pts = gImageFill.getLocalMaxima(&numpts);

	if (!pts || numpts < 1) {
		return;
	}

	if (gridSpacing > 12) {
		numbering_cells = true;

		if (gridSpacing > 16) {
			hFont = CreateFont(12, 0, 0, 0, 500,								
						   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Tahoma");	
		}
		else {
			hFont = CreateFont(10, 0, 0, 0, 500,								
				   0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_NATURAL_QUALITY, 0, "Tahoma");	
		}

		oldFont = (HFONT) SelectObject(hdc, hFont);
		oldBkMode = SetBkMode(hdc, TRANSPARENT);
		oldTextColor = SetTextColor(hdc, RGB(255,255,255));
	}
	else {
		numbering_cells = false;
	}
	

	old_pen = (HPEN) SelectObject(hdc, GetStockObject(BLACK_PEN));
	old_brush = (HBRUSH) SelectObject(hdc, GetStockObject(BLACK_BRUSH));

	for (int i = 0; i < numpts; i++) {
		x = pts[i]._pt.x;
		y = pts[i]._pt.y;
		x *= gridSpacing;
		y *= gridSpacing;

		Rectangle(hdc, x, y, x + gridSpacing, y + gridSpacing); 

		if (numbering_cells) {
			len = sprintf_s(buff, sizeof(buff), "%d", pts[i]._dist);
			GetTextExtentPoint32(hdc, buff, len, &sz);
			TextOut(hdc, x + ((gridSpacing - sz.cx) / 2), y + ((gridSpacing - sz.cy) / 2), buff, len);
		}
	}

	delete [] pts;

	SelectObject(hdc, old_pen);
	SelectObject(hdc, old_brush);

	if (numbering_cells) {
		SelectObject(hdc, oldFont);
		SetBkMode(hdc, oldBkMode);
		SetTextColor(hdc, oldTextColor);
		DeleteObject(hFont);
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void drawConvexHull(HWND hWnd, HDC hdc)
{
	int numPts, offset;
	SortPoint *sp;
	HPEN old_pen, pen;
	
	numPts = 0;
	
	sp = gConvexHull.getConvexHull(&numPts);

	if (!sp || numPts < 3) {
		return;
	}

	pen = CreatePen(PS_SOLID, 2, RGB(0,0,0));
	old_pen = (HPEN) SelectObject(hdc, pen);

	offset = gridSpacing / 2;

	MoveToEx(hdc, offset + (sp[0]._pt.x * gridSpacing), offset + (sp[0]._pt.y * gridSpacing), NULL);

	for (int i = 0; i < numPts; i++) {
		LineTo(hdc, offset + (sp[i]._pt.x * gridSpacing), offset + (sp[i]._pt.y * gridSpacing));
	}

	LineTo(hdc, offset + (sp[0]._pt.x * gridSpacing), offset + (sp[0]._pt.y * gridSpacing));

	SelectObject(hdc, old_pen);
	DeleteObject(pen);
}

/*
  =======================================================================================  
  =======================================================================================
*/
void drawGrid(HWND hWnd, HDC hdc)
{
	HPEN pen, old_pen;
	int i;

	pen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));

	old_pen = (HPEN) SelectObject(hdc, pen);

	for (i = gridSpacing; i < BITMAP_MAX_X; i += gridSpacing) {
		MoveToEx(hdc, i, 0, NULL);
		LineTo(hdc, i, BITMAP_MAX_Y);
	}

	for (i = gridSpacing; i < BITMAP_MAX_Y; i += gridSpacing) {
		MoveToEx(hdc, 0, i, NULL);
		LineTo(hdc, BITMAP_MAX_X, i);
	}

	SelectObject(hdc, old_pen);
	DeleteObject(pen);
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmExitSizeMove(HWND hWnd)
{
	RECT r;
	int h, w;

	GetClientRect(hWnd, &r);

	h = r.bottom - r.top;
	w = r.right - r.left;

	gtb->reSize(w);
	sb->reSize(w, h);

	SetWindowPos(ghWndEdge, 
				NULL, 
				0, 
				TOOL_BAR_HEIGHT, 
				w, 
				h - (TOOL_BAR_HEIGHT + STATUS_BAR_HEIGHT), 
				SWP_NOZORDER);

	updateView();
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXIMIZED) {
		gMaximized = true;
		wmExitSizeMove(hWnd);
		InvalidateRect(hWnd, NULL, TRUE);
	}
	else if (gMaximized) {
		gMaximized = false;
		wmExitSizeMove(hWnd);
		InvalidateRect(hWnd, NULL, TRUE);		
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO *pMinMax = (MINMAXINFO *) lParam;

	pMinMax->ptMinTrackSize.x = 200;	
	pMinMax->ptMinTrackSize.y = 100;
}

/*
  =======================================================================================  
  =======================================================================================
*/
void wmCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case TB_FIRST:
		if (gNumParticles > 0) {
			if (gCurrentParticle > 0) {
				gCurrentParticle = 0;
				getEdgePointsForPlotting(gCurrentParticle);
				tbs._first = false;
				tbs._prev = false;
				tbs._next = (gNumParticles > 1);
				tbs._last = tbs._next;
				gtb->setButtonStates(&tbs);
				updateStatusBar();
				updateView();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}

		break;

	case TB_PREV:
		if (gNumParticles > 0) {
			if (gCurrentParticle > 0) {
				gCurrentParticle--;
				getEdgePointsForPlotting(gCurrentParticle);
				tbs._first = (gCurrentParticle > 0);
				tbs._prev = tbs._first;
				tbs._next = (gNumParticles > 1);
				tbs._last = tbs._next;
				gtb->setButtonStates(&tbs);
				updateStatusBar();
				updateView();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		
		break;

	case TB_NEXT:
		if (gNumParticles > 0) {
			if (gCurrentParticle < (gNumParticles - 1)) {
				gCurrentParticle++;
				getEdgePointsForPlotting(gCurrentParticle);
				tbs._first = true;
				tbs._prev = true;
				tbs._next = (gCurrentParticle < (gNumParticles - 1));
				tbs._last = tbs._next;
				gtb->setButtonStates(&tbs);
				updateStatusBar();
				updateView();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}

		break;

	case TB_LAST:
		if (gNumParticles > 0) {
			if (gCurrentParticle < (gNumParticles - 1)) {
				gCurrentParticle = gNumParticles - 1;
				getEdgePointsForPlotting(gCurrentParticle);
				tbs._first = true;
				tbs._prev = true;
				tbs._next = false;
				tbs._last = false;
				gtb->setButtonStates(&tbs);
				updateStatusBar();
				updateView();
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}

		break;

	case TB_ZOOM_IN:
		if (gridSpacing < MAX_GRID_SPACING) {
			gridSpacing += GRID_ADJUSTMENT;
			tbs._zoomIn = (gridSpacing != MAX_GRID_SPACING);
			tbs._zoomOut = true;
			tbs._zoomNormal = (gridSpacing != DEFAULT_GRID_SPACING);
			gtb->setButtonStates(&tbs);
			updateView();
			InvalidateRect(hWnd, NULL, TRUE);
		}

		break;

	case TB_ZOOM_OUT:
		if (gridSpacing > MIN_GRID_SPACING) {
			gridSpacing -= GRID_ADJUSTMENT;
			tbs._zoomIn = true;
			tbs._zoomOut = (gridSpacing != MIN_GRID_SPACING);
			tbs._zoomNormal = (gridSpacing != DEFAULT_GRID_SPACING);
			gtb->setButtonStates(&tbs);
			updateView();
			InvalidateRect(hWnd, NULL, TRUE);
		}

		break;

	case TB_ZOOM_NORMAL:
		if (gridSpacing != DEFAULT_GRID_SPACING) {
			gridSpacing = DEFAULT_GRID_SPACING;
			tbs._zoomIn = true;
			tbs._zoomOut = true;
			tbs._zoomNormal = false;
			gtb->setButtonStates(&tbs);
			updateView();
			InvalidateRect(hWnd, NULL, TRUE);
		}

		break;

	// for debugging
	case TB_RECALCULATE:
		if (gCurrentParticle >= 0 && gCurrentParticle < gNumParticles) {
			getEdgePointsForPlotting(gCurrentParticle);
			updateView();
			updateStatusBar();
			InvalidateRect(hWnd, NULL, TRUE);			
		}

		break;

	case IDM_FILE_OPEN:
		openEdgeFile(hWnd);
		updateView();
		tbs._first = false;
		tbs._prev = false;
		tbs._next = (gNumParticles > 1);
		tbs._last = (gNumParticles > 1);
		tbs._zoomIn = (gridSpacing != MAX_GRID_SPACING);
		tbs._zoomOut = (gridSpacing != MIN_GRID_SPACING);
		tbs._zoomNormal = (gridSpacing != DEFAULT_GRID_SPACING);
		tbs._recalculate = true;
		gtb->setButtonStates(&tbs);
		updateStatusBar();			
		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case IDM_HELP_ABOUT:
		DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
		break;

	case IDM_FILE_EXIT:
		DestroyWindow(hWnd);
		break;
	}
}

/*
  =======================================================================================  
  =======================================================================================
*/
void updateStatusBar()
{
	PixelChain *pc = &gParticleList[gCurrentParticle];				

	sb->setStats(pc->getId(), 
				pc->getNumPoints(), 
				pc->calculatePerimeter(),
				gConvexHull.getConvexPerimeter(),
				gImageFill.getFillArea(), 
				gImageFill.getWaist(NULL));
}

/*
  =======================================================================================  
  =======================================================================================
*/
void openEdgeFile(HWND hWnd)
{
	OPENFILENAME ofn;
	HANDLE fh;
	char file[MAX_PATH], fileTitle[MAX_PATH];

	memset(file, 0, sizeof(file));
	memset(fileTitle, 0, sizeof(fileTitle));
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = ghInstance;
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = MAX_PATH - 1;
	ofn.nMaxFileTitle = MAX_PATH - 1;
	ofn.Flags = OFN_FILEMUSTEXIST;		
	ofn.lpstrFile = file;
	ofn.lpstrFileTitle = fileTitle;

	ofn.lpstrFilter = "Edge Files (*.edg)\0*.edg\0\0";
	ofn.lpstrDefExt = "edg";

	if (!GetOpenFileName(&ofn)) {
		return;
	}

	fh = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fh < 0) {
		return;
	}

	if (readEdgeFile(fh)) {
		if (strlen(fileTitle) < sizeof(fileTitle) - 32) {
			strncat_s(fileTitle, sizeof(fileTitle), " - Edges", _TRUNCATE);
			SetWindowText(hWnd, fileTitle);
		}
		else {
			SetWindowText(hWnd, SOFTWARE_NAME);
		}

		gCurrentParticle = 0;
		getEdgePointsForPlotting(gCurrentParticle);
	}

	CloseHandle(fh);
}

/*
  =======================================================================================  
  =======================================================================================
*/
bool readEdgeFile(HANDLE fh)
{
	char *buff;
	unsigned long len;
	long num_lines;

	gDrawChain.clear();
	gImageFill.clear();
	gConvexHull.clear();

	gNumParticles = 0;

	num_lines = count_lines_in_file(fh);

	if (num_lines == 0) {
		return false;
	}

	if (num_lines > gNumParticles) {
		if (gParticleList) {
			delete [] gParticleList;
			gParticleList = NULL;
		}

		gParticleList = new PixelChain[num_lines];

		if (!gParticleList) {
			return false;
		}
	}

	LineReader *r = new LineReader(fh);

	if (!r) {
		return false;
	}

	buff = new char[4096];

	if (!buff) {
		delete r;
		return false;
	}

	memset(buff, 0, 4096);
	len = 4000;

	while (r->getNextLine(buff, &len)) {
		if (gParticleList[gNumParticles].loadEdgeChain(buff)) {
			gNumParticles++;

			if (gNumParticles == num_lines) {
				break;
			}
		}
		else {
			break;
		}

		// reset the length for the next getNextLine() call
		len = 4000;
	}

	delete r;
	delete [] buff;

	return (gNumParticles > 0);
}

/*
  =======================================================================================  
  =======================================================================================
*/
bool getEdgePointsForPlotting(int particle_id)
{
	gDrawChain.clear();
	gImageFill.clear();
	gConvexHull.clear();

	gViewOrigin.x = 0;
	gViewOrigin.y = 0;

	if (particle_id >= gNumParticles) {
		return false;
	}

	if (!gDrawChain.setChain(&gParticleList[particle_id])) {
		return false;
	}

	gDrawChain.translateNormalized();

	// TODO: handle the case where gDrawChain._numEdgePoints == 0
	if (!gImageFill.setPointChain(&gDrawChain)) {
		gDrawChain.clear();
		return false;
	}

	gConvexHull.setPoints(&gDrawChain);

	return true;
}

/*
  =======================================================================================  
  		SelectObject(gMemDC, gOldBitmap);
		DeleteObject(gBitmap);
		DeleteDC(gMemDC);

			hMemDC = CreateCompatibleDC(hdc);

	GetClientRect(hWnd, &r);

	gViewSize.cx = r.right - r.left;
	gViewSize.cy = r.bottom - r.top;

	bmp = CreateCompatibleBitmap(hdc, gViewSize.cx, gViewSize.cy);
	oldbmp = (HBITMAP) SelectObject(hMemDC, bmp);
  =======================================================================================
*/
int wmCreate(HWND hWnd)
{
	RECT r;
	HDC hdc;

	GetClientRect(hWnd, &r);

	gtb = new ToolBar(ghInstance, hWnd);

	if (!gtb) {
		return -1;
	}

	gtb->setButtonStates(&tbs);
	
	sb = new StatusBar(ghInstance, hWnd);

	if (!sb) {
		return -1;
	}

	ghWndEdge = CreateWindow(gszEdgesWindowClass,
							"",
							WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 
							r.left,
							r.top + TOOL_BAR_HEIGHT,
							r.right - r.left,
							(r.bottom - r.top) - (TOOL_BAR_HEIGHT + STATUS_BAR_HEIGHT),
							hWnd,
							NULL,
							ghInstance,
							0);


	if (!ghWndEdge) {
		return -1;
	}

	hdc = GetDC(ghWndEdge);

	gMemDC = CreateCompatibleDC(hdc);
	gBitmap = CreateCompatibleBitmap(hdc, BITMAP_MAX_X, BITMAP_MAX_Y);
	gOldBitmap = (HBITMAP) SelectObject(gMemDC, gBitmap);

	ReleaseDC(ghWndEdge, hdc);

	updateView();

	return 0;
}

/*
  =======================================================================================  
  =======================================================================================
*/
BOOL CALLBACK 
AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buff[64];

	switch (msg)
	{
	case WM_INITDIALOG:
		sprintf_s(buff, sizeof(buff), "Version: %d.%d", VERSION_MAJOR, VERSION_MINOR);
		SetDlgItemText(hDlg, IDC_VERSION, buff);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, 1);
			return TRUE;
		}
	}

	return FALSE;
}


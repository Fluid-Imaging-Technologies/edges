/*
 * $Id: StatusBar.cpp,v 1.8 2009/03/30 17:52:54 scott Exp $
 *
 */

#include "StatusBar.h"

#include "resource.h"

#define SB_ID_WIDTH 64
#define SB_NUMPOINTS_WIDTH 80
#define SB_PERIMETER_WIDTH 104
#define SB_CONVEX_PERIMETER_WIDTH 128
#define SB_AREA_WIDTH 76
#define SB_CIRCULARITY_WIDTH 88
#define SB_SMOOTHNESS_WIDTH 104
#define SB_WAIST_WIDTH 72
#define SB_POSITION_WIDTH 72


char StatusBar::StatusBarWinClass[32] = "StatusBarWinClass";
bool StatusBar::Registered = false;


/*
  =======================================================================================
  =======================================================================================
*/
StatusBar::StatusBar(HINSTANCE hInstance, HWND hWndParent)
{
	RECT r;

	_hInstance = hInstance;
	_hWndParent = hWndParent;
	_hWnd = 0;
	_x = _y = 0;
	_id = 0;
	_numPoints = 0;
	_perimeter = 0.0;
	_convexPerimeter = 0.0;
	_area = 0;
	_circularity = 0.0;
	_smoothness = 0.0;
	_waist = 0;

	// something to return
	_minWidth = SB_ID_WIDTH + SB_POSITION_WIDTH;

	if (!StatusBar::Registered) {
		StatusBar::registerWinClass(_hInstance);
	}

	GetClientRect(_hWndParent, &r);

	_hWnd = CreateWindow(StatusBar::StatusBarWinClass,
					"",
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					r.left,
					r.bottom - STATUS_BAR_HEIGHT,
					r.right - r.left,
					STATUS_BAR_HEIGHT,
					_hWndParent,
					NULL,
					_hInstance,
					this);

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
}

/*
  =======================================================================================
  =======================================================================================
*/
StatusBar::~StatusBar()
{
	if (_hWnd) {
		DestroyWindow(_hWnd);	
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::setPosition(int x, int y)
{
	_x = x;
	_y = y;

	InvalidateRect(_hWnd, NULL, FALSE);
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::setStats(int id, int numPoints, double perimeter, double convex_perimeter,
						 int area, int waist)
{
	_id = id;
	_numPoints = numPoints;
	_perimeter = perimeter;
	_convexPerimeter = convex_perimeter;
	_area = area;

	if (_perimeter > 0.0) {
		_circularity = (4.0 * PI * _area) / (_perimeter * _perimeter);
		_smoothness = _convexPerimeter / _perimeter;
	}
	else {
		_circularity = 0.0;	
		_smoothness = 0.0;
	}

	_waist = waist;

	InvalidateRect(_hWnd, NULL, FALSE);
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::wmDestroy()
{
	_hWnd = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::wmCreate(HWND hWnd)
{
	_hWnd = hWnd;
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::wmPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;
	HPEN oldPen, darkGreyPen;
	HBRUSH oldBrush, greyBrush;
	HFONT oldFont;
	RECT r;
	int oldBkMode;
	char buff[128];
	SIZE sz;
	int start, width, len, end;

	hdc = BeginPaint(_hWnd, &ps);

	darkGreyPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	greyBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	GetClientRect(_hWnd, &r);

	oldPen = (HPEN) SelectObject(hdc, darkGreyPen);
	oldBrush = (HBRUSH) SelectObject(hdc, greyBrush);

	Rectangle(hdc, r.left, r.top, r.right, r.bottom);	
	SelectObject(hdc, oldBrush);


	oldFont = (HFONT) SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	oldBkMode = SetBkMode(hdc, TRANSPARENT);

	start = 2;

	// need the height of the font
	len = sprintf_s(buff, sizeof(buff), "Id: %d", _id);

	GetTextExtentPoint32(hdc, buff, len, &sz);

	width = SB_ID_WIDTH;
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);
		
	// num points
	start += width + 4;
	width = SB_NUMPOINTS_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Points: %d", _numPoints);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);

	// perimeter
	start += width + 4;
	width = SB_PERIMETER_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Perimeter: %0.2lf", _perimeter);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);

	// convex perimeter
	start += width + 4;
	width = SB_CONVEX_PERIMETER_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Convex Perim: %0.2lf", _convexPerimeter);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);

	// area
	start += width + 4;
	width = SB_AREA_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Area: %d", _area);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);


	// circularity
	start += width + 4;
	width = SB_CIRCULARITY_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Circularity: %0.2lf", _circularity);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);

	// smoothness
	start += width + 4;
	width = SB_SMOOTHNESS_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Smoothness: %0.2lf", _smoothness);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);


	// waist
	start += width + 4;
	width = SB_WAIST_WIDTH;

	len = sprintf_s(buff, sizeof(buff), "Waist: %d", _waist);
	TextOut(hdc, r.left + start + 4, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);

	SelectObject(hdc, darkGreyPen);
	MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
	LineTo(hdc, r.left + start, r.top + 3);
	LineTo(hdc, r.left + start + width, r.top + 3);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	LineTo(hdc, r.left + start + width, r.bottom - 3);
	LineTo(hdc, r.left + start, r.bottom - 3);


	// remember where we left off
	end = start + SB_WAIST_WIDTH + 4;

	// start positioning from the right
	start = r.right - (SB_POSITION_WIDTH + 4);

	// we don't want the position overwriting other stuff
	if (start > end) {		
		width = SB_POSITION_WIDTH;

		SelectObject(hdc, darkGreyPen);
		MoveToEx(hdc, r.left + start, r.bottom - 4, NULL);
		LineTo(hdc, r.left + start, r.top + 3);
		LineTo(hdc, r.left + start + width, r.top + 3);

		SelectObject(hdc, GetStockObject(WHITE_PEN));
		LineTo(hdc, r.left + start + width, r.bottom - 3);
		LineTo(hdc, r.left + start, r.bottom - 3);
			
		len = sprintf_s(buff, sizeof(buff), "%d, %d", _x, _y);
		TextOut(hdc, r.left + start + 8, 1 + ((r.bottom - r.top) - sz.cy) / 2, buff, len);		
	}

	SelectObject(hdc, oldPen);	
	SelectObject(hdc, oldFont);
	SetBkMode(hdc, oldBkMode);

	DeleteObject(darkGreyPen);
	DeleteObject(greyBrush);

	EndPaint(_hWnd, &ps);
}

/*
  =======================================================================================
  =======================================================================================
*/
void StatusBar::reSize(int width, int height)
{
	SetWindowPos(_hWnd, NULL, 0, height - STATUS_BAR_HEIGHT, width, STATUS_BAR_HEIGHT, SWP_NOZORDER);
	InvalidateRect(_hWnd, NULL, TRUE);
}

/*
  =======================================================================================
  static function
  =======================================================================================
*/
LRESULT CALLBACK 
StatusBar::StatusBarWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT cs;
	StatusBar *win;

	switch (msg)
	{
	case WM_PAINT:
		win = (StatusBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			win->wmPaint();
		}

		break;

	case WM_DESTROY:
		win = (StatusBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			SetWindowLong(hWnd, GWL_USERDATA, 0);
			win->wmDestroy();
		}

		break;

	case WM_CREATE:
		win = (StatusBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			win->wmCreate(hWnd);
		}

		break;

	case WM_NCCREATE:
		cs = (LPCREATESTRUCT) lParam;
		SetWindowLong(hWnd, GWL_USERDATA, (long) cs->lpCreateParams);
		// deliberate fall through

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}


/*
  =======================================================================================
  =======================================================================================
*/
int StatusBar::getMinWidth() 
{
	return _minWidth;
}

/*
  =======================================================================================
  =======================================================================================
*/
HWND StatusBar::getWindowHandle()
{
	return _hWnd;
}

/*
  =======================================================================================
  static function
  =======================================================================================
*/
bool StatusBar::registerWinClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc = StatusBar::StatusBarWinProc;																
	wc.cbWndExtra = sizeof(StatusBar *);							
	wc.hInstance = hInstance; 			
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);   
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wc.lpszClassName = StatusBar::StatusBarWinClass;	
	

	if (RegisterClass(&wc)) {
		StatusBar::Registered = true;
	}

	return StatusBar::Registered;
}
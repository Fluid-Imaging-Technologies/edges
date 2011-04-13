/*
 * $Id: ToolBar.cpp,v 1.2 2009/03/19 19:41:44 scott Exp $
 *
 */

#include "ToolBar.h"

#include "resource.h"


char ToolBar::ToolBarWinClass[32] = "ToolBarWinClass";
bool ToolBar::Registered = false;


/*
  =======================================================================================
  =======================================================================================
*/
ToolBarButtonStates::ToolBarButtonStates()
{
	defaults();
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBarButtonStates::defaults()
{
	_first = false;
	_prev = false;
	_next = false;
	_last = false;
	_zoomIn = false;
	_zoomOut = false;
	_zoomNormal = false;
	_recalculate = false;
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBarButtonStates::disableAll()
{
	_first = false;
	_prev = false;
	_next = false;
	_last = false;
	_zoomIn = false;
	_zoomOut = false;
	_zoomNormal = false;
	_recalculate = false;
}

/*
  =======================================================================================
  =======================================================================================
*/
ToolBar::ToolBar(HINSTANCE hInstance, HWND hWndParent)
{
	RECT r;

	_hInstance = hInstance;
	_hWndParent = hWndParent;
	_hWnd = 0;
	memset(_hWndButtons, 0, sizeof(_hWndButtons));
	_minWidth = 240;

	if (!ToolBar::Registered) {
		ToolBar::registerWinClass(_hInstance);
	}

	GetClientRect(_hWndParent, &r);

	_hWnd = CreateWindow(ToolBar::ToolBarWinClass,
					"",
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					r.left,
					r.top,
					r.right - r.left,
					TOOL_BAR_HEIGHT,
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
ToolBar::~ToolBar()
{
	if (_hWnd) {
		DestroyWindow(_hWnd);	
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::wmDestroy()
{
	_hWnd = 0;
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::wmCreate(HWND hWnd)
{
	_hWnd = hWnd;
	createButtons();
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::setButtonStates(ToolBarButtonStates *tbs)
{
	if (!tbs) {
		return;
	}

	_tbs = *tbs;

	setButtonStatesInternal();
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::setButtonStatesInternal()
{
	EnableWindow(_hWndButtons[TB_FIRST], _tbs._first ? TRUE : FALSE);
	EnableWindow(_hWndButtons[TB_PREV], _tbs._prev ? TRUE : FALSE);
	EnableWindow(_hWndButtons[TB_NEXT], _tbs._next ? TRUE : FALSE);
	EnableWindow(_hWndButtons[TB_LAST], _tbs._last ? TRUE : FALSE);

	EnableWindow(_hWndButtons[TB_ZOOM_IN], _tbs._zoomIn ? TRUE : FALSE);
	EnableWindow(_hWndButtons[TB_ZOOM_OUT], _tbs._zoomOut ? TRUE : FALSE);
	EnableWindow(_hWndButtons[TB_ZOOM_NORMAL], _tbs._zoomNormal ? TRUE : FALSE);

	EnableWindow(_hWndButtons[TB_RECALCULATE], _tbs._recalculate ? TRUE : FALSE);
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::wmCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case TB_FIRST:
	case TB_PREV:
	case TB_NEXT:
	case TB_LAST:
	case TB_ZOOM_IN:
	case TB_ZOOM_OUT:
	case TB_ZOOM_NORMAL:
	case TB_RECALCULATE:
		SendMessage(_hWndParent, WM_COMMAND, wParam, 0);
		SetFocus(_hWndParent);
		break;
	}
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::wmPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;
	HPEN oldPen, darkGreyPen;
	HBRUSH oldBrush, greyBrush;
	RECT r;
	
	hdc = BeginPaint(_hWnd, &ps);

	GetClientRect(_hWnd, &r);

	darkGreyPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	oldPen = (HPEN) SelectObject(hdc, darkGreyPen);

	greyBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	oldBrush = (HBRUSH) SelectObject(hdc, greyBrush);

	Rectangle(hdc, r.left, r.top, r.right, r.bottom);	
	SelectObject(hdc, oldBrush);
	DeleteObject(greyBrush);

	MoveToEx(hdc, r.left, r.bottom - 2, NULL);
	LineTo(hdc, r.right - 1, r.bottom - 2);

	SelectObject(hdc, GetStockObject(WHITE_PEN));
	MoveToEx(hdc, r.left, r.bottom - 1, NULL);
	LineTo(hdc, r.left, r.top);
	LineTo(hdc, r.right - 1, r.top);

	SelectObject(hdc, oldPen);	
	DeleteObject(darkGreyPen);

	EndPaint(_hWnd, &ps);
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::reSize(int width)
{
	SetWindowPos(_hWnd, NULL, 0, 0, width, TOOL_BAR_HEIGHT, SWP_NOMOVE | SWP_NOZORDER);
	InvalidateRect(_hWnd, NULL, TRUE);
}


/*
  =======================================================================================
  static function
  =======================================================================================
*/
LRESULT CALLBACK 
ToolBar::ToolBarWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT cs;
	ToolBar *win;
	

	switch (msg)
	{	
	case WM_DRAWITEM:
		win = (ToolBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			win->drawTBButton((LPDRAWITEMSTRUCT) lParam);
		}

		break;

	case WM_PAINT:
		win = (ToolBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			win->wmPaint();
		}

		break;

	case WM_COMMAND:
		win = (ToolBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			win->wmCommand(wParam, lParam);
		}

		break;

	case WM_DESTROY:
		win = (ToolBar *) GetWindowLong(hWnd, GWL_USERDATA);

		if (win) {
			SetWindowLong(hWnd, GWL_USERDATA, 0);
			win->wmDestroy();
		}

		break;

	case WM_CREATE:
		win = (ToolBar *) GetWindowLong(hWnd, GWL_USERDATA);

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
void ToolBar::drawTBButton(LPDRAWITEMSTRUCT pdis)
{
	HFONT oldFont;
	HBRUSH greyBrush;
	RECT *pRect;
	char buff[32];
	SIZE sz;
	int id;
	COLORREF oldColor;
	int oldMode;

	id = pdis->CtlID;
	pRect = &pdis->rcItem;

	oldFont = (HFONT) SelectObject(pdis->hDC, GetStockObject(DEFAULT_GUI_FONT));

	greyBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	FillRect(pdis->hDC, pRect, greyBrush);
	DeleteObject(greyBrush);

	if (pdis->itemState & ODS_SELECTED) {
		DrawEdge(pdis->hDC, pRect, EDGE_SUNKEN, BF_RECT);		
	}
	else {
		DrawEdge(pdis->hDC, pRect, EDGE_RAISED, BF_RECT);
	}

	memset(buff, 0, sizeof(buff));

	switch(id) {
		case TB_ZOOM_IN:
			strncpy_s(buff, sizeof(buff), "Z+", _TRUNCATE);
			break;

		case TB_ZOOM_OUT:
			strncpy_s(buff, sizeof(buff), "Z-", _TRUNCATE);
			break;

		case TB_ZOOM_NORMAL:
			strncpy_s(buff, sizeof(buff), "Z1", _TRUNCATE);
			break;
	
		case TB_RECALCULATE:
			strncpy_s(buff, sizeof(buff), "Recalculate", _TRUNCATE);
			break;

		default:
			break;
	}

	oldMode = SetBkMode(pdis->hDC, TRANSPARENT);

	GetTextExtentPoint32(pdis->hDC, buff, strlen(buff), &sz);

	if (!IsWindowEnabled(pdis->hwndItem)) {
		oldColor = SetTextColor(pdis->hDC, GetSysColor(COLOR_GRAYTEXT)); 

		TextOut(pdis->hDC, 
			((pRect->right - pRect->left) - sz.cx) / 2, 
			((pRect->bottom - pRect->top) - sz.cy) / 2,
			buff, strlen(buff));

		SetTextColor(pdis->hDC, oldColor);
	}
	else {
		
		TextOut(pdis->hDC, 
			((pRect->right - pRect->left) - sz.cx) / 2, 
			((pRect->bottom - pRect->top) - sz.cy) / 2,
			buff, strlen(buff));
	}

	SetBkMode(pdis->hDC, oldMode);
	SelectObject(pdis->hDC, oldFont);
}

/*
  =======================================================================================
  =======================================================================================
*/
void ToolBar::createButtons()
{
	HICON hi;
	int x, y, w, h;
	int small_space, big_space;
	char buff[32];
	SIZE sz;
	HDC hdc = NULL;

	small_space = 4;
	big_space = 8;

	x = big_space;
	y = 5;
	w = 24;
	h = 24;


	_hWndButtons[TB_FIRST] = CreateWindow("button", NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
								x, y, w, h, 
								_hWnd, (HMENU) TB_FIRST, _hInstance, NULL);

	hi = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_FIRST));
	SendMessage(_hWndButtons[TB_FIRST], BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hi);

	x += w + small_space;

	_hWndButtons[TB_PREV] = CreateWindow("button", NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
								x, y, w, h, 
								_hWnd, (HMENU) TB_PREV, _hInstance, NULL);

	hi = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_PREVIOUS));
	SendMessage(_hWndButtons[TB_PREV], BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hi);

	x += w + small_space;

	y = 5;
	w = 24;
	h = 24;

	_hWndButtons[TB_NEXT] = CreateWindow("button", NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
								x, y, w, h, 
								_hWnd, (HMENU) TB_NEXT, _hInstance, NULL);

	hi = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_NEXT));
	SendMessage(_hWndButtons[TB_NEXT], BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hi);

	x += w + small_space;


	_hWndButtons[TB_LAST] = CreateWindow("button", NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
								x, y, w, h, 
								_hWnd, (HMENU) TB_LAST, _hInstance, NULL);

	hi = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_LAST));
	SendMessage(_hWndButtons[TB_LAST], BM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hi);


	x += w + big_space;

	_hWndButtons[TB_ZOOM_IN] = CreateWindow("button", "Z+", 
									WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
									x, y, w, h, 
									_hWnd, (HMENU) TB_ZOOM_IN, _hInstance, NULL);

	x += w + small_space;

	_hWndButtons[TB_ZOOM_OUT] = CreateWindow("button", "Z-", 
								WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
								x, y, w, h, 
								_hWnd, (HMENU) TB_ZOOM_OUT, _hInstance, NULL);
	
	x += w + small_space;

	_hWndButtons[TB_ZOOM_NORMAL] = CreateWindow("button", "Z1", 
								WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
								x, y, w, h, 
								_hWnd, (HMENU) TB_ZOOM_NORMAL, _hInstance, NULL);

	x += w + big_space;


	if (!hdc) {
		hdc = GetDC(_hWnd);
	}

	strncpy_s(buff, sizeof(buff), "Recalculate", _TRUNCATE);
	GetTextExtentPoint32(hdc, buff, strlen(buff), &sz);

	w = sz.cx + 6;

	_hWndButtons[TB_RECALCULATE] = CreateWindow("button", NULL, 
								WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
								x, y, w, h, 
								_hWnd, (HMENU) TB_RECALCULATE, _hInstance, NULL);

	x += w + big_space;


	if (hdc) {
		ReleaseDC(_hWnd, hdc);
	}

	_minWidth = x + big_space;

	setButtonStatesInternal();
}

/*
  =======================================================================================
  =======================================================================================
*/
int ToolBar::getMinWidth() 
{
	return _minWidth;
}

/*
  =======================================================================================
  =======================================================================================
*/
HWND ToolBar::getWindowHandle()
{
	return _hWnd;
}

/*
  =======================================================================================
  static function
  =======================================================================================
*/
bool ToolBar::registerWinClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc = ToolBar::ToolBarWinProc;																
	wc.cbWndExtra = sizeof(ToolBar *);							
	wc.hInstance = hInstance; 		
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);   			
	wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wc.lpszClassName = ToolBar::ToolBarWinClass;	
	

	if (RegisterClass(&wc)) {
		ToolBar::Registered = true;
	}

	return ToolBar::Registered;
}
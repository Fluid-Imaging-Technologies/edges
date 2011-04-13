/*
 * $Id: main.h,v 1.10 2009/04/06 16:59:30 scott Exp $
 *
 */

#pragma once
#ifndef MAIN_H
#define MAIN_H

#include "version.h"

bool InitApplication(HINSTANCE hInstance);
bool InitInstance(HINSTANCE hInstance, int cmdShow);

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void wmPaint(HWND hWnd);
void wmMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
void wmLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void wmLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
void wmGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam);
void wmCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
int wmCreate(HWND hWnd);
void wmExitSizeMove(HWND hWnd);
void wmSize(HWND hWnd, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK EdgesWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void wmEdgesPaint(HWND hWnd);
void updateStatusBar();

void updateView();
void drawGrid(HWND hWnd, HDC hdc);
void drawEdgePoints(HWND hWnd, HDC hdc);
void numberCells(HWND hWnd, HDC hdc);
void drawFillPoints(HWND hWnd, HDC hdc);
void drawWaist(HWND hWnd, HDC hdc);
void drawConvexHull(HWND hWnd, HDC hdc);
void openEdgeFile(HWND hWnd);
bool readEdgeFile(HANDLE fh);
bool getEdgePointsForPlotting(int particle_id);

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // ifndef MAIN_H

/*
$Date: 2010-05-11 22:13:27 +0900 (2010/05/11 (火)) $
$Rev: 85 $
*/

#ifndef WSDIALOG_H_
#define WSDIALOG_H_

#include <windows.h>

void WsDlgConfInit(HWND hWnd);
LRESULT CALLBACK ConfProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditProcKey(HWND hEditWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditProcJoy(HWND hEditWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabProc1(HWND hCtrl, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabProc2(HWND hCtrl, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabProc3(HWND hCtrl, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabProc4(HWND hCtrl, UINT msg, WPARAM wParam, LPARAM lParam);
LPCTSTR GetJoyName(int joy);
int GetJoyState(void);

#endif

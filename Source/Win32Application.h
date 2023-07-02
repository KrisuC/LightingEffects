#pragma once

#include <windows.h>
#include "D3D12Utility.h"

class Win32Application
{
public:
	static int Run(class Engine* pEngine, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hWnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hWnd;
};

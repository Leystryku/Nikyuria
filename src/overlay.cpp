#pragma once

#include "overlay.h"

#define MAX_FPS 120
#define MIN_DT (1000 / (MAX_FPS))

m_overlay *overlay;

bool bWinHasFocus = true;

void*shit = 0;

MARGINS margins = { -1 };
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	if (!hWnd)
		return 0;

	switch (message)
	{
	case WM_CREATE:
	{

		if (!FAILED(DwmExtendFrameIntoClientArea(hWnd, &margins)))
		{
			Sleep(2);
		}

		DWM_BLURBEHIND bb = { 0 };

		// Specify blur-behind and blur region.
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = true;
		bb.hRgnBlur = NULL;

		// Enable blur-behind.
		DwmEnableBlurBehindWindow(hWnd, &bb);

	}break;

	case WM_ACTIVATE:
	{
		//if (wParam != 0)
		//SetForegroundWindow(g_GameWindow);
	} break;

	case WM_DESTROY:
	{

	} break;

	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

WNDPROC origWinProc;

extern char curChar;

LRESULT CALLBACK HookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	if (!wParam)
		return CallWindowProc(origWinProc, hWnd, message, wParam, lParam);

	if (shit)
	{
		if (GetForegroundWindow() != shit)
		{
			bWinHasFocus = false;
		}
		else{
			bWinHasFocus = true;
		}
	}

	if (menu->is_open)
	{

	
		if (message == WM_CHAR)
		{
		
			char val = wParam;
			curChar = val;

			//Msg("%c\n", val);
		
		

			return 1;
		}

	}

	return CallWindowProc(origWinProc, hWnd, message, wParam, lParam);
}

std::string m_overlay::Initiate()
{
	MSG Msg;
	
	gameWindow = FindWindowA("Valve001", NULL);

	if (gameWindow == NULL)
	{
		MessageBoxA(NULL, "Could not find game Window !", "k", MB_OK);
		return 0;
	}

	SetForegroundWindow(gameWindow);

	RECT rc;

	GetWindowRect(gameWindow, &rc);
	scrW = rc.right - rc.left;
	scrH = rc.bottom - rc.top;



	HINSTANCE hinstance = (HINSTANCE)GetWindowLong(gameWindow, GWL_HINSTANCE);

	const char g_szClassName[] = "cfrapsoverlay";

	wchar_t* g_wchClassName = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, g_szClassName, -1, g_wchClassName, 4096);

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_wchClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	int register_attempt = 0;

	while (true)
	{
		register_attempt++;

		if (register_attempt > 10)
			return "Could not register window!";

		Sleep(50);
		if (RegisterClassEx(&wc))
			break;
	}

	unsigned int TID = { 0 };
	TID = GetWindowThreadProcessId(gameWindow, 0);

	Sleep(1);
	
	int create_attempts = 0;

	while (true)
	{
		create_attempts++;
		
		if (create_attempts > 10)
			return "Could not create window!";

		Sleep(50);

		if (overlayWindow == NULL)
		{
			overlayWindow = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
				g_szClassName,
				"Fraps Overlay",
				WS_POPUP | WS_VISIBLE | WS_SYSMENU,
				1,
				1,
				scrW , scrH,
				gameWindow,
				NULL,
				hinstance,
				NULL);

			if (overlayWindow != NULL)
				break;

			continue;
		}

		break;
	}

	
	SetWindowLong(overlayWindow, GWL_EXSTYLE, GetWindowLong(overlayWindow, GWL_EXSTYLE) | 0x80000 | 0x20);
	//SetLayeredWindowAttributes(overlayWindow, RGB(255, 255, 255), 255, ULW_COLORKEY | LWA_COLORKEY | LWA_ALPHA);
	SetLayeredWindowAttributes(overlayWindow, 255, 255, LWA_ALPHA);
	ShowWindow(overlayWindow, SW_SHOW);
	SetWindowPos(overlayWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	std::string init = d3d->Initiate(overlay->overlayWindow, overlay->scrW, overlay->scrH);

	if (init != "")
	{
		utils::file::WriteToLog("Could not initialize Nikyuria == d3d!\n");
		utils::file::WriteToLog(init.c_str());
		MessageBoxA(NULL, init.c_str(), "!ERROR!", MB_OK);
		return init.c_str();
	}

	if (!d3d || !d3d->instance)
	{
		return "no d3d!\n";
	}

	shit = gameWindow;
	Sleep(100);

	origWinProc = (WNDPROC)SetWindowLongA(gameWindow, GWL_WNDPROC, (long)HookProc);

	while (1)
	{

		Sleep(5);

		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}

		if (!bWinHasFocus)
		{
			d3d->Render(true);
			continue;
		}

		d3d->Render(false);
	

	}

	exit(0);

	return "";
}

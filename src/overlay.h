#pragma once
#pragma comment (lib, "Dwmapi.lib")
#pragma comment (lib, "Winmm.lib")

#include "stdafx.h"
#include <dwmapi.h>
#include <Mmsystem.h>
#include "d3d.h"
#include "utils.h"
#include "menu.h"

class m_overlay
{
public:
	std::string Initiate();
	std::string StartDraw();

	HWND overlayWindow = 0;
	HWND gameWindow = 0;

	int scrW = 0;
	int scrH = 0;

};

extern m_overlay *overlay;


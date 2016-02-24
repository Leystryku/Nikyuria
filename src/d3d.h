#pragma once

#include "stdafx.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "menu.h"
#include "sdk\sdk.h"

#include "global.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

class m_d3d
{
public:

	void DrawText(std::string pString, int x, int y, D3DCOLOR col, ID3DXFont *font, DWORD format = NULL, int w = NULL, int h = NULL);

	void DrawTextW(LPCWSTR pString, int len, int x, int y, D3DCOLOR col, ID3DXFont *font, DWORD format = NULL, int w = NULL, int h = NULL);

	void DrawRectangle(int x, int y, int w, int h, D3DCOLOR BoxColor);

	void DrawOutlinedRectangle(int x, int y, int w, int h, int thickness, D3DCOLOR BoxColor, D3DCOLOR BorderColor);

	void DrawLine(float x, float y, float x2, float y2, float thickness, D3DCOLOR LineCol);

	void Draw3DBox(Vector* points, float thickness, D3DCOLOR Color);

	D3DXVECTOR2 GetTextSize(ID3DXFont* font, const char*txt);

	void Render(bool bReset);

	void DoDraw();

	void DrawMenu();

	void DrawESP();

	void DrawLivingESP(IEntity *pEntity, const char *name, bool is_npc);

	void DrawWeaponESP(IEntity *pEntity, const char *name);

	void DrawHitboxESP(IEntity *pEntity, bool is_npc);

	void DrawViewESP();

	std::string Initiate(HWND hWnd, int ScrW, int ScrH);

	bool bFlushDraw = false;

	LPD3DXFONT font1;
	LPD3DXFONT font2;
	LPD3DXFONT font3;
	LPD3DXFONT font4;
	LPD3DXFONT font5;

	LPD3DXSPRITE sprite;
	ID3DXLine *line;

	IDirect3DDevice9Ex *device;
	IDirect3D9Ex *instance;

	int scrw, scrh;

	bool canrender;

	const char *viewesptxt;

};

extern m_d3d *d3d;
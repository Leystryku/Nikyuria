#pragma once

#include "d3d.h"


m_d3d *d3d = new m_d3d;


void m_d3d::DrawText(std::string pString, int x, int y, D3DCOLOR col, ID3DXFont *font, DWORD format, int w, int h)
{
	if (!device)
		return;

	if (!font)
		return;


	RECT FontRect = { (int)x, (int)y, (int)x, (int)y };

	if (w && h)
	{
		FontRect = { (int)x, (int)y, (int)(w + x), (int)(h + y) };
	}

	if (format)
	{
		font->DrawTextA(NULL, pString.c_str(), pString.length(), &FontRect, format, col);
		return;
	}
	
	font->DrawTextA(NULL, pString.c_str(), pString.length(), &FontRect, DT_NOCLIP, col);

}

void m_d3d::DrawTextW(LPCWSTR pString, int len, int x, int y, D3DCOLOR col, ID3DXFont *font, DWORD format, int w, int h)
{
	if (!device)
		return;

	if (!font)
		return;

	RECT FontRect = { (int)x, (int)y, (int)x, (int)y };

	if (w && h)
	{
		FontRect = { (int)x, (int)y, (int)(w + x), (int)(h + y) };
	}

	
	if (format)
	{
		font->DrawTextW(NULL, pString, len, &FontRect, format, col);
		return;
	}
	
	font->DrawTextW(NULL, pString, len, &FontRect, DT_NOCLIP, col);

}

class CVertexList
{
public:
	FLOAT X, Y, Z;
	DWORD dColor;
};

void m_d3d::DrawRectangle(int x, int y, int w, int h, D3DCOLOR BoxColor)
{

	CVertexList VertexList[4] =
	{
		{ (float)x, (float)y + (float)h, 0.0f, BoxColor },
		{ (float)x, (float)y, 0.0f, BoxColor },
		{ (float)x + (float)w, (float)y + (float)h, 0.0f, BoxColor },
		{ (float)x + (float)w, (float)y, 0.0f, BoxColor },
	};

	device->SetTexture(0, NULL);
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, VertexList, sizeof(CVertexList));
}

void m_d3d::DrawOutlinedRectangle(int x, int y, int w, int h, int thickness, D3DCOLOR BoxColor, D3DCOLOR BorderColor)
{
	DrawRectangle(x, y, w, h, BoxColor);

	//Top horiz line
	DrawRectangle(x, y, w, thickness, BorderColor);
	//Left vertical line
	DrawRectangle(x, y, thickness, h, BorderColor);
	//right vertical line
	DrawRectangle((x + w), y, thickness, h, BorderColor);
	//bottom horiz line
	DrawRectangle(x, y + h, w + thickness, thickness, BorderColor);

}

void m_d3d::DrawLine(float x, float y, float x2, float y2, float thickness, D3DCOLOR LineCol)
{

	CVertexList VertexList[2] =
	{
		{ (float)x, (float)y, 0.0f, LineCol },
		{ (float)x2, (float)y2, 0.0f, LineCol },
	};

	device->SetTexture(0, NULL);
	device->DrawPrimitiveUP(D3DPT_LINELIST, 2, VertexList, sizeof(CVertexList));

	if (thickness && thickness > 1.f)
	{
		int thick = (int)thickness;

		for (int i = 1; i < thick; i++)
		{
			DrawLine(x, y + i, x2, y2 + i, 0, LineCol);
		}
	}
	

}


void m_d3d::Draw3DBox(Vector* points, float thickness, D3DCOLOR Color)
{

	Vector startPos, endPos;

	for (int i = 0; i < 3; i++)
	{
		if (utils::game::WorldToScreen(scrw, scrh, points[i], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[i + 1], endPos, sdk->ViewMatrix))
			DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);
	}
	if (utils::game::WorldToScreen(scrw, scrh, points[0], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[3], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);

	for (int i = 4; i < 7; i++)
	{
		if (utils::game::WorldToScreen(scrw, scrh, points[i], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[i + 1], endPos, sdk->ViewMatrix))
			DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);
	}

	if (utils::game::WorldToScreen(scrw, scrh, points[4], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[7], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);

	if (utils::game::WorldToScreen(scrw, scrh, points[0], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[6], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);

	if (utils::game::WorldToScreen(scrw, scrh, points[1], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[5], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);

	if (utils::game::WorldToScreen(scrw, scrh, points[2], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[4], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);

	if (utils::game::WorldToScreen(scrw, scrh, points[3], startPos, sdk->ViewMatrix) && utils::game::WorldToScreen(scrw, scrh, points[7], endPos, sdk->ViewMatrix))
		DrawLine(startPos.x, startPos.y, endPos.x, endPos.y, thickness, Color);
}

D3DXVECTOR2 m_d3d::GetTextSize(ID3DXFont* font, const char* txt)
{

	RECT _recA;

	_recA.left = 0;
	_recA.top = 0;
	_recA.bottom = 500;
	_recA.right = 500;

	// this does not render to screen
	font->DrawTextA(NULL, txt, -1, &_recA, DT_CALCRECT, D3DCOLOR_RGBA(0, 0, 0, 0));

	D3DXVECTOR2 size;
	size.x = (float)_recA.right;
	size.y = (float)_recA.bottom;

	return size;
}

std::string m_d3d::Initiate(HWND hWnd, int ScrW, int ScrH)
{

	if (!hWnd)
		return "Invalid hWnd!\n";

	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &instance);

	if (FAILED(hr))
		return "Couldn't create instance!\n";

	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = TRUE;    // program windowed, not fullscreen
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;     // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = ScrW;    // set the width of the buffer
	d3dpp.BackBufferHeight = ScrH;    // set the height of the buffer
	d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_RESTRICTED_CONTENT;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	HRESULT res = instance->CreateDeviceEx(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
		&d3dpp,
		NULL,
		&device);

	if (FAILED(res))
		return "Couldn't create device!\n";

	D3DXCreateSprite(device, &sprite);
	D3DXCreateLine(device, &line);

	D3DXCreateFontA(device, 7, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font1);
	D3DXCreateFontA(device, 15, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font2);
	D3DXCreateFontA(device, 20, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font3);
	D3DXCreateFontA(device, 28, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font4);
	D3DXCreateFontA(device, 30, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font5);
	D3DXCreateFontA(device, 32, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font5);
	D3DXCreateFontA(device, 34, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma", &font5);

	scrw = ScrW;
	scrh = ScrH;

	return "";
}


void m_d3d::Render(bool bReset)
{
	if (!device) return;
	if (!bReset && !canrender ) return;
	canrender = false;

	if (!bReset)
	{
		bFlushDraw = true;
		device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1, 0);
		device->BeginScene();

		sprite->Begin(D3DXSPRITE_ALPHABLEND);

		device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

		DoDraw();

		sprite->End();

		device->EndScene();
		device->Present(0, 0, 0, 0);
	}

	if (bReset && bFlushDraw)
	{
		device->Clear(0, 0, D3DCLEAR_TARGET, 0, 1, 0);
		device->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1, 0);
		device->Clear(0, 0, D3DCLEAR_STENCIL, 0, 1, 0);

		device->BeginScene();
		device->EndScene();
		device->Present(0, 0, 0, 0);
		bFlushDraw = false;
	}

}

INT Fps = 0;
FLOAT LastTickCount = 0.0f;
FLOAT CurrentTickCount;
CHAR FrameRate[50] = "";

void m_d3d::DrawMenu()
{
	if (!menu)
		return;

	GetCursorPos(&menu->Cur);

	for (int i = 0; i < menu->num_elements; i++)
	{
		element *e = (element*)menu->elements[i];

		if (!e)
			continue;

		e->Think();

	}

	for (int i = 0; i < menu->num_elements; i++)
	{
		element *e = (element*)menu->elements[i];

		if (!e)
			continue;

		if (e->GetVisible())
			e->Paint();

	}

	if (!menu->is_open)
		return;

	CurrentTickCount = clock() * 0.001f;
	Fps++;

	if ((CurrentTickCount - LastTickCount) > 1.0f)
	{
		LastTickCount = CurrentTickCount;
		sprintf_s(FrameRate, "[ FPS: %d ]", Fps);
		Fps = 0;
	}

	d3d->DrawText(FrameRate, 10, 50, D3DCOLOR_RGBA(255, 255, 255, 255), font2);

	DWORD color = D3DCOLOR_RGBA(122, 139, 139, 255);
	int x = menu->Cur.x;
	int y = menu->Cur.y;


	d3d->DrawRectangle(x, y, 11, 1, color);
	d3d->DrawRectangle(x + 1, y + 1, 9, 1, color);
	d3d->DrawRectangle(x + 2, y + 2, 7, 1, color);
	d3d->DrawRectangle(x + 3, y + 3, 7, 1, color);
	d3d->DrawRectangle(x + 4, y + 4, 8, 1, color);
	d3d->DrawRectangle(x + 5, y + 5, 3, 1, color);
	d3d->DrawRectangle(x + 6, y + 6, 2, 1, color);
}
void m_d3d::DrawHitboxESP(IEntity *pEntity, bool is_npc)
{
	if (!pEntity->GetHBoxSet()) return;

	D3DCOLOR DrawColor = D3DCOLOR_RGBA(0, 0, 0, 255);


	if (pEntity->IsAlive())
	{
		if (!is_npc)
		{
			int myteam = g_pLocalEnt->GetTeam();
			int histeam = pEntity->GetTeam();

			if (myteam == histeam)
				DrawColor = D3DCOLOR_RGBA(0, 255, 0, 255);
			else
				DrawColor = D3DCOLOR_RGBA(255, 0, 0, 255);
		}
		else{
			DrawColor = D3DCOLOR_RGBA(0, 107, 60, 255);
		}

	}

	for (int i = 0; i < pEntity->GetHBoxSet()->numhitboxes; i++)
	{
		mstudiobbox_t* hitbox = pEntity->GetHBoxSet()->hitbox(i);

		if (!hitbox)
			continue;

		int bone = hitbox->bone;
		Vector min = hitbox->bbmax;
		Vector max = hitbox->bbmin;

		Vector points[] = { Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z) };
		
		Vector pointsTransformed[8];

		for (int i = 0; i < 8; i++)
		{
			points[i].x *= 0.9f;
			points[i].y *= 0.9f;
			points[i].z *= 0.9f;
			VectorTransform(points[i], pEntity->GetMatrix()[bone], pointsTransformed[i]);
		}
		
		Draw3DBox(pointsTransformed, 1, DrawColor);
	}
}

void m_d3d::DrawLivingESP(IEntity *pEntity, const char *name, bool is_npc)
{

	Vector origin = pEntity->GetAbsOrigin();

	if (origin.x == 0 && origin.y == 0 && origin.z == 0)
		return;

	Vector AbsScreen = Vector(0, 0, 0);

	if (!utils::game::WorldToScreen(scrw, scrh, origin, AbsScreen, sdk->ViewMatrix))
		return;

	D3DCOLOR DrawColor = D3DCOLOR_RGBA(0, 107, 60, 255);


	if (!is_npc)
	{
		int myteam = g_pLocalEnt->GetTeam();
		int histeam = pEntity->GetTeam();

		if (myteam == histeam)
			DrawColor = D3DCOLOR_RGBA(0, 255, 0, 255);
		else
			DrawColor = D3DCOLOR_RGBA(255, 0, 0, 255);
	}

#ifdef _GMOD
	if (is_npc)
		name = pEntity->GetClassname();
#endif

	int x = (int)AbsScreen.x;
	int y = (int)AbsScreen.y;

	if (menu->esp_name)
	{ 
		int len = 0;
		DrawText(name, x, y, DrawColor, font2);
	}
	if (menu->esp_health)
	{
		int health = pEntity->GetHealth();
		char chealth[12];
		sprintf_s(chealth, "%d", health);
		DrawText(chealth, x, y + 8, DrawColor, font2);
	}
		

	if (menu->esp_hitbox)
	{
		DrawHitboxESP(pEntity, is_npc);
	}

}

void m_d3d::DrawWeaponESP(IEntity *pEntity, const char *name)
{

	if (!pEntity || !name)
		return;

	const model_t *model = pEntity->GetModel();

	if (!model)
		return;

	Vector AbsScreen = Vector(0, 0, 0);
	Vector origin = pEntity->GetOrigin();

	if (origin.x == 0 && origin.y == 0 && origin.z == 0)
		return;

	if (utils::game::WorldToScreen(scrw, scrh, origin, AbsScreen, sdk->ViewMatrix) == false)
		return;

	if (AbsScreen.x == 0 && AbsScreen.y == 0)
		return;

	name = g_pModelInfo->GetModelName(model);

	std::string strName = name;

	strName.erase(0, strName.find("/w_") + 3);

	strName.erase(strName.find(".mdl"));

	strName[0] = toupper(strName[0]);

	for (std::size_t i = 1; i < strName.length(); ++i)
	{
		if (!strName[i])
			continue;

		strName[i] = toupper(strName[i]);

		if (strName[i] == '_')
			strName[i] = ' ';

	}

	name = strName.c_str();

	D3DCOLOR DrawColor = D3DCOLOR_RGBA(255, 255, 0, 255);

	int x = (int)AbsScreen.x;
	int y = (int)AbsScreen.y;

	DrawText(name, x, y, DrawColor, font2);
}





void m_d3d::DrawViewESP()
{
	if (!g_pLocalEnt) return;
	
	if (viewesptxt)
		DrawText(viewesptxt, scrw - 300, 2, D3DCOLOR_RGBA(255, 255, 255, 255), font4);
	else
		DrawText("worldspawn", scrw - 300, 2, D3DCOLOR_RGBA(255, 255, 255, 255), font4);
}

void m_d3d::DrawESP()
{

	if (!g_pEngine->IsInGame() || !g_pLocalEnt)
		return;

	int highestentindex = g_pEntList->GetHighestEntityIndex();
	int customesplen = menu->esp_customesp.length();

	for (int i = 1; i < highestentindex; i++)
	{
		IEntity *pEntity = g_pEntList->GetClientEntity(i);

		if (!pEntity || i == g_pEngine->GetLocalPlayer() )
			continue;

		if (utils::game::IsPlayerIndex(i, g_pGlobals->max_clients))
		{
			if (menu->esp_players == 1)
			{
				if (!pEntity->IsAlive())
					continue;

				if (pEntity->GetObserverMode() != 0)
					continue;

				player_info plInfo;

				g_pEngine->GetPlayerInfo(i, &plInfo);

				
				if (!plInfo.name) //not valid or sth
				{
					continue;
				}
				
				DrawLivingESP(pEntity, plInfo.name, false);

			}
		}

		ClientClass *pClientClass = pEntity->GetClientClass();

		if (!pClientClass)
			continue;

		const char *name = pClientClass->m_pNetworkName;

#ifdef _TF2
		if ( strstr(name, "NPC") || strstr(name, "NextBot") || strstr(name, "Object") )
		{
#else
		if (strstr(name, "NPC") || strstr(name, "NextBot"))
		{
#endif
			if (menu->esp_npcs)
			{
				DrawLivingESP(pEntity, name, true);
			}
			continue;
		}


#ifdef _GMOD

		name = pEntity->GetClassname();

		if (!name)
			continue;
#endif

		if (strstr(name, "Weapon") || strstr(name, "weapon"))
		{
			if (menu->esp_weapons)
			{
				DrawWeaponESP(pEntity, name);
			}

			continue;
		}

		if (customesplen > 0)
		{
			//toadd
		}


	}

	if (menu->esp_view)
	{
		DrawViewESP();
	}
}

void m_d3d::DoDraw()
{


	DrawMenu();
	DrawESP();
}
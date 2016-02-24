#pragma once

#include "../stdafx.h"
#include "../d3d.h"

class EButton : public element
{
private:
	Color buttoncol_pressed;

	bool pressed;
public:
	EButton(element*parent) : element(parent) { pressed = false; SetButtonCol_pressed(Color(99, 99, 99, 255)); }
	EButton() : EButton(NULL) { }

	inline void SetButtonCol_pressed(Color to_col)
	{
		buttoncol_pressed = to_col;
	}

	inline void SetButton_pressed(bool to_pressed)
	{
		pressed = to_pressed;
	}

	inline void PaintOver()
	{

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		Color col = GetColor();
		Color textcol = GetTextColor();

		std::string name = GetName();

		element *parent = GetParent();


		if (pressed)
		{
			d3d->DrawRectangle(x, y, w+1, h+1, D3DCOLOR_RGBA(buttoncol_pressed.r(), buttoncol_pressed.g(), buttoncol_pressed.b(), buttoncol_pressed.a()));
		}

		d3d->DrawTextW(name, x, y, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), (LPD3DXFONT)GetFont(), DT_VCENTER | DT_CENTER, w - 1, h - 1);
	}

	inline void Think()
	{

		if (!GetVisible())
			return;

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		int mouseover = MouseOver(0,0,0,0);

		if (pressed)
		{
			if (mouseover<2)
				pressed = false;
			
			return;
		}

		if (mouseover == 2)
		{
			OnLeftClick();
			pressed = true;
			return;
		}

		if (mouseover == 3)
		{
			OnRightClick();
			pressed = true;	
			return;
		}

	}

	void(*DoRightClick)(element *e)=0;
	void(*DoLeftClick)(element *e)=0;

	inline void OnRightClick()
	{
		if (!DoRightClick) return;
		DoRightClick(this);
	}

	inline void OnLeftClick()
	{
		if (!DoLeftClick) return;
		DoLeftClick(this);
	}
};

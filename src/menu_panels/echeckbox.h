#pragma once

#include "../stdafx.h"
#include "../menu.h"

class ECheckBox : public virtual element
{
private:
	bool *toggled;
	bool pressed;
public:
	ECheckBox(element*parent) : element(parent) { pressed = false; toggled = new bool(false); SetColor(Color(255, 255, 255, 255)); SetBorderColor(Color(38, 138, 214, 255)); }
	ECheckBox() : ECheckBox(NULL) { }

	void SetToggled(bool to_toggled)
	{
		*toggled = to_toggled;
	}

	void SetToggle(bool *toggle)
	{
		toggled = toggle;
	}

	inline void PaintOver()
	{

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		Color col = GetColor();
		Color textcol = GetTextColor();
		Color bordercol = GetBorderColor();

		std::string name = GetName();

		if (!&name)
			name = "";

		if (*toggled)
		{
			d3d->DrawOutlinedRectangle(x+1, y + 1, w-1, h-1, 1, D3DCOLOR_RGBA(bordercol.r(), bordercol.g(), bordercol.b(), bordercol.a()), D3DCOLOR_RGBA(col.r(), col.g(), col.b(), col.a()));
		}

		d3d->DrawTextW(name, x + w + 4, y, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font2);
	}

	inline void Think()
	{

		if (!GetVisible())
			return;

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		int mouseover = MouseOver(0, 0, 0, 0);

		if (pressed)
		{
			if (mouseover<2)
				pressed = false;

			return;
		}

		if (mouseover == 2)
		{
			pressed = true;
			DoToggle(!*toggled);
			return;
		}

	}

	void(*OnToggle)() = 0;

	inline void DoToggle(bool toggle)
	{
		*toggled = toggle;
		if (!OnToggle) return;
		OnToggle();
	}

};

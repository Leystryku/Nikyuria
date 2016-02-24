#pragma once

#include "../stdafx.h"
#include "../menu.h"

class ELabel : public element
{
public:
	ELabel(element* parent) : element(parent) { SetBorderColor(Color(38, 138, 214, 255)); }
	ELabel() : ELabel(NULL) { }

	inline void Paint()
	{
		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		Color col = GetColor();
		Color bordercol = GetBorderColor();
		std::string name = GetName();

		d3d->DrawText(name, x, y, D3DCOLOR_RGBA(col.r(), col.g(), col.b(), col.a()), d3d->font2, NULL, w, h);
	}

	inline void Think()
	{

		if (!GetVisible())
		{
			return;
		}

	}

};

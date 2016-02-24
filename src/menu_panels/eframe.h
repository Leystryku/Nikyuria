#pragma once

#include "../stdafx.h"
#include "../menu.h"

class EFrame : public element
{
private:
	Color closebuttoncol;
	Color closebuttoncol_pressed;
	Color closebuttoncol_hovered;

	bool close_pressed;
	bool close_hovered;
	POINT lastDragPos;
	int closesize = 30;

public:
	EFrame(element* parent) : element(parent) { SetPadding(0, 22); close_hovered = false; close_pressed = false; closebuttoncol = Color(199, 80, 80, 255); closebuttoncol_hovered = Color(224, 67, 67, 255);  closebuttoncol_pressed = Color(153, 61, 61, 255);  SetBorderColor(Color(38, 138, 214, 255)); }
	EFrame() : EFrame(NULL) { }

	inline void SetCloseButtonCol(Color to_col)
	{
		closebuttoncol = to_col;
	}

	inline void SetCloseButtonCol_pressed(Color to_col)
	{
		closebuttoncol_pressed = to_col;
	}

	inline void SetCloseButtonCol_hovered(Color to_col)
	{
		closebuttoncol_hovered = to_col;
	}

	inline void SetCloseButton_pressed(bool pressed)
	{
		close_pressed = pressed;
	}

	inline void PaintOver()
	{

		int x, y;
		GetPos(x, y,true);

		int w, h;
		GetSize(w, h);

		Color col = GetColor();
		Color textcol = GetTextColor();
		Color bordercol = GetBorderColor();
		std::string name = GetName();

		if (!&name)
			name = "";

		d3d->DrawRectangle(x, y, w, 28, D3DCOLOR_RGBA(bordercol.r(), bordercol.g(), bordercol.b(), bordercol.a()));
		d3d->DrawTextW(name, x + 5, y - 1, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font4, DT_CENTER, w, 28);

		if (close_pressed)
		{ 
			d3d->DrawRectangle((x + w) - closesize, y+1, closesize, 22, D3DCOLOR_RGBA(closebuttoncol_pressed.r(), closebuttoncol_pressed.g(), closebuttoncol_pressed.b(), closebuttoncol_pressed.a()));
			d3d->DrawTextW("X", (x + w) - closesize + 8, y + 1, D3DCOLOR_RGBA(255, 255, 255, 255), d3d->font3);
			return;
		}

		if (close_hovered)
		{
			d3d->DrawRectangle((x + w) - closesize, y+1, closesize, 22, D3DCOLOR_RGBA(closebuttoncol_hovered.r(), closebuttoncol_hovered.g(), closebuttoncol_hovered.b(), closebuttoncol_hovered.a()));
			d3d->DrawTextW("X", (x + w) - closesize + 8, y + 1, D3DCOLOR_RGBA(255, 255, 255, 255), d3d->font3);
			return;
		}

		d3d->DrawRectangle((x + w) - closesize, y+1, closesize, 22, D3DCOLOR_RGBA(closebuttoncol.r(), closebuttoncol.g(), closebuttoncol.b(), closebuttoncol.a()));

		d3d->DrawTextW("X", (x + w) - closesize+8, y+1 , D3DCOLOR_RGBA(255, 255, 255, 255), d3d->font3);
	}

	inline void Think()
	{

		if (GetToggleKeyState(VK_F10))
			menu->misc_freeze = !menu->misc_freeze;

		if (GetToggleKeyState(VK_F11))
		{
			menu->is_open = !menu->is_open;
			SetVisible(menu->is_open);
		}

		if (!GetVisible())
		{
			
			return;
		}

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		int mouseover_closebutton = MouseOver((x + w) - closesize, y, 29, 22);

		if (close_pressed)
		{
			if (mouseover_closebutton<2)
			{
				close_pressed = false;
				menu->is_open = false;
				SetVisible(false);
			}

			return;
		}

		if (mouseover_closebutton == 1)
		{
			close_hovered = true;
			return;
		}

		if (mouseover_closebutton == 2)
		{
			close_pressed = true;
			return;
		}

		//d3d->DrawRectangle(x, y, w, 22, D3DCOLOR_RGBA(bordercol.r(), bordercol.g(), bordercol.b(), bordercol.a()));
		
		int mouseover_topbar = MouseOver(x, y, w - closesize, 28);

		if (mouseover_topbar == 2)
		{

			POINT curDragPos = menu->Cur;

			if (lastDragPos.x && lastDragPos.y)
			{
				int deltaX = curDragPos.x - lastDragPos.x;
				int deltaY = curDragPos.y - lastDragPos.y;

				if (deltaX || deltaY)
				{
					int curpos_x, curpos_y;
					GetPos(curpos_x, curpos_y, false);

					if ((curpos_x + deltaX)< 1 || 1 > (curpos_y + deltaY))
						return;

					SetPos(curpos_x + deltaX, curpos_y + deltaY);
				}
			}

			lastDragPos.x = curDragPos.x;
			lastDragPos.y = curDragPos.y;

			return;
		}

		lastDragPos.x = 0;
		lastDragPos.y = 0;
		close_hovered = false;
		close_pressed = false;
	}

};

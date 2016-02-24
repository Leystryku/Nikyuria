#pragma once

#include "../stdafx.h"
#include "../menu.h"

class ETextBox : public virtual element
{
private:
	bool selected;
	bool pressed;
	std::string *textBuf;
	int cursorPos;
public:
	ETextBox(element* parent) : element(parent) { pressed = false; selected = false; cursorPos = 0; textBuf = new std::string(""); SetColor(Color(0, 0, 0, 0)); }
	ETextBox() : ETextBox(NULL) { }

	inline void PaintOver()
	{
		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		std::string name = GetName();

		Color textcol = GetTextColor();

		d3d->DrawText(name.c_str(), x + 3, y - 23, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font2, DT_VCENTER, w, h);
		
		d3d->DrawText(textBuf->c_str(), x + 3, y, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font2, DT_VCENTER, w, h);

		if (selected)
		{
			float cursorx = 0;
			if (textBuf->length()>0)
			{
				int cursorDelta = textBuf->length() - cursorPos;

				if (cursorDelta < 0)
					return;

				std::string cursorBuf = textBuf->c_str();
				cursorBuf.erase(cursorDelta, cursorBuf.length());
				for (unsigned int i = 0; i < cursorBuf.length(); i++)
				{
					if (cursorBuf[i] == ' ')
						cursorBuf[i] = 'l'; //space prob
				}
			
				cursorx = (float)(x + 5 + d3d->GetTextSize(d3d->font2, cursorBuf.c_str()).x);
			
			}
			else{
				cursorx = (float)(x + 5 );
			}

			d3d->DrawLine(cursorx, (float)y, cursorx, (float)y+h, 1.f, D3DCOLOR_RGBA(140, 140, 140, 255));
		}
	}

	inline void Think()
	{

		if (!GetVisible())
		{
			return;
		}

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
			selected = !selected;
			return;
		}


		if (!selected)
			return;

		
		char curChar = GetCurChar();
		//Msg("%i", (int)curChar);
		//8 = backspace, 127 = del, 13=enter

		if (curChar == 13)
		{
			selected = false;
			return;
		}

		if (textBuf->length()>0)
		{
			if (curChar == 8 || curChar == 127)
			{
				if (textBuf->length() == 1)
				{
					textBuf->erase(0, 1);
					return;
				}
				int cursorDelta = textBuf->length() - cursorPos;

				if (cursorDelta < 1)
					return;
				
				
				textBuf->erase(cursorDelta - 1, cursorDelta - 1);
					

			}

			if (GetToggleKeyState(VK_LEFT))
			{
				int cursorDelta = textBuf->length() - cursorPos;

				if (cursorDelta > 0)
				{
					cursorPos += 1;
				}
			}

			if (GetToggleKeyState(VK_RIGHT))
			{
				if (cursorPos > 0)
				{
					cursorPos -= 1;
				}
			}
		}

		if (curChar&& curChar>31)
		{ 
			if (!cursorPos)
			{
				textBuf->push_back(curChar);
				return;
			}

			std::string newBuf = "";

			int i = 0;
			while (textBuf->c_str()[i])
			{
				if (i == textBuf->length() - cursorPos)
					newBuf.push_back(curChar);

				newBuf.push_back(textBuf->c_str()[i]);

				i++;
			}
			textBuf->clear();
			textBuf->append(newBuf);


			Msg("%i\n", textBuf->length());
		}

	}

	void SetTextBuf(std::string *txtbuf)
	{
		textBuf = txtbuf;
	}


};

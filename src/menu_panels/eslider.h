#pragma once

#include "../stdafx.h"
#include "../menu.h"

int roundnum(float d)
{
	return (int)floor(d + 0.5);
}

class ESlider : public virtual element
{
private:
	float *slide = NULL;
	float *slideval = NULL;
	int *islideval = NULL;
	float slide_max;
	bool pressed;
	std::string value;
	std::string slidetexts[2048];

	int thickness;
	
public:
	ESlider(element*parent) : element(parent) { thickness = 2; pressed = false; slide = new float(0); slideval = new float(0); slide_max = 0; SetColor(Color(255, 255, 255, 255)); SetBorderColor(Color(38, 138, 214, 255)); }
	ESlider() : ESlider(NULL) { }

	inline void Paint()
	{
		if (!slide_max)
			return;

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		h = 21; // always 21

		Color col = GetColor();
		Color textcol = GetTextColor();
		Color bordercol = GetBorderColor();
		std::string name = GetName();

		if (!&name)
			name = "";


		int yoffset = 18;

		d3d->DrawText(name, x, y, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font2, DT_LEFT, w, yoffset);

		if (&value)
			d3d->DrawText(value, x, y, D3DCOLOR_RGBA(textcol.r(), textcol.g(), textcol.b(), textcol.a()), d3d->font2, DT_RIGHT, w, yoffset);

		d3d->DrawLine((float)x, (float)(y + yoffset), (float)(x + w), (float)(y + yoffset), (float)thickness, D3DCOLOR_RGBA(140, 140, 140, 255));

		if (!w)
			return;

		float fw = (float)(w - 8);

		if (fw < 1)
			return;

		float fpos = (*slide / slide_max);
		fpos *= fw;

		//if (!fpos)
		//	return;

		float pos = (float)x + fpos;

		d3d->DrawRectangle((int)pos, y + yoffset - 2, 20, 5, D3DCOLOR_RGBA(84, 79, 81, 255));
		d3d->DrawOutlinedRectangle((int)pos, y + yoffset - 2, 20, 5, 1, D3DCOLOR_RGBA(0, 0, 0, 0), D3DCOLOR_RGBA(col.r(), col.g(), col.b(), col.a()));
	}

	inline void Think()
	{

		if (!GetVisible() || !slide)
			return;

		int x, y;
		GetPos(x, y, true);

		int w, h;
		GetSize(w, h);

		h = 21; //always 21

		int mouseover = MouseOver(x, y + 7, w, h);

		char slidenum[256];

		int islide = roundnum(*slide);

		if (islideval)
		{
			sprintf_s(slidenum, "%i", islide);
		}else{
			sprintf_s(slidenum, "%f", *slide);
		}

		if (islide < 2048 && slidetexts[islide].length() > 0)
			value = slidetexts[islide];
		else
			value = slidenum;

		if (pressed)
		{
			if (mouseover<2)
				pressed = false;

			POINT Cur = menu->Cur;

			int xoffset = (Cur.x - x) - 10;

			float newslide = ((float)xoffset / (float)(w - 8)) * slide_max;

			if (newslide > slide_max)
				newslide = slide_max;

			if (newslide < 0)
				newslide = 0.0f;

			if (islideval)
				*islideval = islide;
			
			if (slideval)
				*slideval = newslide;

			OnSlide(newslide);

			return;
		}

		if (mouseover == 2)
		{
			pressed = true;
			return;
		}

	}

	void SetSlide(float to_slide)
	{
		if (to_slide > slide_max)
			to_slide = slide_max;

		if (to_slide < 0)
			to_slide = 0.0f;

		if (islideval)
		{
			*islideval = roundnum(to_slide);
		}

		if (slideval)
			*slideval = to_slide;

		OnSlide(to_slide);
	}

	void SetSlider(float *to_slide)
	{
		slideval = to_slide;
	}

	void SetSlider(int *to_slide)
	{
		islideval = to_slide;
	}

	void SetSlideMax(float to_slide)
	{
		slide_max = to_slide;
	}

	void SetSlideText(int num, std::string slidetext)
	{
		slidetexts[num] = slidetext;
	}

	void SetWidth(int to_thickness)
	{
		thickness = to_thickness;
	}

	int GetThickness()
	{
		return thickness;
	}

	void(*DoSlide)(float slide) = 0;

	void OnSlide(float to_slide)
	{
		*slide = to_slide;

		if (DoSlide)
			DoSlide(to_slide);
	}

};

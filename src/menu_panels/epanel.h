#pragma once

#include "../stdafx.h"
#include "../menu.h"

class EPanel : public element
{
public:
	EPanel(element* parent) : element(parent) {  }
	EPanel() : EPanel(NULL) { }

	inline void Think()
	{

		if (!GetVisible())
		{
			return;
		}

	}

};

#pragma once

#include "../stdafx.h"
#include "vector.h"

class CUserCmd
{
public:

	int header;
	int		command_number;
	int		tick_count;
	QAngle viewangles;

#ifdef _CSGO
	Vector aimdir;
#endif
	QAngle move;
	int		buttons;
	char	impulse;
	int		weaponselect;
	int		weaponsubtype;
	int		random_seed;
	short	mousex;
	short	mousey;
	bool	hasbeenpredicted;
#ifdef _CSGO
	QAngle headangles;
	Vector headoffset;
#endif

#ifdef _GMOD
	//92=context, 96=contextaim

	char padding[284];


	inline bool* context()
	{
		return (bool*)((char*)this + 0x5A);
	}

	inline Vector* contextaim()
	{
		return (Vector*)((char*)this + 0x5C);
	}

	inline char* mousewheel()
	{
		return (char*)((char*)this + 0x59);
	}
#endif

};
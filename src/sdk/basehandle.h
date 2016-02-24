#pragma once
#include "../stdafx.h"

#define INVALID_EHANDLE_INDEX 0xFFFFFFFF

#define	MAX_EDICT_BITS				11			// # of bits needed to represent max edicts
#define NUM_ENT_ENTRY_BITS		(MAX_EDICT_BITS + 1)
#define NUM_ENT_ENTRIES			(1 << NUM_ENT_ENTRY_BITS)
#define ENT_ENTRY_MASK			(NUM_ENT_ENTRIES - 1)

class IHandleEntity
{
public:
	virtual ~IHandleEntity() {}
	virtual void SetRefEHandle(/*const CBaseHandle &handle*/) = 0;
	virtual unsigned long& GetRefEHandle() const = 0;
};

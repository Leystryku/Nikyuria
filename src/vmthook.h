#pragma once

#include "stdafx.h"

class VMTHook
{
public:
	void** m_pOriginalVTable;
	void** m_pNewVTable;
	void*** m_pInstance;

	int m_iNumIndices;
	//funcs

	VMTHook(void* instance);
	~VMTHook();

	//bool unhookTable( );

	int tellCount( );

	void* hookFunction(int iIndex, void* pfnHook);
	void* unhookFunction(int iIndex);
	void* GetMethod(int iIndex);
	void* GetHookedMethod(int iIndex);
	void SetPadding(int pad);

	int padding;

};



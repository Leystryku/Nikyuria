#pragma once

#include "interface.h"

// ======================= INDEXES =========================== //

#define INDEX_ILUASHARED_CREATELUAINTERFACE 4
#define INDEX_ILUASHARED_GETLUAINTERFACE 6

// ======================= CLASS HEADER ======================= //

#define lua_pop(L,n)  g_pLuaShared->lua_settop(L, -(n)-1)
#define lua_popa(L,n)  lua_settop(L, -(n)-1)

SETUP_INTERFACE2(ILuaShared);
	
		luaL_loadbufferx = (int(*)(void*, const char*, size_t, const char*, void*))GetProcAddress(GetModuleHandleA("lua_shared.dll"), "luaL_loadbufferx");
		lua_pcall = (int(*)(void*, int, int, int))GetProcAddress(GetModuleHandleA("lua_shared.dll"), "lua_pcall");
		lua_settop = (int(*)(void*, int))GetProcAddress(GetModuleHandleA("lua_shared.dll"), "lua_settop");
		lua_tolstring = (const char*(*)(void*, int, int))GetProcAddress(GetModuleHandleA("lua_shared.dll"), "lua_tolstring");

		if (!luaL_loadbufferx || !lua_pcall || !lua_settop || !lua_tolstring)
		{
			utils::file::WriteToLog("No luaL_x!\n");
			return;
		}
	}

	int(*luaL_loadbufferx)(void *L, const char *buf, size_t len, const char *name, void*nig);
	int(*lua_pcall)(void *L, int a, int b, int c);
	int(*lua_settop)(void *L, int a);
	const char *(*lua_tolstring)(void *L, int a, int len);

	bool RunCode(void* state, const char*code, const char*name)
	{
		void* realstate = *(void**)((char*)state + 0x4);

		int error = luaL_loadbufferx(realstate, code, strlen(code), name, 0);

		if (error)
		{
			Msg("[Error] %s\n", lua_tolstring(realstate, -1, 0));
			lua_popa(realstate, 1);
			return false;
		}

		if (lua_pcall(realstate, 0, -1, 0))
		{
			lua_popa(realstate, 1);
			return false;
		}

		return true;
	}

	SetupInterfaceFunc(CreateLuaInterface, void*, INDEX_ILUASHARED_CREATELUAINTERFACE, (unsigned char state, bool b), (void* __this, unsigned char state, unsigned char b), (this->vmt, state, b));
	
	SetupInterfaceFunc(GetLuaInterface, void*, INDEX_ILUASHARED_GETLUAINTERFACE, (unsigned char state), (void* __this, unsigned char state), (this->vmt, state));

};


extern ILuaShared *g_pLuaShared;


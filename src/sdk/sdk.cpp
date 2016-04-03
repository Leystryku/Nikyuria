#pragma once

#include "sdk.h"
CGlobalVars *g_pGlobals = NULL;

// ======================= INITIALIZATION ================= //

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

void* getInterface(void* factory, char* name)
{
	CreateInterfaceFn fn = (CreateInterfaceFn)factory;

	void *ret = 0;
	for (int i = 0;i<100; i++)
	{
		char* fmt = (i < 10) ? "%s00%d" : "%s0%d";

		std::string formatted = utils::string::formatString(fmt, name, i);
		void *ptr = (fn)(formatted.c_str(), 0);
		if (ptr)
		{
			Msg(formatted.c_str());
			Msg("\n");

			ret = ptr;
			return ret;
		}

	}

	return ret;
}

void* getInterface2(void* factory, char* name)
{
	CreateInterfaceFn fn = (CreateInterfaceFn)factory;

	void *ret = 0;
	void *ptr = (fn)(name, 0);
	if (ptr)
	{

		ret = ptr;
		return ret;
	}


	return ret;
}

inline bool _readptrf(void* p)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQuery(p, &mbi, sizeof(mbi)))
		return true;

#define	chk(dst, flag)	((dst) &  (flag))

	bool good =
		chk(mbi.Protect, PAGE_READONLY) ||
		chk(mbi.Protect, PAGE_READWRITE) ||
		chk(mbi.Protect, PAGE_WRITECOPY) ||
		chk(mbi.Protect, PAGE_EXECUTE_READ) ||
		chk(mbi.Protect, PAGE_EXECUTE_READWRITE) ||
		chk(mbi.Protect, PAGE_EXECUTE_WRITECOPY);

	if (chk(mbi.Protect, PAGE_GUARD) || chk(mbi.Protect, PAGE_NOACCESS))
		good = false;

	return !good;
}

int vtablecount(void* ptr)
{
	int num = 0;

	if (!ptr)
		return 0;

	if (_readptrf(ptr))
		return 0;

	void** vtbl = *(void***)ptr;

	if (_readptrf(vtbl))
		return 0;

	while (vtbl[num])
	{
		if (num > 900)
			return 0;

		num++;
	}


	return num;
}


#ifndef _CSGO

void(*SpewOutputFunc)(void* func);
void*(*GetSpewOutputFunc)();
const void* (*GetSpewOutputColor)();
#else
void(*LoggingSystem_PushLoggingState)(bool bThreadLocal, bool bClearState);
void(*LoggingSystem_RegisterLoggingListener)(void* listener);

#endif

void(*Msg)(const char *, ...);
float(*Plat_FloatTime)();

void(*RandomSeed)(int);
float(*RandomFloat)(float, float);

std::string m_sdk::Init()
{

	while (!module_engine || !module_client)
	{
		module_engine = GetModuleHandleA("engine.dll");
		module_client = GetModuleHandleA("client.dll");
		module_vstd = GetModuleHandleA("vstdlib.dll");

		Sleep(100);
	}
	
	*(void**)&Msg = GetProcAddress(GetModuleHandleA("tier0.dll"), "Msg");
	Msg("[Nikyuria] Initializing...\n");

	if (!Msg)
		return "No Msg!\n";

	while (!factory_engine || !factory_client)
	{
		factory_engine = (CreateInterfaceFn)GetProcAddress(module_engine, "CreateInterface");
		factory_client = (CreateInterfaceFn)GetProcAddress(module_client, "CreateInterface");
		factory_vstd = (CreateInterfaceFn)GetProcAddress(module_vstd, "CreateInterface");
#ifdef _GMOD
		factory_luashared = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("lua_shared.dll"), "CreateInterface");
#endif
		Sleep(10);
	}

#ifdef _GMOD
	vluashared = getInterface(factory_luashared, "LUASHARED");

	if (!vluashared)
		return "No VLuaShared!\n";
#endif

	vengine = getInterface(factory_engine, "VEngineClient");
	
	if (!vengine)
		return "No VEngineClient!\n";

	vclient = getInterface(factory_client, "VClient");

	if (!vclient)
		return "No VClient!\n";
	
	ventlist = getInterface(factory_client, "VClientEntityList");

	if (!ventlist)
		return "No VEntList!\n";

	vmodelinfo = getInterface(factory_engine, "VModelInfoClient");

	if (!vmodelinfo)
		return "No VModelInfo!\n";

	venginetrace = getInterface(factory_engine, "EngineTraceClient");

	if (!venginetrace)
		return "No VEngineTrace!\n";

	vprediction = getInterface(factory_client, "VClientPrediction");

	if (!vprediction)
		return "No VPrediction!\n";

	vmovement = getInterface(factory_client, "GameMovement");

	if (!vmovement)
		return "No VMovement!\n";


	g_pEngine = new IEngine(vengine);
	g_pClient = new IClient(vclient);
	g_pEntList= new IEntList(ventlist);
	g_pModelInfo = new IModelInfo(vmodelinfo);
	g_pEngineTrace = new IEngineTrace(venginetrace);
	g_pPrediction = new IPrediction(vprediction);

	Msg("\n\n");
	Msg("g_pEngine indexes: %i\n", vtablecount(g_pEngine->vmt));
	Msg("g_pClient indexes: %i\n", vtablecount(g_pClient->vmt));
	Msg("g_pEntList indexes: %i\n", vtablecount(g_pEntList->vmt));
	Msg("g_pModelInfo indexes: %i\n", vtablecount(g_pModelInfo->vmt));
	Msg("g_pEngineTrace indexes: %i\n", vtablecount(g_pEngineTrace->vmt));
	Msg("g_pPrediction indexes: %i\n", vtablecount(g_pPrediction->vmt));
	
#ifdef _GMOD
	g_pLuaShared = new ILuaShared(vluashared);
	Msg("g_pLuaShared indexes: %i\n", vtablecount(g_pLuaShared->vmt));
#endif

	Msg("\n");

#ifndef _CSGO
	*(void**)&SpewOutputFunc = GetProcAddress(GetModuleHandleA("tier0.dll"), "SpewOutputFunc");
	*(void**)&GetSpewOutputFunc = GetProcAddress(GetModuleHandleA("tier0.dll"), "GetSpewOutputFunc");
	*(void**)&GetSpewOutputColor = GetProcAddress(GetModuleHandleA("tier0.dll"), "GetSpewOutputColor");
#else
	*(void**)&LoggingSystem_PushLoggingState = GetProcAddress(GetModuleHandleA("tier0.dll"), "LoggingSystem_PushLoggingState");
	*(void**)&LoggingSystem_RegisterLoggingListener = GetProcAddress(GetModuleHandleA("tier0.dll"), "LoggingSystem_RegisterLoggingListener");

#endif

	*(void**)&Plat_FloatTime = GetProcAddress(GetModuleHandleA("tier0.dll"), "Plat_FloatTime");

	*(void**)&RandomSeed = GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed");
	*(void**)&RandomFloat = GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat");

#ifndef _CSGO

	if (!SpewOutputFunc)
		return "No SpewOutputFunc!\n";

	if (!GetSpewOutputFunc)
		return "No GetSpewOutputFunc!\n";

	if (!GetSpewOutputColor)
		return "No GetSpewOutputColor!\n";

#else
	if (!LoggingSystem_PushLoggingState)
		return "No LoggingSystem_PushLoggingState!\n";

	if (!LoggingSystem_RegisterLoggingListener)
		return "No LoggingSystem_PushLoggingState!\n";

#endif

	if(!RandomSeed)
		return "No RandomSeed!\n";

	if(!RandomFloat)
		return "No RandomFloat!\n";

	if (!Plat_FloatTime)
		return "No Plat_FloatTime!\n";

	void*codecdll1 = (void*)LoadLibraryA("C://Nikyuria//dll//vaudio_silk.dll");

	if (!codecdll1)
		return "No encode codec silk!\n";

	void* factory_audio1 = (CreateInterfaceFn)GetProcAddress((HMODULE)codecdll1, "CreateInterface");
	if (!factory_audio1)
		return "No audio factory silk!\n";

	vencodecodec = (IVoiceCodec*)getInterface2(factory_audio1, "vaudio_silk");

	if (!vencodecodec)
		return "No encode codec silk!\n";

	if (!vencodecodec->Init(4))
		return "Couldn't initialize codec silk!\n";


#ifdef _CSGO

	void *oglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod(vclient, 0), "\xA3????\xE8",0x500, 1);

	if (!oglobals)
		return "no g_pGlobals!";

	g_pGlobals = **(CGlobalVars***)(oglobals);
	return "";
#endif


#ifdef _CSS
	void *oglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod(vclient, 0), "\xA3", 0x300, 1);

	if (!oglobals)
		return "no g_pGlobals!";

	g_pGlobals = **(CGlobalVars***)(oglobals);
	return "";
#endif

	void *vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod(vclient, 0), "\x89\x0D", 0x300, 2);

	if (!vglobals)
		vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod(vclient, 0), "\x51\xA3", 0x300, 2);

	if (!vglobals)
		vglobals = utils::memory::FindSig(utils::memory::GetVirtualMethod(vclient, 0), "\x08\xA3", 0x300, 2);


	if (!vglobals)
	{
		return "no g_pGlobals!";
	}

	g_pGlobals = **(CGlobalVars***)(vglobals);
		
	Msg("\n");


	return "";
}

m_sdk *sdk;
IEngine *g_pEngine = NULL;
IClient *g_pClient = NULL;
IEntList *g_pEntList = NULL;
IModelInfo *g_pModelInfo = NULL;
IEngineTrace *g_pEngineTrace = NULL;
IPrediction *g_pPrediction = NULL;
#ifdef _GMOD
ILuaShared *g_pLuaShared = NULL;
#endif
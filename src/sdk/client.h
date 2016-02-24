#pragma once

#include "interface.h"
#include "vector.h"
#include "clientclass.h"

// ======================= INDEXES =========================== //

#define INDEX_CLIENT_GETALLCLASSES 8
#define INDEX_CLIENT_FRAMESTAGENOTIFY 35
#define INDEX_CLIENT_DISPATCHUSERMESSAGE 36
#define INDEX_CLIENT_LEVELINITPREENTITY 5

#ifdef _CSGO
#undef INDEX_CLIENT_FRAMESTAGENOTIFY
#define INDEX_CLIENT_FRAMESTAGENOTIFY 36
#endif

#ifdef _L4D2
#undef INDEX_CLIENT_GETALLCLASSES
#undef INDEX_CLIENT_FRAMESTAGENOTIFY

#define INDEX_CLIENT_GETALLCLASSES 7
#define INDEX_CLIENT_FRAMESTAGENOTIFY 34
#endif

// ======================= CLASS HEADER ======================= //
class bf_read;

SETUP_INTERFACE(IClient);
	
	SetupInterfaceFunc(LevelInitPreEntity, void, INDEX_CLIENT_LEVELINITPREENTITY, (const char*mapname), (void* __this, const char*mapname), (this->vmt, mapname));

	SetupInterfaceFunc(GetAllClasses, ClientClass *, INDEX_CLIENT_GETALLCLASSES, (), (void* __this), (this->vmt));
	
	SetupInterfaceFunc(FrameStageNotify, void, INDEX_CLIENT_FRAMESTAGENOTIFY, (int curStage), (void* __this, int curStage), (this->vmt, curStage));

	SetupInterfaceFunc(DispatchUserMessage, bool, INDEX_CLIENT_DISPATCHUSERMESSAGE, (int a, bf_read &msg_data), (void* __this, int a, bf_read& msg_data), (this->vmt, a, msg_data));
	
	
};

extern IClient *g_pClient;


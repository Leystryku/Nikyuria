#pragma once

#include "interface.h"
#include "vector.h"
#include "player_info.h"

// ======================= INDEXES =========================== //

#define INDEX_ENGINE_GETSCREENSIZE				5
#define INDEX_ENGINE_SERVERCMD					6
#define INDEX_ENGINE_CLIENTCMD					106//7
#define INDEX_ENGINE_GETPLAYERINFO				8
#define INDEX_ENGINE_GETPLAYERFORUSERID			9
#define INDEX_ENGINE_GETLOCALPLAYER				12
#define INDEX_ENGINE_GETVIEWANGLES				19
#define INDEX_ENGINE_SETVIEWANGLES				20
#define INDEX_ENGINE_ISINGAME						26
#define INDEX_ENGINE_ISCONNECTED					27
#define INDEX_ENGINE_ISDRAWINGLOADINGIMAGE		28
#define INDEX_ENGINE_WORLDTOSCREENMATRIX			36
#define INDEX_ENGINE_GETNETCHANNELINFO			72


// ======================= CLASS HEADER ======================= //

class INetChannel;

SETUP_INTERFACE(IEngine);


	SetupInterfaceFunc(GetScreenSize, void, INDEX_ENGINE_GETSCREENSIZE, (int& width, int& height), (void* __this, int& width, int& height), (this->vmt, width, height));

	SetupInterfaceFunc(ServerCmd, void, INDEX_ENGINE_SERVERCMD, (const char *torun, bool reliable = false), (void* __this, const char *torun, bool reliable), (this->vmt, torun, reliable));

	SetupInterfaceFunc(ClientCmd, void, INDEX_ENGINE_CLIENTCMD, (const char*torun), (void* __this, const char *torun), (this->vmt, torun));

	SetupInterfaceFunc(GetPlayerInfo, bool, INDEX_ENGINE_GETPLAYERINFO, (int ent_num, player_info *pinfo), (void* __this, int ent_num, player_info *pinfo), (this->vmt, ent_num, pinfo));

	SetupInterfaceFunc(GetPlayerForUserID, int, INDEX_ENGINE_GETPLAYERFORUSERID, (int userID), (void* __this, int userID), (this->vmt, userID));

	SetupInterfaceFunc(GetLocalPlayer, int, INDEX_ENGINE_GETLOCALPLAYER, (), (void* __this), (this->vmt));

	SetupInterfaceFunc(GetViewAngles, void, INDEX_ENGINE_GETVIEWANGLES, (QAngle& va), (void* __this, QAngle &va), (this->vmt, va));

	SetupInterfaceFunc(SetViewAngles, void, INDEX_ENGINE_SETVIEWANGLES, (QAngle& va), (void* __this, QAngle &va), (this->vmt, va));

	SetupInterfaceFunc(IsInGameR, bool, INDEX_ENGINE_ISINGAME, (), (void* __this), (this->vmt));

	SetupInterfaceFunc(IsConnected, bool, INDEX_ENGINE_ISCONNECTED, (), (void* __this), (this->vmt));

	SetupInterfaceFunc(IsDrawingLoadingImage, bool, INDEX_ENGINE_ISDRAWINGLOADINGIMAGE, (), (void* __this), (this->vmt));

	SetupInterfaceFunc(WorldToScreenMatrix, const matrix4x4&, INDEX_ENGINE_WORLDTOSCREENMATRIX, (), (void* __this), (this->vmt));
	SetupInterfaceFunc(GetNetChannelInfo, INetChannel *, INDEX_ENGINE_GETNETCHANNELINFO, (), (void* __this), (this->vmt));

	//INetChannel *GetNetChannelInfo();
	//INetChannel *cachedChan;

	bool IsInGame();

};

extern IEngine *g_pEngine;
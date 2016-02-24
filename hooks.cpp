#pragma once

#include "hooks.h"
#include "process_usercmd.h"

#define getebp() stack *_bp; __asm mov _bp, ebp;

struct stack
{
	stack *next;
	char  *ret;

	template<typename T> inline T arg(unsigned int i)
	{
		return *(T *)((void **)this + i + 2);
	}
};

m_hooks *hooks;

#if defined(_GENERIC) || defined(_GMOD) || defined(_TF2)
extern Vector currentSpread;

typedef void(__fastcall* OrigFireBullets)(void* thisptr, int edx, DWORD fb);
OrigFireBullets FireBulletsFn;

void __fastcall hooked_FireBullets(IEntity* thisptr, int edx, DWORD fb)
{
	if (g_pLocalEnt)
	{
		if (thisptr == g_pLocalEnt)
		{
			currentSpread = *(Vector*)(fb + 0x1C);
			//Msg("CALLED FIREBULLETS: %f - %f - %f\n", currentSpread.x, currentSpread.y, currentSpread.z);
		}
	}


	FireBulletsFn(thisptr, edx, fb);
}



#endif

#ifdef _GMOD

void*iclientmode = 0;
float lastsampletime = 0;
typedef bool(__fastcall *OrigCreateMove)(void*thisptr, int edx, float flInputSampleTime, CUserCmd *cmd);
OrigCreateMove CreateMoveFn;
#endif

typedef int(__fastcall* OrigSendNetMsg)(void* thisptr, int edx, INetMessage &msg, bool bForceReliable, bool bVoice);
OrigSendNetMsg SendNetMsgFn;

typedef void(__stdcall *OrigUpdateMessageStats)(int group, int bits);
OrigUpdateMessageStats UpdateMessageStatsFn;

typedef Vector&(__fastcall* OrigGetAbsOrigin)(void* thisptr, int edx);
OrigGetAbsOrigin GetAbsOriginFn;

typedef void(__fastcall* OrigCalcView)(void* thisptr, int edx, Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
OrigCalcView CalcViewFn;

Vector currentSpecPos(0, 0, 0);

Vector& __fastcall hooked_GetAbsOrigin(void*thisptr, int edx)
{
	if (currentSpecPos.IsZero())
		currentSpecPos = GetAbsOriginFn(thisptr, edx);

	if (menu->misc_spechack)
		return currentSpecPos;

	return GetAbsOriginFn(thisptr, edx);
}

int SpeedCmds = 0;

static bool in_speed = false;
int lastcmdnum = 0;

bool commands[512];
void* hooked_CBaseEntity = NULL;
void* hooked_NetChan = NULL;

IEntity *cacheEnt = NULL;

void* right_SetViewAngles = NULL;

//msgtypes::https://code.google.com/p/christopherthorne/source/browse/trunk/gm_sourcenet3/gm_sourcenet3/common/protocol.h?r=10


int __fastcall hooked_SendNetMsg(void* thisptr, int edx, INetMessage &msg, bool bForceReliable, bool bVoice)
{
	const char *name = msg.GetName();

	

	if (name)
	{
#ifdef _GMOD
		if (menu->misc_block_net==1 || menu->misc_block_net == 3)
		{
			if (strstr(name, "clc_GMod_ClientToServer"))
			{
				Msg("Blocked outgoing net traffic\n");
				return 1;
			}
	
		}
#endif
		if (!strstr(name, "clc_Move") && !strstr(name, "net_Tick") && !strstr(name, "clc_BaselineAck") && !strstr(name, "clc_GMod_ClientToServer") && !strstr(name, "svc_GModServerToClient") && !strstr(name, "clc_VoiceData"))
			Msg("Sending:: %s ==== %s\n", name, msg.ToString());
#ifdef _GMOD
		if (menu->misc_block_net == 4 && ( strstr(name, "clc_GMod_ClientToServer") || strstr(name, "svc_GModServerToClient") ) )
			Msg("Sending:: %s ==== %s\n", name, msg.ToString());
#endif
		if (strstr(name, "clc_VoiceData"))
		{
			msg.IsReliable();

			bForceReliable = false;
			bVoice = true;
		}
		
	}

	return SendNetMsgFn(thisptr, edx, msg, bForceReliable, bVoice);
}

bool ProcessMessages(INetChannel*chan, bf_read &buf)
{

	buf.Seek(0);

	int times = 0;
	while (true)
	{
		times++;
		if (times > 100)
			break;

		// Are we at the end?
		if (buf.GetNumBitsLeft() < NETMSG_TYPE_BITS)
		{
			break;
		}

		unsigned char cmd = buf.ReadUBitLong(NETMSG_TYPE_BITS);

		Msg("cmd: %i\n", (int)cmd);
		if (cmd == 15)//svc_VoiceData=15,clc_VoiceData=10
		{
			
			/*
			int cl = buf.ReadByte();
			*/
		}
		return true;
	}

	return true;
}



bool ProcessPacket(INetMessage *netmsg)
{
	const char *name = netmsg->GetName();

	//Msg("%s lol\n", name);
#ifdef _GMOD
	if (menu->misc_block_net == 4 && (strstr(name, "clc_GMod_ClientToServer") || strstr(name, "svc_GModServerToClient")))
		Msg("Sending:: %s ==== %s\n", name, netmsg->ToString());

	if (menu->misc_block_net == 2 || menu->misc_block_net == 3)
	{
		if (strstr(name, "svc_GMod_ServerToClient"))
		{
			Msg("Blocked incoming net traffic\n");
			KillNetMessage(netmsg);
			return false;
		}

	}
#endif

	if (strstr(name, "net_StringCmd"))
	{

		Msg("Receiving stringcmd:: %s\n", netmsg->ToString());

		//KillNetMessage(netmsg);

	}

	if (strstr(name, "svc_VoiceData"))
	{
		if (!menu->misc_voice_imitator || !g_pLocalEnt)
			return true;

		SVC_VoiceData *msg = (SVC_VoiceData*)netmsg;

		//2048= max voice length, 3 bytes more 4 client, proximity, bits

		int client = msg->m_nFromClient;

		if (client == g_pLocalEnt->EntIndex() - 1)
			return true;

		int start = msg->m_DataIn.m_iCurBit;
		int proximity = msg->m_bProximity;
		int bits = msg->m_nLength;
		int bytes = bits / 8;
		Msg("PROXIMITEE %i; bits %i\n", proximity, bits);
		
		//write replay
		char voiceDataBuffer[4096];
		msg->m_DataIn.ReadBits(voiceDataBuffer, bits);

		CLC_VoiceData voiceMsg;
		voiceMsg.m_DataOut.StartWriting(voiceDataBuffer, sizeof(voiceDataBuffer));
		voiceMsg.m_DataOut.SeekToBit(bits);
		voiceMsg.m_nLength = bits;
		

		Msg("WRITTENDATA: %s:::\n", voiceMsg.ToString());
		char max[4000];

		bf_write data(max, sizeof(max));
		voiceMsg.WriteToBuffer(data);

		g_pEngine->GetNetChannelInfo()->SendData(data,false);

		msg->m_DataIn.m_iCurBit = start;
	}

	if (strstr(name, "svc_UserMessage"))
	{

		SVC_UserMessage *msg = (SVC_UserMessage*)netmsg;
		int start = msg->m_DataIn.m_iCurBit;
		int maxbits = msg->m_DataIn.GetNumBitsLeft();
		int maxbytes = msg->m_DataIn.GetNumBytesLeft();

		byte userdata[MAX_USER_MSG_DATA] = { 0 };
		bf_read userMsg("UserMessage(read)", userdata, sizeof(userdata));
		msg->m_DataIn.ReadBits(userdata, msg->m_nLength);
		userMsg.StartReading(userdata, Bits2Bytes(msg->m_nLength));


		int type = msg->m_nMsgType;

		if (type == 40)
		{
			char readstr[MAX_USER_MSG_DATA] = { 0 };
			readstr[0] = 0;
			userMsg.ReadBits(readstr, MAX_USER_MSG_DATA);
			Msg("SendLua: %s\n", readstr);
		}

		//îMsg("TYPE: %i\n", type);

		if (type == 3)
		{

			int client = userMsg.ReadByte();
			int bWantsChat = userMsg.ReadByte();
			char readstr[MAX_USER_MSG_DATA] = { 0 };
			userMsg.ReadString(readstr, sizeof(readstr));

			int something3 = userMsg.ReadByte(); //idk, sometimes 1, might be bchat
			int something4 = userMsg.ReadByte(); //seems to be 1 when teamchatting
			int something5 = userMsg.ReadByte(); //idk, sometimes 1



			if (client && strstr(readstr, "\n"))
			{

				msg->m_DataIn.m_iCurBit = start;

				char writebuf[MAX_USER_MSG_DATA];
				bf_write penorwrites(writebuf, sizeof(writebuf));
				penorwrites.StartWriting(writebuf, Bits2Bytes(msg->m_nLength));

				for (int i = 0; i < maxbytes; i++)
				{
					int curbyte = userMsg.ReadByte();
					if (curbyte != '\n')
						penorwrites.WriteByte(curbyte);
					else
						penorwrites.WriteByte('_');

				}

				penorwrites.m_iCurBit = 0;

				bf_read penorreads(penorwrites.GetBasePointer(), penorwrites.GetNumBytesWritten());

				msg->ReadFromBuffer(penorreads);
			}

			if (menu->misc_text_imitator)
			{
				std::string tosay = "say";
				if (menu->misc_text_team)
					tosay.append("_team");

				tosay.append(" ");
				tosay.append(readstr);

				g_pEngine->GetNetChannelInfo()->StringCommand(tosay.c_str());

			}

		}

		msg->m_DataIn.m_iCurBit = start;

	}

	return true;
}



extern int vtablecount(void* ptr);
extern inline bool _readptrf(void* p);

void*hooked_UpdateMessageStats_stringref = 0;

char*msgstats_ebps[10];

int msgstats_offsets[10];
int msgstats_offsetcounter = 0;


void*hooked_UpdateMessageStats_retaddr = 0;
void __stdcall hooked_UpdateMessageStats(int group, int bits)
{

	INetMessage*netmsg = 0;
	char*vebp = 0;

	__asm mov vebp, ebp;

	if (!hooked_UpdateMessageStats_retaddr&&group == 0)
		hooked_UpdateMessageStats_retaddr = _ReturnAddress();


	UpdateMessageStatsFn(group, bits);

	if (hooked_UpdateMessageStats_retaddr != _ReturnAddress())
		return;

	int myoffset = 0;

	if (msgstats_offsetcounter)
	{

		for (int i = 1; i < msgstats_offsetcounter; i++)
		{
			if (msgstats_ebps[i] == vebp)
			{
				myoffset = msgstats_offsets[i];
				break;
			}
		}

	}
	
	static char* strref = (char*)utils::memory::FindString(GetModuleHandleA("engine.dll"), "net_Tick"); //net_Tick


	if (!myoffset )
	{

		//14*4->16*4

		for (int c = 14; c <19; c++)
		{
			int i = c * 4;

			char* rptr = (vebp - i);
			if (!rptr)
				continue;

			if (_readptrf(rptr))
				continue;
			
			void**vtbl = 0;

			try {
				char*ptr = *(char**)rptr;

				if (_readptrf(ptr))
					continue;

				vtbl = *(void***)ptr;

				if (_readptrf(vtbl))
					continue;
			}
			catch (...)
			{
			}

			if (!vtbl)
				return;

			int found_string = 0;


			for (int x = 60; 0<x; x--)
			{
				try{

					if (!vtbl[x])
						continue;

				}
				catch (...)
				{
					continue;
				}

				int dist = (int)strref - (int)vtbl[x];

				if (1 > dist)
					continue;

				if (dist > 200)
					continue;

				bool fail = true;
				try
				{
					INetMessage *testmsg = *(INetMessage**)rptr;
					
					if (testmsg->GetName() && testmsg->GetGroup() == group)
						fail = false;

				}
				catch (...)
				{
				}

				if (fail)
					continue;

				//Msg("SIZE: %i\n", utils::memory::FindSubSize(vtbl[x]));

				found_string = x;

				break;

			}






			if (found_string)
			{
				msgstats_offsetcounter += 1;
				msgstats_ebps[msgstats_offsetcounter] = vebp;
				msgstats_offsets[msgstats_offsetcounter] = i; 
				myoffset = i;
				Msg("%i :: rightptr == %i\n", i, found_string);
			}


		}
	}

	if (!myoffset)
	{
		//Msg("Didn't find right one!\n");

		return;
	}


	netmsg = *(INetMessage**)(vebp - myoffset);

	if (ProcessPacket(netmsg))
	{

 
	}
}



#ifdef _GMOD
bool __fastcall hooked_CreateMove(void*thisptr, int edx, float flInputSampleTime, CUserCmd *cmd)
{

	iclientmode = thisptr;
	lastsampletime = flInputSampleTime;

	if (right_SetViewAngles)
		return true;


	return CreateMoveFn(thisptr, 0, flInputSampleTime, cmd);
}
#endif

void hookBaseEnt()
{
	hooked_CBaseEntity = cacheEnt;
	currentSpecPos.Zero();

	VMTHook *C_baseEntHook = new VMTHook(cacheEnt);
	if (!C_baseEntHook)
		return;

	GetAbsOriginFn = (OrigGetAbsOrigin)C_baseEntHook->hookFunction(IENTITY__GETABSORIGIN, hooked_GetAbsOrigin);

	currentSpecPos.Zero();

#if defined(_GENERIC) || defined(_GMOD) || defined(_TF2)
	FireBulletsFn = (OrigFireBullets)C_baseEntHook->hookFunction(16, hooked_FireBullets);
#endif
	hooked_CBaseEntity = g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());
	cacheEnt = (IEntity*)hooked_CBaseEntity;

}

void __stdcall hooked_SetViewAngles(QAngle &va)
{
	getebp();

	CUserCmd* pCmd;
	__asm mov pCmd, esi

	DWORD sebp;
	__asm mov sebp, ebp

	int *sequence_number = *(int**)(sebp)+0x2;


	if (hooked_NetChan != g_pEngine->GetNetChannelInfoR())
	{
		g_pEngine->cachedChan = g_pEngine->GetNetChannelInfoR();

		

		VMTHook *INetChanHook = new VMTHook(g_pEngine->cachedChan);
		hooked_NetChan = g_pEngine->cachedChan;
		Msg("Hooked netchan!\n");
		SendNetMsgFn = (OrigSendNetMsg)INetChanHook->hookFunction(40, hooked_SendNetMsg);//36=shutdown, 39=processpacket, 40=sendnetmsg, 41=senddata?, 46=senddatagram, 55=UpdateMessageStats?
		UpdateMessageStatsFn = (OrigUpdateMessageStats)INetChanHook->hookFunction(55, hooked_UpdateMessageStats);

	}

	if (!right_SetViewAngles)
	{
		

		if (pCmd->command_number != *sequence_number || pCmd->command_number < 0x12C || pCmd->command_number > 0xF0000 || !g_pEngine->IsInGame())
		{
			g_pEngine->SetViewAngles(va);
			return;
		}

		right_SetViewAngles = _ReturnAddress();
		g_pEngine->SetViewAngles(va);
		return;
	}

	if (_ReturnAddress() != right_SetViewAngles)
	{
#ifndef _CSGO
		if (pCmd->hasbeenpredicted && menu->wep_norecoil)
			return;
#endif

		g_pEngine->SetViewAngles(va);
		return;
	}



	

	cacheEnt = g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());
	if (!cacheEnt)
		return;

	if (hooked_CBaseEntity != cacheEnt)
	{
#ifdef _GMOD 
		CreateMoveFn(iclientmode, 0, lastsampletime, pCmd);
#endif

		hookBaseEnt();
		return;
	}

	g_pLocalEnt = cacheEnt;

	g_pEngine->SetViewAngles(va);

#ifdef _CSGO
	bool *bSendPacket = ((bool *)_bp->next - 0x1c);
#else
	bool *bSendPacket = (bool*)_bp->next->next->next - 1;
#endif
	
	if (menu->misc_spechack)
	{

		Vector forward;
		Vector right;
		Vector up;

		AngleVectors(pCmd->viewangles, &forward, &right, &up);

		float speed = 5;

		if (pCmd->buttons&IN_SPEED)
		{
			speed *= 2.5;
		}

		if (pCmd->buttons&IN_DUCK)
		{
			speed *= 0.5;
		}

		if (pCmd->buttons&IN_FORWARD)
		{
			currentSpecPos = currentSpecPos + (forward*speed);
		}
		if (pCmd->buttons&IN_BACK)
		{
			currentSpecPos = currentSpecPos - (forward*speed);
		}

		if (pCmd->move.y>0)
		{
			currentSpecPos = currentSpecPos + (right*speed);
		}

		if (pCmd->move.y<0)
		{
			currentSpecPos = currentSpecPos - (right*speed);
		}

		if (pCmd->buttons&IN_JUMP)
		{
			currentSpecPos = currentSpecPos + Vector(0, 0, speed);
		}

		pCmd->buttons &= ~IN_JUMP;
		pCmd->buttons &= ~IN_DUCK;
		pCmd->buttons &= ~IN_BACK;
		pCmd->buttons &= ~IN_FORWARD;
		pCmd->buttons &= ~IN_MOVELEFT;
		pCmd->buttons &= ~IN_MOVERIGHT;

		pCmd->move.Zero();

	}else{
		currentSpecPos.Zero();
	}

	ProcessUserCommand(pCmd, bSendPacket);
	*sequence_number = pCmd->command_number;

#ifdef _GMOD 
	CreateMoveFn(iclientmode, 0, lastsampletime, pCmd);
#endif

	if (SpeedCmds > 0)
	{
		commands[pCmd->command_number % sizeof(commands)] = true;
		SpeedCmds -= 1;
		_bp->next->next->next->next->ret -= 5;//1 + sizeof(void *);	
		in_speed = false;
	}else{
		commands[pCmd->command_number % sizeof(commands)] = false;
		if (GetAsyncKeyState(VK_MBUTTON))
			SpeedCmds = menu->misc_speedhack;
	}

}

//UNDEFINED-1, START0, UPDATESTART1, POSTDATAUPDATESTART2, POSTDATAUPDATEEND3, UPDATEEND4, RENDER5, RENDEREND6

const char *currentCode = "";

QAngle bbakPunch;
QAngle bakPunch;

int count = 0;

extern mstudiohitboxset* ientity__hboxsets[0xFFFFFFF];
extern matrix3x4 ientity__matrixs[0xFFFF][IENTITY__BONEARRAY];

void __stdcall hooked_FrameStageNotify(int curStage)
{
#ifdef _GMOD

	if (strlen(currentCode) > 0)
	{
	
		void*state = g_pLuaShared->GetLuaInterface(2);

		if (g_pEngine->IsInGame())
			state = g_pLuaShared->GetLuaInterface(0);

		if (!g_pLuaShared->RunCode(state, currentCode, "LuaCmd"))
			Msg("An error happened while execution !\n");

		currentCode = "";

		Msg("Done!\n");

	}

#endif

	if (!g_pEngine->IsInGame())
	{
		d3d->canrender = true;
		if (right_SetViewAngles)
		{
			memset(commands, 0, 512 * sizeof(bool));
			g_pLocalEnt = 0;
			hooked_CBaseEntity = 0;
			right_SetViewAngles = 0;
			hooked_UpdateMessageStats_retaddr = 0;
			msgstats_offsetcounter = 0;
			memset(msgstats_offsets, 0, sizeof(msgstats_offsets));
			memset(msgstats_ebps, 0, sizeof(msgstats_ebps));
			memset(ientity__hboxsets, 0, sizeof(ientity__hboxsets));
			memset(ientity__matrixs, 0, sizeof(ientity__matrixs));
		}
	}

	if (curStage == 5)
		d3d->canrender = true; sdk->ViewMatrix = g_pEngine->WorldToScreenMatrix();
	
	g_pClient->FrameStageNotify(curStage);


	


}
void AntiAntiAimProxyX(const CRecvProxyData *pData, IEntity *ent, void *pOut)
{
	float flPitch = pData->m_Value.m_Float;

	if (!menu->misc_antiaimproxy)
	{
		*(float*)pOut = flPitch;
		return;
	}

	Msg("PITCH: %f\n", flPitch);
	
	static bool lookedDownLastTick[64];


	if (flPitch > 180)
	{
		flPitch -= 360;
	}

	flPitch = utils::maths::clamp<float, float>(flPitch, -90, 90);

	bool tmp = lookedDownLastTick[ent->EntIndex()];
	lookedDownLastTick[ent->EntIndex()] = (flPitch == 90);

	if (flPitch == -90)
	{
		if (tmp)
		{
			flPitch = 90;
		}
	}

	*(float*)pOut = flPitch;

	/*
	static int lastfixed[200];
	int magicswitch = lastfixed[ent->EntIndex()];
	int needsfix = 0;

	if (flPitch >= 90)
		needsfix = 1;

	if (flPitch <= -90)
		needsfix = 2;

	if (lastfixed[ent->EntIndex()] == -89 && needsfix==1)
		needsfix = 2;

	if (lastfixed[ent->EntIndex()] == 89 && needsfix==2)
		needsfix = 1;

	if (needsfix == 1)
	{
		flPitch = -89;
		Msg("AA1\n");

		lastfixed[ent->EntIndex()] = -89;
		*(float*)pOut = flPitch;
		return;
	}

	if (needsfix == 2)
	{
		flPitch = 89;
		Msg("AA2\n");
		lastfixed[ent->EntIndex()] = 89;
		*(float*)pOut = flPitch;
		return;
	}

	lastfixed[ent->EntIndex()] = 0;*/
	*(float*)pOut = flPitch;
}

void AntiAntiAimProxyY(const CRecvProxyData *pData, IEntity *ent, void *pOut) //thanks paradoxxx/kamay
{

	if (!menu->misc_antiaimproxy)
	{
		*(float*)pOut = pData->m_Value.m_Float;
		return;
	}

	float flYaw = utils::maths::clamp<float, float>(pData->m_Value.m_Float, -181, 181);

	if (flYaw == -181)
	{
		flYaw = 179;
	}

	if (flYaw == 181)
	{
		flYaw = -179;
	}

	*(float*)pOut = flYaw;
}



/* HOOK DT_LocalPlaryer::m_nTickBase __ ( recvproxydata &data, void* t, recvproxyresult &out )
if(t==LocalPlayer())
{
	static int sim;
	out.i32 = data.value.i32 == sim ? (IEntity*t)->GetTickCount() + 1 : data.value.i32
	sim = data.value.i32;
}else{
	out.i32 = data.value.i32;
}
*/
void *g_pMoveHelper = NULL;


void __stdcall hooked_RunCommand(IEntity *ent, CUserCmd* pCmd, void* mv)
{
	if (!g_pMoveHelper)
	{
		g_pMoveHelper = mv;
	}

	void* thisptr = NULL;
	__asm mov thisptr, ecx

	if (!thisptr)
		thisptr = g_pPrediction->vmt;

	if (commands[pCmd->command_number % sizeof(commands)])
	{
		
		g_pPrediction->DoSmartPred(pCmd, mv, sdk->vmovement, g_pPrediction->vmt);
		return;
	}

	g_pPrediction->RunCommand(thisptr, ent, pCmd, mv);

	
}

bool __fastcall hooked_InPrediction(void*thisptr, int edx)
{
	
	bool ret = g_pPrediction->InPrediction(thisptr, edx);

	void*vesi;
	__asm mov vesi, esi;

	if (!g_pLocalEnt || g_pLocalEnt != vesi)
		return ret;

	void*cret = _ReturnAddress();

	int diff = utils::memory::FindSubSize(cret, 800);

	if (diff < 315 || diff > 370)
		return ret;

	if (!menu->wep_norecoil)
		return ret;

	count++;
	
	
	if (count==3)
	{
		g_pLocalEnt->GetViewPunch().x = bakPunch.x;
		g_pLocalEnt->GetViewPunch().y = bakPunch.y;
		g_pLocalEnt->GetViewPunch().z = bakPunch.z;

		bakPunch.Zero();
		count = 0;
	}

	if (count==2)
	{
		QAngle curPunch = g_pLocalEnt->GetViewPunch();
		bakPunch.x = curPunch.x;
		bakPunch.y = curPunch.y;
		bakPunch.z = curPunch.z;
		g_pLocalEnt->GetViewPunch().Zero();
	}

	return ret;
}


#if defined(_GENERIC) || defined (_TF2)
void *iinput;
__declspec(naked) void hooked_GetUserCmd(int sequence_number)
{

	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90
	__asm _emit 0x90

}
#endif

std::string m_hooks::HookStuff()
{

	Chat = new m_Chat;
	BunnyHop = new m_BunnyHop;
	Weapon = new m_Weapon;
	Bots = new m_Bots;
	HvH = new m_HvH;
	NameChanger = new m_NameChanger;

	VMTHook *engineHook = new VMTHook(g_pEngine->vmt);

	g_pEngine->vmt = &engineHook->m_pOriginalVTable;

	engineHook->hookFunction(INDEX_ENGINE_SETVIEWANGLES, hooked_SetViewAngles);


	VMTHook *clientHook = new VMTHook(g_pClient->vmt);
	g_pClient->vmt = &clientHook->m_pOriginalVTable;

	clientHook->hookFunction(INDEX_CLIENT_FRAMESTAGENOTIFY, hooked_FrameStageNotify);

#ifdef _GMOD
	char* ref = (char*)utils::memory::FindString(GetModuleHandleA("client.dll"), "scripts/vgui_screens.txt") - 17;
	//char* ref2 = (char*)utils::memory::FindSig(GetModuleHandleA("client.dll"), "\xA3????\xE8????\x8B\x10", 60000000);

	void**clientmode = *(void***)(ref + 4);
	//Msg("LOL %i LOL\n", ref - ref2);
	Msg("ClientMode at: %x\n", clientmode);

	VMTHook *clmodeHook = new VMTHook(clientmode);
	CreateMoveFn = (OrigCreateMove)clmodeHook->hookFunction(22, hooked_CreateMove);
#endif

	VMTHook *predHook = new VMTHook(g_pPrediction->vmt);
	g_pPrediction->vmt = &predHook->m_pOriginalVTable;

	predHook->hookFunction(INDEX__PRED_RUNCOMMAND, hooked_RunCommand);
	predHook->hookFunction(INDEX__PRED_INPREDICTION, hooked_InPrediction);


#ifdef _CSGO
	return "";
#endif

#ifdef _TF22
	
	iinput = **(void***)((char*)clientHook->GetMethod(16) + 0x2);


	if (!iinput)
		return "No VInput!";

	VMTHook *inputHook = new VMTHook(iinput);
	utils::file::WriteToLog("iinput indexes: %i\n", inputHook->tellCount());
	Msg("iinput indexes: %i\n", inputHook->tellCount());

	unsigned long oldflags = 0;
	VirtualProtect((void*)hooked_GetUserCmd, 0x120, 0x40, &oldflags);

	memcpy(hooked_GetUserCmd, inputHook->GetMethod(8), 0x64);

	void* topatch = utils::memory::FindSig(hooked_GetUserCmd, "\x0F", 0x120);
	
	memset(topatch, 0x90, 3);

	VirtualProtect((void*)hooked_GetUserCmd, 0x120, oldflags, &oldflags);



	inputHook->hookFunction(8, hooked_GetUserCmd);
#endif

	g_pNetworkMan->HookProp("*", "m_angEyeAngles[0]", AntiAntiAimProxyX);
	g_pNetworkMan->HookProp("*", "m_angEyeAngles[1]", AntiAntiAimProxyY);

	/*
	void* ptrCL_Move_chokedcommands = utils::memory::findsig((void*)GetModuleHandleA("engine.dll"), "\x8B\x15????\xA1????\x8D\x74\x0A\x01..");
	void* ptrCL_Move_lastoutgoing = utils::memory::findsig((void*)GetModuleHandleA("engine.dll"), "\x8B\x0D????\x8B\x15????\xA1????\x8D\x74\x0A\x01..");

	if (!ptrCL_Move_chokedcommands)
		Msg("didn't find ptrCL_Move_chokedcommands!\n");

	if (!ptrCL_Move_lastoutgoing)
		Msg("didn't find ptrCL_Move_lastoutgoing!\n");*/

	return "";
}
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
	if (g_pLocalEnt&&thisptr == g_pLocalEnt)
	{
		currentSpread = *(Vector*)(fb + 0x1C);
		//Msg("CALLED FIREBULLETS: %f - %f - %f\n", currentSpread.x, currentSpread.y, currentSpread.z);
	}


	FireBulletsFn(thisptr, edx, fb);
}



#endif

VMTHook *C_baseEntHook = 0;
VMTHook *NetChanHook = 0;

typedef int(__fastcall* OrigSendNetMsg)(void* thisptr, int edx, INetMessage &msg, bool bForceReliable, bool bVoice);
OrigSendNetMsg SendNetMsgFn;

typedef void(__stdcall *OrigUpdateMessageStats)(int group, int bits);
OrigUpdateMessageStats UpdateMessageStatsFn;

typedef Vector&(__fastcall* OrigGetAbsOrigin)(void* thisptr, int edx);
OrigGetAbsOrigin GetAbsOriginFn;

Vector currentSpecPos(0, 0, 0);

Vector& __fastcall hooked_GetAbsOrigin(void*thisptr, int edx)
{
	if (currentSpecPos.IsZero())
		currentSpecPos = GetAbsOriginFn(thisptr, edx);

	if (menu->misc_spechack)
		return currentSpecPos;

	return GetAbsOriginFn(thisptr, edx);
}

bool commands[1024];
bool hooked_CBaseEntity = false;
bool hooked_CBaseEntityA = false;
void* hooked_NetChan = NULL;


extern int dolel;
int servercount = 0;
int servercrc = 0;

int __fastcall hooked_SendNetMsg(void* thisptr, int edx, INetMessage &msg, bool bForceReliable, bool bVoice)
{
	const char *name = msg.GetName();

	if (name)
	{
#ifdef _GMOD
		if (menu->misc_block_net == 1 || menu->misc_block_net == 3)
		{
			if (strstr(name, "clc_GMod_ClientToServer"))
			{
				Msg("Blocked outgoing net traffic\n");
				return 1;
			}

		}
#endif
		if (!strstr(name, "clc_Move") && !strstr(name, "net_Tick") && !strstr(name, "clc_BaselineAck") && !strstr(name, "clc_GMod_ClientToServer") && !strstr(name, "svc_GModServerToClient") && !strstr(name, "clc_VoiceData") && !strstr(name, "net_StringCmd") && !strstr(name, "net_SetConVar"))
			Msg("Sending:: %s ==== %s\n", name, msg.ToString());
#ifdef _GMOD
		if (menu->misc_block_net == 4 && (strstr(name, "clc_GMod_ClientToServer") || strstr(name, "svc_GModServerToClient")))
			Msg("Sending:: %s ==== %s\n", name, msg.ToString());
#endif

	}

	return SendNetMsgFn(thisptr, edx, msg, bForceReliable, bVoice);
}

int cachecount = 0;

bool blockload = false;

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
			return false;
		}

	}
#endif

	if (strstr(name, "svc_FixAngle") || strstr(name, "svc_FixView") || strstr(name, "svc_SetView"))
	{
		if (menu->wep_norecoil)
			return false;
	}

	if (strstr(name, "svc_GetCvarValue"))
	{
		return false;
	}

	if (strstr(name, "net_StringCmd"))
	{
		const char *tostring = netmsg->ToString();
		Msg("Receiving stringcmd:: %s\n", tostring);

		//KillNetMessage(netmsg);

	}

	if (strstr(name, "net_SignonState"))
	{
		const char *tostring = netmsg->ToString();

		if (strstr(tostring, "state 7"))
			hooked_CBaseEntityA = false;

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
		int len = msg->m_nLength;

		char chReceived[4096];
		msg->m_DataIn.ReadBits(chReceived, len);

		char cbuf[4096];
		bf_write writebuf(cbuf, 4096);
		writebuf.WriteUBitLong(10, NET_MESSAGE_BITS);
		writebuf.WriteWord(len);
		writebuf.WriteBits(chReceived, len);


		g_pEngine->GetNetChannelInfo()->SendData(writebuf);

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
			char readstr[1024] = { 0 };
			readstr[0] = 0;
			userMsg.ReadBits(readstr, 1024);
			Msg("SendLua: %s\n", readstr);
		}

		//Msg("TYPE: %i\n", type);

		if (type == 3)
		{

			int client = userMsg.ReadByte();
			//int bWantsChat = userMsg.ReadByte();
			char readstr[MAX_USER_MSG_DATA] = { 0 };
			userMsg.ReadString(readstr, sizeof(readstr));

			int something3 = userMsg.ReadByte(); //idk, sometimes 1, might be bchat
			int something4 = userMsg.ReadByte(); //seems to be 1 when teamchatting
			int something5 = userMsg.ReadByte(); //idk, sometimes 1



			if (client && strstr(readstr, "\n"))
			{
				return false;
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
bool blocknetmsglog = false;

void __stdcall hooked_UpdateMessageStats(int group, int bits)
{

	if (!hooked_UpdateMessageStats_retaddr)
		hooked_UpdateMessageStats_retaddr = _ReturnAddress();

	UpdateMessageStatsFn(group, bits);

	char*vebp;
	char *vesi;
	__asm mov vesi, esi;
	__asm mov vebp, ebp;

#ifndef _GMOD



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


	if (!myoffset)
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
	

	INetMessage *netmsg = *(INetMessage**)(vebp - myoffset);
#else
	
	INetMessage *netmsg = (INetMessage*)vesi;
#endif


	if (netmsg && !ProcessPacket(netmsg))
	{
		blocknetmsglog = true;

		char**vebp = 0;
		__asm mov vebp, ebp;
		char*oldebp = *vebp;

		//Msg("CURBLOCK: %s\n", *(const char**)(oldebp - 12));
		*(const char**)(oldebp - 12) = netmsg->GetName();//"1";

		 //-12=none, -20 = net message name

	}

}

void hookBaseEnt()
{
	void* ent = g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());
	

	if (!ent)
		return;

	if (g_pEngine->IsDrawingLoadingImage())
		return;

	if (!g_pEngine->IsInGame())
		return;

	if (hooked_CBaseEntity)
		return;


	C_baseEntHook = new VMTHook(ent);
	
	GetAbsOriginFn = (OrigGetAbsOrigin)C_baseEntHook->hookFunction(IENTITY__GETABSORIGIN, hooked_GetAbsOrigin);

	currentSpecPos.Zero();

#if defined(_GENERIC) || defined(_GMOD) || defined(_TF2)
	FireBulletsFn = (OrigFireBullets)C_baseEntHook->hookFunction(16, hooked_FireBullets);

#endif

	hooked_CBaseEntity = true;

}


void __stdcall hooked_SetViewAngles(QAngle &va)
{
	getebp();

	CUserCmd* pCmd;
	__asm mov pCmd, esi

	unsigned long sebp;
	__asm mov sebp, ebp

	int *sequence_number = *(int**)(sebp)+0x2;
	



	static void* right_SetViewAngles = 0;
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
		if (!hooked_CBaseEntity)
		{
			hookBaseEnt();
			return;
		}

#ifndef _CSGO
		if (pCmd->hasbeenpredicted && menu->wep_norecoil)
			return;
#endif

		g_pEngine->SetViewAngles(va);
		return;
	}


	g_pLocalEnt = g_pEntList->GetClientEntity(g_pEngine->GetLocalPlayer());

	g_pEngine->SetViewAngles(va);

#ifdef _CSGO
	bool *bSendPacket = ((bool *)_bp->next - 0x1c);
#else
	bool *bSendPacket = (bool*)_bp->next->next->next - 1;
#endif

	if (!menu->misc_spechack)
	{
		currentSpecPos.Zero();
	}else{
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
		
	}

	ProcessUserCommand(pCmd, bSendPacket);
	*sequence_number = pCmd->command_number;

	static int speedCmds = 0;
	if (speedCmds > 0)
	{
		commands[pCmd->command_number % sizeof(commands)] = true;
		speedCmds -= 1;
		_bp->next->next->next->next->ret -= 5;
	}else{
		commands[pCmd->command_number % sizeof(commands)] = false;
		if (GetAsyncKeyState(VK_MBUTTON))
			speedCmds = menu->misc_speedhack;
	}

}

//UNDEFINED-1, START0, UPDATESTART1, POSTDATAUPDATESTART2, POSTDATAUPDATEEND3, UPDATEEND4, RENDER5, RENDEREND6

const char *currentCode = "";
extern mstudiohitboxset* ientity__hboxsets[0xFFFFFFF];
extern matrix3x4 ientity__matrixs[0xFFFF][IENTITY__BONEARRAY];

extern char *g_pMicInputFileData;
extern int g_nMicInputFileBytes;
extern int g_CurMicInputFileByte;
extern double g_MicStartTime;

//11025 = speex, 16000 = silk

#define VOICE_OUTPUT_SAMPLE_RATESILK			16000	// Sample rate that we feed to the mixer.
#define BYTES_PER_SAMPLE	2

WAVEFORMATEX g_VoiceSampleFormatSilk =
{
	WAVE_FORMAT_PCM,		// wFormatTag
	1,						// nChannels
	VOICE_OUTPUT_SAMPLE_RATESILK,					// nSamplesPerSec
	VOICE_OUTPUT_SAMPLE_RATESILK * 2,					// nAvgBytesPerSec
	2,						// nBlockAlign
	16,						// wBitsPerSample
	sizeof(WAVEFORMATEX)	// cbSize
};


int Voice_SamplesPerSecSilk()
{
	return g_VoiceSampleFormatSilk.nSamplesPerSec;
}

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
	}
	else{
		if (hooked_CBaseEntityA && !g_pEngine->IsDrawingLoadingImage())
		{
			hooked_CBaseEntityA = false;
			hooked_CBaseEntity = true;
		}
	}

	if (curStage == 5)
	{
		d3d->canrender = true;
		sdk->ViewMatrix = g_pEngine->WorldToScreenMatrix();

		bool send = false;

		static int sendwaits = 10;
		sendwaits--;

		if (dolel == 2 && sendwaits<1)
		{
			sendwaits = 10;
			char tempData[8192];
			int gotten = 0;

			if (g_pMicInputFileData)
			{
				double curtime = Plat_FloatTime();
				int nShouldGet = (curtime - g_MicStartTime) * Voice_SamplesPerSecSilk();

				gotten = min(sizeof(tempData) / BYTES_PER_SAMPLE, min(nShouldGet, (g_nMicInputFileBytes - g_CurMicInputFileByte) / BYTES_PER_SAMPLE));
				memcpy(tempData, &g_pMicInputFileData[g_CurMicInputFileByte], gotten*BYTES_PER_SAMPLE);
				g_CurMicInputFileByte += gotten * BYTES_PER_SAMPLE;
				g_MicStartTime = curtime;
			}

			static char cbuf[8192];
			static char voicedata[8192];
			memset(cbuf, 0, sizeof(cbuf));
			memset(voicedata, 0, sizeof(voicedata));

			bf_write writebuf(cbuf, 8192);
			writebuf.WriteUBitLong(10, NET_MESSAGE_BITS);

			IVoiceCodec *curcodec = sdk->vencodecodec;

			int nCompressedBytes = curcodec->Compress(tempData, gotten, voicedata, sizeof(voicedata), false);
			if (!nCompressedBytes)
				nCompressedBytes = curcodec->Compress(tempData, gotten, voicedata, sizeof(voicedata), true);

			writebuf.WriteWord(nCompressedBytes * 8);
			writebuf.WriteBits(voicedata, nCompressedBytes * 8);
		//	writebuf.SeekToBit(0);//nCompressedBytes * 8);

			Msg("[VOICE] Sending %i bytes!\n", nCompressedBytes);

			g_pEngine->GetNetChannelInfo()->SendData(writebuf);

		}
	}



	if (dolel == 1)
	{
		IVoiceCodec *curcodec = sdk->vencodecodec;
		curcodec->ResetState();

		int a, b, c;
		bool success = false;
		
		success = utils::file::ReadWaveFile("C://Nikyuria//musicinput.wav", g_pMicInputFileData, g_nMicInputFileBytes, a, b, c);
		

		if (!g_pMicInputFileData)
			success = false;

		if (!success)
			MessageBoxA(NULL, "failed reading wav", "k", MB_OK);

		g_CurMicInputFileByte = 0;
		g_MicStartTime = Plat_FloatTime();
		dolel = 2;
	}

	g_pClient->FrameStageNotify(curStage);


}

void AntiAntiAimProxyX(const CRecvProxyData *pData, IEntity *ent, void *pOut)
{
	float fl = pData->m_Value.m_Float;

	if (!menu->misc_antiaimproxy)
	{
		*(float*)pOut = fl;
		return;
	}

	if (fl > 180.0f)
		fl -= 360.0f;
	if (fl < -180.0f)
		fl += 360.0f;




	*(float*)pOut = fl;
}

void AntiAntiAimProxyY(const CRecvProxyData *pData, IEntity *ent, void *pOut)
{

	if (!menu->misc_antiaimproxy)
	{
		*(float*)pOut = pData->m_Value.m_Float;
		return;
	}

	float fl = pData->m_Value.m_Float;

	if (fl < -90)
		fl = 270.0f;
	if (fl > 90)
		fl = 90.0f;

	*(float*)pOut = fl;
}


void *g_pMoveHelper = NULL;


void __stdcall hooked_RunCommand(IEntity *ent, CUserCmd* pCmd, void* mv)
{
	if (!g_pMoveHelper)
	{
		g_pMoveHelper = mv;
	}

	void* thisptr = NULL;
	__asm mov thisptr, ecx


	if (commands[pCmd->command_number % sizeof(commands)])
	{

		g_pPrediction->DoSmartPred(ent, pCmd, mv, sdk->vmovement, g_pPrediction->vmt);
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

	static int calcviewcount = 0;
	static QAngle bakPunch;


	calcviewcount++;

	if (calcviewcount == 3)
	{
		g_pLocalEnt->GetViewPunch().x = bakPunch.x;
		g_pLocalEnt->GetViewPunch().y = bakPunch.y;
		g_pLocalEnt->GetViewPunch().z = bakPunch.z;

		bakPunch.Zero();
		calcviewcount = 0;
	}

	if (calcviewcount == 2)
	{
		QAngle curPunch = g_pLocalEnt->GetViewPunch();
		bakPunch.x = curPunch.x;
		bakPunch.y = curPunch.y;
		bakPunch.z = curPunch.z;
		g_pLocalEnt->GetViewPunch().Zero();
	}

	return ret;
}


#if defined(_GENERIC) || defined (_TF2) || defined(_GMOD)
void *iinput;
__declspec(naked) void* hooked_GetUserCmd(int sequence_number)
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

void InitSessionStuff()
{
	g_pLocalEnt = 0;
	hooked_CBaseEntity = false;
	hooked_CBaseEntityA = false;
	hooked_UpdateMessageStats_retaddr = 0;
	msgstats_offsetcounter = 0;

	memset(commands, 0, 512 * sizeof(bool));
	memset(msgstats_offsets, 0, sizeof(msgstats_offsets));
	memset(msgstats_ebps, 0, sizeof(msgstats_ebps));
	memset(ientity__hboxsets, 0, sizeof(ientity__hboxsets));
	memset(ientity__matrixs, 0, sizeof(ientity__matrixs));

	NetChanHook = new VMTHook(g_pEngine->GetNetChannelInfo());
#ifdef _CSGO
	NetChanHook->SetPadding(1);
#endif


	SendNetMsgFn = (OrigSendNetMsg)NetChanHook->hookFunction(40, hooked_SendNetMsg);//40=sendnetmsg, 41=senddata?, 46=senddatagram, 55=UpdateMessageStats?
	UpdateMessageStatsFn = (OrigUpdateMessageStats)NetChanHook->hookFunction(55, hooked_UpdateMessageStats);


	hooked_NetChan = g_pEngine->GetNetChannelInfo();

	Msg("Hooked netchan!\n");


}

void*oldspewfunc = 0;

void*retaddr_netspew = 0;
SpewRetval_t SpewFunc(SpewType_t spewType, const char*msg)
{
	if (!msg) return SPEW_CONTINUE;

	if (blocknetmsglog)
	{
		blocknetmsglog = false;
		return SPEW_CONTINUE;
	}

	SpewRetval_t retval = ((SpewRetval_t(*)(SpewType_t spewType, const char*msg))oldspewfunc)(spewType, msg);

	if (!retaddr_netspew)
	{
		if ( strstr(msg, "Connected to"))
		{
			retaddr_netspew = _ReturnAddress();
			InitSessionStuff();
		}
	}else{
		
		if (retaddr_netspew == _ReturnAddress()&&strstr(msg, "Connected to"))
			InitSessionStuff();

	}

	return retval;
}

struct LoggingContext_t
{
	int m_ChannelID;
	int m_Flags;
	int m_Severity;
	Color m_Color;
};

class ILoggingListener
{
public:
	virtual void Log(const LoggingContext_t *pContext, const char *pMessage) = 0;
};

class CNikyuriaListener : public ILoggingListener
{
public:
	void Log(const LoggingContext_t *pContext, const char *msg)
	{
		if (strstr(msg, "Connected to"))
		{
			InitSessionStuff();
		}
	}
} g_NikyuriaListener;

std::string m_hooks::HookStuff()
{

	Chat = new m_Chat;
	BunnyHop = new m_BunnyHop;
	Weapon = new m_Weapon;
	Bots = new m_Bots;
	HvH = new m_HvH;
	NameChanger = new m_NameChanger;

#if defined(_GENERIC) || defined(_GMOD)

	char** clientvmt = *(char***)g_pClient->vmt;

	iinput = **(void***)(clientvmt[16] + 0x2);


	if (!iinput)
		return "No VInput!";

	VMTHook *inputHook = new VMTHook(iinput);
	utils::file::WriteToLog("iinput indexes: %i\n", inputHook->tellCount());
	Msg("iinput indexes: %i\n", inputHook->tellCount());

	unsigned long oldflags = 0;
	VirtualProtect((void*)hooked_GetUserCmd, 0x64, PAGE_EXECUTE_READWRITE, &oldflags);

	memcpy(hooked_GetUserCmd, inputHook->GetMethod(8), 0x64);

	void* topatch = utils::memory::FindSig(hooked_GetUserCmd, "\x0F", 0x120);

	memset(topatch, 0x90, 3);

	unsigned long trashflags = 0;

	VirtualProtect((void*)hooked_GetUserCmd, 0x64, oldflags, &trashflags);



	inputHook->hookFunction(8, hooked_GetUserCmd);
#endif

	VMTHook *engineHook = new VMTHook(g_pEngine->vmt);

	g_pEngine->vmt = &engineHook->m_pOriginalVTable;

	engineHook->hookFunction(INDEX_ENGINE_SETVIEWANGLES, hooked_SetViewAngles);


	VMTHook *clientHook = new VMTHook(g_pClient->vmt);
	g_pClient->vmt = &clientHook->m_pOriginalVTable;

	clientHook->hookFunction(INDEX_CLIENT_FRAMESTAGENOTIFY, hooked_FrameStageNotify);

	VMTHook *predHook = new VMTHook(g_pPrediction->vmt);
	g_pPrediction->vmt = &predHook->m_pOriginalVTable;

	predHook->hookFunction(INDEX__PRED_RUNCOMMAND, hooked_RunCommand);
	predHook->hookFunction(INDEX__PRED_INPREDICTION, hooked_InPrediction);

#ifndef _CSGO
	oldspewfunc = GetSpewOutputFunc();
	SpewOutputFunc(SpewFunc);

#else
	LoggingSystem_PushLoggingState(false, false);
	LoggingSystem_RegisterLoggingListener(&g_NikyuriaListener);

	return "";
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
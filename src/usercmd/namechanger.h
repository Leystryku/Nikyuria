#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

extern char letters[99];
extern char* swapletters[99];
auto initkeyvalue = (void*(__cdecl*)(int size))0;
auto initerkeyvalues = (void(__stdcall*)(int thisptr, int name, char *toset, void*value))0;

class m_NameChanger
{
	int cooldown;

public:


	__forceinline void DoThink(CUserCmd *pCmd, bool NameChange, bool InvisName
#ifdef _GMOD
		,bool DarkRP
#endif
	)
	{
		if (cooldown)
			cooldown--;

		cooldown = 30;


		INetChannel *chan = g_pEngine->GetNetChannelInfo();
		
		if (!chan)
			return;

		if (InvisName)
		{
			const char* filler = "\xE2\x80\x8F\xE2\x80\x8F";
#ifdef _GMOD
			if (DarkRP)
			{
				std::string darkrpname = "say /rpname ";
				darkrpname = darkrpname + filler;
				chan->StringCommand(darkrpname.c_str());
			}
#endif

			 
			chan->SetName(filler);
			return;
		}

		if (!NameChange)
			return;

		srand((unsigned int)time(NULL));

		player_info_s pInfo;

		int shitint = 0;

		while (shitint++<100)
		{
			int i = (rand() % g_pGlobals->max_clients);

			if (i == g_pEngine->GetLocalPlayer())
				continue;

			if (g_pEngine->GetPlayerInfo(i, &pInfo) == false)
				continue;

			if (!pInfo.name)
				continue;

			break;
		}

		if (!pInfo.name)
			return;

		std::string name1(pInfo.name);
		std::string name2(pInfo.name);
		
		name1.erase(name1.length() / 2, name1.length());
		name2.erase(0, name2.length() / 2);

		const char* filler = "\xE2\x80\x8F";

		std::string newname = filler + name1 + filler + name2;
		std::string baknewname = std::string(newname);

		//swapstart

		
		//maxlen: 31 chars
		
		bool canswap = true;

		if (newname.length() >= 30)
			canswap = false;

		int maxswaps = 31 - newname.length();
		if (maxswaps == 1)
			canswap = false;

		float fmaxswaps = (float)maxswaps / 2.f;

		if (round(fmaxswaps) > fmaxswaps)
			fmaxswaps -= 1;

		maxswaps = (int)round(fmaxswaps);

		if (maxswaps == 1 || maxswaps == 0)
			canswap = false;

		canswap = false;

		if (canswap)
		{

			std::string donename = "";
			for (int i = 0; i < (int)newname.length(); i++)
			{

				char* toadd = 0;

				for (int swap = 0; swap < sizeof(letters); swap++)
				{
					if (newname[i] == letters[swap])
					{
						if (swapletters[swap])
						{
							toadd = swapletters[swap];
							break;
						}else{
							Msg("Missing swapletter for: %i\n", letters[swap]);
						}
					
					}
				}

				if (toadd)
				{
					maxswaps--;

					if (maxswaps > 0)
						donename.append(toadd);
				}else{
					donename.push_back(newname[i]);
				}
			}

			newname = donename.c_str();
			

		}

		//swapend

#ifdef _GMOD
		if (DarkRP)
		{
			std::string darkrpname = "say /rpname ";
			darkrpname = darkrpname + baknewname;
			chan->StringCommand(darkrpname.c_str());
		}
#endif

		chan->SetName(newname.c_str());
	}


};

extern m_NameChanger *NameChanger;
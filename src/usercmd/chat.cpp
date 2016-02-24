#pragma once

#include "chat.h"



m_Chat *Chat = NULL;

const char* filler = ":";

std::string newlines_max = "";


void m_Chat::DoThink(CUserCmd *pCmd, std::string SpamText, bool TeamText, bool TextImitator, bool TextSpammer, bool TextJammer
#ifdef _GMOD
	,bool ULXText
#endif
	)
{
	static int coolb = 0;

	coolb--;

	if (coolb > 0)
		return;

	coolb = 10;

	INetChannel *chan = g_pEngine->GetNetChannelInfo();

	if (!chan)
		return;

	std::string tosay = "say ";

	if (TeamText)
		tosay = "say_team ";

	if (TextSpammer)
	{
		std::string saytxt = "";
		saytxt.append(tosay);

		saytxt += SpamText.c_str();
		chan->StringCommand(saytxt.c_str());
	}

	if (TextJammer)
	{

		if (newlines_max.length() < 5)
		{
			newlines_max.append(220, '\n');
		}




		if (SpamText.length() > 0)
		{
			std::string newlines = ":";
			newlines.append(125 - (SpamText.length() + 1), '\n');
			tosay += newlines + SpamText;

		}
		else{
			tosay += filler + newlines_max + filler;


		}

		chan->StringCommand(tosay.c_str());
	}


#ifdef _GMOD
	if (ULXText)
	{
		std::string ulxtxt = "ulx psay \"";

		player_info_s pinfo;

		for (int i = 0; i < 200; i++)
		{

			if (g_pEngine->GetLocalPlayer() == i) continue;
			if (!utils::game::IsPlayerIndex(i, g_pGlobals->max_clients)) continue;


			g_pEngine->GetPlayerInfo(i, &pinfo);
			if (!pinfo.name || strlen(pinfo.name) > 31) continue;

			std::string playersay = "";
			playersay.append(ulxtxt.c_str());
			playersay.append(pinfo.name);
			playersay.append("\" ");
			playersay.append(SpamText.c_str());
			chan->StringCommand(playersay.c_str());

		}
	}
#endif



}
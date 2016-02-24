#pragma once

#include "stdafx.h"
#include "global.h"
#include "menu.h"
#include "usercmd\bunnyhop.h" // done
#include "usercmd\chat.h"  // text imitator missing
#include "usercmd\namechanger.h"//SendStringCmd missing
#include "usercmd\weapon.h" // nospread, vis norecoil then its done
#include "usercmd\bots.h" 
#include "usercmd\hvh.h"

bool DrawViewESP_CustomCollision(IHandleEntity *handleent)
{
	IEntity *ent = (IEntity*)handleent;

	if (ent && ( g_pLocalEnt == ent || g_pLocalEnt->EntIndex() == ent->EntIndex() ) )
		return false;

	return true;
}

inline void ViewESP()
{
	Vector eyePos = g_pLocalEnt->GetEyePosition(g_pGlobals->interval_per_tick);
	QAngle eyeAngles = g_pLocalEnt->GetEyeAngles();

	trace_t tr;

	CTraceFilterHitAll filter;

	Vector vecForward;
	AngleVectors(eyeAngles, &vecForward);

	Vector end = eyePos + (vecForward * MAX_TRACE_LENGTH);

	Ray_t pRay;
	pRay.Init(eyePos, end);
	filter.CustomCollisionFunc = &DrawViewESP_CustomCollision;

	g_pEngineTrace->TraceRay(pRay, 0xFFFFFFFF, &filter, &tr);

	if (!tr.m_pEnt)
	{
		d3d->viewesptxt = "worldspawn";
		return;
	}

	ClientClass *clientClass = tr.m_pEnt->GetClientClass();

	if (!clientClass)
	{
		d3d->viewesptxt = "worldspawn";
		return;
	}

	d3d->viewesptxt = clientClass->m_pNetworkName;

#ifdef _GMOD
	d3d->viewesptxt = tr.m_pEnt->GetClassname();
#endif
}

inline void ProcessUserCommand(CUserCmd *pCmd, bool *bSendPacket)
{
	if (!g_pLocalEnt||!pCmd)
		return;

	if (g_pLocalEnt->GetActiveWeapon() && !g_pLocalEnt->GetActiveWeapon()->CanShoot1(true, true))
		*bSendPacket = true;

	for (int i = -1; i < g_pGlobals->max_clients+1; i++)
	{
		IEntity*ent = g_pEntList->GetClientEntity(i);
		if (ent)
			ent->UpdateBones();
	}



#ifdef _GMOD
	Chat->DoThink(pCmd, menu->misc_text_spam_txt, menu->misc_text_team, menu->misc_text_imitator, menu->misc_text_spam, menu->misc_text_jam, menu->misc_text_ulxspam);
	NameChanger->DoThink(pCmd, menu->misc_namechange, menu->misc_invisname, menu->misc_namechange_darkrp);
#else
	Chat->DoThink(pCmd, menu->misc_text_spam_txt, menu->misc_text_team, menu->misc_text_imitator, menu->misc_text_spam, menu->misc_text_jam);
	NameChanger->DoThink(pCmd, menu->misc_namechange, menu->misc_invisname);
#endif

	ViewESP();


	if (!g_pLocalEnt->IsAlive())
		return;


	if (menu->misc_freeze)
	{
#ifdef _CSGO
		pCmd->command_number = 0xFFFFF;
#else
		pCmd->tick_count = 0xFFFFFFF;
#endif
	}

	
	if (menu->misc_flashlightspam)
		pCmd->impulse = 100;
	
	Bots->DoThink(pCmd, menu->bot_aim_prediction, menu->bot_aim_targetmode, menu->bot_hitgroup, menu->bot_spawnprotection, menu->bot_aim_dist, menu->bot_aim_fov, menu->bot_aim_smoothing, menu->bot_aim, menu->bot_aim_silent, menu->bot_aim_silent_p, menu->bot_canshoot, menu->bot_killnpc, menu->bot_killplayer, menu->bot_teamkill, menu->bot_teamshoot, menu->bot_trigger, menu->bot_delayselection, bSendPacket);
	BunnyHop->DoThink(pCmd, menu->misc_safemode, menu->misc_bhop, menu->misc_autostrafe);
	Weapon->DoThink(pCmd, bSendPacket, menu->bot_aim, menu->misc_safemode, menu->wep_noshake, menu->wep_autopistol, menu->wep_autopistol_alt, menu->wep_autoreload, menu->wep_norecoil_punch, menu->wep_norecoil, menu->wep_nospread, menu->wep_nospread_seed);
	HvH->DoThink(pCmd, bSendPacket, menu->misc_antiaim, menu->misc_antiaimproxy, menu->misc_fakecrouch, menu->misc_fakelag, menu->misc_fakelag2);

}



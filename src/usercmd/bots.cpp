#pragma once

#include "bots.h"

m_Bots *Bots = NULL;
IEntity *m_Bots::Target = NULL;
Vector m_Bots::HitPos;
Vector oldSmoothAngles;

CTraceFilter filter;
Ray_t pRay;
trace_t tr;

int iToUpdate = 0;
bool g_TeamShoot = false;
float g_FOV = 0;

bool Bots_Filter(IHandleEntity *handleent)
{
	IEntity *ent = (IEntity*)handleent;

	if (ent)
	{
		if (g_pLocalEnt == ent || g_pLocalEnt->EntIndex() == ent->EntIndex())
			return false;

		if (ent && g_TeamShoot && utils::game::IsPlayerIndex(ent->EntIndex(), g_pGlobals->max_clients) && g_pLocalEnt->GetTeam() == ent->GetTeam())
			return false;
	}

	return true;
}

extern void* g_pMoveHelper;

void m_Bots::DoPrediction(CUserCmd *pCmd, int predtype)
{

	int *pFlags = g_pLocalEnt->GetFlags();
	int Flags = *pFlags;

	QAngle vecPunchAng(g_pLocalEnt->GetViewPunch());
	float frametime = g_pGlobals->frame_time;
	float curtime = g_pGlobals->curtime;
	bool attack1 = false;
	bool attack2 = false;

	if ((pCmd->buttons & IN_ATTACK))
	{
		attack1 = true;
		pCmd->buttons &= ~IN_ATTACK;
	}

	if ((pCmd->buttons & IN_ATTACK2))
	{
		attack2 = true;
		pCmd->buttons &= ~IN_ATTACK2;
	}

	if (predtype == 1)
	{
		g_pPrediction->RunCommand(g_pPrediction->vmt, g_pLocalEnt, pCmd, g_pMoveHelper);
	}
	else{
		g_pPrediction->DoSmartPred(g_pLocalEnt, pCmd, g_pMoveHelper, sdk->vmovement, g_pPrediction->vmt);
	}

	if (attack1)
		pCmd->buttons |= IN_ATTACK;

	if (attack2)
		pCmd->buttons |= IN_ATTACK2;

	*pFlags = Flags;
	g_pLocalEnt->GetViewPunch().x = vecPunchAng.x;
	g_pLocalEnt->GetViewPunch().y = vecPunchAng.y;
	g_pLocalEnt->GetViewPunch().z = vecPunchAng.z;

	g_pGlobals->frame_time = frametime;
	g_pGlobals->curtime = curtime;


}

bool m_Bots::IsVisible(Vector& vecAbsStart, Vector& vecAbsEnd, IEntity* target)
{
	pRay.Init(vecAbsStart, vecAbsEnd);
	filter.CustomCollisionFunc = &Bots_Filter;

	g_pEngineTrace->TraceRay(pRay, 0x46004003, &filter, &tr); /*0x4600400B */

	if (tr.fraction == 1.0f)
	{
		return true;
	}

	if (tr.m_pEnt && target)
	{
		if (tr.m_pEnt == target)
		{
			return true;
		}
	}
	
	return false;
}

int HitGroupDamage(int iGroup)
{
	if (iGroup == 1) //head
		return 5;

	if (iGroup == 2)//chest
		return 4;

	if (iGroup == 3)//stomach
		return 3;

	if (iGroup == 4 || iGroup == 5)//arms
		return 2;

	if (iGroup == 6 || iGroup == 7)//legs
		return 1;

	return 0;
}

bool m_Bots::CalculateAimVector(IEntity *ent, Vector &vecEyePos, int hitgroup)
{
	HitPos.Zero();
	if (!ent->GetHBoxSet())
		return false;

	if (hitgroup == 8)
	{
		srand((unsigned int)time(NULL));
		hitgroup = (rand() % 7);
	}

	int groupdmg = -1;

	bool Found = false;

	Vector tmpVec;

	for (int i = 0; i < ent->GetHBoxSet()->numhitboxes; i++)
	{
		mstudiobbox_t* hitbox = ent->GetHBoxSet()->hitbox(i);

		if (!hitbox)
			continue;

		if (hitbox->group > 7)
			continue;

		if (hitgroup && hitbox->group != hitgroup)
			continue;

		Vector middle = ((hitbox->bbmin + hitbox->bbmax) * .5f);


		VectorTransform(middle, ent->GetMatrix()[hitbox->bone], tmpVec);

		if (hitbox->group == 1)
			tmpVec.z -= 3;
	
		if (!IsVisible(vecEyePos, tmpVec, ent))
			continue;


		if (hitgroup)
		{
			HitPos = tmpVec;

			return true;
		}

		Found = true;

		int tmpgroupdmg = HitGroupDamage(hitbox->group);

		if (tmpgroupdmg == 5)
		{
			HitPos = tmpVec;
			return true;
		}

		if (tmpgroupdmg>groupdmg)
		{
			HitPos = tmpVec;
			groupdmg = tmpgroupdmg;
		}

	}


	return Found;
}

float m_Bots::DangerRate(Vector origin, IEntity *ent, int TargetMode)
{
	float rate = 0.f;

	Vector plAbsOrigin = ent->GetAbsOrigin();

	if (TargetMode == 0)
	{
		rate = origin.DistTo(plAbsOrigin);
	}

	if (TargetMode == 1)
	{
		rate = (float)ent->GetHealth();
	}

	if (TargetMode == 2)
	{
		Vector path = (origin - plAbsOrigin).Angle();
		Vector enemyeyepos = ent->GetEyeAngles();
		enemyeyepos.Normalize();

		rate = path.DistTo(enemyeyepos);
		
	}

	if (TargetMode == 3)
	{
		Vector path = (origin - plAbsOrigin).Angle();
		Vector enemyeyepos = ent->GetEyeAngles();
		enemyeyepos.Normalize();

		rate = path.DistTo(enemyeyepos);

		IEntity *wep = ent->GetActiveWeapon();
		if (wep)
		{
			rate *= 0.5f;

			const char *networkname = wep->GetClientClass()->m_pNetworkName;
#ifdef _CSS
			if (!strcmp(networkname, "CDEagle"))
				rate *= 0.5f;

			if (!strcmp(networkname, "CWeaponAWP") || !strcmp(networkname, "CWeaponScout"))
				rate *= 0.3f;
#endif

		}
	}


	return rate;
}

bool m_Bots::IsValidTarget(IEntity *ent, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill)
{

	if (!ent)
		return false;

	if (ent == g_pLocalEnt)
		return false;

	ClientClass *pClientClass = ent->GetClientClass();

	if(!pClientClass||!pClientClass->m_pNetworkName)
		return false;

	if (!ent->IsAlive())
		return false;

	if (ent->IsDormant())
		return false;

	bool is_Player = false;
	bool is_NPC = false;


	if (KillPlayer)
	{
		if (utils::game::IsPlayerIndex(ent->EntIndex(), g_pGlobals->max_clients))
			is_Player = true;

	}

	if (KillNPC)
	{
		if (!is_Player)
		{
			if (strstr(pClientClass->m_pNetworkName, "NPC") || strstr(pClientClass->m_pNetworkName, "NextBot") || strstr(pClientClass->m_pNetworkName, "Object"))
				is_NPC = true;
		}
	}

	if (!is_Player && !is_NPC)
		return false;

	if (is_Player && !TeamKill&&g_pLocalEnt->GetTeam() == ent->GetTeam())
		return false;

	if (is_Player &&ent->GetObserverMode() != 0)
		return false;

	if (SpawnProtection && ent->IsSpawnProtected(SpawnProtection))
		return false;

#ifdef _TF2
	if (ent->HasCond(TFCond_Ubercharged))
	{
		return false;
	}
#endif
	


	if (AimDistance && ent->GetAbsOrigin().DistTo(g_pLocalEnt->GetAbsOrigin()) > AimDistance)
		return false;

	if (is_NPC)
		ent->UpdateBones();

	Vector myeyes = g_pLocalEnt->GetEyePosition(g_pGlobals->interval_per_tick);

	bool gothitbox = CalculateAimVector(ent, myeyes, HitGroup);

	if (!gothitbox) return false;

	if (FOV>0.1)
	{
		QAngle eyes = g_pLocalEnt->GetEyeAngles();

		QAngle ang = (HitPos - myeyes).Angle();


		float fl = eyes.x - ang.x;
		float fl2 = eyes.y - ang.y;

		NormalizeAngle(fl);
		NormalizeAngle(fl2);

		float ady = abs(fl);
		float adp = abs(fl2);

		if (ady > FOV || adp > FOV)
			return false;
	}


	return true;
}

void m_Bots::UpdateTarget(int TargetMode, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot)
{

	float smallestRate = 999999999.f;

	IEntity *winner = 0;
	Vector winnerHitPos(0, 0, 0);

	int maxents = g_pGlobals->max_clients;

	if (KillNPC)
		maxents = g_pEntList->GetHighestEntityIndex();

	Vector vecLocalAbsOrigin = g_pLocalEnt->GetAbsOrigin();

	for (int i = 1; i <maxents+1; ++i)
	{

		IEntity *ent = g_pEntList->GetClientEntity(i);


		if (!IsValidTarget(ent, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill))
			continue;

		float danger = DangerRate(vecLocalAbsOrigin, ent, TargetMode);

		if (danger < smallestRate)
		{
			winnerHitPos = HitPos;
			winner = ent;
			smallestRate = danger;

		}

	}

	HitPos = winnerHitPos;
	Target = winner;
}

void m_Bots::FindTarget(int TargetMode, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot, bool DelaySelection)
{
	if (!DelaySelection)
	{
		UpdateTarget(TargetMode, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill, TeamShoot);
		return;
	}

	if (!IsValidTarget(Target, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill))
	{
		iToUpdate = 5;
		UpdateTarget(TargetMode, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill, TeamShoot);
		return;
	}

	if (1>iToUpdate)
	{
		iToUpdate = 5;
		UpdateTarget(TargetMode, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill, TeamShoot);
	}

	iToUpdate--;



}

void m_Bots::FixMove(CUserCmd *cmd, QAngle& va)
{
	float yaw, speed;
	Vector& move = *(Vector*)&cmd->move[0];
	Vector move2 = *(Vector*)&cmd->move;

	speed = move2.Length2D();

	yaw = Rad2Deg(atan2(move.y, move.x));
	yaw = Deg2Rad(cmd->viewangles.y - va.y + yaw);

	cmd->move[0] = cos(yaw) * speed;
	cmd->move[1] = sin(yaw) * speed;

	if (cmd->viewangles.x < -90.f || cmd->viewangles.x > 90.f)
	{
		cmd->move.x = FloatNegate(cos(FloatNegate(yaw)) * speed);
		cmd->move.y = FloatNegate(sin(FloatNegate(yaw)) * speed);
	}

}

QAngle oldView;
Vector oldMove;
bool psilent_forcereset = false;

void m_Bots::AimBot(CUserCmd* pCmd, int Prediction, int TargetMode, int HitGroup, int SpawnProtection, int AimDistance, float FOV, int SmoothAim, bool SilentAim, bool SilentAim_P, bool IgnoreCanShoot, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot, bool TriggerBot, bool DelaySelection, bool* bSendPacket)
{
	if (Prediction)
	{
		DoPrediction(pCmd, Prediction);
		g_pLocalEnt->GetAbsOrigin();
		g_pLocalEnt->GetAbsVelocity();
	}
		

	if ((!oldView.IsZero() || !oldMove.IsZero()) && !*bSendPacket)
	{
		pCmd->viewangles = oldView;
		pCmd->move = oldMove;
		oldView.Zero();
		oldMove.Zero();

		*bSendPacket = true;
	}

	if (!KillNPC && !KillPlayer)
		return;

	FindTarget(TargetMode, SpawnProtection, AimDistance, FOV, HitGroup, KillNPC, KillPlayer, TeamKill, TeamShoot, DelaySelection);

	if (!Target)
		return;

	Vector vecAimPos = HitPos - g_pLocalEnt->GetEyePosition(g_pGlobals->interval_per_tick);
	QAngle angAimAngles(0, 0, 0);

	VectorAngles(vecAimPos, angAimAngles);
	NormalizeAngles(angAimAngles);

	bool finalpos = true;

	oldSmoothAngles = pCmd->viewangles;

	if (SmoothAim > 0)
	{
		Vector snap = (angAimAngles - oldSmoothAngles);

		NormalizeAngles(snap);

		if (snap.Normalize() > (float)SmoothAim)
		{
			angAimAngles = oldSmoothAngles + snap * (float)SmoothAim;
			NormalizeAngles(angAimAngles);
			finalpos = false;
		}

	}

	IEntity *wep = g_pLocalEnt->GetActiveWeapon();

	if (!wep)
		return;


	if (!wep->CanShoot1(true, !IgnoreCanShoot))
	{
		if (SilentAim_P)
		{
			*bSendPacket = true;
			if (!oldView.IsZero() || !oldMove.IsZero())
			{
				pCmd->viewangles = oldView;
				pCmd->move = oldMove;
				oldView.Zero();
				oldMove.Zero();

			}
		}
		return;
	}

	if (SilentAim)
	{

		if (SilentAim_P)
		{
			oldMove = pCmd->move;
			oldView = pCmd->viewangles;
			*bSendPacket = false;
		}

		pCmd->buttons |= IN_ATTACK;

		QAngle fixView = pCmd->viewangles;

		pCmd->viewangles = angAimAngles;

		FixMove(pCmd, fixView);

		return;

	}

	pCmd->viewangles = angAimAngles;
	g_pEngine->SetViewAngles(pCmd->viewangles);

	if (TriggerBot&&finalpos)
		pCmd->buttons |= IN_ATTACK;

	oldSmoothAngles = pCmd->viewangles;
}

void m_Bots::TriggerBot(CUserCmd *pCmd, int SpawnProtection, bool IgnoreCanShoot, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot)
{

	if (!g_pLocalEnt->GetActiveWeapon())
		return;

	if (!g_pLocalEnt->GetActiveWeapon()->CanShoot1(true, !IgnoreCanShoot))
		return;

	Vector Forward, vDirection;
	trace_t pTrace;

	Vector eyePos = g_pLocalEnt->GetEyePosition(g_pGlobals->interval_per_tick);
	QAngle viewAng = pCmd->viewangles;

	AngleVectors(viewAng, &vDirection);
	vDirection = vDirection * 8192 + eyePos;

	Ray_t pRay;
	pRay.Init(eyePos, vDirection);

	CTraceFilterHitAll filter;
	filter.CustomCollisionFunc = &Bots_Filter;

	g_pEngineTrace->TraceRay(pRay, 0x46004003, &filter, &pTrace);

	if (!pTrace.m_pEnt)
		return;

	if (!pTrace.m_pEnt->IsAlive())
		return;

	bool is_living = false;
	bool is_npc = false;

	ClientClass *pClientClass = pTrace.m_pEnt->GetClientClass();

	if (!pClientClass)
		return;

	const char *classname = pClientClass->m_pNetworkName;

	if (!classname)
		return;

	if (KillPlayer && utils::game::IsPlayerIndex(pTrace.m_pEnt->EntIndex(), g_pGlobals->max_clients))
		is_living = true;

	if (KillNPC && (strstr(classname, "NPC") || strstr(classname, "NextBot") || strstr(classname, "Object")))
	{
		is_living = true;
		is_npc = true;
	}

	if (!is_living)
		return;

	if (!is_npc)
	{
		if (!TeamKill)
		{
			if (g_pLocalEnt->GetTeam() == pTrace.m_pEnt->GetTeam())
				return;
		}

	}

	pCmd->buttons |= IN_ATTACK;
}


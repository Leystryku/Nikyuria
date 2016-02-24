#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

extern bool g_TeamShoot;
extern float g_FOV;


class m_Bots
{
public:
	void DoPrediction(CUserCmd *pCmd, int predtype);

	bool IsVisible(Vector& vecAbsStart, Vector& vecAbsEnd, IEntity* target);
	Vector &GetHitbox(IEntity *ent, Vector &vecEyePos, int hitgroup);
	bool CalculateAimVector(IEntity *ent, Vector &vecEyePos, int hitgroup = 0);

	float DangerRate(Vector origin, IEntity *ent, int TargetMode);
	bool IsValidTarget(IEntity *ent, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill);
	void UpdateTarget(int TargetMode, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot);
	void FindTarget(int TargetMode, int SpawnProtection, int AimDistance, float FOV, int HitGroup, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot, bool DelaySelection);

	void FixMove(CUserCmd *cmd, QAngle& va);
	void AimBot(CUserCmd* pCmd, int Prediction, int TargetMode, int HitGroup, int SpawnProtection, int AimDistance, float FOV, int SmoothAim, bool SilentAim, bool SilentAim_P, bool IgnoreCanShoot, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot, bool TriggerBot, bool DelaySelection, bool *bSendPacket);
	void TriggerBot(CUserCmd *pCmd, int SpawnProtection, bool IgnoreCanShoot, bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot);

	__forceinline void DoThink(CUserCmd *pCmd, int Prediction, int TargetMode, int HitGroup, int SpawnProtection, int AimDistance,
		float FOV, int SmoothAim, bool aim_bot, bool SilentAim, bool SilentAim_P, bool IgnoreCanShoot,
		bool KillNPC, bool KillPlayer, bool TeamKill, bool TeamShoot, bool trigger_bot, bool DelaySelection, bool* bSendPacket)
	{
		//danger level, prediction nr2
		g_TeamShoot = TeamShoot;
		g_FOV = FOV;

		if (SilentAim && (pCmd->buttons & IN_ATTACK))
			return;

		if (aim_bot)
		{
			AimBot(pCmd, Prediction, TargetMode, HitGroup, SpawnProtection, AimDistance, FOV, SmoothAim, SilentAim, SilentAim_P, IgnoreCanShoot, KillNPC, KillPlayer, TeamKill, TeamShoot, trigger_bot, DelaySelection, bSendPacket);
			return;
		}

		if (trigger_bot)
		{
			TriggerBot(pCmd, SpawnProtection, IgnoreCanShoot, KillNPC, KillPlayer, TeamKill, TeamShoot);
		}

	}

	static IEntity *Target;
	static Vector HitPos;

};

extern m_Bots *Bots;
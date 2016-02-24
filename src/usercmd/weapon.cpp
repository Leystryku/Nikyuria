#pragma once

#include "weapon.h"
#include "../sdk/md5.h"
#include "bots.h"

m_Weapon *Weapon = NULL;
Vector currentSpread;

void m_Weapon::AutoPistol(CUserCmd *pCmd, bool safemode, bool altautopist)
{
	static int skip = 15;

	if (safemode)
	{
		skip = skip - 1;

		if (1 > skip)
		{
			if (skip < -5)
				skip = 15;

			return;
		}
	}

	if (altautopist)
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			static bool shootswitch = false;

			if (shootswitch)
				pCmd->buttons &= ~IN_ATTACK;
			else
				pCmd->buttons |= IN_ATTACK;

			shootswitch = !shootswitch;
		}

		return;
	}


	int m_iClip1 = 0;

	IEntity *Weapon = g_pLocalEnt->GetActiveWeapon();

	if (Weapon)
		m_iClip1 = Weapon->GetClip1();

	if (1 > m_iClip1)
		return;

	if ((pCmd->buttons & IN_ATTACK))
	{
		if (!Weapon->CanShoot1())
			pCmd->buttons &= ~IN_ATTACK;

	}

}

void m_Weapon::AutoReload(CUserCmd *pCmd)
{

	static int iNextReload = 0;

	int m_iClip1 = 0;

	IEntity *Weapon = g_pLocalEnt->GetActiveWeapon();

	if (Weapon)
		m_iClip1 = Weapon->GetClip1();

	if (m_iClip1 == 0)
	{
		clock_t t;
		t = clock();

		if (iNextReload > t)
		{
			return;
		}

		iNextReload = t + (CLOCKS_PER_SEC * 3);

		pCmd->buttons |= IN_RELOAD;

		if ((pCmd->buttons & IN_ATTACK))
			pCmd->buttons &= ~IN_ATTACK;

	}
}
void m_Weapon::NoRecoil(CUserCmd *pCmd, bool norecoil_vis)
{

	QAngle punch = g_pLocalEnt->GetViewPunch();
	//Msg("%f - %f - %f\n", punch.x, punch.y, punch.z);

	float normal = punch.Normalize();

	normal = max(normal - (10.f + normal * 0.5f) * g_pGlobals->interval_per_tick, 0.f);
	punch *= normal;
	punch.z = 0.0f;

#ifdef _CSS
		punch *= 2.0f;
#endif

	pCmd->viewangles -= punch;


}

void m_Weapon::NoSpread(CUserCmd *pCmd)
{
	IEntity *wep = g_pLocalEnt->GetActiveWeapon();

#ifdef _CSS

	typedef void(__thiscall* CS_UpdateAccuracyPenaltyFn)(void*);
	typedef float(__thiscall* CS_GetSpreadFn)(void*);

	static int m_fUpdateAccuracyPenalty = 0x5CC + 8;
	static int m_fGetWeaponSpread = 0x5CC + 0;
	static int m_fGetWeaponCone = 0x5CC + 4;

	CS_UpdateAccuracyPenaltyFn UpdateAccuracyPenalty = ((CS_UpdateAccuracyPenaltyFn)(*(PDWORD)(*(PDWORD)wep + m_fUpdateAccuracyPenalty)));
	CS_GetSpreadFn GetWeaponSpread = ((CS_GetSpreadFn)(*(PDWORD)(*(PDWORD)wep + m_fGetWeaponSpread)));
	CS_GetSpreadFn GetWeaponCone = ((CS_GetSpreadFn)(*(PDWORD)(*(PDWORD)wep + m_fGetWeaponCone)));

	Vector vForward, vRight, vUp;

	// Credits: LuaStoned

	UpdateAccuracyPenalty(wep);

	float flSpread = GetWeaponSpread(wep);
	float flCone = GetWeaponCone(wep);

	int uSeed = MD5_PseudoRandom(pCmd->command_number) & 255;

	RandomSeed(uSeed + 1);

	float flRandom1 = RandomFloat(0.f, (float)M_PI * 2.f); // 6.283185f
	float flRandom2 = RandomFloat(0.f, flSpread);
	float flRandom3 = RandomFloat(0.f, (float)M_PI * 2.f); // 6.283185f
	float flRandom4 = RandomFloat(0.f, flCone);

	float flLeftX = cosf(flRandom1) * flRandom2;
	float flLeftY = sinf(flRandom1) * flRandom2;

	float flRightX = cosf(flRandom3) * flRandom4;
	float flRightY = sinf(flRandom3) * flRandom4;

	Vector vSpread = Vector(FloatNegate(flLeftX + flRightX), FloatNegate(flLeftY + flRightY), 0.f);

	NormalizeAngles(pCmd->viewangles);
	AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);

	Vector vNewAngles = vForward + (vRight * vSpread.x) + (vUp * vSpread.y);
	vNewAngles.NormalizeInPlace();

	VectorAngles(vNewAngles, Vector(0,0,0), pCmd->viewangles);
	NormalizeAngles(pCmd->viewangles);
	


	return;
#endif

#ifndef _CSS
	static IEntity* lastw = NULL;

	if (lastw != wep)
	{
		currentSpread.Zero();
		lastw = wep;
	}

	if (currentSpread.IsZero())
		return;

	RandomSeed(MD5_PseudoRandom(pCmd->command_number) & 255);
	float flRandomX = RandomFloat(-0.5f, 0.5f) + RandomFloat(-0.5f, 0.5f);
	float flRandomY = RandomFloat(-0.5f, 0.5f) + RandomFloat(-0.5f, 0.5f);

	Vector vForward, vRight, vUp;
	Vector vSpread = Vector(FloatNegate(currentSpread.x), FloatNegate(currentSpread.y), 0.f);

	AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);

	Vector vNewAngles = vForward + (vRight * vSpread.x * flRandomX) + (vUp * vSpread.y * flRandomY);

	VectorAngles(vNewAngles, Vector(0,0,0), pCmd->viewangles);
	NormalizeAngles(pCmd->viewangles);

#endif


}

void m_Weapon::NoSpread_Seed(CUserCmd *pCmd, bool safemode)
{
	int seed = 142; //gmod,tf2

#if defined(_CSS) | defined (_CSGO)
		seed = 2087;
#endif

	int num = 0;

	static int idealSeed = MD5_PseudoRandom(seed) & 255;

	int niceSeed = idealSeed;

	IEntity *wep = g_pLocalEnt->GetActiveWeapon();

	if (!wep || !wep->CanShoot1())
		return;

#ifdef _TF2

	ClientClass *cc = wep->GetClientClass();

	if (strstr(cc->m_pNetworkName, "Minigun"))
	{
		niceSeed = MD5_PseudoRandom(188) & 255;
		seed = 188;
	}

#endif

	for (int i = pCmd->command_number + 1; !num; i++)
	{
		int uSeed = MD5_PseudoRandom(i);

		if (!uSeed)
			continue;

		if ((uSeed & 255) != niceSeed)
			continue;

		num = i;

	}

	pCmd->command_number = num;
}


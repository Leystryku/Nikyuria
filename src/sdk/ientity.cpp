#pragma once

#include "ientity.h"
#include "entlist.h"
#include "modelinfo.h"
#include "globals.h"
#include "client.h"
//(int) for offsets
//(char*) for pointers

IEntity *g_pLocalEnt = NULL;

bool IEntity::IsSpawnProtected(int type)
{
	if (type == 1 || type == 2)
	{
		if (GetColor().exponent != -1)
			return true;

		if (type == 2 && g_pLocalEnt)
			if (g_pLocalEnt->GetColor().exponent != -1)
				return true;

	}

	if (type==3)
	{

		//placeholder4material name check
	}

	if (type == 4)
	{
		const char *mdlname = g_pModelInfo->GetModelName(GetModel()); // for zs

		if (!strcmp(mdlname, "models/crow.mdl"))
			return true;

	}

	return false;
}

bool IEntity::HasFlag(int flag)
{
	static int offset = g_pNetworkMan->GetOffset("DT_BasePlayer", "m_fFlags");

	int flags = *(int*)((char*)(this) + offset);

	if ((flags) & (flag))
		return true;

	return false;
}

#ifdef _TF2
bool IEntity::HasCond(int cond)
{
	static int offset1 = g_pNetworkMan->GetOffset("*", "m_Shared");
	static int offset2 = g_pNetworkMan->GetOffset("*", "m_nPlayerCond");

	if(!offset1||!offset2)
		return false;

	int conds = *(int*)((char*)(this) + offset1 + offset2);

	if ((conds)& (cond))
		return true;

	return false;
}
#endif

typedef const char* (__thiscall* origGetClassname)(void* thisptr);
origGetClassname GetClassnameFn;

typedef Vector (__thiscall* origGetAbsVelocity)(void* thisptr);
origGetAbsVelocity GetAbsVelocityFn;

const char* IEntity::GetClassname()
{
	
	if (!GetClassnameFn)
	{
		void* ref = (char*)utils::memory::FindString(GetModuleHandleA("client.dll"), "C_BaseAnimating::SetupBones") + 82;
		GetClassnameFn = (origGetClassname)utils::memory::CalcAbsAddress(ref);
		
	}

	return GetClassnameFn(this);
}

Vector IEntity::GetAbsVelocity()
{
	return Vector(0, 0, 0);
	if (!GetAbsVelocityFn)
	{
		char* ref2 = (char*)utils::memory::FindString(GetModuleHandleA("server.dll"), "playeractivity");
		char* ref = (char*)utils::memory::FindSig(GetModuleHandleA("server.dll"), "\xE8????\xD9\x86????\x83\xEC\x0C\xD9\x86", 0xFFFFFFF);

		Msg("LMAO %i", ref - ref2);
		GetAbsVelocityFn = (origGetAbsVelocity)utils::memory::CalcAbsAddress(ref+1);

	}
	
	return GetAbsVelocityFn(this);
}


int IEntity::GetActiveWeaponIndex()
{
	HANDLE hWeapon = GetActiveWeaponH();

	if (!hWeapon)
		return NULL;

	int iWeapon = (unsigned long)hWeapon & 0xFFF;

	return iWeapon;
}

IEntity *IEntity::GetActiveWeapon()
{
	if (!g_pLocalEnt || !g_pEntList)
		return NULL;

	int WeaponIndex = g_pLocalEnt->GetActiveWeaponIndex();

	IEntity *wep = g_pEntList->GetClientEntity(WeaponIndex);

	return wep;
}

bool IEntity::CanShoot1(bool checkAmmo, bool checkCanShoot)
{
	float servertime = (float)g_pLocalEnt->GetTickBase() * g_pGlobals->interval_per_tick;

	if (checkCanShoot)
	{

		if (GetNextPrimaryAttack() >= servertime)
			return false;
	}

	if (checkAmmo)
	{
		if (GetClip1() == 0)
			return false;
	}

	return true;
}
bool IEntity::CanShoot2(bool checkAmmo, bool checkCanShoot)
{
	float servertime = (float)g_pLocalEnt->GetTickBase() * g_pGlobals->interval_per_tick;

	if (checkCanShoot)
	{
		if (GetNextSecondaryAttack() >= servertime)
			return false;
	}

	if (checkAmmo)
	{
		if (GetClip2() == 0)
			return false;
	}

	return true;
}

mstudiohitboxset* ientity__hboxsets[0xFFFFFFF];
matrix3x4 ientity__matrixs[0xFFFF][IENTITY__BONEARRAY];

bool IEntity::UpdateBones()
{
	int index = EntIndex();

	if (!index)
		return false;

	const model_t *mdl = GetModel();
	
	if (!mdl)
		return false;
	
	void* stdhdr = g_pModelInfo->GetStudioModel(mdl);

	if (!stdhdr)
		return false;



	//aka BONE_USED_BY_HITBOX

	if (index > 0xFFFF)
		index = 0xFFFF;

	if (!SetupBones(ientity__matrixs[index], IENTITY__BONEARRAY, 0x00000100, GetSimulationTime()))
		return false;

	int set = *(int*)((unsigned long)stdhdr + 0xB0);
	
	ientity__hboxsets[index] = (mstudiohitboxset*)(((DWORD)stdhdr) + set);
	
	if (ientity__hboxsets[index])
		return true;

	ientity__hboxsets[index] = 0;

	return false;
}

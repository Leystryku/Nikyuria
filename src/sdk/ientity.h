#pragma once

#include "../stdafx.h"
#include "externfuncs.h"
#include "color.h"
#include "netvars.h"
#include "vector.h"

template<typename Fn>
inline Fn GetVFunc(unsigned long** v, int i)
{
	return reinterpret_cast<Fn>((*v)[i]);
}

#define	IENTITY__BONEARRAY			128
//this + 0x4 = IClientRenderable;
//this + 0x8 = IClientNetworkable

#define IENTITY__GETCLIENTCLASS		2
#define IENTITY__ISDORMANT			8
#define IENTITY__ENTINDEX			9
#define IENTITY__GETMODEL			9
#define IENTITY__GETABSORIGIN		9
#define IENTITY__GETABSANGLES		10
#define IENTITY__GETRENDERBOUNDS	20

#if defined(_CSGO)
#undef IENTITY__GETMODEL
#undef IENTITY__GETRENDERBOUNDS

#define IENTITY__GETMODEL			8
#define IENTITY__GETRENDERBOUNDS	17

#undef IENTITY__ISDORMANT
#undef IENTITY__GETABSORIGIN
#undef IENTITY__ENTINDEX
#undef IENTITY__GETABSANGLES

#define IENTITY__ISDORMANT			9
#define IENTITY__GETABSORIGIN		10
#define IENTITY__ENTINDEX			10
#define IENTITY__GETABSANGLES		11

#endif

struct mstudiobbox_t
{
	int                                     bone;
	int                                     group;
	Vector                                   bbmin;
	Vector                                   bbmax;
	int                                     szhitboxnameindex;
	int                                     pad[8];
};

struct mstudiohitboxset
{
	int	sznameindex;
	int	numhitboxes;
	int	hitboxindex;

	inline mstudiobbox_t* hitbox(int i) const
	{
		return (mstudiobbox_t*)(((DWORD)this) + hitboxindex) + i;
	};
};

struct model_t;
struct ColorRGBExp32 { BYTE r, g, b; signed char exponent; };

inline void**& getvtable(void* inst, size_t offset = 0)
{
	return *reinterpret_cast<void***>((size_t)inst + offset);
}

inline const void** getvtable(const void* inst, size_t offset = 0)
{
	return *reinterpret_cast<const void***>((size_t)inst + offset);
}
template< typename Fn >

inline Fn getvfunc(const void* inst, size_t index, size_t offset = 0)
{
	return reinterpret_cast<Fn>(getvtable(inst, offset)[index]);
}

//((void*)(this + 0x4)) = ClientRenderable
//((void*)(this + 0x8)) = ClientNetworkable

#define SetupNetVar(name, datable, netvarname, ret, cast) inline ret name() { static int offset= g_pNetworkMan->GetOffset(datable, netvarname); return cast((char*)(this) + offset); }

#define SetupEntFunc(name, ret, index) inline ret name() { typedef ret (__thiscall* OriginalFn)(void*); return getvfunc<OriginalFn>(this, index)(this); }
#define SetupClientNetworkable(name, ret, index) inline ret name() { typedef ret (__thiscall* OriginalFn)(void*); return getvfunc<OriginalFn>(((void*)(this + 0x8)), index)(((void*)(this + 0x8))); }
#define SetupClientRenderable(name, ret, index) inline ret name() { typedef ret (__thiscall* OriginalFn)(void*); return getvfunc<OriginalFn>(((void*)(this + 0x4)), index)(((void*)(this + 0x4))); }



extern mstudiohitboxset* ientity__hboxsets[0xFFFFFFF];
extern matrix3x4 ientity__matrixs[0xFFFF][IENTITY__BONEARRAY];

class IEntity
{
public:

	SetupClientNetworkable(GetClientClass, ClientClass*, IENTITY__GETCLIENTCLASS);
	SetupClientNetworkable(IsDormant, bool, IENTITY__ISDORMANT);
	SetupClientNetworkable(EntIndex, int, IENTITY__ENTINDEX);
	SetupClientRenderable(GetModel, const model_t*, IENTITY__GETMODEL);

	SetupEntFunc(GetAbsOrigin, Vector&, IENTITY__GETABSORIGIN);
	SetupEntFunc(GetAbsAngles, Vector&, IENTITY__GETABSANGLES);

	inline bool SetupBones(matrix3x4* matrix, int bones, int mask, float time)
	{
		static auto OriginalFn = (bool(__thiscall *)(void *, matrix3x4 *, int, int, float))utils::memory::FindSubStart(utils::memory::FindString(GetModuleHandleA("client.dll"), "*** ERROR: Bone access not allowed (entity %i:%s)\n"));
		if (!(this + 0x4)) return false;

		return OriginalFn(((void*)(this + 0x4)), matrix, bones, mask, time);
	}


	SetupNetVar(GetFlags, "DT_BasePlayer", "m_fFlags", int*, (int*));
	SetupNetVar(GetTeam, "DT_BaseEntity", "m_iTeamNum", int, *(int*));
	SetupNetVar(GetHealth, "DT_BasePlayer", "m_iHealth", int, *(int*));
	SetupNetVar(GetLifeState, "DT_BasePlayer", "m_lifeState", BYTE, *(BYTE*));
	SetupNetVar(GetEyeAngles, "*", "m_angEyeAngles[0]", QAngle, *(QAngle*));
	SetupNetVar(GetOrigin, "DT_BaseEntity", "m_vecOrigin", Vector, *(Vector*));
	SetupNetVar(GetActiveWeaponH, "DT_BaseCombatCharacter", "m_hActiveWeapon", HANDLE, *(HANDLE*));
	SetupNetVar(GetClip1, "*", "m_iClip1", int, *(int*));
	SetupNetVar(GetClip2, "*", "m_iClip2", int, *(int*));
	SetupNetVar(GetNextPrimaryAttack, "*", "m_flNextPrimaryAttack", float, *(float*));
	SetupNetVar(GetNextSecondaryAttack, "*", "m_flNextSecondaryAttack", float, *(float*));
	SetupNetVar(GetSimulationTime, "*", "m_flSimulationTime", float, *(float*));
	SetupNetVar(GetVelocity, "DT_LocalPlayerExclusive", "m_vecVelocity[0]", Vector, *(Vector*));
	SetupNetVar(GetTickBase, "DT_LocalPlayerExclusive", "m_nTickBase", int, *(int*));
	SetupNetVar(GetWaterLevel, "*", "m_nWaterLevel", int, *(int*));
	SetupNetVar(GetObserverMode, "DT_BasePlayer", "m_iObserverMode", int, *(int*));
	SetupNetVar(GetObserverTarget, "DT_BasePlayer", "m_hObserverTarget", HANDLE, *(HANDLE*));
	SetupNetVar(GetCollisionGroup, "DT_BaseEntity", "m_CollisionGroup", int, *(int*));
	SetupNetVar(GetColor, "DT_BaseEntity", "m_clrRender", ColorRGBExp32, *(ColorRGBExp32*));
	SetupNetVar(GetOverrideMaterial, "DT_BaseEntity", "m_OverrideMaterial", ColorRGBExp32, *(ColorRGBExp32*));

	inline QAngle &GetViewPunch()
	{
#ifndef _CSGO
		static int offset = g_pNetworkMan->GetOffset("DT_Local", "m_vecPunchAngle");
#else
		static int offset = g_pNetworkMan->GetOffset("DT_Local", "m_aimPunchAngle");
#endif
		static int offset2 = g_pNetworkMan->GetOffset("DT_LocalPlayerExclusive", "m_Local");

		return *(QAngle*)((char*)(this) + offset2 + offset);
	}

	inline Vector IEntity::GetEyePosition(float intpertick)
	{
		return  GetServerEyePosition() + GetVelocity() *intpertick;
	}

	inline Vector IEntity::GetServerEyePosition()
	{
		static int offset = g_pNetworkMan->GetOffset("*", "m_vecViewOffset[0]");
		Vector m_vecViewOffset = *(Vector*)((char*)(this) + offset);

		return GetOrigin() + m_vecViewOffset;
	}

	inline bool IEntity::IsAlive()
	{

		if (GetHealth() < 1)
			return false;

		if (GetHealth() > 0xFFFFFFF)
			return false;

		if (GetLifeState() != 0)
			return false;

		return true;
	}

#ifdef _L4D2
	inline bool IEntity::IsAlive_Zomb()
	{
		if (ent->GetSolid() & 4)
			return false;
		
		if (norm && ent->GetSequence() > 300)
			return false;

		return true;
	}
#endif

	inline mstudiohitboxset* GetHBoxSet()
	{
		int index = EntIndex();
		if (!index)
			return 0;

		return ientity__hboxsets[index];
	}

	inline matrix3x4* GetMatrix()
	{
		int index = EntIndex();
		if (!index)
			return 0;

		return ientity__matrixs[index];
	}

	bool UpdateBones();

	bool HasFlag(int flag);

#ifdef _TF2
	bool HasCond(int cond);
#endif
	bool SetupBones();

	Vector GetAbsVelocity();

	const char* GetClassname();

	int GetActiveWeaponIndex();
	
	IEntity *GetActiveWeapon();

	bool CanShoot1(bool checkAmmo=true, bool checkCanShoot=true);
	bool CanShoot2(bool checkAmmo=true, bool checkCanShoot=true);

	bool IsSpawnProtected(int type);
};

extern IEntity *g_pLocalEnt;
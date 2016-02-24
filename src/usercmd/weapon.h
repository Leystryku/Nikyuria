#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

class m_Weapon
{
	QAngle bakview;

public:

	void AutoPistol(CUserCmd *pCmd, bool safemode, bool AltAutoPistol);
	void AutoReload(CUserCmd *pCmd);
	void NoRecoil(CUserCmd *pCmd, bool norecoil_vis);
	void NoSpread(CUserCmd *pCmd);
	void NoSpread_Seed(CUserCmd* pCmd, bool safemode);
	
	__forceinline void DoThink(CUserCmd *pCmd, bool* bSendPacket, bool aimbot, bool noshake, bool safemode, bool autopist, bool altautopist, bool autoreload, bool norecoil, bool norecoil_vis, bool no_spread, bool nospread_seed)
	{

		if (autopist || altautopist)
		{
			AutoPistol(pCmd, safemode, altautopist);
		}

		if (autoreload)
		{
			AutoReload(pCmd);
		}

		if (!g_pLocalEnt->GetActiveWeapon())
			return;

		if (!g_pLocalEnt->GetActiveWeapon()->CanShoot1(true, true))
		{
			if (noshake&&!aimbot&&!bakview.IsZero())
			{
				pCmd->viewangles = bakview;
				*bSendPacket = true;
				bakview.Zero();
				
			}

			return;
		}

		if (!(pCmd->buttons&IN_ATTACK))
			return;

		if (noshake)
		{
			*bSendPacket = false;
			bakview = pCmd->viewangles;
		}

		if (norecoil)
		{
			NoRecoil(pCmd, norecoil_vis);
		}

		if (nospread_seed)
		{
			NoSpread_Seed(pCmd, safemode);
		}

		if (no_spread)
		{
			NoSpread(pCmd);
		}

	}
};

extern m_Weapon *Weapon;
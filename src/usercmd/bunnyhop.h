#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

class m_BunnyHop
{
public:

	__forceinline void DoThink(CUserCmd *pCmd, bool bSafeJump, bool bShouldHop, bool bStrafe)
	{

		static float move = pCmd->move.x;// value from cl_forwardmove
		float s_move = move * 0.5065f;

		static bool even = false;
		even = !even;

		if (bStrafe)
		{
			if (((pCmd->buttons & IN_JUMP) || !g_pLocalEnt->HasFlag(FL_ONGROUND)) && g_pLocalEnt->GetWaterLevel() < 2)
			{
				pCmd->move.x = move * 0.015f;
				pCmd->move.y += (float)(((pCmd->tick_count % 2) * 2) - 1) * s_move;

				if (pCmd->mousex)
					pCmd->move.y = (float)utils::maths::clamp(pCmd->mousex, -1, 1) * s_move;

				static float strafe = pCmd->viewangles.y;

				float rt = pCmd->viewangles.y, rotation;
				rotation = strafe - rt;

				if (rotation < FloatNegate(g_pGlobals->interval_per_tick))
					pCmd->move.y = -s_move;

				if (rotation > g_pGlobals->interval_per_tick)
					pCmd->move.y = s_move;

				strafe = rt;
			}

			if (!(pCmd->buttons & IN_JUMP) && g_pLocalEnt->HasFlag(FL_ONGROUND))
			{
				if (((pCmd->buttons & IN_FORWARD) && (pCmd->buttons & IN_MOVELEFT)) || ((pCmd->buttons & IN_BACK) && (pCmd->buttons & IN_MOVERIGHT)))
					if (even)
					{
						pCmd->move.y += s_move;
						pCmd->move.x += s_move;
					}
					else
					{
						pCmd->move.y -= s_move;
						pCmd->move.x -= s_move;
					}
				else if (((pCmd->buttons & IN_FORWARD) && (pCmd->buttons & IN_MOVERIGHT)) || ((pCmd->buttons & IN_BACK) && (pCmd->buttons & IN_MOVELEFT)))
					if (even)
					{
						pCmd->move.y += s_move;
						pCmd->move.x -= s_move;
					}
					else
					{
						pCmd->move.y -= s_move;
						pCmd->move.x += s_move;
					}
				else if ((pCmd->buttons & IN_FORWARD) || (pCmd->buttons & IN_BACK))
					if (even)pCmd->move.y += s_move;
					else pCmd->move.y -= s_move;
				else if ((pCmd->buttons & IN_MOVELEFT) || (pCmd->buttons & IN_MOVERIGHT))
					if (even)pCmd->move.x += s_move;
					else
						pCmd->move.x -= s_move;
			}


		}

		if (bShouldHop)
		{
			if (bSafeJump)
			{
				static bool firstjump = false, fakejmp;

				if ((pCmd->buttons & IN_JUMP) && g_pLocalEnt->GetWaterLevel() < 2)
					if (!firstjump)
						firstjump = fakejmp = true;
					else if (!g_pLocalEnt->HasFlag(FL_ONGROUND) && !g_pLocalEnt->HasFlag(FL_PARTIALGROUND))
						if (fakejmp && g_pLocalEnt->GetVelocity().z < 0.0f)
							fakejmp = false;
						else
							pCmd->buttons &= ~IN_JUMP;
					else
						fakejmp = true;
				else
					firstjump = false;
				return;
			}

			if ((pCmd->buttons & IN_JUMP) && !g_pLocalEnt->HasFlag(FL_ONGROUND))
				pCmd->buttons &= ~IN_JUMP;
		}

	}

};

extern m_BunnyHop *BunnyHop;
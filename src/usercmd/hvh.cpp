#pragma once

#include "hvh.h"

m_HvH *HvH = new m_HvH;

void HvH_FixMove(CUserCmd *cmd, QAngle& va)
{
	float yaw, speed;
	Vector& move = *(Vector*)&cmd->move[0];
	Vector move2 = *(Vector*)&cmd->move;

	speed = move2.Length2D();

	QAngle view = cmd->viewangles;
	NormalizeAngles(view);

	yaw = Rad2Deg(atan2(move.y, move.x));
	yaw = Deg2Rad(view.y - va.y + yaw);

	cmd->move[0] = cos(yaw) * speed;
	cmd->move[1] = sin(yaw) * speed;

	if (view.x < -90.f || view.x > 90.f)
	{
		cmd->move.x = FloatNegate(cos(FloatNegate(yaw)) * speed);
		cmd->move.y = FloatNegate(sin(FloatNegate(yaw)) * speed);
	}


}

extern bool Bots_Filter(IHandleEntity *handleent);

bool shouldswitch = 0;

void m_HvH::DoAntiAim(CUserCmd *pCmd, bool* bSendPacket)
{
	if ((pCmd->buttons & IN_ATTACK))
	{
		*bSendPacket = true;
		return;
	}
		

	QAngle oldView = pCmd->viewangles;


	shouldswitch = !shouldswitch;

	if (shouldswitch)
	{
		pCmd->viewangles.x = -181;
		pCmd->viewangles.y = 1100;

		HvH_FixMove(pCmd, oldView);
		*bSendPacket = false;
	}else{
		pCmd->viewangles.x = -1815;
		pCmd->viewangles.y = -1100;

		HvH_FixMove(pCmd, oldView);

		*bSendPacket = true;
	}


}

bool bLagSwitch = false;
int lasttick = 0;

void m_HvH::DoFakeLag(CUserCmd *pCmd, bool* bSendPacket, int fakelag, int fakelag2)
{
	if ((pCmd->buttons & IN_ATTACK))
	{
		lasttick = 0;
		return;
	}

	bool bLagSwitch = true;

	if (lasttick < g_pGlobals->tick_count)
	{
		bLagSwitch = false;

		if(lasttick<g_pGlobals->tick_count+fakelag2)
			lasttick = g_pGlobals->tick_count + fakelag;
	}
	

	if (bLagSwitch)
		*bSendPacket = false;
	else
		*bSendPacket = true;

}


void m_HvH::DoFakeCrouch(CUserCmd *pCmd, bool* bSendPacket)
{
	if ((pCmd->buttons & IN_ATTACK) && g_pLocalEnt->CanShoot1())
	{
		*bSendPacket = true;
		return;
	}

#if defined(_CSS) || defined(_CSGO)
	if (pCmd->tick_count % 2 == 0)
	{
		pCmd->buttons  |= IN_DUCK;
		*bSendPacket = 0;
	}

	return;

#endif
	
	static bool bDuck = false;

	static int waittime = 0;
	
	if (waittime)
	{
		waittime--;
		return;
	}
	bDuck = !bDuck;

	if (bDuck)
	{
		pCmd->buttons |= IN_DUCK;
		*bSendPacket = false;
		waittime = 15;
	}
	else{
		pCmd->buttons &= ~IN_DUCK;
		*bSendPacket = true;
	}

}


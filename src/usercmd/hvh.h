#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

class m_HvH
{
public:
	void DoAntiAim(CUserCmd *pCmd, bool* bSendPacket);
	void DoFakeLag(CUserCmd *pCmd, bool* bSendPacket, int fakelag, int fakelag2);
	void DoFakeCrouch(CUserCmd *pCmd, bool* bSendPacket);

	__forceinline void DoThink(CUserCmd *pCmd, bool* bSendPacket, bool antiaim, bool antiaim_proxy, bool fakecrouch, int fakelag, int fakelag2)
	{

		if (antiaim)
			DoAntiAim(pCmd, bSendPacket);

		if (fakelag)
			DoFakeLag(pCmd, bSendPacket, fakelag, fakelag2);

		if (fakecrouch)
			DoFakeCrouch(pCmd, bSendPacket);
	}

};

extern m_HvH *HvH;
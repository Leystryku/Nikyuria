#pragma once

#include "..\stdafx.h"
#include "..\sdk\sdk.h"
#include "..\global.h"

class m_Chat
{
public:

#ifdef _GMOD
	void DoThink(CUserCmd *pCmd, std::string SpamText, bool TeamText, bool TextImitator, bool TextSpammer, bool TextJammer, bool ULXText);
#else
	void DoThink(CUserCmd *pCmd, std::string SpamText, bool TeamText, bool TextImitator, bool TextSpammer, bool TextJammer);
#endif
	
};

extern m_Chat *Chat;
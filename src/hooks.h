#pragma once

#include "stdafx.h"
#include "utils.h"
#include "sdk/sdk.h"
#include "overlay.h"
#include "d3d.h"
#include "menu.h"
#include "vmthook.h"

class m_hooks
{
public:
	std::string HookStuff();

};

extern m_hooks *hooks;
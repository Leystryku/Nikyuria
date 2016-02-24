#pragma once

#include "stdafx.h"
#include "utils.h"
#include "sdk/sdk.h"
#include "overlay.h"
#include "d3d.h"
#include "menu.h"
#include "vmthook.h"
#include "hooks.h"

namespace Nikyuria
{
	int Init();
	int Detach();

	bool Initiated;
};

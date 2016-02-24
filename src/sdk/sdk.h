#pragma once

#include "../stdafx.h"

//GENERIC STUFF//
#include "color.h"
#include "externfuncs.h"

//INCLUDE ENGINE FILES
#include "cusercmd.h"
#include "engine.h"
#include "client.h"
#include "entlist.h"
#include "enginetrace.h"
#include "modelinfo.h"
#include "pred.h"
#include "globals.h"
#include "inetchannel.h"
#include "dbg.h"
#include "ivoicecodec.h"

#ifdef _GMOD
#include "lua.h"
#endif

class m_sdk
{
public:

	std::string Init();

	HMODULE module_engine = NULL;
	HMODULE module_client = NULL;
	HMODULE module_vstd = NULL;

	void *factory_engine = NULL;
	void *factory_client = NULL;
	void *factory_vstd = NULL;
#ifdef _GMOD
	void *factory_luashared = NULL;
	void *vluashared = NULL;
#endif

	void *vengine = NULL;
	void *vclient = NULL;
	void *ventlist = NULL;
	void *vmodelinfo = NULL;
	void *venginetrace = NULL;
	void *vprediction = NULL;
	void *vmovement = NULL;
	IVoiceCodec *vencodecodec = NULL;

	matrix4x4 ViewMatrix;
};

extern m_sdk *sdk;
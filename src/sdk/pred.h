#pragma once

#include "interface.h"
#include "vector.h"
#include "clientclass.h"
#include "cusercmd.h"


// ======================= INDEXES =========================== //

#define INDEX__PRED_INPREDICTION 14
#define INDEX__PRED_RUNCOMMAND 17
#define INDEX__PRED_SETUPMOVE 18
#define INDEX__PRED_FINISHMOVE 19

#ifdef _CSGO

#undef INDEX__PRED_RUNCOMMAND
#undef INDEX__PRED_SETUPMOVE
#undef INDEX__PRED_FINISHMOVE

#define INDEX__PRED_RUNCOMMAND 19
#define INDEX__PRED_SETUPMOVE 20
#define INDEX__PRED_FINISHMOVE 21

#endif



// ======================= CLASS HEADER ======================= //


SETUP_INTERFACE(IPrediction);

	SetupInterfaceFuncFC(InPrediction, bool, INDEX__PRED_INPREDICTION, (void* __this, int edx), (void* __this, int edx), (__this, edx));

	SetupInterfaceFunc(RunCommand, void, INDEX__PRED_RUNCOMMAND, (void* __this, IEntity* ent, CUserCmd* cmd, void* mv), (void* __this, IEntity* ent, CUserCmd* cmd, void* mv), (__this, ent, cmd, mv));

	SetupInterfaceFunc(SetupMove, void, INDEX__PRED_SETUPMOVE, (void* ent, void* cmd, void* helper, void* movedata), (void* __this, void* ent, void* cmd, void* helper, void* movedata), (this->vmt, ent, cmd, helper, movedata));

	SetupInterfaceFunc(FinishMove, void, INDEX__PRED_FINISHMOVE, (void* ent, void* cmd, void* movedata), (void* __this, void* ent, void* cmd, void* movedata), (this->vmt, ent, cmd, movedata));
	
	inline bool DoSmartPred(IEntity* ent, CUserCmd *pCmd, void*helper, void*vmovement, void* vprediction)
	{
		static void* RunCommandAddr = utils::memory::GetVirtualMethod(vprediction, INDEX__PRED_RUNCOMMAND);
		static void* addr = utils::memory::FindSig(RunCommandAddr, "\x89?????\xE8", 500, 2);

		if (!addr)
		{
			Msg("No addr!\n");
			return false;
		}


		memset(predmd, 0, 255);


		CUserCmd **predcmd = (CUserCmd **)((char*)ent + *(int*)addr);
		if (!predcmd)
			return false;

		if (!*predcmd)
			return false;

		*predcmd = pCmd;

		typedef void(__thiscall* ProcessMovementFn)(void* __this, IEntity*ent, void* movedata);

		SetupMove(ent, pCmd, 0, predmd);
		utils::memory::GetVirtualFunc<ProcessMovementFn>(vmovement, 1)(vmovement, ent, predmd);
		FinishMove(ent, pCmd, predmd);

		return true;
	}

public:
	char predmd[255];
	



};

extern IPrediction *g_pPrediction;
#pragma once

#include "interface.h"

// ======================= INDEXES =========================== //

#define INDEX_MODELINFO_GETSTUDIOMODEL 28
#define INDEX_MODELINFO_GETMODELNAME 3

#ifdef _CSGO
#undef INDEX_MODELINFO_GETSTUDIOMODEL
#define INDEX_MODELINFO_GETSTUDIOMODEL 30
#endif

// ======================= CLASS HEADER ======================= //

struct model_t;

SETUP_INTERFACE(IModelInfo);

	SetupInterfaceFunc(GetModelName, const char*, INDEX_MODELINFO_GETMODELNAME, (const model_t *mdl), (void* __this, const model_t *mdl), (this->vmt, mdl));

	SetupInterfaceFunc(GetStudioModel, void*, INDEX_MODELINFO_GETSTUDIOMODEL, (const model_t *mdl), (void* __this, const model_t *mdl), (this->vmt, mdl));


};

extern IModelInfo *g_pModelInfo;
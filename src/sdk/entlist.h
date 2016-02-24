#pragma once

#include "interface.h"

// ======================= INDEXES =========================== //

#define INDEX_ENTLIST_GETCLIENTENTITY 3
#define INDEX_ENTLIST_GETHIGHESTENTITYINDEX 6


// ======================= CLASS HEADER ======================= //

class IEntity;

SETUP_INTERFACE(IEntList);
	
	SetupInterfaceFunc(GetClientEntity, IEntity*, INDEX_ENTLIST_GETCLIENTENTITY, (int ent_num), (void* __this, int ent_num), (this->vmt, ent_num));
	
	SetupInterfaceFunc(GetHighestEntityIndex, int, INDEX_ENTLIST_GETHIGHESTENTITYINDEX, (), (void* __this), (this->vmt));

};

extern IEntList *g_pEntList;
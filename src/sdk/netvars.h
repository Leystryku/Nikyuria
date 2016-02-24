#pragma once

#include "../stdafx.h"
#include "../utils.h"
#include "clientclass.h"
#include "client.h"

class CNetVarMan
{
public:

    int GetOffset( const char *tableName, const char *propName );
 
 
	bool HookProp(const char *tableName, const char *propName, void* proxy);
private:
 
	RecvProp *RetProp(RecvTable *tbl, const char *propName, void*proxy = 0);

	RecvTable* FindTable(const char* tableName, const char* propName);

	RecvProp* FindPropInTable(const char *tblname, const char *propName, void* proxy=0);

	
};


extern CNetVarMan *g_pNetworkMan;
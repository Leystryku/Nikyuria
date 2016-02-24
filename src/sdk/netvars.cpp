#pragma once

#include "netvars.h"

CNetVarMan *g_pNetworkMan;

void *exposedTables[999999];
int iexposedTables = 0;

bool bExposedTables = false;

void ExposeTables2(RecvTable *recvTable)
{
	exposedTables[iexposedTables++] = recvTable;

	for (int i = 0; i < recvTable->m_nProps; i++)
	{
		RecvProp* prop = &recvTable->m_pProps[i];

		if (!prop->m_pDataTable)
			continue;

		ExposeTables2(prop->m_pDataTable);
	}

}

void ExposeTables()
{
	ClientClass *clientClass = g_pClient->GetAllClasses();

	if (!clientClass)
	{
		utils::file::WriteToLog("ClientClass was not found\n");
		return;
	}

	for (; clientClass; clientClass = clientClass->m_pNext)
	{
		RecvTable *recvTable = clientClass->m_pRecvTable;

		if (!recvTable)
			continue;

		ExposeTables2(recvTable);
	}

	bExposedTables = true;
}

RecvProp *CNetVarMan::RetProp(RecvTable *tbl, const char *propName, void*proxy)
{

	RecvProp*proxyprop = 0;

	for (int i = 0; i < tbl->m_nProps; i++)
	{
		RecvProp* prop = &tbl->m_pProps[i];

		if (prop->m_pDataTable)
		{
			RecvProp *ret = RetProp(prop->m_pDataTable, propName, proxy);
			if (!proxy && ret)
				return ret;
		}
		if (!strcmp(propName, prop->m_pVarName))
		{
			if (proxy)
			{
				prop->m_ProxyFn = proxy;
				proxyprop = prop;
			}
				
			else
				return prop;
		}
			

	}

	return proxyprop;
}

RecvProp* CNetVarMan::FindPropInTable(const char *tableName, const char *propName, void*proxy)
{

	for (int i = 0; i < iexposedTables; i++)
	{
		RecvTable *tbl = (RecvTable*)exposedTables[i];

		if (!tableName || strstr(tableName, tbl->m_pNetTableName))
		{

			RecvProp* found = RetProp(tbl, propName, proxy);
			if (found)
				return found;
		}
	}

	return NULL;
}

int CNetVarMan::GetOffset(const char *tableName, const char *propName)
{
	if (!bExposedTables)
		ExposeTables();

	ClientClass *clientClass = g_pClient->GetAllClasses();

	if (!clientClass)
	{
		utils::file::WriteToLog("ClientClass was not found\n");
		return false;
	}

	RecvProp* prop;

	if (!strcmp(tableName, "*"))
		prop = FindPropInTable(NULL, propName);
	else
		prop = FindPropInTable(tableName, propName);
	
	if (prop)
	{
		return prop->m_Offset;
	}

	utils::file::WriteToLog("Failed to find offset for prop: %s from table: %s\n", propName, tableName);

	return NULL;
}



bool CNetVarMan::HookProp(const char *tableName, const char *propName, void* proxy)
{
	if (!bExposedTables)
		ExposeTables();

	ClientClass *clientClass = g_pClient->GetAllClasses();

	if (!clientClass)
	{
		utils::file::WriteToLog("ClientClass was not found\n");
		return false;
	}


	RecvProp* prop;

	if (!strcmp(tableName, "*"))
		prop = FindPropInTable(NULL, propName, proxy);
	else
		prop = FindPropInTable(tableName, propName, proxy);

	if (prop)
	{
		prop->m_ProxyFn = proxy;
		return true;
	}

	utils::file::WriteToLog("Failed to hook prop: %s from table: %s\n", propName, tableName);

	return false;
}
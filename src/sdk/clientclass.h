#pragma once

#include "../stdafx.h"

class RecvTable;
class RecvProp;

class DVariant // class shamelessly rip'd from nanoheckle 2.o
{
public:
	union
	{
		float	m_Float;
		long	m_Int;
		char*	m_pString;
		void*	m_pData;
		float	m_Vector[3];
	};

	int			m_Type;
};

class CRecvProxyData
{
public:
	const RecvProp	*m_pRecvProp;		// The property it's receiving.

	DVariant		m_Value;			// The value given to you to store.

	int				m_iElement;			// Which array element you're getting.

	int				m_ObjectID;			// The object being referred to.
};

class RecvProp
{
public:
	char*			m_pVarName;
	int				m_RecvType;
	int				m_Flags;
	int				m_StringBufferSize;
	bool			m_bInsideArray;
	void*			m_pExtraData;
	RecvProp*		m_pArrayProp;
	void*			m_ArrayLengthProxy;
	void*			m_ProxyFn;
	void*			m_DataTableProxyFn;
	RecvTable*		m_pDataTable;
	int				m_Offset;
	int				m_ElementStride;
	int				m_nElements;
	const char*		m_pParentArrayPropName;
};

class RecvTable
{
public:
	RecvProp		*m_pProps;
	int				m_nProps;
	void*	*m_pDecoder; // CRecvDecoder
	const char		*m_pNetTableName;
	bool			m_bInitialized;
	bool			m_bInMainList;
};

class ClientClass
{
public:
	void*penis;
	void*doublepenis;

	const char *m_pNetworkName;
	RecvTable *m_pRecvTable;
	ClientClass *m_pNext;
	int	m_ClassID;
};
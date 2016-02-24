#pragma once

#include "../stdafx.h"
#include "ientity.h"
#include "basehandle.h"

// ======================= INDEXES =========================== //

#define INDEX_ENGINETRACE_TRACERAY 4

#ifdef _CSGO
#undef INDEX_ENGINETRACE_TRACERAY
#define INDEX_ENGINETRACE_TRACERAY 5
#endif

// ======================= CLASS HEADER ======================= //

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IEntity for props through the filter, unlike all other filters
};

class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pEntityHandle, int contentsMask) = 0;
	virtual TraceType_t	GetTraceType() const = 0;

	bool(*CustomCollisionFunc)(IHandleEntity *pEntityHandle) = NULL;
};

class CTraceFilter : public ITraceFilter
{
public:

	bool ShouldHitEntity(IHandleEntity* pEntityHandle, int contentsMask)
	{
		if (CustomCollisionFunc)
		{
			return CustomCollisionFunc(pEntityHandle);
		}

		return false;
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

};

class CTraceFilterHitE : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pEntityHandle, int contentsMask)
	{
		if (CustomCollisionFunc)
		{
			return CustomCollisionFunc(pEntityHandle);
		}

		return false;
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

};


class CTraceFilterHitAll : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pEntityHandle, int contentsMask)
	{
		if (CustomCollisionFunc)
		{
			return CustomCollisionFunc(pEntityHandle);
		}

		return true;
	}

	virtual TraceType_t	GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}
};

static float MAX_TRACE_LENGTH = (float)(32768 * 1.732050807569);


struct trace_t
{
	Vector			startpos;
	Vector			endpos;
	Vector	normal;
	float	dist;
	BYTE	type;
	BYTE	signbits;
	BYTE	pad[2];
	float			fraction;
	int				contents;
	unsigned short	dispFlags;
	bool			allsolid;
	bool			startsolid;
	float		fractionleftsolid;
	const char	*name;
	short		surfaceProps;
	unsigned short	flags;
	int			hitgroup;
	short		physicsbone;
#ifdef _CSGO
	unsigned short	worldSurfaceIndex;	// Index of the msurface2_t, if applicable
#endif
	IEntity *m_pEnt;
	int			hitbox;

};

struct Ray_t
{
	VectorAligned  m_Start;
	VectorAligned  m_Delta;
	VectorAligned  m_StartOffset;
	VectorAligned  m_Extents;
#ifdef _CSGO
	const matrix3x4 *m_pWorldAxisTransform;
#endif
	bool	m_IsRay;
	bool	m_IsSwept;

#ifdef _CSGO
	Ray_t() : m_pWorldAxisTransform(NULL)	{}
#endif
	void Init(Vector const& start, Vector const& end)
	{
		VectorSubtract(end, start, m_Delta);

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.Zero();

#ifdef _CSGO
		m_pWorldAxisTransform = NULL;
#endif

		m_IsRay = true;

		// Offset m_Start to be in the center of the box...
		m_StartOffset.Zero();
		VectorCopy(start, m_Start);

	}

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs)
	{
		VectorSubtract(end, start, m_Delta);

#ifdef _CSGO
		m_pWorldAxisTransform = NULL;
#endif

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		VectorSubtract(maxs, mins, m_Extents);
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		// Offset m_Start to be in the center of the box...
		VectorAdd(mins, maxs, m_StartOffset);
		m_StartOffset *= 0.5f;
		VectorAdd(start, m_StartOffset, m_Start);
		m_StartOffset *= -1.0f;

#ifdef _CSGO
		m_pWorldAxisTransform = NULL;
#endif
	}

};

SETUP_INTERFACE(IEngineTrace);

	SetupInterfaceFunc(TraceRay, void, INDEX_ENGINETRACE_TRACERAY, (const Ray_t &ray, unsigned int fMask, ITraceFilter *filter, trace_t *trace), (void* __this, const Ray_t &ray, unsigned int fMask, ITraceFilter *filter, trace_t *trace), (this->vmt, ray, fMask, filter, trace));

};

extern IEngineTrace *g_pEngineTrace;
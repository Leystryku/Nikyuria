//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "buf.h"


#if _WIN32
#define FAST_BIT_SCAN 1
#if _X360
#define CountLeadingZeros(x) _CountLeadingZeros(x)
inline unsigned int CountTrailingZeros(unsigned int elem)
{
	// this implements CountTrailingZeros() / BitScanForward()
	unsigned int mask = elem - 1;
	unsigned int comp = ~elem;
	elem = mask & comp;
	return (32 - _CountLeadingZeros(elem));
}
#else
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

inline unsigned int CountLeadingZeros(unsigned int x)
{
	unsigned long firstBit;
	if (_BitScanReverse(&firstBit, x))
		return 31 - firstBit;
	return 32;
}
inline unsigned int CountTrailingZeros(unsigned int elem)
{
	unsigned long out;
	if (_BitScanForward(&out, elem))
		return out;
	return 32;
}

#endif
#else
#define FAST_BIT_SCAN 0
#endif


static BitBufErrorHandler g_BitBufErrorHandler = 0;

#define LOG2_BITS_PER_INT	5
#define BITS_PER_INT		32


inline unsigned GetEndMask(int numBits)
{
	static unsigned bitStringEndMasks[] =
	{
		0xffffffff,
		0x00000001,
		0x00000003,
		0x00000007,
		0x0000000f,
		0x0000001f,
		0x0000003f,
		0x0000007f,
		0x000000ff,
		0x000001ff,
		0x000003ff,
		0x000007ff,
		0x00000fff,
		0x00001fff,
		0x00003fff,
		0x00007fff,
		0x0000ffff,
		0x0001ffff,
		0x0003ffff,
		0x0007ffff,
		0x000fffff,
		0x001fffff,
		0x003fffff,
		0x007fffff,
		0x00ffffff,
		0x01ffffff,
		0x03ffffff,
		0x07ffffff,
		0x0fffffff,
		0x1fffffff,
		0x3fffffff,
		0x7fffffff,
	};

	return bitStringEndMasks[numBits % BITS_PER_INT];
}


inline int GetBitForBitnum(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
		(1 << 8),
		(1 << 9),
		(1 << 10),
		(1 << 11),
		(1 << 12),
		(1 << 13),
		(1 << 14),
		(1 << 15),
		(1 << 16),
		(1 << 17),
		(1 << 18),
		(1 << 19),
		(1 << 20),
		(1 << 21),
		(1 << 22),
		(1 << 23),
		(1 << 24),
		(1 << 25),
		(1 << 26),
		(1 << 27),
		(1 << 28),
		(1 << 29),
		(1 << 30),
		(1 << 31),
	};

	return bitsForBitnum[(bitNum)& (BITS_PER_INT - 1)];
}

inline int GetBitForBitnumByte(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
	};

	return bitsForBitnum[bitNum & 7];
}

inline int BitForBitnum(int bitnum)
{
	return GetBitForBitnum(bitnum);
}

void InternalBitBufErrorHandler(BitBufErrorType errorType, const char *pDebugName)
{
	if (g_BitBufErrorHandler)
		g_BitBufErrorHandler(errorType, pDebugName);
}


void SetBitBufErrorHandler(BitBufErrorHandler fn)
{
	g_BitBufErrorHandler = fn;
}


// #define BB_PROFILING


// Precalculated bit masks for WriteUBitLong. Using these tables instead of 
// doing the calculations gives a 33% speedup in WriteUBitLong.
unsigned long g_BitWriteMasks[32][33];

// (1 << i) - 1
unsigned long g_ExtraMasks[32];

class CBitWriteMasksInit
{
public:
	CBitWriteMasksInit()
	{
		for (unsigned int startbit = 0; startbit < 32; startbit++)
		{
			for (unsigned int nBitsLeft = 0; nBitsLeft < 33; nBitsLeft++)
			{
				unsigned int endbit = startbit + nBitsLeft;
				g_BitWriteMasks[startbit][nBitsLeft] = BitForBitnum(startbit) - 1;
				if (endbit < 32)
					g_BitWriteMasks[startbit][nBitsLeft] |= ~(BitForBitnum(endbit) - 1);
			}
		}

		for (unsigned int maskBit = 0; maskBit < 32; maskBit++)
			g_ExtraMasks[maskBit] = BitForBitnum(maskBit) - 1;
	}
};
CBitWriteMasksInit g_BitWriteMasksInit;


// ---------------------------------------------------------------------------------------- //
// old_bf_write
// ---------------------------------------------------------------------------------------- //

old_bf_write::old_bf_write()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; // set to -1 so we generate overflow on any operation
	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

old_bf_write::old_bf_write(const char *pDebugName, void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartWriting(pData, nBytes, 0, nBits);
}

old_bf_write::old_bf_write(void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
	StartWriting(pData, nBytes, 0, nBits);
}

void old_bf_write::StartWriting(void *pData, int nBytes, int iStartBit, int nBits)
{
	// Make sure it's dword aligned and padded.
	Assert((nBytes % 4) == 0);
	Assert(((unsigned long)pData & 3) == 0);

	// The writing code will overrun the end of the buffer if it isn't dword aligned, so truncate to force alignment
	nBytes &= ~3;

	m_pData = (unsigned char*)pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = nBytes << 3;
	}
	else
	{
		Assert(nBits <= nBytes * 8);
		m_nDataBits = nBits;
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

void old_bf_write::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}


void old_bf_write::SetAssertOnOverflow(bool bAssert)
{
	m_bAssertOnOverflow = bAssert;
}


const char* old_bf_write::GetDebugName()
{
	return m_pDebugName;
}


void old_bf_write::SetDebugName(const char *pDebugName)
{
	m_pDebugName = pDebugName;
}


void old_bf_write::SeekToBit(int bitPos)
{
	m_iCurBit = bitPos;
}


// Sign bit comes first
void old_bf_write::WriteSBitLong(int data, int numbits)
{
	// Do we have a valid # of bits to encode with?
	Assert(numbits >= 1);

	// Note: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	if (data < 0)
	{
#ifdef _DEBUG
		if (numbits < 32)
		{
			// Make sure it doesn't overflow.

			if (data < 0)
			{
				Assert(data >= -(BitForBitnum(numbits - 1)));
			}
			else
			{
				Assert(data < (BitForBitnum(numbits - 1)));
			}
		}
#endif

		WriteUBitLong((unsigned int)(0x80000000 + data), numbits - 1, false);
		WriteOneBit(1);
	}
	else
	{
		WriteUBitLong((unsigned int)data, numbits - 1);
		WriteOneBit(0);
	}
}

#if _WIN32
inline unsigned int BitCountNeededToEncode(unsigned int data)
{
#if defined(_X360)
	return (32 - CountLeadingZeros(data + 1)) - 1;
#else
	unsigned long firstBit;
	_BitScanReverse(&firstBit, data + 1);
	return firstBit;
#endif
}
#endif	// _WIN32

// writes an unsigned integer with variable bit length
void old_bf_write::WriteUBitVar(unsigned int data)
{
	if ((data & 0xf) == data)
	{
		WriteUBitLong(0, 2);
		WriteUBitLong(data, 4);
	}
	else
	{
		if ((data & 0xff) == data)
		{
			WriteUBitLong(1, 2);
			WriteUBitLong(data, 8);
		}
		else
		{
			if ((data & 0xfff) == data)
			{
				WriteUBitLong(2, 2);
				WriteUBitLong(data, 12);
			}
			else
			{
				WriteUBitLong(0x3, 2);
				WriteUBitLong(data, 32);
			}
		}
	}
#if 0
#if !FAST_BIT_SCAN
	unsigned int bits = 0;
	unsigned int base = 0;

	while (data > (base << 1))
	{
		bits++;
		base = BitForBitnum(bits) - 1;
	}
#else
	unsigned int bits = BitCountNeededToEncode(data);
	unsigned int base = GetBitForBitnum(bits) - 1;
#endif

	// how many bits do we use
	WriteUBitLong(0, bits);

	// end marker
	WriteOneBit(1);

	// write the value
	if (bits > 0)
		WriteUBitLong(data - base, bits);
#endif
}

void old_bf_write::WriteBitLong(unsigned int data, int numbits, bool bSigned)
{
	if (bSigned)
		WriteSBitLong((int)data, numbits);
	else
		WriteUBitLong(data, numbits);
}

bool old_bf_write::WriteBits(const void *pInData, int nBits)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::WriteBits");
#endif

	unsigned char *pOut = (unsigned char*)pInData;
	int nBitsLeft = nBits;

	// Bounds checking..
	if ((m_iCurBit + nBits) > m_nDataBits)
	{
		SetOverflowFlag();
		CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, GetDebugName());
		return false;
	}

	// Align output to dword boundary
	while (((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8)
	{

		WriteUBitLong(*pOut, 8, false);
		++pOut;
		nBitsLeft -= 8;
	}

	if (IsPC() && (nBitsLeft >= 32) && (m_iCurBit & 7) == 0)
	{
		// current bit is byte aligned, do block copy
		int numbytes = nBitsLeft >> 3;
		int numbits = numbytes << 3;

		Q_memcpy(m_pData + (m_iCurBit >> 3), pOut, numbytes);
		pOut += numbytes;
		nBitsLeft -= numbits;
		m_iCurBit += numbits;
	}

	// X360TBD: Can't write dwords in WriteBits because they'll get swapped
	if (IsPC() && nBitsLeft >= 32)
	{
		unsigned long iBitsRight = (m_iCurBit & 31);
		unsigned long iBitsLeft = 32 - iBitsRight;
		unsigned long bitMaskLeft = g_BitWriteMasks[iBitsRight][32];
		unsigned long bitMaskRight = g_BitWriteMasks[0][iBitsRight];

		unsigned long *pData = &((unsigned long*)m_pData)[m_iCurBit >> 5];

		// Read dwords.
		while (nBitsLeft >= 32)
		{
			unsigned long curData = *(unsigned long*)pOut;
			pOut += sizeof(unsigned long);

			*pData &= bitMaskLeft;
			*pData |= curData << iBitsRight;

			pData++;

			if (iBitsLeft < 32)
			{
				curData >>= iBitsLeft;
				*pData &= bitMaskRight;
				*pData |= curData;
			}

			nBitsLeft -= 32;
			m_iCurBit += 32;
		}
	}


	// write remaining bytes
	while (nBitsLeft >= 8)
	{
		WriteUBitLong(*pOut, 8, false);
		++pOut;
		nBitsLeft -= 8;
	}

	// write remaining bits
	if (nBitsLeft)
	{
		WriteUBitLong(*pOut, nBitsLeft, false);
	}

	return !IsOverflowed();
}


bool old_bf_write::WriteBitsFromBuffer(bf_read *pIn, int nBits)
{
	// This could be optimized a little by
	while (nBits > 32)
	{
		WriteUBitLong(pIn->ReadUBitLong(32), 32);
		nBits -= 32;
	}

	WriteUBitLong(pIn->ReadUBitLong(nBits), nBits);
	return !IsOverflowed() && !pIn->IsOverflowed();
}


void old_bf_write::WriteBitAngle(float fAngle, int numbits)
{
	int d;
	unsigned int mask;
	unsigned int shift;

	shift = BitForBitnum(numbits);
	mask = shift - 1;

	d = (int)((fAngle / 360.0) * shift);
	d &= mask;

	WriteUBitLong((unsigned int)d, numbits);
}

void old_bf_write::WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::WriteBitCoordMP");
#endif
	int		signbit = (f <= -(bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION));
	int		intval = (int)abs(f);
	int		fractval = bLowPrecision ?
		(abs((int)(f*COORD_DENOMINATOR_LOWPRECISION)) & (COORD_DENOMINATOR_LOWPRECISION - 1)) :
		(abs((int)(f*COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1));


	bool    bInBounds = intval < (1 << COORD_INTEGER_BITS_MP);

	WriteOneBit(bInBounds);

	if (bIntegral)
	{
		// Send the sign bit
		WriteOneBit(intval);
		if (intval)
		{
			WriteOneBit(signbit);
			// Send the integer if we have one.
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			if (bInBounds)
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS_MP);
			}
			else
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
			}
		}
	}
	else
	{
		// Send the bit flags that indicate whether we have an integer part and/or a fraction part.
		WriteOneBit(intval);
		// Send the sign bit
		WriteOneBit(signbit);

		if (intval)
		{
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			if (bInBounds)
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS_MP);
			}
			else
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
			}
		}
		WriteUBitLong((unsigned int)fractval, bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
	}
}

void old_bf_write::WriteBitCoord(const float f)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::WriteBitCoord");
#endif
	int		signbit = (f <= -COORD_RESOLUTION);
	int		intval = (int)abs(f);
	int		fractval = abs((int)(f*COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1);


	// Send the bit flags that indicate whether we have an integer part and/or a fraction part.
	WriteOneBit(intval);
	WriteOneBit(fractval);

	if (intval || fractval)
	{
		// Send the sign bit
		WriteOneBit(signbit);

		// Send the integer if we have one.
		if (intval)
		{
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
		}

		// Send the fraction if we have one
		if (fractval)
		{
			WriteUBitLong((unsigned int)fractval, COORD_FRACTIONAL_BITS);
		}
	}
}

void old_bf_write::WriteBitFloat(float val)
{
	long intVal;

	Assert(sizeof(long) == sizeof(float));
	Assert(sizeof(float) == 4);

	intVal = *((long*)&val);
	WriteUBitLong(intVal, 32);
}

void old_bf_write::WriteBitVec3Coord(const Vector& fa)
{
	int		xflag, yflag, zflag;

	xflag = (fa.x >= COORD_RESOLUTION) || (fa.x <= -COORD_RESOLUTION);
	yflag = (fa.y >= COORD_RESOLUTION) || (fa.y <= -COORD_RESOLUTION);
	zflag = (fa.z >= COORD_RESOLUTION) || (fa.z <= -COORD_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);
	WriteOneBit(zflag);

	if (xflag)
		WriteBitCoord(fa.x);
	if (yflag)
		WriteBitCoord(fa.y);
	if (zflag)
		WriteBitCoord(fa.z);
}

void old_bf_write::WriteBitNormal(float f)
{
	int	signbit = (f <= -NORMAL_RESOLUTION);

	// NOTE: Since +/-1 are valid values for a normal, I'm going to encode that as all ones
	unsigned int fractval = abs((int)(f*NORMAL_DENOMINATOR));

	// clamp..
	if (fractval > NORMAL_DENOMINATOR)
		fractval = NORMAL_DENOMINATOR;

	// Send the sign bit
	WriteOneBit(signbit);

	// Send the fractional component
	WriteUBitLong(fractval, NORMAL_FRACTIONAL_BITS);
}

void old_bf_write::WriteBitVec3Normal(const Vector& fa)
{
	int		xflag, yflag;

	xflag = (fa.x >= NORMAL_RESOLUTION) || (fa.x <= -NORMAL_RESOLUTION);
	yflag = (fa.y >= NORMAL_RESOLUTION) || (fa.y <= -NORMAL_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);

	if (xflag)
		WriteBitNormal(fa.x);
	if (yflag)
		WriteBitNormal(fa.y);

	// Write z sign bit
	int	signbit = (fa.z <= -NORMAL_RESOLUTION);
	WriteOneBit(signbit);
}

void old_bf_write::WriteBitAngles(const QAngle& fa)
{
	// FIXME:
	Vector tmp(fa.x, fa.y, fa.z);
	WriteBitVec3Coord(tmp);
}

void old_bf_write::WriteChar(int val)
{
	WriteSBitLong(val, sizeof(char) << 3);
}

void old_bf_write::WriteByte(int val)
{
	WriteUBitLong(val, sizeof(unsigned char) << 3);
}

void old_bf_write::WriteShort(int val)
{
	WriteSBitLong(val, sizeof(short) << 3);
}

void old_bf_write::WriteWord(int val)
{
	WriteUBitLong(val, sizeof(unsigned short) << 3);
}

void old_bf_write::WriteLong(long val)
{
	WriteSBitLong(val, sizeof(long) << 3);
}

void old_bf_write::WriteLongLong(int64 val)
{
	uint *pLongs = (uint*)&val;

	// Insert the two DWORDS according to network endian
	const short endianIndex = 0x0100;
	byte *idx = (byte*)&endianIndex;
	WriteUBitLong(pLongs[*idx++], sizeof(long) << 3);
	WriteUBitLong(pLongs[*idx], sizeof(long) << 3);
}

void old_bf_write::WriteFloat(float val)
{
	// Pre-swap the float, since WriteBits writes raw data
	LittleFloat(&val, &val);

	WriteBits(&val, sizeof(val) << 3);
}

bool old_bf_write::WriteBytes(const void *pBuf, int nBytes)
{
	return WriteBits(pBuf, nBytes << 3);
}

bool old_bf_write::WriteString(const char *pStr)
{
	if (pStr)
	{
		do
		{
			WriteChar(*pStr);
			++pStr;
		} while (*(pStr - 1) != 0);
	}
	else
	{
		WriteChar(0);
	}

	return !IsOverflowed();
}

// ---------------------------------------------------------------------------------------- //
// old_bf_read
// ---------------------------------------------------------------------------------------- //

old_bf_read::old_bf_read()
{
	m_pData = NULL;
	m_nDataBytes = 0;
	m_nDataBits = -1; // set to -1 so we overflow on any operation
	m_iCurBit = 0;
	m_bOverflow = false;
	m_bAssertOnOverflow = true;
	m_pDebugName = NULL;
}

old_bf_read::old_bf_read(const void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	StartReading(pData, nBytes, 0, nBits);
}

old_bf_read::old_bf_read(const char *pDebugName, const void *pData, int nBytes, int nBits)
{
	m_bAssertOnOverflow = true;
	m_pDebugName = pDebugName;
	StartReading(pData, nBytes, 0, nBits);
}

void old_bf_read::StartReading(const void *pData, int nBytes, int iStartBit, int nBits)
{
	// Make sure we're dword aligned.
	Assert(((unsigned long)pData & 3) == 0);

	m_pData = (unsigned char*)pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = m_nDataBytes << 3;
	}
	else
	{
		Assert(nBits <= nBytes * 8);
		m_nDataBits = nBits;
	}

	m_iCurBit = iStartBit;
	m_bOverflow = false;
}

void old_bf_read::Reset()
{
	m_iCurBit = 0;
	m_bOverflow = false;
}

void old_bf_read::SetAssertOnOverflow(bool bAssert)
{
	m_bAssertOnOverflow = bAssert;
}

const char* old_bf_read::GetDebugName()
{
	return m_pDebugName;
}

void old_bf_read::SetDebugName(const char *pName)
{
	m_pDebugName = pName;
}

unsigned int old_bf_read::CheckReadUBitLong(int numbits)
{
	// Ok, just read bits out.
	int i, nBitValue;
	unsigned int r = 0;

	for (i = 0; i < numbits; i++)
	{
		nBitValue = ReadOneBitNoCheck();
		r |= nBitValue << i;
	}
	m_iCurBit -= numbits;

	return r;
}

void old_bf_read::ReadBits(void *pOutData, int nBits)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::ReadBits");
#endif

	unsigned char *pOut = (unsigned char*)pOutData;
	int nBitsLeft = nBits;


	// align output to dword boundary
	while (((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		*pOut = (unsigned char)ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// X360TBD: Can't read dwords in ReadBits because they'll get swapped
	if (IsPC())
	{
		// read dwords
		while (nBitsLeft >= 32)
		{
			*((unsigned long*)pOut) = ReadUBitLong(32);
			pOut += sizeof(unsigned long);
			nBitsLeft -= 32;
		}
	}

	// read remaining bytes
	while (nBitsLeft >= 8)
	{
		*pOut = ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// read remaining bits
	if (nBitsLeft)
	{
		*pOut = ReadUBitLong(nBitsLeft);
	}

}

float old_bf_read::ReadBitAngle(int numbits)
{
	float fReturn;
	int i;
	float shift;

	shift = (float)(BitForBitnum(numbits));

	i = ReadUBitLong(numbits);
	fReturn = (float)((float)i * (360.0f / shift));

	return fReturn;
}

unsigned int old_bf_read::PeekUBitLong(int numbits)
{
	unsigned int r;
	int i, nBitValue;
#ifdef BIT_VERBOSE
	int nShifts = numbits;
#endif

	old_bf_read savebf;

	savebf = *this;  // Save current state info

	r = 0;
	for (i = 0; i < numbits; i++)
	{
		nBitValue = ReadOneBit();

		// Append to current stream
		if (nBitValue)
		{
			r |= BitForBitnum(i);
		}
	}

	*this = savebf;

#ifdef BIT_VERBOSE
	Con_Printf("PeekBitLong:  %i %i\n", nShifts, (unsigned int)r);
#endif

	return r;
}

// Append numbits least significant bits from data to the current bit stream
int old_bf_read::ReadSBitLong(int numbits)
{
	int r, sign;

	r = ReadUBitLong(numbits - 1);

	// Note: it does this wierdness here so it's bit-compatible with regular integer data in the buffer.
	// (Some old code writes direct integers right into the buffer).
	sign = ReadOneBit();
	if (sign)
		r = -((BitForBitnum(numbits - 1)) - r);

	return r;
}

const byte g_BitMask[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
const byte g_TrailingMask[8] = { 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80 };

inline int old_bf_read::CountRunOfZeros()
{
	int bits = 0;
	if (m_iCurBit + 32 < m_nDataBits)
	{
#if !FAST_BIT_SCAN
		while (true)
		{
			int value = (m_pData[m_iCurBit >> 3] & g_BitMask[m_iCurBit & 7]);
			++m_iCurBit;
			if (value)
				return bits;
			++bits;
		}
#else
		while (true)
		{
			int value = (m_pData[m_iCurBit >> 3] & g_TrailingMask[m_iCurBit & 7]);
			if (!value)
			{
				int zeros = (8 - (m_iCurBit & 7));
				bits += zeros;
				m_iCurBit += zeros;
			}
			else
			{
				int zeros = CountTrailingZeros(value) - (m_iCurBit & 7);
				m_iCurBit += zeros + 1;
				bits += zeros;
				return bits;
			}
		}
#endif
	}
	else
	{
		while (ReadOneBit() == 0)
			bits++;
	}
	return bits;
}

unsigned int old_bf_read::ReadUBitVar()
{
	switch (ReadUBitLong(2))
	{
	case 0:
		return ReadUBitLong(4);

	case 1:
		return ReadUBitLong(8);

	case 2:
		return ReadUBitLong(12);

	default:
	case 3:
		return ReadUBitLong(32);
	}
#if 0
	int bits = CountRunOfZeros();

	unsigned int data = BitForBitnum(bits) - 1;

	// read the value
	if (bits > 0)
		data += ReadUBitLong(bits);

	return data;
#endif
}


unsigned int old_bf_read::ReadBitLong(int numbits, bool bSigned)
{
	if (bSigned)
		return (unsigned int)ReadSBitLong(numbits);
	else
		return ReadUBitLong(numbits);
}


// Basic Coordinate Routines (these contain bit-field size AND fixed point scaling constants)
float old_bf_read::ReadBitCoord(void)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::ReadBitCoord");
#endif
	int		intval = 0, fractval = 0, signbit = 0;
	float	value = 0.0;


	// Read the required integer and fraction flags
	intval = ReadOneBit();
	fractval = ReadOneBit();

	// If we got either parse them, otherwise it's a zero.
	if (intval || fractval)
	{
		// Read the sign bit
		signbit = ReadOneBit();

		// If there's an integer, read it in
		if (intval)
		{
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
		}

		// If there's a fraction, read it in
		if (fractval)
		{
			fractval = ReadUBitLong(COORD_FRACTIONAL_BITS);
		}

		// Calculate the correct floating point value
		value = (float)((float)intval + ((float)fractval * COORD_RESOLUTION));

		// Fixup the sign if negative.
		if (signbit)
			value = -value;
	}

	return value;
}

float old_bf_read::ReadBitCoordMP(bool bIntegral, bool bLowPrecision)
{
#if defined( BB_PROFILING )
	VPROF("old_bf_write::ReadBitCoordMP");
#endif
	int		intval = 0, fractval = 0, signbit = 0;
	float	value = 0.0;


	bool bInBounds = ReadOneBit() ? true : false;

	if (bIntegral)
	{
		// Read the required integer and fraction flags
		intval = ReadOneBit();
		// If we got either parse them, otherwise it's a zero.
		if (intval)
		{
			// Read the sign bit
			signbit = ReadOneBit();

			// If there's an integer, read it in
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			if (bInBounds)
			{
				value = (float)(ReadUBitLong(COORD_INTEGER_BITS_MP) + 1);
			}
			else
			{
				value = (float)(ReadUBitLong(COORD_INTEGER_BITS) + 1);
			}
		}
	}
	else
	{
		// Read the required integer and fraction flags
		intval = ReadOneBit();

		// Read the sign bit
		signbit = ReadOneBit();

		// If we got either parse them, otherwise it's a zero.
		if (intval)
		{
			if (bInBounds)
			{
				intval = ReadUBitLong(COORD_INTEGER_BITS_MP) + 1;
			}
			else
			{
				intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
			}
		}

		// If there's a fraction, read it in
		fractval = ReadUBitLong(bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);

		// Calculate the correct floating point value
		value = (float)((float)intval + ((float)fractval * (bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION)));
	}

	// Fixup the sign if negative.
	if (signbit)
		value = -value;

	return value;
}

void old_bf_read::ReadBitVec3Coord(Vector& fa)
{
	int		xflag, yflag, zflag;

	// This vector must be initialized! Otherwise, If any of the flags aren't set, 
	// the corresponding component will not be read and will be stack garbage.
	fa.Zero();

	xflag = ReadOneBit();
	yflag = ReadOneBit();
	zflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitCoord();
	if (yflag)
		fa[1] = ReadBitCoord();
	if (zflag)
		fa[2] = ReadBitCoord();
}

float old_bf_read::ReadBitNormal(void)
{
	// Read the sign bit
	int	signbit = ReadOneBit();

	// Read the fractional part
	unsigned int fractval = ReadUBitLong(NORMAL_FRACTIONAL_BITS);

	// Calculate the correct floating point value
	float value = (float)((float)fractval * NORMAL_RESOLUTION);

	// Fixup the sign if negative.
	if (signbit)
		value = -value;

	return value;
}

void old_bf_read::ReadBitVec3Normal(Vector& fa)
{
	int xflag = ReadOneBit();
	int yflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitNormal();
	else
		fa[0] = 0.0f;

	if (yflag)
		fa[1] = ReadBitNormal();
	else
		fa[1] = 0.0f;

	// The first two imply the third (but not its sign)
	int znegative = ReadOneBit();

	float fafafbfb = fa[0] * fa[0] + fa[1] * fa[1];
	if (fafafbfb < 1.0f)
		fa[2] = sqrt(1.0f - fafafbfb);
	else
		fa[2] = 0.0f;

	if (znegative)
		fa[2] = -fa[2];
}

void old_bf_read::ReadBitAngles(QAngle& fa)
{
	Vector tmp;
	ReadBitVec3Coord(tmp);
	fa.x = tmp.x;
	fa.y = tmp.y;
	fa.z = tmp.z;

}

int old_bf_read::ReadChar()
{
	return ReadSBitLong(sizeof(char) << 3);
}

int old_bf_read::ReadByte()
{
	return ReadUBitLong(sizeof(unsigned char) << 3);
}

int old_bf_read::ReadShort()
{
	return ReadSBitLong(sizeof(short) << 3);
}

int old_bf_read::ReadWord()
{
	return ReadUBitLong(sizeof(unsigned short) << 3);
}

long old_bf_read::ReadLong()
{
	return ReadSBitLong(sizeof(long) << 3);
}

int64 old_bf_read::ReadLongLong()
{
	int64 retval;
	uint *pLongs = (uint*)&retval;

	// Read the two DWORDs according to network endian
	const short endianIndex = 0x0100;
	byte *idx = (byte*)&endianIndex;
	pLongs[*idx++] = ReadUBitLong(sizeof(long) << 3);
	pLongs[*idx] = ReadUBitLong(sizeof(long) << 3);

	return retval;
}

float old_bf_read::ReadFloat()
{
	float ret;
	Assert(sizeof(ret) == 4);
	ReadBits(&ret, 32);

	// Swap the float, since ReadBits reads raw data
	LittleFloat(&ret, &ret);
	return ret;
}

bool old_bf_read::ReadBytes(void *pOut, int nBytes)
{
	ReadBits(pOut, nBytes << 3);
	return !IsOverflowed();
}

bool old_bf_read::ReadString(char *pStr, int maxLen, bool bLine, int *pOutNumChars)
{
	Assert(maxLen != 0);

	bool bTooSmall = false;
	int iChar = 0;
	while (1)
	{
		char val = ReadChar();
		if (val == 0)
			break;
		else if (bLine && val == '\n')
			break;

		if (iChar < (maxLen - 1))
		{
			pStr[iChar] = val;
			++iChar;
		}
		else
		{
			bTooSmall = true;
		}
	}

	// Make sure it's null-terminated.
	Assert(iChar < maxLen);
	pStr[iChar] = 0;

	if (pOutNumChars)
		*pOutNumChars = iChar;

	return !IsOverflowed() && !bTooSmall;
}


char* old_bf_read::ReadAndAllocateString(bool *pOverflow)
{
	char str[2048];

	int nChars;
	bool bOverflow = !ReadString(str, sizeof(str), false, &nChars);
	if (pOverflow)
		*pOverflow = bOverflow;

	// Now copy into the output and return it;
	char *pRet = new char[nChars + 1];
	for (int i = 0; i <= nChars; i++)
		pRet[i] = str[i];

	return pRet;
}

void old_bf_read::ExciseBits(int startbit, int bitstoremove)
{
	int endbit = startbit + bitstoremove;
	int remaining_to_end = m_nDataBits - endbit;

	old_bf_write temp;
	temp.StartWriting((void *)m_pData, m_nDataBits << 3, startbit);

	Seek(endbit);

	for (int i = 0; i < remaining_to_end; i++)
	{
		temp.WriteOneBit(ReadOneBit());
	}

	Seek(startbit);

	m_nDataBits -= bitstoremove;
	m_nDataBytes = m_nDataBits >> 3;
}




//CBITREAD/WRITE

void CBitWrite::StartWriting(void *pData, int nBytes, int iStartBit, int nBits)
{
	// Make sure it's dword aligned and padded.
	Assert((nBytes % 4) == 0);
	Assert(((unsigned long)pData & 3) == 0);
	Assert(iStartBit == 0);
	m_pData = (uint32 *)pData;
	m_pDataOut = m_pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = nBytes << 3;
	}
	else
	{
		Assert(nBits <= nBytes * 8);
		m_nDataBits = nBits;
	}
	m_bOverflow = false;
	m_nOutBufWord = 0;
	m_nOutBitsAvail = 32;
	m_pBufferEnd = m_pDataOut + (nBytes >> 2);
}

const uint32 CBitBuffer::s_nMaskTable[33] = {
	0,
	(1 << 1) - 1,
	(1 << 2) - 1,
	(1 << 3) - 1,
	(1 << 4) - 1,
	(1 << 5) - 1,
	(1 << 6) - 1,
	(1 << 7) - 1,
	(1 << 8) - 1,
	(1 << 9) - 1,
	(1 << 10) - 1,
	(1 << 11) - 1,
	(1 << 12) - 1,
	(1 << 13) - 1,
	(1 << 14) - 1,
	(1 << 15) - 1,
	(1 << 16) - 1,
	(1 << 17) - 1,
	(1 << 18) - 1,
	(1 << 19) - 1,
	(1 << 20) - 1,
	(1 << 21) - 1,
	(1 << 22) - 1,
	(1 << 23) - 1,
	(1 << 24) - 1,
	(1 << 25) - 1,
	(1 << 26) - 1,
	(1 << 27) - 1,
	(1 << 28) - 1,
	(1 << 29) - 1,
	(1 << 30) - 1,
	0x7fffffff,
	0xffffffff,
};

bool CBitWrite::WriteString(const char *pStr)
{
	if (pStr)
	{
		while (*pStr)
		{
			WriteChar(*(pStr++));
		}
	}
	WriteChar(0);
	return !IsOverflowed();
}


void CBitWrite::WriteLongLong(int64 val)
{
	uint *pLongs = (uint*)&val;

	// Insert the two DWORDS according to network endian
	const short endianIndex = 0x0100;
	byte *idx = (byte*)&endianIndex;
	WriteUBitLong(pLongs[*idx++], sizeof(long) << 3);
	WriteUBitLong(pLongs[*idx], sizeof(long) << 3);
}

bool CBitWrite::WriteBits(const void *pInData, int nBits)
{
	unsigned char *pOut = (unsigned char*)pInData;
	int nBitsLeft = nBits;

	// Bounds checking..
	if ((GetNumBitsWritten() + nBits) > m_nDataBits)
	{
		SetOverflowFlag();
		CallErrorHandler(BITBUFERROR_BUFFER_OVERRUN, m_pDebugName);
		return false;
	}

	// !! speed!! need fast paths
	// write remaining bytes
	while (nBitsLeft >= 8)
	{
		WriteUBitLong(*pOut, 8, false);
		++pOut;
		nBitsLeft -= 8;
	}

	// write remaining bits
	if (nBitsLeft)
	{
		WriteUBitLong(*pOut, nBitsLeft, false);
	}

	return !IsOverflowed();
}

void CBitWrite::WriteBytes(const void *pBuf, int nBytes)
{
	WriteBits(pBuf, nBytes << 3);
}

void CBitWrite::WriteBitCoord(const float f)
{
	int		signbit = (f <= -COORD_RESOLUTION);
	int		intval = (int)abs(f);
	int		fractval = abs((int)(f*COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1);


	// Send the bit flags that indicate whether we have an integer part and/or a fraction part.
	WriteOneBit(intval);
	WriteOneBit(fractval);

	if (intval || fractval)
	{
		// Send the sign bit
		WriteOneBit(signbit);

		// Send the integer if we have one.
		if (intval)
		{
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
		}

		// Send the fraction if we have one
		if (fractval)
		{
			WriteUBitLong((unsigned int)fractval, COORD_FRACTIONAL_BITS);
		}
	}
}

void CBitWrite::WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision)
{
	int		signbit = (f <= -(bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION));
	int		intval = (int)abs(f);
	int		fractval = bLowPrecision ?
		(abs((int)(f*COORD_DENOMINATOR_LOWPRECISION)) & (COORD_DENOMINATOR_LOWPRECISION - 1)) :
		(abs((int)(f*COORD_DENOMINATOR)) & (COORD_DENOMINATOR - 1));

	bool    bInBounds = intval < (1 << COORD_INTEGER_BITS_MP);

	WriteOneBit(bInBounds);

	if (bIntegral)
	{
		// Send the sign bit
		WriteOneBit(intval);
		if (intval)
		{
			WriteOneBit(signbit);
			// Send the integer if we have one.
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			if (bInBounds)
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS_MP);
			}
			else
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
			}
		}
	}
	else
	{
		// Send the bit flags that indicate whether we have an integer part and/or a fraction part.
		WriteOneBit(intval);
		// Send the sign bit
		WriteOneBit(signbit);

		// Send the integer if we have one.
		if (intval)
		{
			// Adjust the integers from [1..MAX_COORD_VALUE] to [0..MAX_COORD_VALUE-1]
			intval--;
			if (bInBounds)
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS_MP);
			}
			else
			{
				WriteUBitLong((unsigned int)intval, COORD_INTEGER_BITS);
			}
		}
		WriteUBitLong((unsigned int)fractval, bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);
	}
}

void CBitWrite::SeekToBit(int nBit)
{
	TempFlush();
	m_pDataOut = m_pData + (nBit / 32);
	m_nOutBufWord = *(m_pDataOut);
	m_nOutBitsAvail = 32 - (nBit & 31);
}



void CBitWrite::WriteBitVec3Coord(const Vector& fa)
{
	int		xflag, yflag, zflag;

	xflag = (fa.x >= COORD_RESOLUTION) || (fa.x <= -COORD_RESOLUTION);
	yflag = (fa.y >= COORD_RESOLUTION) || (fa.y <= -COORD_RESOLUTION);
	zflag = (fa.z >= COORD_RESOLUTION) || (fa.z <= -COORD_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);
	WriteOneBit(zflag);

	if (xflag)
		WriteBitCoord(fa.x);
	if (yflag)
		WriteBitCoord(fa.y);
	if (zflag)
		WriteBitCoord(fa.z);
}

void CBitWrite::WriteBitNormal(float f)
{
	int	signbit = (f <= -NORMAL_RESOLUTION);

	// NOTE: Since +/-1 are valid values for a normal, I'm going to encode that as all ones
	unsigned int fractval = abs((int)(f*NORMAL_DENOMINATOR));

	// clamp..
	if (fractval > NORMAL_DENOMINATOR)
		fractval = NORMAL_DENOMINATOR;

	// Send the sign bit
	WriteOneBit(signbit);

	// Send the fractional component
	WriteUBitLong(fractval, NORMAL_FRACTIONAL_BITS);
}

void CBitWrite::WriteBitVec3Normal(const Vector& fa)
{
	int		xflag, yflag;

	xflag = (fa.x >= NORMAL_RESOLUTION) || (fa.x <= -NORMAL_RESOLUTION);
	yflag = (fa.y >= NORMAL_RESOLUTION) || (fa.y <= -NORMAL_RESOLUTION);

	WriteOneBit(xflag);
	WriteOneBit(yflag);

	if (xflag)
		WriteBitNormal(fa.x);
	if (yflag)
		WriteBitNormal(fa.y);

	// Write z sign bit
	int	signbit = (fa.z <= -NORMAL_RESOLUTION);
	WriteOneBit(signbit);
}

void CBitWrite::WriteBitAngle(float fAngle, int numbits)
{

	unsigned int shift = GetBitForBitnum(numbits);
	unsigned int mask = shift - 1;

	int d = (int)((fAngle / 360.0) * shift);
	d &= mask;

	WriteUBitLong((unsigned int)d, numbits);
}

bool CBitWrite::WriteBitsFromBuffer(bf_read *pIn, int nBits)
{
	// This could be optimized a little by
	while (nBits > 32)
	{
		WriteUBitLong(pIn->ReadUBitLong(32), 32);
		nBits -= 32;
	}

	WriteUBitLong(pIn->ReadUBitLong(nBits), nBits);
	return !IsOverflowed() && !pIn->IsOverflowed();
}

void CBitWrite::WriteBitAngles(const QAngle& fa)
{
	// FIXME:
	Vector tmp(fa.x, fa.y, fa.z);
	WriteBitVec3Coord(tmp);
}

bool CBitRead::Seek(int nPosition)
{
	bool bSucc = true;
	if (nPosition < 0 || nPosition > m_nDataBits)
	{
		SetOverflowFlag();
		bSucc = false;
		nPosition = m_nDataBits;
	}
	int nHead = m_nDataBytes & 3;							// non-multiple-of-4 bytes at head of buffer. We put the "round off"
	// at the head to make reading and detecting the end efficient.

	int nByteOfs = nPosition / 8;
	if ((m_nDataBytes < 4) || (nHead && (nByteOfs < nHead)))
	{
		// partial first dword
		uint8 const *pPartial = (uint8 const *)m_pData;
		if (m_pData)
		{
			m_nInBufWord = *(pPartial++);
			if (nHead > 1)
				m_nInBufWord |= (*pPartial++) << 8;
			if (nHead > 2)
				m_nInBufWord |= (*pPartial++) << 16;
		}
		m_pDataIn = (uint32 const *)pPartial;
		m_nInBufWord >>= (nPosition & 31);
		m_nBitsAvail = (nHead << 3) - (nPosition & 31);
	}
	else
	{
		int nAdjPosition = nPosition - (nHead << 3);
		m_pDataIn = reinterpret_cast<uint32 const *> (
			reinterpret_cast<uint8 const *>(m_pData)+((nAdjPosition / 32) << 2) + nHead);
		if (m_pData)
		{
			m_nBitsAvail = 32;
			GrabNextDWord();
		}
		else
		{
			m_nInBufWord = 0;
			m_nBitsAvail = 1;
		}
		m_nInBufWord >>= (nAdjPosition & 31);
		m_nBitsAvail = min(m_nBitsAvail, 32 - (nAdjPosition & 31));	// in case grabnextdword overflowed
	}
	return bSucc;
}


void CBitRead::StartReading(const void *pData, int nBytes, int iStartBit, int nBits)
{
	// Make sure it's dword aligned and padded.
	Assert(((unsigned long)pData & 3) == 0);
	m_pData = (uint32 *)pData;
	m_pDataIn = m_pData;
	m_nDataBytes = nBytes;

	if (nBits == -1)
	{
		m_nDataBits = nBytes << 3;
	}
	else
	{
		Assert(nBits <= nBytes * 8);
		m_nDataBits = nBits;
	}
	m_bOverflow = false;
	m_pBufferEnd = reinterpret_cast<uint32 const *> (reinterpret_cast< uint8 const *> (m_pData)+nBytes);
	if (m_pData)
		Seek(iStartBit);

}

bool CBitRead::ReadString(char *pStr, int maxLen, bool bLine, int *pOutNumChars)
{
	Assert(maxLen != 0);

	bool bTooSmall = false;
	int iChar = 0;
	while (1)
	{
		char val = ReadChar();
		if (val == 0)
			break;
		else if (bLine && val == '\n')
			break;

		if (iChar < (maxLen - 1))
		{
			pStr[iChar] = val;
			++iChar;
		}
		else
		{
			bTooSmall = true;
		}
	}

	// Make sure it's null-terminated.
	Assert(iChar < maxLen);
	pStr[iChar] = 0;

	if (pOutNumChars)
		*pOutNumChars = iChar;

	return !IsOverflowed() && !bTooSmall;
}

char* CBitRead::ReadAndAllocateString(bool *pOverflow)
{
	char str[2048];

	int nChars;
	bool bOverflow = !ReadString(str, sizeof(str), false, &nChars);
	if (pOverflow)
		*pOverflow = bOverflow;

	// Now copy into the output and return it;
	char *pRet = new char[nChars + 1];
	for (int i = 0; i <= nChars; i++)
		pRet[i] = str[i];

	return pRet;
}

int64 CBitRead::ReadLongLong(void)
{
	int64 retval;
	uint *pLongs = (uint*)&retval;

	// Read the two DWORDs according to network endian
	const short endianIndex = 0x0100;
	byte *idx = (byte*)&endianIndex;
	pLongs[*idx++] = ReadUBitLong(sizeof(long) << 3);
	pLongs[*idx] = ReadUBitLong(sizeof(long) << 3);
	return retval;
}

void CBitRead::ReadBits(void *pOutData, int nBits)
{
	unsigned char *pOut = (unsigned char*)pOutData;
	int nBitsLeft = nBits;


	// align output to dword boundary
	while (((unsigned long)pOut & 3) != 0 && nBitsLeft >= 8)
	{
		*pOut = (unsigned char)ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// X360TBD: Can't read dwords in ReadBits because they'll get swapped
	if (IsPC())
	{
		// read dwords
		while (nBitsLeft >= 32)
		{
			*((unsigned long*)pOut) = ReadUBitLong(32);
			pOut += sizeof(unsigned long);
			nBitsLeft -= 32;
		}
	}

	// read remaining bytes
	while (nBitsLeft >= 8)
	{
		*pOut = ReadUBitLong(8);
		++pOut;
		nBitsLeft -= 8;
	}

	// read remaining bits
	if (nBitsLeft)
	{
		*pOut = ReadUBitLong(nBitsLeft);
	}

}

bool CBitRead::ReadBytes(void *pOut, int nBytes)
{
	ReadBits(pOut, nBytes << 3);
	return !IsOverflowed();
}

float CBitRead::ReadBitAngle(int numbits)
{
	float shift = (float)(GetBitForBitnum(numbits));

	int i = ReadUBitLong(numbits);
	float fReturn = (float)((float)i * (360.0 / shift));

	return fReturn;
}

// Basic Coordinate Routines (these contain bit-field size AND fixed point scaling constants)
float CBitRead::ReadBitCoord(void)
{
	int		intval = 0, fractval = 0, signbit = 0;
	float	value = 0.0;


	// Read the required integer and fraction flags
	intval = ReadOneBit();
	fractval = ReadOneBit();

	// If we got either parse them, otherwise it's a zero.
	if (intval || fractval)
	{
		// Read the sign bit
		signbit = ReadOneBit();

		// If there's an integer, read it in
		if (intval)
		{
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
		}

		// If there's a fraction, read it in
		if (fractval)
		{
			fractval = ReadUBitLong(COORD_FRACTIONAL_BITS);
		}

		// Calculate the correct floating point value
		value = (float)((float)intval + ((float)fractval * COORD_RESOLUTION));

		// Fixup the sign if negative.
		if (signbit)
			value = -value;
	}

	return value;
}

float CBitRead::ReadBitCoordMP(bool bIntegral, bool bLowPrecision)
{
	int		intval = 0, fractval = 0, signbit = 0;
	float	value = 0.0;

	bool bInBounds = ReadOneBit() ? true : false;

	if (bIntegral)
	{
		// Read the required integer and fraction flags
		intval = ReadOneBit();
		// If we got either parse them, otherwise it's a zero.
		if (intval)
		{
			// Read the sign bit
			signbit = ReadOneBit();

			// If there's an integer, read it in
			// Adjust the integers from [0..MAX_COORD_VALUE-1] to [1..MAX_COORD_VALUE]
			if (bInBounds)
			{
				value = (float)( ReadUBitLong(COORD_INTEGER_BITS_MP) + 1);
			}
			else
			{
				value = (float)( ReadUBitLong(COORD_INTEGER_BITS) + 1);
			}
		}
	}
	else
	{
		// Read the required integer and fraction flags
		intval = ReadOneBit();

		// Read the sign bit
		signbit = ReadOneBit();

		// If we got either parse them, otherwise it's a zero.
		if (intval)
		{
			if (bInBounds)
			{
				intval = ReadUBitLong(COORD_INTEGER_BITS_MP) + 1;
			}
			else
			{
				intval = ReadUBitLong(COORD_INTEGER_BITS) + 1;
			}
		}

		// If there's a fraction, read it in
		fractval = ReadUBitLong(bLowPrecision ? COORD_FRACTIONAL_BITS_MP_LOWPRECISION : COORD_FRACTIONAL_BITS);

		// Calculate the correct floating point value
		value = (float)((float)intval + ((float)fractval * (bLowPrecision ? COORD_RESOLUTION_LOWPRECISION : COORD_RESOLUTION)));
	}

	// Fixup the sign if negative.
	if (signbit)
		value = -value;

	return value;
}

void CBitRead::ReadBitVec3Coord(Vector& fa)
{
	int		xflag, yflag, zflag;

	// This vector must be initialized! Otherwise, If any of the flags aren't set, 
	// the corresponding component will not be read and will be stack garbage.
	fa.Zero();

	xflag = ReadOneBit();
	yflag = ReadOneBit();
	zflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitCoord();
	if (yflag)
		fa[1] = ReadBitCoord();
	if (zflag)
		fa[2] = ReadBitCoord();
}

float CBitRead::ReadBitNormal(void)
{
	// Read the sign bit
	int	signbit = ReadOneBit();

	// Read the fractional part
	unsigned int fractval = ReadUBitLong(NORMAL_FRACTIONAL_BITS);

	// Calculate the correct floating point value
	float value = (float)((float)fractval * NORMAL_RESOLUTION);

	// Fixup the sign if negative.
	if (signbit)
		value = -value;

	return value;
}

void CBitRead::ReadBitVec3Normal(Vector& fa)
{
	int xflag = ReadOneBit();
	int yflag = ReadOneBit();

	if (xflag)
		fa[0] = ReadBitNormal();
	else
		fa[0] = 0.0f;

	if (yflag)
		fa[1] = ReadBitNormal();
	else
		fa[1] = 0.0f;

	// The first two imply the third (but not its sign)
	int znegative = ReadOneBit();

	float fafafbfb = fa[0] * fa[0] + fa[1] * fa[1];
	if (fafafbfb < 1.0f)
		fa[2] = sqrt(1.0f - fafafbfb);
	else
		fa[2] = 0.0f;

	if (znegative)
		fa[2] = -fa[2];
}

void CBitRead::ReadBitAngles(QAngle& fa)
{
	Vector tmp;
	ReadBitVec3Coord(tmp);
	fa.x = tmp.x;
	fa.y = tmp.y;
	fa.z = tmp.z;
}
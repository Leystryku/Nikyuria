/*
#ifdef _WIN32
#define DLL_IMPORT				extern "C" __declspec( dllimport )
#else
#define DLL_IMPORT   extern "C"
#endif

#define DBG_INTERFACE	DLL_IMPORT
*/

enum SpewType_t
{
	SPEW_MESSAGE = 0,
	SPEW_WARNING,
	SPEW_ASSERT,
	SPEW_ERROR,
	SPEW_LOG,

	SPEW_TYPE_COUNT
};

enum SpewRetval_t
{
	SPEW_DEBUGGER = 0,
	SPEW_CONTINUE,
	SPEW_ABORT
};

/*
#define dbg_SpewOutputFunc_t  (void(*)(void*func))
#define dbg_GetSpewOutputFunc_t  (void*(*)())
#define dbg_GetSpewOutputColor_t (const void*(*)())

class m_dbg
{
public:
	void(*SpewOutputFunc)(void* func);
	void*(*GetSpewOutputFunc)();
	const void* (*GetSpewOutputColor)();

};

extern m_dbg *dbg;

*/
/*
DBG_INTERFACE void   SpewOutputFunc(SpewOutputFunc_t func);

DBG_INTERFACE SpewOutputFunc_t GetSpewOutputFunc(void);

DBG_INTERFACE const Color* GetSpewOutputColor(void);*/
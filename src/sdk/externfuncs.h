extern void(*Msg)(const char *, ...);

#ifndef _CSGO
extern void(*SpewOutputFunc)(void* func);
extern void*(*GetSpewOutputFunc)();
extern const void* (*GetSpewOutputColor)();
#else
extern void(*LoggingSystem_PushLoggingState)(bool bThreadLocal, bool bClearState);
extern void(*LoggingSystem_RegisterLoggingListener)(void* listener);
#endif

extern void(* RandomSeed)(int);
extern float(* RandomFloat)(float, float);
extern float(*Plat_FloatTime)();

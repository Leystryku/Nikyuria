#include "../stdafx.h"

#define SetupInterfaceFuncNI(name, ret, index, args, args2, args3) ret name args \
		{ \
		typedef ret (calltype* OriginalFn) args2 ; \
		return this->GetFunction<OriginalFn>(this->vmt, index) args3; \
		} \

#define SetupInterfaceFunc(name, ret, index, args, args2, args3) inline ret name args \
	{ \
		typedef ret (__thiscall* OriginalFn) args2 ; \
		return this->GetFunction<OriginalFn>(this->vmt, index) args3; \
	} \

#define SetupInterfaceFunc2(name, ret, index, args, args2, args3) inline ret name args \
		{ \
		typedef ret (__thiscall* OriginalFn) args2 ; \
		return this->GetFunction<OriginalFn>(this, index) args3; \
		} \


#define SetupInterfaceFunc2FC(name, ret, index, args, args2, args3) inline ret name args \
				{ \
		typedef ret (__fastcall* OriginalFn) args2 ; \
		return this->GetFunction<OriginalFn>(this, index) args3; \
				} \

#define SetupInterfaceFuncFC(name, ret, index, args, args2, args3) inline ret name args \
		{ \
		typedef ret (__fastcall* OriginalFn) args2 ; \
		return this->GetFunction<OriginalFn>(this->vmt, index) args3; \
		} \


#define SETUP_INTERFACE(name) class name { protected:\
 										template <typename T> T inline GetFunction(void* instance, unsigned int uiIndex)\
																						{\
											void **vInstance = *(void***)instance;\
											return (T)vInstance[uiIndex];\
																						};\
											public:\
											void* vmt;\
											inline name (void*vClass) { this->vmt = vClass; }

#define SETUP_INTERFACE2(name) class name { protected:\
 										template <typename T> T inline GetFunction(void* instance, unsigned int uiIndex)\
																																												{\
											void **vInstance = *(void***)instance;\
											return (T)vInstance[uiIndex];\
																																												};\
											public:\
											void* vmt;\
											inline name (void*vClass) { this->vmt = vClass;

#define SETUP_INTERFACE3(name) class name { protected:\
 										template <typename T> T inline GetFunction(void* instance, unsigned int uiIndex)\
																																												{\
											void **vInstance = *(void***)instance;\
											return (T)vInstance[uiIndex];\
																																												};\
											public:

#define SETUP_INTERFACE3NI(name) class name { protected:\
 										template <typename T> T GetFunction(void* instance, unsigned int uiIndex)\
																																																																																								{\
											void **vInstance = *(void***)instance;\
											return (T)vInstance[uiIndex];\
																																																																																								};\
											public:
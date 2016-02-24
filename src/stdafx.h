#pragma once

//#pragma comment(linker,"/ENTRY:main")
//#pragma comment(linker, "/NODEFAULTLIB")

///NODEFAULTLIB /DYNAMICBASE:NO /MANIFEST:NO /MERGE:.rdata=.text

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define jackebp()  __asm mov esp, ebp; pop ebp;
#define restoreebp() __asm push ebp; __asm mov ebp, esp;


#include "targetver.h"


// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>



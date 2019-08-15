#include "VideoHack.h"

VideoHack::VideoHack(HMODULE baseModule)
{
	this->baseModule = baseModule;
}

float WidthCorrection = 1.0f;
DWORD returnVideoCorrection;
void __declspec(naked) videoCorrectionLowLevel()
{
	__asm
	{
		fild[eax + 0x160]		//Frame Width
		fmul[WidthCorrection]
		mov edx, [esp + 4 + 0x8]
		fistp[edx]	//requires SSE3

		fild[eax + 0x164]		//Frame Width
		//fmul [WidthCorrection]
		mov edx, [esp + 4 + 0xC]
		fistp[edx]	//requires SSE3

		jmp[returnVideoCorrection]
	}
}


void VideoHack::InstallVideoHack(int* surfaceWidth, int* surfaceHeight)
{
	WidthCorrection = *surfaceWidth * 1.0f / *surfaceHeight;
	HookInsideFunction(0x00826CB9, &videoCorrectionLowLevel, &returnVideoCorrection, 0x18);
}

VideoHack::~VideoHack()
{
}

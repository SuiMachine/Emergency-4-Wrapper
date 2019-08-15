#include "Emergency4Hacks.h"
#define ClassName 0x00A034E8


void ExternHookThreadFunction(LPVOID arg)
{
	Emergency4Hacks* ths = (Emergency4Hacks*)arg;
	HMODULE visModule;
	while ((visModule = GetModuleHandle("vision71.dll")) == NULL)
		Sleep(10);

	UnprotectModule(visModule);
	ths->visRenderer = new VisRenderer();
	ths->visRenderer->InstallDetourPerspectiveToAngles(&ths->bWidth, &ths->bHeight);
	ths->visRenderer->InstallDetourVisVideoCLSetMode(ths->bWidth, ths->bHeight, ths->bFullscreen);
	ths->visRenderer->InstallDetourVisRendererSetMaxAnistropy(1.0f);
}

//Window Parameters detour
int bPosX = 0;
int bPosY = 0;

DWORD windowParametersDetourReturn;
void __declspec(naked) windowParametersDetour()
{
	__asm
	{
		push    ebx //lpParam
		push    edi //hInstance
		push    ebx //hMenu
		push    ebx //hWndParent
		push    ecx //nHeight
		push    edx //nWidth
		push    DS : bPosY //Y
		push    DS : bPosX //X
		push    ebx //dwStyle
		push    eax //lpWindowName
		push    ClassName //"BLWNDCLS"
		push    0x0L //dwExStyle
		jmp[windowParametersDetourReturn]
	}
}

//Window Rect Clip fix
DWORD windowRectFixReturn;
void __declspec(naked) windowRectFix()
{
	__asm
	{
		mov eax, DS:bPosX
		mov[esp + 0x4], eax
		mov eax, DS : bPosY
		mov[esp + 0x8], eax
		mov eax, 2
		jmp[windowRectFixReturn]
	}
}

Emergency4Hacks::Emergency4Hacks()
{			
	//Get base module
	baseModule = GetModuleHandle(NULL);
	UnprotectModule(baseModule);

	CIniReader configReader("");
	//Load info from ini
	bWidth = configReader.ReadInteger("MAIN", "Width", 0);
	bHeight = configReader.ReadInteger("MAIN", "Height", 0);
	if (bWidth == 0 || bHeight == 0)
	{
		bWidth = 1024;
		bHeight = 768;
	}

	bPosX = configReader.ReadInteger("MAIN", "PosX", 0);
	bPosY = configReader.ReadInteger("MAIN", "PosY", 0);
	bFullscreen = configReader.ReadInteger("MAIN", "Windowed", 0) != 1;
	bSkipIntros = configReader.ReadInteger("MAIN", "SkipIntros", 0) != 0;
	bCorrectAspectRatioOfCinematics = configReader.ReadInteger("MAIN", "CorrectVideoAspectRatio", 0) != 0;

	if (bSkipIntros)
	{
		//Modify jump to skip intros
		*(byte*)((DWORD)baseModule + 0x790D3) = 0xE9;
		*(DWORD*)((DWORD)baseModule + 0x790D4) = 0x000000C1;
	}

	if (bCorrectAspectRatioOfCinematics)
	{
		auto tmp = new VideoHack(baseModule);
		tmp->InstallVideoHack(&bWidth, &bHeight);
	}

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ExternHookThreadFunction, this, NULL, NULL);

}

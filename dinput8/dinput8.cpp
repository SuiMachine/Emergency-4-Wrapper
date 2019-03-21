#include "dinput8.h"
#define ClassName 0x00A034E8

//Global Variables
int bPosX = 0;
int bPosY = 0;
int bWidth = 1024;
int bHeight = 768;
bool bFullscreen;
bool bSkipIntros;
bool bCorrectAspectRatioOfCinematics;

HMODULE baseModule;
VisRenderer * visRenderer;

void ExternHookThreadFunction()
{
	HMODULE visModule;
	while ((visModule = GetModuleHandle("vision71.dll")) == NULL)
		Sleep(10);

	UnprotectModule(visModule);
	visRenderer = new VisRenderer(baseModule, visModule);
	visRenderer->InstallDetourPerspectiveToAngles(&bWidth, &bHeight);
	visRenderer->InstallDetourVisVideoCLSetMode(bWidth, bHeight, bFullscreen);
	visRenderer->InstallDetourVisRendererSetMaxAnistropy(1.0f);
}

//Window Parameters detour
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
		push    DS:bPosY //Y
		push    DS:bPosX //X
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
		mov [esp+0x4], eax
		mov eax, DS:bPosY
		mov [esp+0x8], eax
		mov eax, 2
		jmp[windowRectFixReturn]
	}
}

//Dll Main
bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			//Get module location and its ini file
			char path[MAX_PATH];
			HMODULE hm = NULL;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)"dinput8.dll", &hm);
			GetModuleFileNameA(hm, path, sizeof(path));
			*strrchr(path, '\\') = '\0';
			strcat_s(path, "\\dinput8.ini");
			CIniReader configReader(path);

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

			//Get dll from Windows directory
			GetSystemDirectory(path, MAX_PATH);
			strcat_s(path, "\\dinput8.dll");

			//Set pointers
			dinput8.dll = LoadLibraryA(path);
			dinput8.DirectInput8Create = (LPWDirectInput8Create)GetProcAddress(dinput8.dll, "DirectInput8Create");
			dinput8.DllCanUnloadNow = (LPWDllCanUnloadNow)GetProcAddress(dinput8.dll, "DllCanUnloadNow");
			dinput8.DllGetClassObject = (LPWDllGetClassObject)GetProcAddress(dinput8.dll, "DllGetClassObject");
			dinput8.DllRegisterServer = (LPWDllRegisterServer)GetProcAddress(dinput8.dll, "DllRegisterServer");
			dinput8.DllUnregisterServer = (LPWDllUnregisterServer)GetProcAddress(dinput8.dll, "DllUnregisterServer");

			//Get base module
			baseModule = GetModuleHandleA("em4.exe");
			UnprotectModule(baseModule);



			if (false)
			{
				//Modify window properties
				HookInsideFunction((DWORD)baseModule + 0x33AAFE, windowParametersDetour, &windowParametersDetourReturn, 0x11);
				HookInsideFunction((DWORD)baseModule + 0x4E88EA, windowRectFix, &windowRectFixReturn, 0x5);
			}

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
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ExternHookThreadFunction, NULL, NULL, NULL);

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			FreeLibrary(hModule);
			break;
		}
		return true;
	}

	return TRUE;
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	HRESULT hr = dinput8.DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	return hr;
}

HRESULT WINAPI DllCanUnloadNow()
{
	return dinput8.DllCanUnloadNow();
}

HRESULT WINAPI DllGetClassObject(REFCLSID riidlsid, REFIID riidltf, LPVOID whatever)
{
	return dinput8.DllGetClassObject(riidlsid, riidltf, whatever);
}

HRESULT WINAPI DllRegisterServer()
{
	return dinput8.DllRegisterServer();
}

HRESULT WINAPI DllUnregisterServer()
{
	return dinput8.DllUnregisterServer();
}
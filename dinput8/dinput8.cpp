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
}

//Detours
DWORD initializeOverrideReturn;
void __declspec(naked) initializeOverride()
{
	_asm
	{
		mov eax, [eax+0x18]
		mov ecx, DS:[0xBD9538]
		mov edx, DS:[0xBD94C8]
		push 0
		push eax
		movzx eax, [bFullscreen]
		mov DS:[0xBD937C],eax
		push    0
		push    eax
		mov     eax, DS:[0xBD9458]
		push    ecx
		mov     ecx, DS:[0xBD93E8]
		push    edx
		push    eax
		push    ecx
		jmp[initializeOverrideReturn]
	}
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

			//Load info from ini
			bWidth = GetPrivateProfileInt("MAIN", "Width", 0, path);
			bHeight = GetPrivateProfileInt("MAIN", "Height", 0, path);
			if (bWidth == 0 || bHeight == 0)
			{
				bWidth = 1024;
				bHeight = 768;
			}

			bPosX = GetPrivateProfileInt("MAIN", "PosX", 0, path);
			bPosY = GetPrivateProfileInt("MAIN", "PosY", 0, path);
			bFullscreen = GetPrivateProfileInt("MAIN", "Windowed", 0, path) != 1;
			bSkipIntros = GetPrivateProfileInt("MAIN", "SkipIntros", 0, path) != 0;
			bCorrectAspectRatioOfCinematics = GetPrivateProfileInt("MAIN", "CorrectVideoAspectRatio", 0, path) != 0;

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



			if (bFullscreen == 0)
			{
				//Modify window properties
				Hook((DWORD)baseModule + 0x33AAFE, windowParametersDetour, &windowParametersDetourReturn, 0x11);

				//Modify initialization to enable Windowed mode
				*(byte*)((DWORD)baseModule + 0x7843F) = (byte)0x0;
				Hook((DWORD)baseModule + 0x406DA6, initializeOverride, &initializeOverrideReturn, 0x2B);
				Hook((DWORD)baseModule + 0x4E88EA, windowRectFix, &windowRectFixReturn, 0x5);
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

bool Hook(DWORD targetToHook, void * ourFunction, DWORD * returnAddress, int overrideLenght)
{
	if (overrideLenght < 5)
		return false;

	*returnAddress = targetToHook + overrideLenght;

	DWORD curProtectionFlag;
	VirtualProtect((void*)targetToHook, overrideLenght, PAGE_EXECUTE_READWRITE, &curProtectionFlag);
	memset((void*)targetToHook, 0x90, overrideLenght);
	DWORD relativeAddress = ((DWORD)ourFunction - (DWORD)targetToHook) - 5;

	*(BYTE*)targetToHook = 0xE9;
	*(DWORD*)((DWORD)targetToHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect((void*)targetToHook, overrideLenght, curProtectionFlag, &temp);
	return true;
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